CFLAGS = -g -Wall -Werror -I ../core/


ifneq ($(ACTION), release)
CFLAGS += -DDEBUG
endif

_OBJS = $(shell ls *.c)
OBJS = $(_OBJS:.c=.o)


all: $(OBJS)

-include .depend

%.o: %.c
	cc $(CFLAGS) -o $@ -c $<


clean:
	rm -fr $(OBJS) 

distclean: clean
	rm -fr .depend

dep depend:
	cc $(CFLAGS) -M -MM $(_OBJS) > .depend
