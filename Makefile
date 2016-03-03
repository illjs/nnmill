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

.PHONY: run install check test segfault

LIB=$(shell pwd)/opt
nanomsg=-Wno-implicit-function-declaration $(LIB)/lib/libnanomsg.a
libmill=$(LIB)/lib/libmill.a
sodium=$(LIB)/lib/libsodium.a
includes=-I$(LIB)/include -I$(LIB)/include/nanomsg -std=gnu99
clone=git clone --depth 1 https://github.com/
args=--disable-shared --prefix=$(LIB)
build=./autogen.sh && ./configure $(args) && make -j 8 && make install

ifeq ($(shell uname -s), Darwin)
  flags=$(nanomsg) $(libmill) $(sodium) $(includes)
else
  flags=$(sodium) $(nanomsg) $(libmill) -lanl -lrt -lpthread $(includes) -fvisibility=hidden -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -D_GNU_SOURCE -O3
endif


all: install

install:
	@echo libraries will install now into $(shell pwd)/opt/lib
	sleep 2; rm -rf opt build; mkdir build
	cd build; $(clone)sustrik/libmill.git && $(clone)nanomsg/nanomsg.git && $(clone)jedisct1/libsodium
	cd build/libmill && $(build) && cd ../nanomsg && $(build) && cd ../libsodium && $(build)

check:
	cc -o inproc test/inproc.c $(flags)
	cc -o ipc test/ipc.c $(flags)
	cc -o tcp test/tcp.c $(flags)
	cc -o ch test/ch.c $(flags)
	cc -o so test/sodium.c $(flags)
	./inproc && ./ipc && ./tcp && ./ch && ./so && rm inproc ipc tcp ch so
	@echo verified consistent behavior among common transports

so:
	cc -o so test/sodium.c $(flags)
	./so && rm so

run: example.c
	cc -o example example.c $(flags)
	./example

segfault: segfault
	cc -o segfault segfault.c
	./segfault

test: check
