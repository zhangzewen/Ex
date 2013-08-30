/*
* * Copyriht (C) Zhang Zewen
* *
*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	unsigned int enable;
}ngx_http_write_back_request_loc_conf_t;

static ngx_command_t ngx_http_write_back_request_commands[] = {
	{
		ngx_string("write_back_request"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		NULL,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_write_back_request_loc_conf_t, enable),
		NULL
	},
	ngx_null_command
};


static ngx_http_module_t ngx_http_write_back_request_module_ctx = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ngx_http_write_back_request_loc_conf,
	ngx_http_write_back_request_merge_loc_conf
};

ngx_module_t ngx_http_write_back_request_module = {
	NGX_MODULE_V1,
	&ngx_http_write_back_request_module_ctx,
	ngx_http_write_back_request_commands,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
};


static ngx_int_t ngx_http_write_back_request_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_write_back_request_loc_conf_t *wbrcf;
	wbrcf = ngx_http_get_module_loc_conf(r, ngx_http_write_back_request_module);

	if(!(r->method & (NGX_HTTP_HEAD | NGX_HTTP_GET| NGX_HTTP_POST))) {
		return NGX_HTTP_NOT_ALLOWED;
	}

	r->headers_out.content_type.len = sizeof("text/html") - 1;
	r->headers_out.content_type.data = (u_char *)"text/html";
	r->headers_out.status = NGX_HTTP_OK;
	
	if(r->method == NGX_HTTP_HEAD)
	{
		rc = ngx_http_send_header(r);
		if(rc != NGX_OK)
		{
			return rc;
		}
	}

	b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
	
	if(b == NULL)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	if(wbrlcf->enable == 0) {
		return ngx_http_output_filter(r, &out);
	}

	
}
