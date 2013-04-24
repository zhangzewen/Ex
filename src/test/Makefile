

CFLAGS ?= -g -Wall -Werror

CFLAGS += -I ../../smartcommon -I /usr/include/libxml2 -I../../smartcommon/ -I../../smartlog
LDFLAGS = -lxml2 -lssl -lcrypto -lpthread -llog4c

VPATH := ../../smartcommon/

ifneq ($(ACTION), release)
CFLAGS += -DDEBUG
endif


ifdef VERSION
CFLAGS += -DVERSION=\"$(VERSION)\"
endif

_OBJS = $(shell ls *.c)
OBJS = $(_OBJS:.c=.o)


all: daemon4

daemon4: $(OBJS) ../../smartcommon/libcomm.a  ../../smartlog/liblog4c_user.a
	cc $(LDFLAGS) -o $@ $^

-include .depend

%.o: %.c
	cc $(CFLAGS) -o $@ -c $<

../../smartcommon/libcomm.a:
	make -C ../../smartcommon ACTION=$(ACTION) VERSION=$(VERSION)

clean:
	rm -fr $(OBJS) daemon4

distclean: clean
	rm -fr .depend

dep depend:
	cc $(CFLAGS) -M -MM $(_OBJS) > .depend
