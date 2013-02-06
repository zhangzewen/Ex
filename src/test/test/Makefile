

CFLAGS += -g -Wall -Werror




_OBJS = $(shell ls *.c)
OBJS = $(_OBJS:.c=.o)


all: daemon

daemon: $(OBJS) 
	cc -g -o $@ $^

-include .depend

%.o: %.c
	cc $(CFLAGS) -o $@ -c $<


clean:
	rm -fr $(OBJS) daemon

distclean: clean
	rm -fr .depend

dep depend:
	cc $(CFLAGS) -M -MM $(_OBJS) > .depend
