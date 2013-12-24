
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


/*
 * On Linux up to 2.4.21 sendfile() (syscall #187) works with 32-bit
 * offsets only, and the including <sys/sendfile.h> breaks the compiling,
 * if off_t is 64 bit wide.  So we use own sendfile() definition, where offset
 * parameter is int32_t, and use sendfile() for the file parts below 2G only,
 * see src/os/unix/ngx_linux_config.h
 *
 * Linux 2.4.21 has the new sendfile64() syscall #239.
 *
 * On Linux up to 2.6.16 sendfile() does not allow to pass the count parameter
 * more than 2G-1 bytes even on 64-bit platforms: it returns EINVAL,
 * so we limit it to 2G-1 bytes.
 */

#define NGX_SENDFILE_LIMIT  2147483647L


#if (IOV_MAX > 64)
#define NGX_HEADERS  64
#else
#define NGX_HEADERS  IOV_MAX
#endif


ngx_chain_t *
ngx_linux_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{
		syslog(LOG_INFO, "[%s:%s:%d]", __FILE__, __func__, __LINE__);
    int            rc, tcp_nodelay;
    off_t          size, send, prev_send, aligned, sent, fprev;
    u_char        *prev;
    size_t         file_size;
    ngx_err_t      err;
    ngx_buf_t     *file;
    ngx_uint_t     eintr, complete;
    ngx_array_t    header;
    ngx_event_t   *wev;
    ngx_chain_t   *cl;
    struct iovec  *iov, headers[NGX_HEADERS];
#if (NGX_HAVE_SENDFILE64)
    off_t          offset;
#else
    int32_t        offset;
#endif

    wev = c->write;

    if (!wev->ready) {
        return in;
    }


    /* the maximum limit size is 2G-1 - the page size */

    if (limit == 0 || limit > (off_t) (NGX_SENDFILE_LIMIT - ngx_pagesize)) {
        limit = NGX_SENDFILE_LIMIT - ngx_pagesize;
    }


    send = 0;

    header.elts = headers;
    header.size = sizeof(struct iovec);
    header.nalloc = NGX_HEADERS;
    header.pool = c->pool;

    for ( ;; ) {
        file = NULL;
        file_size = 0;
        eintr = 0;
        complete = 0;
        prev_send = send;

        header.nelts = 0;

        prev = NULL;
        iov = NULL;

        /* create the iovec and coalesce the neighbouring bufs */

        for (cl = in; cl && send < limit; cl = cl->next) {

            if (ngx_buf_special(cl->buf)) {
                continue;
            }

#if 1
            if (!ngx_buf_in_memory(cl->buf) && !cl->buf->in_file) {
                ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                              "[%s:%d]zero size buf in sendfile "
                              "t:%d r:%d f:%d %p %p-%p %p %O-%O",
															__func__, __LINE__,
                              cl->buf->temporary,
                              cl->buf->recycled,
                              cl->buf->in_file,
                              cl->buf->start,
                              cl->buf->pos,
                              cl->buf->last,
                              cl->buf->file,
                              cl->buf->file_pos,
                              cl->buf->file_last);

                ngx_debug_point();

                return NGX_CHAIN_ERROR;
            }
#endif

            if (!ngx_buf_in_memory_only(cl->buf)) {
                break;
            }

            size = cl->buf->last - cl->buf->pos;

            if (send + size > limit) {
                size = limit - send;
            }

            if (prev == cl->buf->pos) {
                iov->iov_len += (size_t) size;

            } else {
                if (header.nelts >= IOV_MAX) {
                    break;
                }

                iov = ngx_array_push(&header);
                if (iov == NULL) {
                    return NGX_CHAIN_ERROR;
                }

                iov->iov_base = (void *) cl->buf->pos;
                iov->iov_len = (size_t) size;
            }

            prev = cl->buf->pos + (size_t) size;
            send += size;
        }

        /* set TCP_CORK if there is a header before a file */

        if (c->tcp_nopush == NGX_TCP_NOPUSH_UNSET
            && header.nelts != 0
            && cl
            && cl->buf->in_file)
        {
            /* the TCP_CORK and TCP_NODELAY are mutually exclusive */

            if (c->tcp_nodelay == NGX_TCP_NODELAY_SET) {

                tcp_nodelay = 0;

                if (setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY,
                               (const void *) &tcp_nodelay, sizeof(int)) == -1)
                {
                    err = ngx_errno;

                    /*
                     * there is a tiny chance to be interrupted, however,
                     * we continue a processing with the TCP_NODELAY
                     * and without the TCP_CORK
                     */

                    if (err != NGX_EINTR) {
                        wev->error = 1;
                        ngx_connection_error(c, err,
                                             "setsockopt(TCP_NODELAY) failed");
                        return NGX_CHAIN_ERROR;
                    }

                } else {
                    c->tcp_nodelay = NGX_TCP_NODELAY_UNSET;

                    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, c->log, 0,
                                   "no tcp_nodelay");
                }
            }

            if (c->tcp_nodelay == NGX_TCP_NODELAY_UNSET) {

                if (ngx_tcp_nopush(c->fd) == NGX_ERROR) {
                    err = ngx_errno;

                    /*
                     * there is a tiny chance to be interrupted, however,
                     * we continue a processing without the TCP_CORK
                     */

                    if (err != NGX_EINTR) {
                        wev->error = 1;
                        ngx_connection_error(c, err,
                                             ngx_tcp_nopush_n " failed");
                        return NGX_CHAIN_ERROR;
                    }

                } else {
                    c->tcp_nopush = NGX_TCP_NOPUSH_SET;

                    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                                   "tcp_nopush");
                }
            }
        }

        /* get the file buf */
