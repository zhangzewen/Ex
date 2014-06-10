make clean;

	./configure --prefix=/usr/local/nginx \
	--without-http_charset_module \
  --without-http_gzip_module \
  --without-http_ssi_module \
	--without-http_userid_module \
  --without-http_auth_basic_module \
  --without-http_autoindex_module \
  --without-http_geo_module       \
  --without-http_map_module       \
  --without-http_split_clients_module \
  --without-http_referer_module      \
  --without-http_uwsgi_module        \
  --without-http_scgi_module         \
  --without-http_memcached_module \
  --without-http_limit_conn_module \
  --without-http_limit_req_module \
  --without-http_empty_gif_module \
  --without-http_upstream_ip_hash_module  \
  --without-http_upstream_least_conn_module  \
  --without-http_upstream_keepalive_module \
	--with-pcre=/home/zhangjie/Documents/tar/pcre-8.33 \
	--add-module=/home/zhangjie/Documents/GitHub/zewen/Ex/nginx_copy_to_exercise/ngx_modules/ngx_http_write_back_request_module/ \
	--with-debug \

exit 0;
