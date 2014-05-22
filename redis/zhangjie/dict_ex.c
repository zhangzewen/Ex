#include <stdio.h>
#include <stdlib.h>
#include "dict.h"
#include "sds.h"

#if 0
static unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len) {
    unsigned int hash = (unsigned int)dict_hash_function_seed;

    while (len--)
        hash = ((hash << 5) + hash) + (tolower(*buf++)); /* hash * 33 + c */
    return hash;
}
#endif


static unsigned int dictSdsCaseHash(const void *key) {
    return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}

static int dictSdsKeyCaseCompare(void *privdata, const void *key1,
        const void *key2)
{
    DICT_NOTUSED(privdata);

    return strcasecmp(key1, key2) == 0;
}




dictType strTableDictType = {
	dictSdsCaseHash,
	NULL,
	NULL,
	dictSdsKeyCaseCompare,
	NULL
};


int main(int argc, char *argv[])
{
	sds ex_1 = sdsnewlen("zhangjie", 8);
	sds ex_2 = sdsnewlen("zhbngjie", 8);
	sds ex_3 = sdsnewlen("zhcngjie", 8);
	sds ex_4 = sdsnewlen("zhengjie", 8);
	sds ex_5 = sdsnewlen("zh3ngjie", 8);
	sds ex_6 = sdsnewlen("zhrngjie", 8);
	sds ex_7 = sdsnewlen("zhfngjie", 8);
	sds ex_8 = sdsnewlen("zhgngjie", 8);
	sds ex_9 = sdsnewlen("zhingjie", 8);
	sds ex_10= sdsnewlen("zhajgjie", 8);
	sds ex_11= sdsnewlen("zhkngjie", 8);
	sds ex_12= sdsnewlen("zhjngjie", 8);
	sds ex_13= sdsnewlen("zhakgjie", 8);
	sds ex_14= sdsnewlen("zhtngjie", 8);
	sds ex_15= sdsnewlen("zhvngjie", 8);
	sds ex_16= sdsnewlen("zhyngjie", 8);
	sds ex_17= sdsnewlen("zhaqgjie", 8);
	sds ex_18= sdsnewlen("shangjie", 8);
	sds ex_19= sdsnewlen("zdangjie", 8);
	sds ex_20= sdsnewlen("shangjie", 8);
	sds ex_21= sdsnewlen("zhfngjie", 8);

	sds v_1 = sdsnewlen("zhangjie", 8);
	sds v_2 = sdsnewlen("zhangjie", 8);
	sds v_3 = sdsnewlen("zhangjie", 8);
	sds v_4 = sdsnewlen("zhangjie", 8);
	sds v_5 = sdsnewlen("zhangjie", 8);
	sds v_6 = sdsnewlen("zhangjie", 8);
	sds v_7 = sdsnewlen("zhangjie", 8);
	sds v_8 = sdsnewlen("zhangjie", 8);
	sds v_9 = sdsnewlen("zhangjie", 8);
	sds v_10= sdsnewlen("zhangjie", 8);
	sds v_11= sdsnewlen("zhangjie", 8);
	sds v_12= sdsnewlen("zhangjie", 8);
	sds v_13= sdsnewlen("zhangjie", 8);
	sds v_14= sdsnewlen("zhangjie", 8);
	sds v_15= sdsnewlen("zhangjie", 8);
	sds v_16= sdsnewlen("zhangjie", 8);
	sds v_17= sdsnewlen("zhangjie", 8);
	sds v_18= sdsnewlen("zhangjie", 8);
	sds v_19= sdsnewlen("zhangjie", 8);
	sds v_20= sdsnewlen("zhangjie", 8);
	sds v_21= sdsnewlen("zhangjie", 8);

	dict *d = dictCreate(&strTableDictType, NULL);
	dictAdd(d, (void *)ex_1, (void *)v_1);
	dictAdd(d, (void *)ex_2, (void *)v_2);
	dictAdd(d, (void *)ex_3, (void *)v_3);
	dictAdd(d, (void *)ex_4, (void *)v_4);
	dictAdd(d, (void *)ex_5, (void *)v_5);
	dictAdd(d, (void *)ex_6, (void *)v_6);
	dictAdd(d, (void *)ex_7, (void *)v_7);
	dictAdd(d, (void *)ex_8, (void *)v_8);
	dictAdd(d, (void *)ex_9, (void *)v_9);
	dictAdd(d, (void *)ex_10,(void *)v_10);
	dictAdd(d, (void *)ex_11,(void *)v_11);
	dictAdd(d, (void *)ex_12,(void *)v_12);
	dictAdd(d, (void *)ex_13,(void *)v_13);
	dictAdd(d, (void *)ex_14,(void *)v_14);
	dictAdd(d, (void *)ex_15,(void *)v_15);
	dictAdd(d, (void *)ex_16,(void *)v_16);
	dictAdd(d, (void *)ex_17,(void *)v_17);
	dictAdd(d, (void *)ex_18,(void *)v_18);
	dictAdd(d, (void *)ex_19,(void *)v_19);
	dictAdd(d, (void *)ex_20,(void *)v_20);
	dictAdd(d, (void *)ex_21,(void *)v_21);
	
		
	return 0;
}
