
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CACHE_H_INCLUDED_
#define _NGX_HTTP_CACHE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#define NGX_HTTP_CACHE_MISS          1
#define NGX_HTTP_CACHE_BYPASS        2
#define NGX_HTTP_CACHE_EXPIRED       3
#define NGX_HTTP_CACHE_STALE         4
#define NGX_HTTP_CACHE_UPDATING      5
#define NGX_HTTP_CACHE_HIT           6
#define NGX_HTTP_CACHE_SCARCE        7

#define NGX_HTTP_CACHE_KEY_LEN       16


typedef struct {
    ngx_uint_t                       status;
    time_t                           valid;
} ngx_http_cache_valid_t;


typedef struct {
    ngx_rbtree_node_t                node;
    ngx_queue_t                      queue;

    u_char                           key[NGX_HTTP_CACHE_KEY_LEN
                                         - sizeof(ngx_rbtree_key_t)];
//引用计数
    unsigned                         count:20;
//多少请求在使用
    unsigned                         uses:10;
    unsigned                         valid_msec:10;
//cache的状态
    unsigned                         error:10;
//是存在对应的cache文件
    unsigned                         exists:1;
//是否正在更新
    unsigned                         updating:1;
//是否正在删除
    unsigned                         deleting:1;
                                     /* 11 unused bits */
//文件的uniq
    ngx_file_uniq_t                  uniq;
//cache的失效时间
    time_t                           expire;
//比如cache control中的max-age
    time_t                           valid_sec;
//其实应该是body的大小
    size_t                           body_start;
//文件大小
    off_t                            fs_size;
} ngx_http_file_cache_node_t;


struct ngx_http_cache_s {
    ngx_file_t                       file;
    ngx_array_t                      keys;
    uint32_t                         crc32;
    u_char                           key[NGX_HTTP_CACHE_KEY_LEN];

    ngx_file_uniq_t                  uniq;
    time_t                           valid_sec;
    time_t                           last_modified;
    time_t                           date;

    size_t                           header_start;
    size_t                           body_start;
    off_t                            length;
    off_t                            fs_size;

    ngx_uint_t                       min_uses;
    ngx_uint_t                       error;
    ngx_uint_t                       valid_msec;

    ngx_buf_t                       *buf;

    ngx_http_file_cache_t           *file_cache;
    ngx_http_file_cache_node_t      *node;

    ngx_msec_t                       lock_timeout;
    ngx_msec_t                       wait_time;

    ngx_event_t                      wait_event;

    unsigned                         lock:1;
    unsigned                         waiting:1;

    unsigned                         updated:1;
    unsigned                         updating:1;
    unsigned                         exists:1;
    unsigned                         temp_file:1;
};


typedef struct {
    time_t                           valid_sec;
    time_t                           last_modified;
    time_t                           date;
    uint32_t                         crc32;
    u_short                          valid_msec;
    u_short                          header_start;
    u_short                          body_start;
} ngx_http_file_cache_header_t;


typedef struct {
    ngx_rbtree_t                     rbtree;
    ngx_rbtree_node_t                sentinel;
    ngx_queue_t                      queue;
//cold表示这个cache是否已经被loader进程load过了
    ngx_atomic_t                     cold;
//那个进程正在load这个cache
    ngx_atomic_t                     loading;
//文件的大小
    off_t                            size;
} ngx_http_file_cache_sh_t;


struct ngx_http_file_cache_s {
    ngx_http_file_cache_sh_t        *sh;
    ngx_slab_pool_t                 *shpool;
//cache的目录
    ngx_path_t                      *path;
//当前path下的所有cache文件的最大值
    off_t                            max_size;
    size_t                           bsize;
//如果多久不使用就删除
    time_t                           inactive;
//当前有多少个cache（超过loader_files之后就会被清0）
    ngx_uint_t                       files;
//这个值也就是一个阀值，当load的文件个树大于这个值之后，load进程就会短暂的休眠（loader_sleep）
    ngx_uint_t                       loader_files;
//最后被manage或者loader访问的时间
    ngx_msec_t                       last;
//和上面的loader_files配合使用，当文件个数大于loader_files，就会休眠
    ngx_msec_t                       loader_sleep;
//配合上面的last，也就是loader便利的休眠间隔
    ngx_msec_t                       loader_threshold;
//共享内存的地址
    ngx_shm_zone_t                  *shm_zone;
};


ngx_int_t ngx_http_file_cache_new(ngx_http_request_t *r);
ngx_int_t ngx_http_file_cache_create(ngx_http_request_t *r);
void ngx_http_file_cache_create_key(ngx_http_request_t *r);
ngx_int_t ngx_http_file_cache_open(ngx_http_request_t *r);
void ngx_http_file_cache_set_header(ngx_http_request_t *r, u_char *buf);
void ngx_http_file_cache_update(ngx_http_request_t *r, ngx_temp_file_t *tf);
ngx_int_t ngx_http_cache_send(ngx_http_request_t *);
void ngx_http_file_cache_free(ngx_http_cache_t *c, ngx_temp_file_t *tf);
time_t ngx_http_file_cache_valid(ngx_array_t *cache_valid, ngx_uint_t status);

char *ngx_http_file_cache_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_http_file_cache_valid_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


extern ngx_str_t  ngx_http_cache_status[];


#endif /* _NGX_HTTP_CACHE_H_INCLUDED_ */
