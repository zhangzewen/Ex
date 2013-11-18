CXX = gcc
CXXFLAGS +=-g3 -Wall -Wextra -lrt
 
REDIS_ROOT =/root/tar/redis-2.6.16/src
 
TARGETS =sting_ex
TARGETS_C_FILE= $(TARGETS).c
 
CLEANUP = rm -f $(TARGETS) *.o
 
all:$(TARGETS)
 
clean:
	$(CLEANUP)
 
CORE_INCS =-I. \
-I$(REDIS_ROOT)/ \
-I$(REDIS_ROOT)/compat/sys \
 
REDIS_SDS =$(REDIS_ROOT)/sds.o
REDIS_ZMALLOC =$(REDIS_ROOT)/zmalloc.o
 
$(TARGETS):$(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(REDIS_SDS) $(REDIS_ZMALLOC) $^ -o $@
