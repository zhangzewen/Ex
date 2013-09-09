/*
* * Copyriht (C) Zhang Zewen
* *
*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_http_write_back_request_handler(ngx_http_request_t *r);
static ngx_int_t write_back_request_init(ngx_conf_t *cf);
static void *ngx_http_write_back_request_create_loc_conf(ngx_conf_t *cf);

typedef struct {
	unsigned int enable;
}ngx_http_write_back_request_loc_conf_t;

static ngx_command_t ngx_http_write_back_request_commands[] = {
	{
		ngx_string("write_back_request"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_write_back_request_loc_conf_t, enable),
		NULL
	},
	ngx_null_command
};


static ngx_http_module_t ngx_http_write_back_request_module_ctx = {
	NULL,
	write_back_request_init,	
	NULL,
	NULL,
	NULL,
	NULL,
	ngx_http_write_back_request_create_loc_conf,
	NULL
};

ngx_module_t ngx_http_write_back_request_module = {
	NGX_MODULE_V1,
	&ngx_http_write_back_request_module_ctx,
	ngx_http_write_back_request_commands,
	NGX_HTTP_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
};

static ngx_int_t write_back_request_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt *h;
	ngx_http_core_main_conf_t *cmcf;


	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

	h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
		
	if(NULL == h) {
		return NGX_ERROR;
	}

	*h = ngx_http_write_back_request_handler;
	
	return NGX_OK;
	
}

static ngx_int_t ngx_http_write_back_request_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t *out = NULL;
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
	out->buf = b;
	out->next = NULL;
	if(wbrcf->enable != 0) {
		b->pos = r->request_start;
		b->last = r->request_end;	
		b->memory = 1;
		b->last_buf = 1;
	}

	rc = ngx_http_send_header(r);

	if(rc != NGX_ERROR) {
		return rc;
	}


	return ngx_http_output_filter(r, out);
	
}


static void *ngx_http_write_back_request_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_write_back_request_loc_conf_t *conf;
	conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_write_back_request_loc_conf_t));

	if( conf == NULL) {
		return NGX_CONF_ERROR;
	}

	conf->enable = 0;
	
	return conf;
}



