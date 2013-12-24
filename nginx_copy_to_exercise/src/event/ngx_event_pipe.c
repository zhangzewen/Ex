
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_pipe.h>


static ngx_int_t ngx_event_pipe_read_upstream(ngx_event_pipe_t *p);
static ngx_int_t ngx_event_pipe_write_to_downstream(ngx_event_pipe_t *p);

static ngx_int_t ngx_event_pipe_write_chain_to_temp_file(ngx_event_pipe_t *p);
static ngx_inline void ngx_event_pipe_remove_shadow_links(ngx_buf_t *buf);
static ngx_int_t ngx_event_pipe_drain_chains(ngx_event_pipe_t *p);


ngx_int_t
ngx_event_pipe(ngx_event_pipe_t *p, ngx_int_t do_write)
{
    u_int         flags;
    ngx_int_t     rc;
    ngx_event_t  *rev, *wev;

    for ( ;; ) {
        if (do_write) {
            p->log->action = "sending to client";
//写数据到downstream
            rc = ngx_event_pipe_write_to_downstream(p);

            if (rc == NGX_ABORT) {
                return NGX_ABORT;
            }

            if (rc == NGX_BUSY) {
                return NGX_OK;
            }
        }

        p->read = 0;
        p->upstream_blocked = 0;

        p->log->action = "reading upstream";
//读取数据
        if (ngx_event_pipe_read_upstream(p) == NGX_ABORT) {
            return NGX_ABORT;
        }
//判断是否需要退出循环，p->read表示是否已经读取了upstream的数据，upstream_blocked表示是否downstream可写
        if (!p->read && !p->upstream_blocked) {
            break;
        }
//可以看到如果读取了数据就准备写数据到downstream
        do_write = 1;
    }
//判断是否需要挂载读事件
    if (p->upstream->fd != -1) {
        rev = p->upstream->read;

        flags = (rev->eof || rev->error) ? NGX_CLOSE_EVENT : 0;

        if (ngx_handle_read_event(rev, flags) != NGX_OK) {
            return NGX_ABORT;
        }

        if (rev->active && !rev->ready) {
            ngx_add_timer(rev, p->read_timeout);

        } else if (rev->timer_set) {
            ngx_del_timer(rev);
        }
    }
//挂载写事件
    if (p->downstream->fd != -1 && p->downstream->data == p->output_ctx) {
        wev = p->downstream->write;
        if (ngx_handle_write_event(wev, p->send_lowat) != NGX_OK) {
            return NGX_ABORT;
        }

        if (!wev->delayed) {
            if (wev->active && !wev->ready) {
                ngx_add_timer(wev, p->send_timeout);

            } else if (wev->timer_set) {
                ngx_del_timer(wev);
            }
        }
    }

    return NGX_OK;
}
/*
//
//																					read upstream
//															/                 |                     \
//                         p->preread_bufs      p->free_raw_bufs        p->bufs
//																\                   \										/				
//                                 \                   \                 /
//                                  \                   upstream->recv_chain  																
//                                   \                     /
//                                    \                   /
//                                     \                 /
//                                     p->input_filter_chain
//                                             |
//                                        write downstream
*/



