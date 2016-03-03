/*

  Copyright (c) 2016 Bent Cardan
  Copyright (c) 2016 Fatih Kaya
  Copyright (c) 2016 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <stdio.h>
#include <libmill.h>

static coroutine void c1 (int i,  chan o) { chs(o, int, ++i);           }
static coroutine void c2 (chan i, chan o) { chs(o, int, ++chr(i, int)); }
static coroutine void c3 (chan i, chan o) { chs(o, int, ++chr(i, int)); }
static coroutine void c4 (chan i, chan o) { chs(o, int, ++chr(i, int)); }
static coroutine void c5 (chan i, chan o) { chs(o, int, ++chr(i, int)); }
static coroutine void c6 (chan i, chan o) { chs(o, int, ++chr(i, int)); }

int main (const int argc, const char **argv) {

  chan ch   = chmake(int, 0);
  chan ch2  = chmake(int, 0);
  chan ch3  = chmake(int, 0);
  chan ch4  = chmake(int, 0);
  chan ch5  = chmake(int, 0);
  chan ch6  = chmake(int, 0);

  go( c1( 1,    ch)  );
  go( c2( ch,   ch2) );
  go( c3( ch2,  ch3) );
  go( c4( ch3,  ch4) );
  go( c5( ch4,  ch5) );
  go( c6( ch5,  ch6) );

  printf("incremented to: %d\n", chr(ch6, int)); // incremented to 7

  return 0;
}
