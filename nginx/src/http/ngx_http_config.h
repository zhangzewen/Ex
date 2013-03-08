
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CONFIG_H_INCLUDED_
#define _NGX_HTTP_CONFIG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    void        **main_conf;
    void        **srv_conf;
    void        **loc_conf;
} ngx_http_conf_ctx_t;


typedef struct {
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);//在创建和读取该模块的配置信息之前被调用
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);//在创建和读取该模块的配置信息之后被调用

    void       *(*create_main_conf)(ngx_conf_t *cf);//调用该函数创建本模块位于http block 的配置信息存储结构。该函数成功的时候，返回创建的配置对象。失败的话，返回NULL
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);//调用该函数初始化本模块位于http block的配置信息存储结构，该函数成功的时候，返回NGX_CONF_OK。失败的话，返回NGX_CONF_ERROR或者错误字符串

    void       *(*create_srv_conf)(ngx_conf_t *cf);//调用该函数创建本模块位于http block的配置信息存储结构，该函数成功的时候，返回创建的配置对象，失败的话，返回NULL
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);//因为有些配置指令可以出现在http block，也可以出现在http server block中，在遇到此种情况，每个server都会有自己的存储机构来存储该server的配置，但是在这种情况下当在http block中的配置与server block中的配置信息冲突的时候，就需要调用此函数进行合并，该函数可以不必须提供，但安全起见，建议提供，该函数成功的时候，返回NGX_CONF_OK,失败返回NGX_CONF_ERROR或者错误字符串

    void       *(*create_loc_conf)(ngx_conf_t *cf);//同 void *(*create_srv_conf)(ngx_conf_t *cf)的作用相同，只是创建模块位于location block的配置信息存储结构
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf); //合并函数
} ngx_http_module_t;


#define NGX_HTTP_MODULE           0x50545448   /* "HTTP" */

#define NGX_HTTP_MAIN_CONF        0x02000000
#define NGX_HTTP_SRV_CONF         0x04000000
#define NGX_HTTP_LOC_CONF         0x08000000
#define NGX_HTTP_UPS_CONF         0x10000000
#define NGX_HTTP_SIF_CONF         0x20000000
#define NGX_HTTP_LIF_CONF         0x40000000
#define NGX_HTTP_LMT_CONF         0x80000000


#define NGX_HTTP_MAIN_CONF_OFFSET  offsetof(ngx_http_conf_ctx_t, main_conf)
#define NGX_HTTP_SRV_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, srv_conf)
#define NGX_HTTP_LOC_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, loc_conf)


#define ngx_http_get_module_main_conf(r, module)                             \
    (r)->main_conf[module.ctx_index]
#define ngx_http_get_module_srv_conf(r, module)  (r)->srv_conf[module.ctx_index]
#define ngx_http_get_module_loc_conf(r, module)  (r)->loc_conf[module.ctx_index]


#define ngx_http_conf_get_module_main_conf(cf, module)                        \
    ((ngx_http_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_http_conf_get_module_srv_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_http_conf_get_module_loc_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->loc_conf[module.ctx_index]

#define ngx_http_cycle_get_module_main_conf(cycle, module)                    \
    (cycle->conf_ctx[ngx_http_module.index] ?                                 \
        ((ngx_http_conf_ctx_t *) cycle->conf_ctx[ngx_http_module.index])      \
            ->main_conf[module.ctx_index]:                                    \
        NULL)


#endif /* _NGX_HTTP_CONFIG_H_INCLUDED_ */
