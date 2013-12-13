
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_os_io_t  ngx_io;


static void ngx_drain_connections(void);

//! init the ngx_listening_t struct ,not binding ,listening yet!
/*!
	\param cf a pointer to ngx_conf_t struct that the config args
	\param sockaddr the ip and port that binded to socket fd ，void * will be transformed to struct sockaddr *
	\param socklen 
	\return get the inited pointer to ngx_listening_t 
*/
ngx_listening_t *
ngx_create_listening(ngx_conf_t *cf, void *sockaddr, socklen_t socklen)
{
    size_t            len;
    ngx_listening_t  *ls;
    struct sockaddr  *sa;
    u_char            text[NGX_SOCKADDR_STRLEN];

    ls = ngx_array_push(&cf->cycle->listening);
    if (ls == NULL) {
        return NULL;
    }

    ngx_memzero(ls, sizeof(ngx_listening_t));

    sa = ngx_palloc(cf->pool, socklen);
    if (sa == NULL) {
        return NULL;
    }

    ngx_memcpy(sa, sockaddr, socklen);

    ls->sockaddr = sa;
    ls->socklen = socklen;

    len = ngx_sock_ntop(sa, text, NGX_SOCKADDR_STRLEN, 1);
    ls->addr_text.len = len;

    switch (ls->sockaddr->sa_family) {
    case AF_INET:
         ls->addr_text_max_len = NGX_INET_ADDRSTRLEN;
         break;
    default:
         ls->addr_text_max_len = NGX_SOCKADDR_STRLEN;
         break;
    }

    ls->addr_text.data = ngx_pnalloc(cf->pool, len);
    if (ls->addr_text.data == NULL) {
        return NULL;
    }

    ngx_memcpy(ls->addr_text.data, text, len);

    ls->fd = (ngx_socket_t) -1;
    ls->type = SOCK_STREAM;

    ls->backlog = NGX_LISTEN_BACKLOG;
    ls->rcvbuf = -1;
    ls->sndbuf = -1;


    return ls;
}


ngx_int_t
ngx_set_inherited_sockets(ngx_cycle_t *cycle)
{
    size_t                     len;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;
    socklen_t                  olen;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        ls[i].sockaddr = ngx_palloc(cycle->pool, NGX_SOCKADDRLEN);
        if (ls[i].sockaddr == NULL) {
            return NGX_ERROR;
        }

        ls[i].socklen = NGX_SOCKADDRLEN;
        if (getsockname(ls[i].fd, ls[i].sockaddr, &ls[i].socklen) == -1) {
            ngx_log_error(NGX_LOG_CRIT, cycle->log, ngx_socket_errno,
                          "[%s:%d]getsockname() of the inherited "
                          "socket #%d failed",
													__func__, __LINE__,
													 ls[i].fd);
            ls[i].ignore = 1;
            continue;
        }

        switch (ls[i].sockaddr->sa_family) {



        case AF_INET:
             ls[i].addr_text_max_len = NGX_INET_ADDRSTRLEN;
             len = NGX_INET_ADDRSTRLEN + sizeof(":65535") - 1;
             break;

        default:
            ngx_log_error(NGX_LOG_CRIT, cycle->log, ngx_socket_errno,
                          "[%s:%d]the inherited socket #%d has "
                          "an unsupported protocol family",
													 __func__, __LINE__,
													 ls[i].fd);
            ls[i].ignore = 1;
            continue;
        }

        ls[i].addr_text.data = ngx_pnalloc(cycle->pool, len);
        if (ls[i].addr_text.data == NULL) {
            return NGX_ERROR;
        }

        len = ngx_sock_ntop(ls[i].sockaddr, ls[i].addr_text.data, len, 1);
        if (len == 0) {
            return NGX_ERROR;
        }

        ls[i].addr_text.len = len;

        ls[i].backlog = NGX_LISTEN_BACKLOG;

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (void *) &ls[i].rcvbuf,
                       &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "[%s:%d]getsockopt(SO_RCVBUF) %V failed, ignored",
													__func__, __LINE__,
                          &ls[i].addr_text);

            ls[i].rcvbuf = -1;
        }

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (void *) &ls[i].sndbuf,
                       &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "[%s:%d]getsockopt(SO_SNDBUF) %V failed, ignored",
													__func__, __LINE__,
                          &ls[i].addr_text);

            ls[i].sndbuf = -1;
        }



    }

    return NGX_OK;
}

