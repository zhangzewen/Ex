CXX = gcc
CXXFLAGS +=-g3 
LDFLAGS = ../deps/jemalloc/lib/libjemalloc.a -ldl -lpthread
 
REDIS_ROOT =/home/zhangjie/Documents/GitHub/zewen/Ex/redis
 
TARGETS =dict_ex
TARGETS_C_FILE= $(TARGETS).c
 
CLEANUP = rm -f $(TARGETS) *.o
 
all:$(TARGETS)
 
clean:
	$(CLEANUP)
 
CORE_INCS =-I. \
-I$(REDIS_ROOT)/src \
-I/root/Desktop/project/Http/trunk/redis/deps/jemalloc/include \
 
REDIS_SDS =$(REDIS_ROOT)/src/sds.o
REDIS_ZMALLOC =$(REDIS_ROOT)/src/zmalloc.o
REDIS_UTIL=$(REDIS_ROOT)/src/endianconv.o
REDIS_DEBUG=$(REDIS_ROOT)/src/debug.o
REDIS_DICT=$(REDIS_ROOT)/src/dict.o
#REDIS_LIB=$(patsubst %.c, %.o, $(wildcard $(REDIS_ROOT)/src/*.c))
REDIS_LIB=$(wildcard $(REDIS_ROOT)/src/*.o)
 
$(TARGETS):$(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  $(REDIS_LIB) $^ $(LDFLAGS) -o $@
#	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(REDIS_SDS) $(REDIS_UTIL) $(REDIS_ZMALLOC) $(REDIS_DEBUG) $(REDIS_DICT) $^ $(LDFLAGS) -o $@
