#include <stdio.h>
#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_palloc.h"
#include "ngx_http_request.h"
#include "ngx_http.h"
#include "ngx_buf.h"

int main()
{
	
	char buff[] = "GET / HTTP/1.1\r\nHost: 192.168.10.65\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:19.0) Gecko/20100101 Firefox/19.0\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: zh-cn,en-us;q=0.8,en;q=0.5,zh;q=0.3\r\n\r\n";
	ngx_http_request_t *my_request;
	ngx_pool_t *my_pool;
	ngx_buf_t *my_buf;

	my_pool = ngx_create_pool(1024, NULL);
	my_request = ngx_pcalloc(my_pool, sizeof(ngx_http_request_t));

	if (NULL == my_request)
		exit(1);

	my_buf = ngx_create_temp_buf(my_pool, 512);	
	
	if (NULL == my_buf)
		exit(1);

	strcpy(my_buf->pos, buff);
	my_buf->last = my_buf + strlen(buff);
	
	my_buf->start = my_buf->pos;

	ngx_http_parse_request_line(my_request, my_buf);
	
	return 0;
}

