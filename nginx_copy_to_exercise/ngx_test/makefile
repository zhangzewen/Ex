#!/bin/bash

#
# use this makefile, we should not include any module when config nginx.
# and we have to compile the nginx before starting compile this demo.
# because of the dependence.
# we just use nginx to write demo programs.
#

CC=gcc

CFLAGS=-O -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -Werror -g 

INCLUDEPATH=-I. -I../../../objs/ \
                        -I../../os/unix/ \
                        -I../../core/ \
                        -I../../event/ \
                        -I../../os/ 

OBJPATH=../../../objs/*.o \
           ../../../objs/src/os/unix/*.o \
           ../../../objs/src/core/*.o \
           ../../../objs/src/event/*.o \
           ../../../objs/src/event/modules/*.o \
           ../../../objs/src/http/*.o \
           ../../../objs/src/http/modules/*.o 

LFLAGS= -lstdc++ -lpthread -ldl -rdynamic -lcrypt -lpthread -lpcre -lcrypto -lcrypto -lz

SOURCE=$(wildcard *.c)
DIRECTORIES=$(notdir $(SOURCE))
OBJECTS=$(patsubst %.c,%.o,$(DIRECTORIES) )
TARGETS=$(patsubst %.c,%,$(DIRECTORIES) )

all: $(TARGETS)

$(OBJECTS):%.o:%.c
        $(CC) -c $(CFLAGS) $(INCLUDEPATH) $< -o $@

$(TARGETS):$(OBJECTS)
        $(CC) $(CFLAGS) $(INCLUDES) $(OBJPATH) $(LFLAGS) -o $@ 

.PHONEY:clean

clean:
	rm -f $(TARGETS) $(OBJECTS) *~