static ngx_int_t
ngx_event_pipe_read_upstream(ngx_event_pipe_t *p)
{
    ssize_t       n, size;
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t  *chain, *cl, *ln;

    if (p->upstream_eof || p->upstream_error || p->upstream_done) {
        return NGX_OK;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "[%s:%d]pipe read upstream: %d",
									__func__, __LINE__,
									 p->upstream->read->ready);

    for ( ;; ) {
//判断upstream的状态
        if (p->upstream_eof || p->upstream_error || p->upstream_done) {
            break;
        }
//如果preread_bufs为空（上面的初始化中这个buf也就是upstream读取头的时候，解析完头，然后剩
//余的buf），并且upstream并不可读，此时则说明对数据也没有任何操作和读取的必要
        if (p->preread_bufs == NULL && !p->upstream->read->ready) {
            break;
        }
//如果preread_bufs存在
        if (p->preread_bufs) {

            /* use the pre-read bufs if they exist */
//使用preread_bufs
            chain = p->preread_bufs;
//可以看到设置preread_bufs为空，这样子，下次循环，则会进入另外的处理，也就是需要从upstream读取数据
            p->preread_bufs = NULL;
//n也就是u->buf的大小
            n = p->preread_size;

            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "[%s:%d]pipe preread: %z",
													__func__, __LINE__,
													 n);

            if (n) {
//设置read，也就是当前upstream中存在读取还没有发送的数据 
                p->read = 1;
            }

        } else {
//当p->preread_bufs为空的情况，此时就需要从upstream来读取数据，而读取之前则需要分配buf，以供upstream使用

#if (NGX_HAVE_KQUEUE)

            /*
             * kqueue notifies about the end of file or a pending error.
             * This test allows not to allocate a buf on these conditions
             * and not to call c->recv_chain().
             */

            if (p->upstream->read->available == 0
                && p->upstream->read->pending_eof)
            {
                p->upstream->read->ready = 0;
                p->upstream->read->eof = 1;
                p->upstream_eof = 1;
                p->read = 1;

                if (p->upstream->read->kq_errno) {
                    p->upstream->read->error = 1;
                    p->upstream_error = 1;
                    p->upstream_eof = 0;

                    ngx_log_error(NGX_LOG_ERR, p->log,
                                  p->upstream->read->kq_errno,
                                  "[%s:%d]kevent() reported that upstream "
                                  "closed connection",
																	__func__, __LINE__);
                }

                break;
            }
#endif
//首先看free_raw_bufs是否存在，如果存在，则直接使用它
            if (p->free_raw_bufs) {

                /* use the free bufs if they exist */

                chain = p->free_raw_bufs;
                if (p->single_buf) {
                    p->free_raw_bufs = p->free_raw_bufs->next;
                    chain->next = NULL;
                } else {
                    p->free_raw_bufs = NULL;
                }

            } else if (p->allocated < p->bufs.num) {
//如果free_raw_bufs不存在，并且分配的buf数量没有超过bufs的个数，此时则创建新的buf
                /* allocate a new buf if it's still allowed */

                b = ngx_create_temp_buf(p->pool, p->bufs.size);
                if (b == NULL) {
                    return NGX_ABORT;
                }

                p->allocated++;

                chain = ngx_alloc_chain_link(p->pool);
                if (chain == NULL) {
                    return NGX_ABORT;
                }

                chain->buf = b;
                chain->next = NULL;

            } else if (!p->cacheable
                       && p->downstream->data == p->output_ctx
                       && p->downstream->write->ready
                       && !p->downstream->write->delayed)
            {
                /*
                 * if the bufs are not needed to be saved in a cache and
                 * a downstream is ready then write the bufs to a downstream
                 */
//如果已经分配的bufs的个数大于预设定的个数，并且没有打开cache，而且downstream可写，则设置
//upstream_blocked，准备写数据到upstream（这个是为了发送数据之后，数据buf能够被使用读
                p->upstream_blocked = 1;

                ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]pipe downstream ready",
																__func__, __LINE__);

                break;

            } else if (p->cacheable
                       || p->temp_file->offset < p->max_temp_file_size)
            {

                /*
                 * if it is allowed, then save some bufs from r->in
                 * to a temporary file, and add them to a r->out chain
                 */
// 到达这里有两个情况，一个是cacheable打开，一个是当buf不够用了，此时就会将一部分数据buf到temp file中，这个函数

                rc = ngx_event_pipe_write_chain_to_temp_file(p);

                ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]pipe temp offset: %O",
																__func__, __LINE__,
																 p->temp_file->offset);

                if (rc == NGX_BUSY) {
                    break;
                }

                if (rc == NGX_AGAIN) {
                    if (ngx_event_flags & NGX_USE_LEVEL_EVENT
                        && p->upstream->read->active
                        && p->upstream->read->ready)
                    {
                        if (ngx_del_event(p->upstream->read, NGX_READ_EVENT, 0)
                            == NGX_ERROR)
                        {
                            return NGX_ABORT;
                        }
                    }
                }

                if (rc != NGX_OK) {
                    return rc;
                }