/*!as we known ,that we init the ngx_listening_t from function ngx_create_listening(ngx_conf_t *cf, void *sockaddr, sockent_t socklen),now wo band it and listen it
	\param cycle it contains all of the arags parased from the conf/nginx.conf 
	\return NGX_ERROR for listening fall ,NGX_OK for listening successes!
*/
ngx_int_t
ngx_open_listening_sockets(ngx_cycle_t *cycle)
{
    int               reuseaddr;
    ngx_uint_t        i, tries, failed;
    ngx_err_t         err;
    ngx_log_t        *log;
    ngx_socket_t      s;
    ngx_listening_t  *ls;

    reuseaddr = 1;
#if (NGX_SUPPRESS_WARN)
    failed = 0;
#endif

    log = cycle->log;

    /* TODO: configurable try number */

    for (tries = 5; tries; tries--) {
        failed = 0;

        /* for each listening socket */

        ls = cycle->listening.elts;
        for (i = 0; i < cycle->listening.nelts; i++) {

            if (ls[i].ignore) {
                continue;
            }

            if (ls[i].fd != -1) { //过滤掉已经正在监听的
                continue;
            }

            if (ls[i].inherited) {

                /* TODO: close on exit */
                /* TODO: nonblocking */
                /* TODO: deferred accept */

                continue;
            }
						//socket(domain, type , flags)
            s = ngx_socket(ls[i].sockaddr->sa_family, ls[i].type, 0);

            if (s == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              ngx_socket_n "[%s:%d] %V failed",
															__func__, __LINE__,
															 &ls[i].addr_text);
                return NGX_ERROR;
            }
						//set socket address reuseadble允许重用本地地址
            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                           (const void *) &reuseaddr, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "[%s:%d]setsockopt(SO_REUSEADDR) %V failed",
															__func__, __LINE__,
                              &ls[i].addr_text);

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n "[%s:%d] %V failed",
																	__func__, __LINE__,
                                  &ls[i].addr_text);
                }

                return NGX_ERROR;
            }

            /* TODO: close on exit */

            if (!(ngx_event_flags & NGX_USE_AIO_EVENT)) {
                if (ngx_nonblocking(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_nonblocking_n "[%s:%d] %V failed",
																	__func__, __LINE__,
                                  &ls[i].addr_text);

                    if (ngx_close_socket(s) == -1) {
                        ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                      ngx_close_socket_n "[%s:%d] %V failed",
																			__func__, __LINE__,
                                      &ls[i].addr_text);
                    }

                    return NGX_ERROR;
                }
            }

            ngx_log_debug4(NGX_LOG_DEBUG_CORE, log, 0,
                           "[%s:%d]bind() %V #%d ",
													__func__, __LINE__,
													 &ls[i].addr_text, s);
						//bind(fd, struct sockaddr *, socklen_t)
            if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
                err = ngx_socket_errno;

                if (err == NGX_EADDRINUSE && ngx_test_config) {
                    continue;
                }

                ngx_log_error(NGX_LOG_EMERG, log, err,
                              "[%s:%d]bind() to %V failed",
															__func__, __LINE__,
															 &ls[i].addr_text);

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n "[%s:%d] %V failed",
																	__func__, __LINE__,
                                  &ls[i].addr_text);
                }

                if (err != NGX_EADDRINUSE) {
                    return NGX_ERROR;
                }

                failed = 1;

                continue;
            }

						//listen(fd, backlog)
            if (listen(s, ls[i].backlog) == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "[%s:%d]listen() to %V, backlog %d failed",
															__func__, __LINE__,
                              &ls[i].addr_text, ls[i].backlog);

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n "[%s:%d] %V failed",
																	__func__, __LINE__,
                                  &ls[i].addr_text);
                }

                return NGX_ERROR;
            }

            ls[i].listen = 1;

            ls[i].fd = s;
        }

        if (!failed) {
            break;
        }

        /* TODO: delay configurable */

        ngx_log_error(NGX_LOG_NOTICE, log, 0,
                      "[%s:%d]try again to bind() after 500ms", __func__, __LINE__);

        ngx_msleep(500);
    }

    if (failed) {
        ngx_log_error(NGX_LOG_EMERG, log, 0, "[%s:%d]still could not bind()", __func__, __LINE__);
        return NGX_ERROR;
    }

    return NGX_OK;
}

