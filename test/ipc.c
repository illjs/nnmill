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
#include <assert.h>
#include <string.h>

#include <libmill.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pubsub.h>

#include "../nnmill.c"

#define printline() printf("%s - at line: %d\n", __func__, __LINE__)

static coroutine void sndr (int s, const char *msg, chan endch) {
  printline();
  for(int count = 0; count < 5; count++) {
    nm_send (s, msg, strlen(msg), 0, -1);
    printline();
  }

  printline();
  chs(endch, int, 1);
}

static coroutine void rcvr (int s, chan endch) {
  char buf[64];
  int count = 0;

  for(; count < 5; count++) {
    printline();
    printf("rcvr count: %d | s: %d\n", count, s);
    buf[nm_recv (s, buf, 64, 0, -1)] = '\0';
  }

  printf("recevied: %s %dx\n", buf, count);
  chs(endch, int, 1);
  printline();
}

static void cleanup (int *s, int sz) {
  while (sz--)
    nn_close(s[sz]);
}

int main (const int argc, const char **argv) {

  int pr = nn_socket(AF_SP, NN_PAIR);
  int pr2 = nn_socket(AF_SP, NN_PAIR);

  nn_bind(pr, "ipc://pr");
  nn_connect(pr2, "ipc://pr");

  chan prch = chmake(int, 0);

  go(sndr(pr, "ipc pair", prch));
  go(rcvr(pr2, prch));

  chr(prch, int);
  chr(prch, int);

  chclose(prch);

  printf("----------------------------");

  int pub = nn_socket (AF_SP, NN_PUB);
  int sub = nn_socket (AF_SP, NN_SUB);
  assert (nn_setsockopt (sub, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) == 0);

  nn_bind (pub, "ipc://pubsub");
  nn_connect (sub, "ipc://pubsub");
  msleep(now() + 100);

  chan pubsubch = chmake (int, 0);

  printline();
  go(rcvr(sub, pubsubch));
  go(sndr(pub, "ipc pubsub", pubsubch));

  chr(pubsubch, int);
  printline();
  chr(pubsubch, int);
  printline();
  chclose(pubsubch);

  int close[4] = { pr, pr2, pub, sub };
  cleanup(close, 4);

  return 0;
}
