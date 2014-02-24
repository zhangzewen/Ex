#include "http_resolver.h"
#include <stdio.h>
#include <stdlib.h>


resolver_init();
resolve_name();
resolver_distory();
static name_find_cache();//先在rbtree中查找，若有且没有过期立即返回，否则通过dns查询，并把查询的结果插入到rbtree中去！
