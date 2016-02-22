.PHONY: run install tcp

LIB=$(shell pwd)/opt
CFLAGS=-I$(LIB)/include -I$(LIB)/include/nanomsg -O3
LDFLAGS=-lanl -lrt -lpthread
NANOMSG=$(LIB)/lib/libnanomsg.a
LIBMILL=$(LIB)/lib/libmill.a

all: install

install:
	@echo libraries will install now into $(shell pwd)/opt/lib
	sleep 2 && echo ...
	git clone --depth 1 git@github.com:sustrik/libmill.git
	cd libmill && ./autogen.sh && ./configure --prefix=$(LIB) && make && make install
	rm -rf libmill && git clone --depth 1 git@github.com:nanomsg/nanomsg.git
	cd nanomsg && ./autogen.sh && ./configure --prefix=$(LIB) && make && make install
	rm -rf nanomsg

run: inproccoroutines.c
	cc -o coros inproccoroutines.c $(NANOMSG) $(LIBMILL) $(LDFLAGS) $(CFLAGS)
	./coros

tcp: tcp/twocoros.c
	cc -o nntcpcoros tcp/twocoros.c $(NANOMSG) $(LIBMILL) $(LDFLAGS) $(CFLAGS)
	./nntcpcoros
