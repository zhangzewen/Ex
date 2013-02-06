


DIRS = core http event


all: libhttp_zhangjie.a 

.PHONY: all
libhttp_zhangjie.a:: dep
	for i in $(DIRS); do make -C $$i || exit -1; done;
	find . -name "*.o" -print | xargs -l ar cr libhttp_zhangjie.a


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