//说明写成功，此时free_raw_bufs已经被重新赋值，也就是我们可以使用，所以类似上面free_raw_bufs存在的处理
                chain = p->free_raw_bufs;
                if (p->single_buf) {
                    p->free_raw_bufs = p->free_raw_bufs->next;
                    chain->next = NULL;
                } else {
                    p->free_raw_bufs = NULL;
                }

            } else {

                /* there are no bufs to read in */

                ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]no pipe bufs to read in",
																__func__, __LINE__);

                break;
            }
//开始从后端读取数据，可以看到书记员被读取进chain，n表示读到的字节数
            n = p->upstream->recv_chain(p->upstream, chain);

            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "[%s:%d]pipe recv chain: %z",
														__func__, __LINE__, n);
//如果将chain添加到free_raw_bufs的开头
            if (p->free_raw_bufs) {
                chain->next = p->free_raw_bufs;
            }
            p->free_raw_bufs = chain;
//设置error
            if (n == NGX_ERROR) {
                p->upstream_error = 1;
                return NGX_ERROR;
            }

            if (n == NGX_AGAIN) {
                if (p->single_buf) {
                    ngx_event_pipe_remove_shadow_links(chain->buf);
                }

                break;
            }
//设置read，表示已经读取了数据
            p->read = 1;
//如果返回0，则说明对端关闭了连接
            if (n == 0) {
                p->upstream_eof = 1;
                break;
            }
        }
//更新已经读取了的字节数
        p->read_length += n;
        cl = chain;
        p->free_raw_bufs = NULL;
//开始遍历chain，
        while (cl && n > 0) {
//首先remove shadow buf
            ngx_event_pipe_remove_shadow_links(cl->buf);
//得到当前的chain buf的空间大小（因为读取数据，是从cl->buf->last开始的）
            size = cl->buf->end - cl->buf->last;
//如果已经读取的字节数大于等于chain buf，ze对当前的buf更新
            if (n >= size) {
//更新last
                cl->buf->last = cl->buf->end;

                /* STUB */ cl->buf->num = p->num++;
//调用input_filter
                if (p->input_filter(p, cl->buf) == NGX_ERROR) {
                    return NGX_ABORT;
                }
//更新n, cl最终free chain
                n -= size;
                ln = cl;
                cl = cl->next;
                ngx_free_chain(p->pool, ln);

            } else {
//否则说明当前的chain是最后一个chain，因此更新last然后设置你，以便退出循环
//这里要注意，可以看到nginx并没有调用input_filter，这事因为，nginx会尽量的使cl->buf最大情况下调用
//p->input_filter,不过这里会有个问题，当cl->buf没有最大，这次就会少调用一次p->input_filter, 不过nginx在最后会处理这个问题
                cl->buf->last += n;
                n = 0;
            }
        }
//如果cl还存在，则说明我们开始设置的chain，只有一部分被使用了，因此此时将这写chain保存到
//free_raw_bufs中。可以看到如果chain只有一部分被使用，然后当再次循环，则使用的chain会直接使用free_raw_bufs，也就是我们前一次没有使用完全的chain
        if (cl) {
            for (ln = cl; ln->next; ln = ln->next) { /* void */ }

            ln->next = p->free_raw_bufs;
            p->free_raw_bufs = cl;
        }
    }

