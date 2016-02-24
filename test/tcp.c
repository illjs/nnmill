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
#include <string.h>

#include "libmill.h"
#include "nn.h"
#include "pair.h"
#include "tcp.h"

coroutine void sender (int s, chan ch) {
  int i = 26;
  char msg[256];
  char a[] = "I am sending #";
  char b[] = " msg now";

  while (i--) {
    snprintf(msg, sizeof msg, "%s%d%s", a, i, b);
    nn_send (s, msg, strlen(msg), 0);
  }
}

coroutine void recvr (int s, chan ch) {
  int i = 26, rc;
  while (i) {
    char buf[256] = { 0 };
    rc = nn_recv (s, &buf, 256, 0);
    printf("got %d bytes: %s\n", rc, buf); i--;
  }
  printf("\n\nrecvr coroutine collected 25 unique msgs, of %d bytes\n\n", rc);
}

int main (const int argc, const char **argv) {
  int deva = nn_socket (AF_SP_RAW, NN_PAIR);
  int devb = nn_socket (AF_SP_RAW, NN_PAIR);

  // why does setting up this channel make it happen? (try commenting it out)
  chan ch = chmake(int, 0);

  nn_bind (deva,    "tcp://127.0.0.1:5555");
  nn_connect (devb, "tcp://127.0.0.1:5555");
  nn_sleep (50);

  go(sender(deva, ch));
  go(recvr(devb, ch));

  return 0;
}
