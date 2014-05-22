CXX = gcc
CXXFLAGS +=-g3 
LDFLAGS = ../deps/jemalloc/lib/libjemalloc.a -ldl -lpthread -lm
 
REDIS_ROOT =/home/zhangjie/Documents/GitHub/zewen/Ex/redis
 
TARGETS =dict_ex
TARGETS_C_FILE= $(TARGETS).c
 
CLEANUP = rm -f $(TARGETS) *.o
 
all:$(TARGETS)
 
 
CORE_INCS =-I. \
-I$(REDIS_ROOT)/src \
-I/root/Desktop/project/Http/trunk/redis/deps/jemalloc/include \
-I../deps/hiredis -I../deps/linenoise -I../deps/lua/src
 
REDIS_SDS =$(REDIS_ROOT)/src/sds.o
REDIS_ZMALLOC =$(REDIS_ROOT)/src/zmalloc.o
REDIS_UTIL=$(REDIS_ROOT)/src/endianconv.o
REDIS_DEBUG=$(REDIS_ROOT)/src/debug.o
REDIS_DICT=$(REDIS_ROOT)/src/dict.o
#REDIS_LIB=$(patsubst %.c, %.o, $(wildcard $(REDIS_ROOT)/src/*.c))
LIB= adlist.o scripting.o ae.o redis.o anet.o dict.o  sds.o zmalloc.o lzf_c.o lzf_d.o pqsort.o zipmap.o sha1.o ziplist.o release.o networking.o util.o object.o db.o replication.o rdb.o t_string.o t_list.o t_set.o t_zset.o t_hash.o config.o aof.o pubsub.o multi.o debug.o sort.o intset.o syncio.o migrate.o endianconv.o slowlog.o  bio.o rio.o rand.o memtest.o crc64.o bitops.o sentinel.o
REDIS_LIB=$(addprefix ../src/, $(LIB)) 
$(TARGETS):$(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  $(REDIS_LIB) $^ $(LDFLAGS) ../deps/hiredis/libhiredis.a ../deps/lua/src/liblua.a -o $@
#	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(REDIS_SDS) $(REDIS_UTIL) $(REDIS_ZMALLOC) $(REDIS_DEBUG) $(REDIS_DICT) $^ $(LDFLAGS) -o $@
#
clean:
	rm -fr *.o
	rm -fr $(TARGETS)
	rm -fr core.*
