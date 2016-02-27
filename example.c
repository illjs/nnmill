/*

  Copyright (c) 2016 Fatih Kaya
  Copyright (c) 2016 Bent Cardan
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
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include "nnmill.c"

static coroutine void sender(int s) {
  int rc;
  for(;;) {
    rc = nm_send (s, "test", 4, 0, -1);

    if (rc != 4)
      break; // TODO: print error

    msleep(now() + 1000);
  }
}

static coroutine void receiver(int s) {
  char buf[5];
  int rc;

  buf[4] = '\0';

  for(;;) {
    rc = nm_recv (s, buf, 4, 0, -1);

    if (rc != 4)
      break; // TODO: print error

    printf("recevied: %s\n", buf);
  }
}

int main(int argc, char *argv[]) {
  int s = nn_socket(AF_SP, NN_PAIR);
  int s2 = nn_socket(AF_SP, NN_PAIR);
  int rc;
  chan endch = chmake(int, 1);

  nn_bind(s, "tcp://127.0.0.1:7458");
  nn_connect(s2, "tcp://127.0.0.1:7458");

  go(sender(s));
  go(receiver(s2));

  chr(endch, int);

  return 0;
}