/*! config the ngx_listening_t args such as rcvbuf, sndbuf, and so on use setsockopt flags SO_RCVBUF, SNDBUF to size socket recv and send buff
	\param cycle it contines all of the args parsed from conf/nginx.conf 
	\return noting
*/
void
ngx_configure_listening_sockets(ngx_cycle_t *cycle)
{
    int                        keepalive;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;


    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        ls[i].log = *ls[i].logp;
				//set SO_RCVBUF
        if (ls[i].rcvbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF,
                           (const void *) &ls[i].rcvbuf, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]setsockopt(SO_RCVBUF, %d) %V failed, ignored",
															__func__, __LINE__,
                              ls[i].rcvbuf, &ls[i].addr_text);
            }
        }
				//set SO_SNDBUF
        if (ls[i].sndbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF,
                           (const void *) &ls[i].sndbuf, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]setsockopt(SO_SNDBUF, %d) %V failed, ignored",
															__func__, __LINE__,
                              ls[i].sndbuf, &ls[i].addr_text);
            }
        }
				//set SO_KEEPALIVE
        if (ls[i].keepalive) {
            keepalive = (ls[i].keepalive == 1) ? 1 : 0;

            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE,
                           (const void *) &keepalive, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]setsockopt(SO_KEEPALIVE, %d) %V failed, ignored",
															__func__, __LINE__,
                              keepalive, &ls[i].addr_text);
            }
        }

#if (NGX_HAVE_KEEPALIVE_TUNABLE)

        if (ls[i].keepidle) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPIDLE,
                           (const void *) &ls[i].keepidle, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]setsockopt(TCP_KEEPIDLE, %d) %V failed, ignored",
															__func__, __LINE__,
                              ls[i].keepidle, &ls[i].addr_text);
            }
        }

        if (ls[i].keepintvl) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPINTVL,
                           (const void *) &ls[i].keepintvl, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                             "[%s:%d]setsockopt(TCP_KEEPINTVL, %d) %V failed, ignored",
															__func__, __LINE__,
                             ls[i].keepintvl, &ls[i].addr_text);
            }
        }

        if (ls[i].keepcnt) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPCNT,
                           (const void *) &ls[i].keepcnt, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]setsockopt(TCP_KEEPCNT, %d) %V failed, ignored",
															__func__, __LINE__,
                              ls[i].keepcnt, &ls[i].addr_text);
            }
        }

#endif



        if (ls[i].listen) {

            /* change backlog via listen() */

            if (listen(ls[i].fd, ls[i].backlog) == -1) {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "[%s:%d]listen() to %V, backlog %d failed, ignored",
															__func__, __LINE__,
                              &ls[i].addr_text, ls[i].backlog);
            }
        }

        /*
         * setting deferred mode should be last operation on socket,
         * because code may prematurely continue cycle on failure
         */

    }

    return;
}

/*!close socket fd
	\param cycle it contains all of args parsed from conf/nginx.conf
	\return noting 
*/
void
ngx_close_listening_sockets(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
        return;
    }

    ngx_accept_mutex_held = 0;
    ngx_use_accept_mutex = 0;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        c = ls[i].connection;

        if (c) {
            if (c->read->active) {
                if (ngx_event_flags & NGX_USE_RTSIG_EVENT) {
                    ngx_del_conn(c, NGX_CLOSE_EVENT);

                } else if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

                    /*
                     * it seems that Linux-2.6.x OpenVZ sends events
                     * for closed shared listening sockets unless
                     * the events was explicitly deleted
                     */

                    ngx_del_event(c->read, NGX_READ_EVENT, 0);

                } else {
                    ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
                }
            }

            ngx_free_connection(c);

            c->fd = (ngx_socket_t) -1;
        }

        ngx_log_debug4(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                       "[%s:%d]close listening %V #%d ",
											__func__, __LINE__,
											 &ls[i].addr_text, ls[i].fd);

        if (ngx_close_socket(ls[i].fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                          ngx_close_socket_n "[%s:%d] %V failed",
													__func__, __LINE__,
													 &ls[i].addr_text);
        }


        ls[i].fd = (ngx_socket_t) -1;
    }

    cycle->listening.nelts = 0;
}


