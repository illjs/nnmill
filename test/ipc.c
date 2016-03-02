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
#include <libmill.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pubsub.h>
#include "../nnmill.c"

static coroutine void sndr (int s, const char *msg) {
  for(;;)
    nm_send (s, msg, strlen(msg), 0, -1);
}

static coroutine void rcvr (int s, chan c) {
  int i = 0;
  char buf[64];
  for(;;) {
    buf[nm_recv (s, buf, 64, 0, -1)] = '\0';
    if (++i == 5){
      printf("recevied: %s %dx\n", buf, i);
      chs(c, int, 0);
      chclose(c);
    }
  }
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

  go(sndr(pr, "ipc pair"));
  go(rcvr(pr2, prch));

  chr(prch, int);

  int pub = nn_socket (AF_SP, NN_PUB);
  int sub = nn_socket (AF_SP, NN_SUB);
  assert (nn_setsockopt (sub, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) == 0);

  nn_bind (pub, "ipc://pubsub");
  nn_connect (sub, "ipc://pubsub");

  chan pubsubch = chmake (int, 0);

  go(rcvr(sub, pubsubch));
  go(sndr(pub, "ipc pubsub"));

  chr(pubsubch, int);

  int close[4] = { pr, pr2, pub, sub };
  cleanup(close, 4);

  return 0;
}
