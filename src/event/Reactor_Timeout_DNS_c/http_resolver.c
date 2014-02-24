#include "http_resolver.h"
#include <stdio.h>
#include <stdlib.h>


void resolver_init(struct resolver_st *resolver);
struct resolve_result *resolve_name(struct resolver_st *resolver, const char *host);
void resolver_distory(struct resolver_st *resolver);
static name_find_cache(strcut rbtree_st *root, const char *host);//先在rbtree中查找，若有且没有过期立即返回，否则通过dns查询，并把查询的结果插入到rbtree中去！