#if (NGX_DEBUG)

    for (cl = p->busy; cl; cl = cl->next) {
        ngx_log_debug10(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe buf busy s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %z",
												__func__, __LINE__,
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->out; cl; cl = cl->next) {
        ngx_log_debug10(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe buf out  s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %z",
												__func__, __LINE__,
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->in; cl; cl = cl->next) {
        ngx_log_debug10(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe buf in   s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %z",
												__func__, __LINE__,
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->free_raw_bufs; cl; cl = cl->next) {
        ngx_log_debug10(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe buf free s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %z",
												__func__, __LINE__,
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "[%s:%d]pipe length: %O",
										__func__, __LINE__,
										 p->length);

#endif

    if (p->free_raw_bufs && p->length != -1) {
        cl = p->free_raw_bufs;

        if (cl->buf->last - cl->buf->pos >= p->length) {

            p->free_raw_bufs = cl->next;

            /* STUB */ cl->buf->num = p->num++;

            if (p->input_filter(p, cl->buf) == NGX_ERROR) {
                 return NGX_ABORT;
            }

            ngx_free_chain(p->pool, cl);
        }
    }

    if (p->length == 0) {
        p->upstream_done = 1;
        p->read = 1;
    }
//上面的p->input_filter我们后面再分析，先来看函数剩余的部分剩余的这一部分
//主要是处理free_raw_bufs，调用p->input_filter，将free_raw_bufs中的数据保存发送chain中，
//这个就是为了解决前面少调用一次p->input_filter的情况。
    if ((p->upstream_eof || p->upstream_error) && p->free_raw_bufs) {

        /* STUB */ p->free_raw_bufs->buf->num = p->num++;

        if (p->input_filter(p, p->free_raw_bufs->buf) == NGX_ERROR) {
            return NGX_ABORT;
        }

        p->free_raw_bufs = p->free_raw_bufs->next;

        if (p->free_bufs && p->buf_to_file == NULL) {
            for (cl = p->free_raw_bufs; cl; cl = cl->next) {
                if (cl->buf->shadow == NULL) {
                    ngx_pfree(p->pool, cl->buf->start);
                }
            }
        }
    }
//如果cache打开，并且p->in存在*也就是有读取的数据），则写数据到temp file
    if (p->cacheable && p->in) {
        if (ngx_event_pipe_write_chain_to_temp_file(p) == NGX_ABORT) {
            return NGX_ABORT;
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_event_pipe_write_to_downstream(ngx_event_pipe_t *p)
{
    u_char            *prev;
    size_t             bsize;
    ngx_int_t          rc;
    ngx_uint_t         flush, flushed, prev_last_shadow;
    ngx_chain_t       *out, **ll, *cl, file;
    ngx_connection_t  *downstream;

    downstream = p->downstream;

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "[%s:%d]pipe write downstream: %d",
										__func__, __LINE__,
										 downstream->write->ready);

    flushed = 0;

    for ( ;; ) {
        if (p->downstream_error) {
            return ngx_event_pipe_drain_chains(p);
        }

        if (p->upstream_eof || p->upstream_error || p->upstream_done) {

            /* pass the p->out and p->in chains to the output filter */

            for (cl = p->busy; cl; cl = cl->next) {
                cl->buf->recycled = 0;
            }

            if (p->out) {
                ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]pipe write downstream flush out", __func__, __LINE__);

                for (cl = p->out; cl; cl = cl->next) {
                    cl->buf->recycled = 0;
                }

                rc = p->output_filter(p->output_ctx, p->out);

                if (rc == NGX_ERROR) {
                    p->downstream_error = 1;
                    return ngx_event_pipe_drain_chains(p);
                }

                p->out = NULL;
            }

            if (p->in) {
                ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]pipe write downstream flush in", __func__, __LINE__);

                for (cl = p->in; cl; cl = cl->next) {
                    cl->buf->recycled = 0;
                }

                rc = p->output_filter(p->output_ctx, p->in);

                if (rc == NGX_ERROR) {
                    p->downstream_error = 1;
                    return ngx_event_pipe_drain_chains(p);
                }

                p->in = NULL;
            }

            if (p->cacheable && p->buf_to_file) {

                file.buf = p->buf_to_file;
                file.next = NULL;

                if (ngx_write_chain_to_temp_file(p->temp_file, &file)
                    == NGX_ERROR)
                {
                    return NGX_ABORT;
                }
            }

            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "[%s:%d]pipe write downstream done", __func__, __LINE__);

            /* TODO: free unused bufs */

            p->downstream_done = 1;
            break;
        }

        if (downstream->data != p->output_ctx
            || !downstream->write->ready
            || downstream->write->delayed)
        {
            break;
        }

        /* bsize is the size of the busy recycled bufs */

        prev = NULL;
        bsize = 0;

        for (cl = p->busy; cl; cl = cl->next) {

            if (cl->buf->recycled) {
                if (prev == cl->buf->start) {
                    continue;
                }

                bsize += cl->buf->end - cl->buf->start;
                prev = cl->buf->start;
            }
        }

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe write busy: %uz",
											__func__, __LINE__,
											 bsize);

        out = NULL;

        if (bsize >= (size_t) p->busy_size) {
            flush = 1;
            goto flush;
        }

        flush = 0;
        ll = NULL;
        prev_last_shadow = 1;

        for ( ;; ) {
            if (p->out) {
                cl = p->out;

                if (cl->buf->recycled) {
                    ngx_log_error(NGX_LOG_ALERT, p->log, 0,
                                  "[%s:%d]recycled buffer in pipe out chain", __func__, __LINE__);
                }

                p->out = p->out->next;

            } else if (!p->cacheable && p->in) {
                cl = p->in;

                ngx_log_debug5(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "[%s:%d]pipe write buf ls:%d %p %z",
																__func__, __LINE__,
                               cl->buf->last_shadow,
                               cl->buf->pos,
                               cl->buf->last - cl->buf->pos);

                if (cl->buf->recycled && prev_last_shadow) {
                    if (bsize + cl->buf->end - cl->buf->start > p->busy_size) {
                        flush = 1;
                        break;
                    }

                    bsize += cl->buf->end - cl->buf->start;
                }

                prev_last_shadow = cl->buf->last_shadow;

                p->in = p->in->next;

            } else {
                break;
            }

            cl->next = NULL;

            if (out) {
                *ll = cl;
            } else {
                out = cl;
            }
            ll = &cl->next;
        }

    flush:

        ngx_log_debug4(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe write: out:%p, f:%d",
												__func__, __LINE__,
											 out, flush);

        if (out == NULL) {

            if (!flush) {
                break;
            }

            /* a workaround for AIO */
            if (flushed++ > 10) {
                return NGX_BUSY;
            }
        }

        rc = p->output_filter(p->output_ctx, out);

        ngx_chain_update_chains(p->pool, &p->free, &p->busy, &out, p->tag);

        if (rc == NGX_ERROR) {
            p->downstream_error = 1;
            return ngx_event_pipe_drain_chains(p);
        }

        for (cl = p->free; cl; cl = cl->next) {

            if (cl->buf->temp_file) {
                if (p->cacheable || !p->cyclic_temp_file) {
                    continue;
                }

                /* reset p->temp_offset if all bufs had been sent */

                if (cl->buf->file_last == p->temp_file->offset) {
                    p->temp_file->offset = 0;
                }
            }

            /* TODO: free buf if p->free_bufs && upstream done */

            /* add the free shadow raw buf to p->free_raw_bufs */

            if (cl->buf->last_shadow) {
                if (ngx_event_pipe_add_free_buf(p, cl->buf->shadow) != NGX_OK) {
                    return NGX_ABORT;
                }

                cl->buf->last_shadow = 0;
            }

            cl->buf->shadow = NULL;
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_event_pipe_write_chain_to_temp_file(ngx_event_pipe_t *p)
{
    ssize_t       size, bsize, n;
    ngx_buf_t    *b;
    ngx_uint_t    prev_last_shadow;
    ngx_chain_t  *cl, *tl, *next, *out, **ll, **last_out, **last_free, fl;

    if (p->buf_to_file) {
        fl.buf = p->buf_to_file;
        fl.next = p->in;
        out = &fl;

    } else {
        out = p->in;
    }

    if (!p->cacheable) {

        size = 0;
        cl = out;
        ll = NULL;
        prev_last_shadow = 1;

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "[%s:%d]pipe offset: %O",
												__func__, __LINE__,
												 p->temp_file->offset);

        do {
            bsize = cl->buf->last - cl->buf->pos;

            ngx_log_debug6(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "[%s:%d]pipe buf ls:%d %p, pos %p, size: %z",
													__func__, __LINE__,
                           cl->buf->last_shadow, cl->buf->start,
                           cl->buf->pos, bsize);

            if (prev_last_shadow
                && ((size + bsize > p->temp_file_write_size)
                    || (p->temp_file->offset + size + bsize
                        > p->max_temp_file_size)))
            {
                break;
            }

            prev_last_shadow = cl->buf->last_shadow;

            size += bsize;
            ll = &cl->next;
            cl = cl->next;

        } while (cl);

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0, "[%s:%d]size: %z",__func__, __LINE__, size);

        if (ll == NULL) {
            return NGX_BUSY;
        }

        if (cl) {
           p->in = cl;
           *ll = NULL;

        } else {
           p->in = NULL;
           p->last_in = &p->in;
        }

    } else {
        p->in = NULL;
        p->last_in = &p->in;
    }

    n = ngx_write_chain_to_temp_file(p->temp_file, out);

    if (n == NGX_ERROR) {
        return NGX_ABORT;
    }

    if (p->buf_to_file) {
        p->temp_file->offset = p->buf_to_file->last - p->buf_to_file->pos;
        n -= p->buf_to_file->last - p->buf_to_file->pos;
        p->buf_to_file = NULL;
        out = out->next;
    }

    if (n > 0) {
        /* update previous buffer or add new buffer */

        if (p->out) {
            for (cl = p->out; cl->next; cl = cl->next) { /* void */ }

            b = cl->buf;

            if (b->file_last == p->temp_file->offset) {
                p->temp_file->offset += n;
                b->file_last = p->temp_file->offset;
                goto free;
            }

            last_out = &cl->next;

        } else {
            last_out = &p->out;
        }

        cl = ngx_chain_get_free_buf(p->pool, &p->free);
        if (cl == NULL) {
            return NGX_ABORT;
        }

        b = cl->buf;

        ngx_memzero(b, sizeof(ngx_buf_t));

        b->tag = p->tag;

        b->file = &p->temp_file->file;
        b->file_pos = p->temp_file->offset;
        p->temp_file->offset += n;
        b->file_last = p->temp_file->offset;

        b->in_file = 1;
        b->temp_file = 1;

        *last_out = cl;
    }

free:

    for (last_free = &p->free_raw_bufs;
         *last_free != NULL;
         last_free = &(*last_free)->next)
    {
        /* void */
    }

    for (cl = out; cl; cl = next) {
        next = cl->next;

        cl->next = p->free;
        p->free = cl;

        b = cl->buf;

        if (b->last_shadow) {

            tl = ngx_alloc_chain_link(p->pool);
            if (tl == NULL) {
                return NGX_ABORT;
            }

            tl->buf = b->shadow;
            tl->next = NULL;

            *last_free = tl;
            last_free = &tl->next;

            b->shadow->pos = b->shadow->start;
            b->shadow->last = b->shadow->start;

            ngx_event_pipe_remove_shadow_links(b->shadow);
        }
    }

    return NGX_OK;
}
/*

nginx默认实现的一个ngx_event_pipe_copy_input_filter，其中proxy等模块都是调用这个filter。它主要是拷贝buf(不是buf的内容，
只是buf的属性)到p->in或者p->last_in,这两个域都是用来和write数据的时候交互用的。这两个域的区别是这样子的
p->in只能保存一个chain，而p->in这条链上的剩余的chain都保存在p->last_in中，这么做的原因还不太清楚，而且搜索
了下代码，last_in也没有被使用到.
*/

/* the copy input filter */

ngx_int_t
ngx_event_pipe_copy_input_filter(ngx_event_pipe_t *p, ngx_buf_t *buf)
{
    ngx_buf_t    *b;
    ngx_chain_t  *cl;

    if (buf->pos == buf->last) {
        return NGX_OK;
    }
//如果free存在，则从free中取得缓存的buf
    if (p->free) {
        cl = p->free;
        b = cl->buf;
        p->free = cl->next;
        ngx_free_chain(p->pool, cl);

    } else {
//否则分配buf
        b = ngx_alloc_buf(p->pool);
        if (b == NULL) {
            return NGX_ERROR;
        }
    }
//拷贝buf的属性
    ngx_memcpy(b, buf, sizeof(ngx_buf_t));
    b->shadow = buf;
    b->tag = p->tag;
    b->last_shadow = 1;
    b->recycled = 1;
    buf->shadow = b;
//分配chain
    cl = ngx_alloc_chain_link(p->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = NULL;

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0, "[%s:%d]input buf #%d",__func__, __LINE__, b->num);

    if (p->in) {
        *p->last_in = cl;
    } else {
        p->in = cl;
    }
    p->last_in = &cl->next;

    if (p->length == -1) {
        return NGX_OK;
    }

    p->length -= b->last - b->pos;

    return NGX_OK;
}


static ngx_inline void
ngx_event_pipe_remove_shadow_links(ngx_buf_t *buf)
{
    ngx_buf_t  *b, *next;

    b = buf->shadow;

    if (b == NULL) {
        return;
    }

    while (!b->last_shadow) {
        next = b->shadow;

        b->temporary = 0;
        b->recycled = 0;

        b->shadow = NULL;
        b = next;
    }

    b->temporary = 0;
    b->recycled = 0;
    b->last_shadow = 0;

    b->shadow = NULL;

    buf->shadow = NULL;
}


ngx_int_t
ngx_event_pipe_add_free_buf(ngx_event_pipe_t *p, ngx_buf_t *b)
{
    ngx_chain_t  *cl;

    cl = ngx_alloc_chain_link(p->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    if (p->buf_to_file && b->start == p->buf_to_file->start) {
        b->pos = p->buf_to_file->last;
        b->last = p->buf_to_file->last;

    } else {
        b->pos = b->start;
        b->last = b->start;
    }

    b->shadow = NULL;

    cl->buf = b;

    if (p->free_raw_bufs == NULL) {
        p->free_raw_bufs = cl;
        cl->next = NULL;

        return NGX_OK;
    }

    if (p->free_raw_bufs->buf->pos == p->free_raw_bufs->buf->last) {

        /* add the free buf to the list start */

        cl->next = p->free_raw_bufs;
        p->free_raw_bufs = cl;

        return NGX_OK;
    }

    /* the first free buf is partially filled, thus add the free buf after it */

    cl->next = p->free_raw_bufs->next;
    p->free_raw_bufs->next = cl;

    return NGX_OK;
}


static ngx_int_t
ngx_event_pipe_drain_chains(ngx_event_pipe_t *p)
{
    ngx_chain_t  *cl, *tl;

    for ( ;; ) {
        if (p->busy) {
            cl = p->busy;
            p->busy = NULL;

        } else if (p->out) {
            cl = p->out;
            p->out = NULL;

        } else if (p->in) {
            cl = p->in;
            p->in = NULL;

        } else {
            return NGX_OK;
        }

        while (cl) {
            if (cl->buf->last_shadow) {
                if (ngx_event_pipe_add_free_buf(p, cl->buf->shadow) != NGX_OK) {
                    return NGX_ABORT;
                }

                cl->buf->last_shadow = 0;
            }

            cl->buf->shadow = NULL;
            tl = cl->next;
            cl->next = p->free;
            p->free = cl;
            cl = tl;
        }
    }
}
