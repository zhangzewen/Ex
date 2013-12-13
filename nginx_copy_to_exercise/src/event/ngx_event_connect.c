
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>


ngx_int_t
ngx_event_connect_peer(ngx_peer_connection_t *pc)
{
    int                rc;
    ngx_int_t          event;
    ngx_err_t          err;
    ngx_uint_t         level;
    ngx_socket_t       s;
    ngx_event_t       *rev, *wev;
    ngx_connection_t  *c;
//取得我们要发送的upstream对端
    rc = pc->get(pc, pc->data);
    if (rc != NGX_OK) {
        return rc;
    }
//新建socket
    s = ngx_socket(pc->sockaddr->sa_family, SOCK_STREAM, 0);

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, pc->log, 0, "[%s:%d]socket %d",__func__, __LINE__,s);

    if (s == -1) {
        ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                      ngx_socket_n "[%s:%d] failed", __func__, __LINE__);
        return NGX_ERROR;
    }

//取得连接
    c = ngx_get_connection(s, pc->log);

    if (c == NULL) {
        if (ngx_close_socket(s) == -1) {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          ngx_close_socket_n "[%s:%d]failed", __func__, __LINE__);
        }

        return NGX_ERROR;
    }
//设置recvbuf的大小
    if (pc->rcvbuf) {
        if (setsockopt(s, SOL_SOCKET, SO_RCVBUF,
                       (const void *) &pc->rcvbuf, sizeof(int)) == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          "[%s:%d]setsockopt(SO_RCVBUF) failed", __func__, __LINE__);
            goto failed;
        }
    }
//设置非阻塞
    if (ngx_nonblocking(s) == -1) {
        ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                      ngx_nonblocking_n "[%s:%d] failed", __func__, __LINE__);

        goto failed;
    }

    if (pc->local) {
        if (bind(s, pc->local->sockaddr, pc->local->socklen) == -1) {
            ngx_log_error(NGX_LOG_CRIT, pc->log, ngx_socket_errno,
                          "[%s:%d]bind(%V) failed",
														__func__, __LINE__,
													 &pc->local->name);

            goto failed;
        }
    }
//开始挂载对应的读写函数
    c->recv = ngx_recv;
    c->send = ngx_send;
    c->recv_chain = ngx_recv_chain;
    c->send_chain = ngx_send_chain;

    c->sendfile = 1;

    c->log_error = pc->log_error;

    if (pc->sockaddr->sa_family != AF_INET) {
        c->tcp_nopush = NGX_TCP_NOPUSH_DISABLED;
        c->tcp_nodelay = NGX_TCP_NODELAY_DISABLED;

#if (NGX_SOLARIS)
        /* Solaris's sendfilev() supports AF_NCA, AF_INET, and AF_INET6 */
        c->sendfile = 0;
#endif
    }

    rev = c->read;
    wev = c->write;

    rev->log = pc->log;
    wev->log = pc->log;

    pc->connection = c;

    c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);

#if (NGX_THREADS)

    /* TODO: lock event when call completion handler */

    rev->lock = pc->lock;
    wev->lock = pc->lock;
    rev->own_lock = &c->lock;
    wev->own_lock = &c->lock;

#endif

    if (ngx_add_conn) {
        if (ngx_add_conn(c) == NGX_ERROR) {
            goto failed;
        }
    }

    ngx_log_debug5(NGX_LOG_DEBUG_EVENT, pc->log, 0,
                   "[%s:%d]connect to %V, fd:%d #%d",
										__func__, __LINE__,
									 pc->name, s, c->number);

    rc = connect(s, pc->sockaddr, pc->socklen);

    if (rc == -1) {
        err = ngx_socket_errno;


        if (err != NGX_EINPROGRESS
#if (NGX_WIN32)
            /* Winsock returns WSAEWOULDBLOCK (NGX_EAGAIN) */
            && err != NGX_EAGAIN
#endif
            )
        {
            if (err == NGX_ECONNREFUSED
#if (NGX_LINUX)
                /*
                 * Linux returns EAGAIN instead of ECONNREFUSED
                 * for unix sockets if listen queue is full
                 */
                || err == NGX_EAGAIN
#endif
                || err == NGX_ECONNRESET
                || err == NGX_ENETDOWN
                || err == NGX_ENETUNREACH
                || err == NGX_EHOSTDOWN
                || err == NGX_EHOSTUNREACH)
            {
                level = NGX_LOG_ERR;

            } else {
                level = NGX_LOG_CRIT;
            }

            ngx_log_error(level, c->log, err, "[%s:%d]connect() to %V failed",
													__func__, __LINE__,
                          pc->name);

            ngx_close_connection(c);
            pc->connection = NULL;

            return NGX_DECLINED;
        }
    }

    if (ngx_add_conn) {
        if (rc == -1) {

            /* NGX_EINPROGRESS */

            return NGX_AGAIN;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, pc->log, 0, "[%s:%d]connected", __func__, __LINE__);

        wev->ready = 1;

        return NGX_OK;
    }

    if (ngx_event_flags & NGX_USE_AIO_EVENT) {

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, pc->log, ngx_socket_errno,
                       "[%s:%d]connect(): %d",
											__func__, __LINE__,
											 rc);

        /* aio, iocp */

        if (ngx_blocking(s) == -1) {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          ngx_blocking_n "[%s:%d] failed", __func__, __LINE__);
            goto failed;
        }

        /*
         * FreeBSD's aio allows to post an operation on non-connected socket.
         * NT does not support it.
         *
         * TODO: check in Win32, etc. As workaround we can use NGX_ONESHOT_EVENT
         */

        rev->ready = 1;
        wev->ready = 1;

        return NGX_OK;
    }

    if (ngx_event_flags & NGX_USE_CLEAR_EVENT) {

        /* kqueue */

        event = NGX_CLEAR_EVENT;

    } else {

        /* select, poll, /dev/poll */

        event = NGX_LEVEL_EVENT;
    }

    if (ngx_add_event(rev, NGX_READ_EVENT, event) != NGX_OK) {
        goto failed;
    }

    if (rc == -1) {

        /* NGX_EINPROGRESS */

        if (ngx_add_event(wev, NGX_WRITE_EVENT, event) != NGX_OK) {
            goto failed;
        }

        return NGX_AGAIN;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, pc->log, 0, "[%s:%d]connected", __func__, __LINE__);

    wev->ready = 1;

    return NGX_OK;

failed:

    ngx_close_connection(c);
    pc->connection = NULL;

    return NGX_ERROR;
}


ngx_int_t
ngx_event_get_peer(ngx_peer_connection_t *pc, void *data)
{
    return NGX_OK;
}
