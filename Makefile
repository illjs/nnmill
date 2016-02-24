#
# Copyright (c) 2016 Bent Cardan
# Copyright (c) 2016 Fatih Kaya
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

.PHONY: run install check test

LIB=$(shell pwd)/opt
CFLAGS=-I$(LIB)/include -I$(LIB)/include/nanomsg -O3
LDFLAGS=-lanl -lrt -lpthread
NANOMSG=$(LIB)/lib/libnanomsg.a
LIBMILL=$(LIB)/lib/libmill.a

all: install

install:
	@echo libraries will install now into $(shell pwd)/opt/lib
	sleep 2; rm -rf opt
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