ngx_connection_t *
ngx_get_connection(ngx_socket_t s, ngx_log_t *log)
{
    ngx_uint_t         instance;
    ngx_event_t       *rev, *wev;
    ngx_connection_t  *c;

    /* disable warning: Win32 SOCKET is u_int while UNIX socket is int */

    if (ngx_cycle->files && (ngx_uint_t) s >= ngx_cycle->files_n) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "[%s:%d]the new socket has number %d, "
                      "but only %ui files are available",
											__func__, __LINE__,
                      s, ngx_cycle->files_n);
        return NULL;
    }

    /* ngx_mutex_lock */

    c = ngx_cycle->free_connections;

    if (c == NULL) {
        ngx_drain_connections();
        c = ngx_cycle->free_connections;
    }

    if (c == NULL) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "[%s:%d]%ui worker_connections are not enough",
											__func__, __LINE__,
                      ngx_cycle->connection_n);

        /* ngx_mutex_unlock */

        return NULL;
    }

    ngx_cycle->free_connections = c->data;
    ngx_cycle->free_connection_n--;

    /* ngx_mutex_unlock */

    if (ngx_cycle->files) {
        ngx_cycle->files[s] = c;
    }

    rev = c->read;
    wev = c->write;

    ngx_memzero(c, sizeof(ngx_connection_t));

    c->read = rev;
    c->write = wev;
    c->fd = s;
    c->log = log;

    instance = rev->instance;

    ngx_memzero(rev, sizeof(ngx_event_t));
    ngx_memzero(wev, sizeof(ngx_event_t));

    rev->instance = !instance;
    wev->instance = !instance;

    rev->index = NGX_INVALID_INDEX;
    wev->index = NGX_INVALID_INDEX;

    rev->data = c;
    wev->data = c;

    wev->write = 1;

    return c;
}

/*!delete connection from connection table
	\param c the connection to be delete
	\return noting
*/
void
ngx_free_connection(ngx_connection_t *c)
{
    /* ngx_mutex_lock */

    c->data = ngx_cycle->free_connections;
    ngx_cycle->free_connections = c;
    ngx_cycle->free_connection_n++;

    /* ngx_mutex_unlock */

    if (ngx_cycle->files) {
        ngx_cycle->files[c->fd] = NULL;
    }
}

/*!close the connection 
	\param c the connection to be closed
	\return noting
*/
void
ngx_close_connection(ngx_connection_t *c)
{
    ngx_err_t     err;
    ngx_uint_t    log_error, level;
    ngx_socket_t  fd;

    if (c->fd == -1) {
        ngx_log_error(NGX_LOG_ALERT, c->log, 0, "[%s:%d]connection already closed", __func__, __LINE__);
        return;
    }
		//删除读事件的timer
    if (c->read->timer_set) {
        ngx_del_timer(c->read);
    }
    //删除写事件的timer
    if (c->write->timer_set) {
        ngx_del_timer(c->write);
    }

    if (ngx_del_conn) {
        ngx_del_conn(c, NGX_CLOSE_EVENT);

    } else {
        if (c->read->active || c->read->disabled) {
            ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
        }

        if (c->write->active || c->write->disabled) {
            ngx_del_event(c->write, NGX_WRITE_EVENT, NGX_CLOSE_EVENT);
        }
    }


    if (c->read->prev) {
        ngx_delete_posted_event(c->read);
    }

    if (c->write->prev) {
        ngx_delete_posted_event(c->write);
    }

    c->read->closed = 1;
    c->write->closed = 1;


    ngx_reusable_connection(c, 0);

    log_error = c->log_error;

    ngx_free_connection(c);

    fd = c->fd;
    c->fd = (ngx_socket_t) -1;

    if (ngx_close_socket(fd) == -1) {

        err = ngx_socket_errno;

        if (err == NGX_ECONNRESET || err == NGX_ENOTCONN) {

            switch (log_error) {

            case NGX_ERROR_INFO:
                level = NGX_LOG_INFO;
                break;

            case NGX_ERROR_ERR:
                level = NGX_LOG_ERR;
                break;

            default:
                level = NGX_LOG_CRIT;
            }

        } else {
            level = NGX_LOG_CRIT;
        }

        /* we use ngx_cycle->log because c->log was in c->pool */

        ngx_log_error(level, ngx_cycle->log, err,
                      ngx_close_socket_n "[%s:%d] %d failed",
											__func__, __LINE__,
										  fd);
    }
}


