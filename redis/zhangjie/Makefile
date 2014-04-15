CXX = gcc
CXXFLAGS +=-g3 -Wall -Wextra -lrt
LDFLAGS = ../deps/jemalloc/lib/libjemalloc.a -ldl -lpthread
 
REDIS_ROOT =/home/zhangjie/Documents/GitHub/zewen/Ex/redis
 
TARGETS =intset_ex
TARGETS_C_FILE= $(TARGETS).c
 
CLEANUP = rm -f $(TARGETS) *.o
 
all:$(TARGETS)
 
clean:
	$(CLEANUP)
 
CORE_INCS =-I. \
-I$(REDIS_ROOT)/src \
-I/root/Desktop/project/Http/trunk/redis/deps/jemalloc/include \
 
REDIS_SDS =$(REDIS_ROOT)/src/intset.o
REDIS_ZMALLOC =$(REDIS_ROOT)/src/zmalloc.o
REDIS_UTIL=$(REDIS_ROOT)/src/endianconv.o
 
$(TARGETS):$(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(REDIS_SDS) $(REDIS_ZMALLOC) $^ $(LDFLAGS) -o $@
