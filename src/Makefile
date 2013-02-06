


DIRS = core http event


all: libhttp_zhangjie.a 

.PHONY: all
libhttp_zhangjie.a:: dep
	for i in $(DIRS); do make -C $$i || exit -1; done;
	find . -name "*.o" -print | xargs -l ar cr libhttp_zhangjie.a

#liblog4c:: log4c-localtime/src/log4c.h
#	cd ./log4c-localtime && ./configure --enable-static --without-expat && make;
#	cp log4c-localtime/src/log4c/.libs/liblog4c.a liblog4c.a


clean:
	rm -fr libhttp_zhangjie.a 
#	cd ./log4c-localtime && make clean;
	for i in $(DIRS); do make -C $$i $@ || exit -1; done;

distclean:
	rm -fr libhttp_zhangjie.a 
#	cd ./log4c-localtime && make distclean;
	for i in $(DIRS); do make -C $$i $@ || exit -1; done;

dep depend:
	for i in $(DIRS); do make -C $$i $@ || exit -1; done;