void
ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable)
{
    ngx_log_debug3(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "[%s:%d]reusable connection: %ui",
									__func__, __LINE__,
									 reusable);

    if (c->reusable) {
        ngx_queue_remove(&c->queue);
    }

    c->reusable = reusable;

    if (reusable) {
        /* need cast as ngx_cycle is volatile */

        ngx_queue_insert_head(
            (ngx_queue_t *) &ngx_cycle->reusable_connections_queue, &c->queue);
    }
}


static void
ngx_drain_connections(void)
{
    ngx_int_t          i;
    ngx_queue_t       *q;
    ngx_connection_t  *c;

    for (i = 0; i < 32; i++) {
        if (ngx_queue_empty(&ngx_cycle->reusable_connections_queue)) {
            break;
        }

        q = ngx_queue_last(&ngx_cycle->reusable_connections_queue);
        c = ngx_queue_data(q, ngx_connection_t, queue);

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, c->log, 0,
                       "[%s:%d]reusing connection", __func__, __LINE__);

        c->close = 1;
        c->read->handler(c->read);
    }
}

/*! set local_socaddr ,getsockname return the load address bind to the socket fd
	\param c connection
	\param s domain to be seted 
	\param port port
*/
ngx_int_t
ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port)
{
    socklen_t             len;
    ngx_uint_t            addr;
    u_char                sa[NGX_SOCKADDRLEN];
    struct sockaddr_in   *sin;

    sin = (struct sockaddr_in *) c->local_sockaddr;
    addr = sin->sin_addr.s_addr;

    if (addr == 0) { //means that did not set the local address

        len = NGX_SOCKADDRLEN;

        if (getsockname(c->fd, (struct sockaddr *) &sa, &len) == -1) { // get the local address
            ngx_connection_error(c, ngx_socket_errno, "getsockname() failed");
            return NGX_ERROR;
        }

        c->local_sockaddr = ngx_palloc(c->pool, len);
        if (c->local_sockaddr == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(c->local_sockaddr, &sa, len);
    }

    if (s == NULL) {
        return NGX_OK;
    }

    s->len = ngx_sock_ntop(c->local_sockaddr, s->data, s->len, port); //set the local address by s and  port via ngx_sock_ntop

    return NGX_OK;
}


ngx_int_t
ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text)
{
    ngx_uint_t  level;

    /* Winsock may return NGX_ECONNABORTED instead of NGX_ECONNRESET */

    if ((err == NGX_ECONNRESET
        ) && c->log_error == NGX_ERROR_IGNORE_ECONNRESET)
    {
        return 0;
    }


    if (err == 0
        || err == NGX_ECONNRESET
        || err == NGX_EPIPE
        || err == NGX_ENOTCONN
        || err == NGX_ETIMEDOUT
        || err == NGX_ECONNREFUSED
        || err == NGX_ENETDOWN
        || err == NGX_ENETUNREACH
        || err == NGX_EHOSTDOWN
        || err == NGX_EHOSTUNREACH)
    {
        switch (c->log_error) {

        case NGX_ERROR_IGNORE_EINVAL:
        case NGX_ERROR_IGNORE_ECONNRESET:
        case NGX_ERROR_INFO:
            level = NGX_LOG_INFO;
            break;

        default:
            level = NGX_LOG_ERR;
        }

    } else {
        level = NGX_LOG_ALERT;
    }

    ngx_log_error(level, c->log, err, text);

    return NGX_ERROR;
}