//可以看到如果header的大小不为0则说明前面有需要发送的buf，因此我们就跳过in file的处理
        if (header.nelts == 0 && cl && cl->buf->in_file && send < limit) {
//得到file
            file = cl->buf;

            /* coalesce the neighbouring file bufs */
//开始合并
            do {
//得到大小
                size = cl->buf->file_last - cl->buf->file_pos;
//如果太大则进行对齐处理
                if (send + size > limit) {
                    size = limit - send;

                    aligned = (cl->buf->file_pos + size + ngx_pagesize - 1)
                               & ~((off_t) ngx_pagesize - 1);

                    if (aligned <= cl->buf->file_last) {
                        size = aligned - cl->buf->file_pos;
                    }
                }
//设置file_size
                file_size += (size_t) size;
//设置需要发送的大小
                send += size;
//和上面的in memory处理一样就是保存这次的last 
                fprev = cl->buf->file_pos + size;
                cl = cl->next;

            } while (cl
                     && cl->buf->in_file
                     && send < limit
                     && file->file->fd == cl->buf->file->fd
                     && fprev == cl->buf->file_pos);
        }

        if (file) {
#if 1
            if (file_size == 0) {
                ngx_debug_point();
                return NGX_CHAIN_ERROR;
            }
#endif
#if (NGX_HAVE_SENDFILE64)
            offset = file->file_pos;
#else
            offset = (int32_t) file->file_pos;
#endif

            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "sendfile: @%O %uz", file->file_pos, file_size);

            rc = sendfile(c->fd, file->file->fd, &offset, file_size);

            if (rc == -1) {
                err = ngx_errno;

                switch (err) {
                case NGX_EAGAIN:
                    break;

                case NGX_EINTR:
                    eintr = 1;
                    break;

                default:
                    wev->error = 1;
                    ngx_connection_error(c, err, "sendfile() failed");
                    return NGX_CHAIN_ERROR;
                }

                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                               "sendfile() is not ready");
            }

            sent = rc > 0 ? rc : 0;

            ngx_log_debug4(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "sendfile: %d, @%O %O:%uz",
                           rc, file->file_pos, sent, file_size);

        } else {
            rc = writev(c->fd, header.elts, header.nelts);

            if (rc == -1) {
                err = ngx_errno;

                switch (err) {
                case NGX_EAGAIN:
                    break;

                case NGX_EINTR:
                    eintr = 1;
                    break;

                default:
                    wev->error = 1;
                    ngx_connection_error(c, err, "writev() failed");
                    return NGX_CHAIN_ERROR;
                }

                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                               "writev() not ready");
            }

            sent = rc > 0 ? rc : 0;

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "writev: %O", sent);
        }
//如果send - prev_send == send则说明该发送的都发送完毕了
        if (send - prev_send == sent) {
            complete = 1;
        }
//更新congnect的sent域
        c->sent += sent;
//开始重新遍历chain，这里是为了防止没有发送完全的情况，此时我们就需要切割buf了
        for (cl = in; cl; cl = cl->next) {

            if (ngx_buf_special(cl->buf)) {
                continue;
            }

            if (sent == 0) {
                break;
            }
//得到buf size
            size = ngx_buf_size(cl->buf);
//如果大于当前的size，则说明这个buf的数据已经被完全发送完毕了，因此更新它的域
            if (sent >= size) {
//更新send域
                sent -= size;
//如果在内存中则更新pos
                if (ngx_buf_in_memory(cl->buf)) {
                    cl->buf->pos = cl->buf->last;
                }
//如果在filezhong
                if (cl->buf->in_file) {
                    cl->buf->file_pos = cl->buf->file_last;
                }

                continue;
            }
//到这里说明当前buf只有一部分被发送出去了，一次这里我们只需要修改指针，以便下次发送
            if (ngx_buf_in_memory(cl->buf)) {
                cl->buf->pos += (size_t) sent;
            }

            if (cl->buf->in_file) {
                cl->buf->file_pos += sent;
            }

            break;
        }
//nginx中如果发送未完成的话，将会直接返回，返回的计时没有发送完毕的chain，
//它的buf也已经更新

        if (eintr) {
            continue;
        }
//如果未完成，则返回
        if (!complete) {
            wev->ready = 0;
            return cl;
        }

        if (send >= limit || cl == NULL) {
            return cl;
        }
//更新in，也就是开始处理下一个chain 
        in = cl;
    }
}
