CXX = gcc
CXXFLAGS +=-g -Wall -Wextra
 
NGX_ROOT =/root/tar/nginx-1.2.6
 
TARGETS =ngx_request_parse_test
TARGETS_C_FILE= $(TARGETS).c
 
CLEANUP = rm -f $(TARGETS) *.o
 
all:$(TARGETS)
 
clean:
	$(CLEANUP)
 
CORE_INCS =-I. \
-I$(NGX_ROOT)/src/core \
-I$(NGX_ROOT)/src/event \
-I$(NGX_ROOT)/src/event/modules \
-I$(NGX_ROOT)/src/os/unix \
-I$(NGX_ROOT)/objs \
 
NGX_PALLOC =$(NGX_ROOT)/objs/src/core/ngx_palloc.o
NGX_ALLOC =$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o
NGX_PARSE =$(NGX_ROOT)/objs/src/http/ngx_http_parse.o 
NGX_BUFF =$(NGX_ROOT)/objs/src/core/ngx_buf.o
NGX_REQUEST =$(NGX_ROOT)/objs/src/http/ngx_http_request.o
$(TARGETS):$(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(NGX_PALLOC) $(NGX_ALLOC) $(NGX_PARSE) $(NGX_BUFF) $(NGX_REQUEST)  $^ -o $@

