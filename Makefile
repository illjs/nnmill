.PHONY: run install check test

LIB=$(shell pwd)/opt
CFLAGS=-I$(LIB)/include -I$(LIB)/include/nanomsg -O3
LDFLAGS=-lanl -lrt -lpthread
NANOMSG=$(LIB)/lib/libnanomsg.a
LIBMILL=$(LIB)/lib/libmill.a

all: install

install:
	@echo libraries will install now into $(shell pwd)/opt/lib
	sleep 2; ...
	git clone --depth 1 git@github.com:sustrik/libmill.git
	cd libmill && ./autogen.sh && ./configure --prefix=$(LIB) && make && make install
	rm -rf libmill && git clone --depth 1 git@github.com:nanomsg/nanomsg.git
	cd nanomsg && ./autogen.sh && ./configure --prefix=$(LIB) && make && make install
	rm -rf nanomsg

check:
	cc -o inproc test/inproc.c $(NANOMSG) $(LIBMILL) $(LDFLAGS) $(CFLAGS)
	cc -o ipc test/ipc.c $(NANOMSG) $(LIBMILL) $(LDFLAGS) $(CFLAGS)
	cc -o tcp test/tcp.c $(NANOMSG) $(LIBMILL) $(LDFLAGS) $(CFLAGS)
	./inproc && ./ipc && ./tcp && rm da inproc ipc tcp
	@echo verified consistent behavior among common transports

run: example.c
	cc -o example example.c $(CFLAGS) $(NANOMSG) $(LIBMILL) $(LDFLAGS)
	./example

test: check
