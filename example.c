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
#include <stdlib.h>
#include <stdbool.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <libmill.h>

struct nn_mill_msg {
  int size;
  char *msg;
};

static int nn_mill_getfd (int s) {
  int rc, fd;
  size_t fdsz = sizeof fd;

  if ( nn_getsockopt (s, NN_SOL_SOCKET, NN_RCVFD, &fd, &fdsz) != 0 )
    return -1;

  /* TODO: we might as well return both FDs, NN_SNDFD too */
  return fd;
}

static coroutine void receiver(int s, chan inch, int *stopfdwait) {
  int fd = nn_mill_getfd(s);
  struct nn_pollfd pfd[1];
  int rc;

  for(;;) {
    int events = fdwait(fd, FDW_IN, now() + 200);
    if (*stopfdwait == true) {
      printf("*stopfdwait == true\n");
      break;
    }

    if (!(events & FDW_IN))
      continue;

    pfd[0] = (struct nn_pollfd) {
      .fd = s,
      .events = NN_POLLIN
    };
    nn_poll (pfd, 1, -1);

    if (!(pfd[0].revents & NN_POLLIN))
      continue;

    for (;;) {
      char *buf;
      struct nn_mill_msg msg;
      rc = nn_recv (s, &buf, NN_MSG, NN_DONTWAIT);

      if (rc <= 0)
        break;

      msg.size = rc;
      msg.msg = buf;

      chs(inch, struct nn_mill_msg, msg);
    }

  }

  printf("int stopfdwait: %d\n", *stopfdwait);
  exit(0);
}

static coroutine void sender(int s, chan outch) {
  for(;;) {
    struct nn_mill_msg msg;
    msg = chr(outch, struct nn_mill_msg);
    nn_send(s, msg.msg, msg.size, NN_DONTWAIT);
  }
}

int main(int argc, char *argv[]) {
  // setup
  chan rcv = chmake(struct nn_mill_msg, 0);
  chan snd = chmake(struct nn_mill_msg, 0);
  chan rcv2 = chmake(struct nn_mill_msg, 0);
  chan snd2 = chmake(struct nn_mill_msg, 0);

  int s = nn_socket(AF_SP, NN_PAIR);
  int s2 = nn_socket(AF_SP, NN_PAIR);
  int rc;

  nn_bind(s, "tcp://127.0.0.1:7458");
  nn_connect(s2, "tcp://127.0.0.1:7458");

  // start
  int stopfdwait = false;
  // receiver
  go(receiver(s, rcv, &stopfdwait));
  go(sender(s2, snd2));
  // use
  struct nn_mill_msg msg;
  int cnt = 0;
  while (1) {
    choose {
      in(rcv, struct nn_mill_msg, msg): {
        printf("received message: %.*s\n", msg.size, msg.msg);
        cnt++;
        if (cnt == 5) {
          stopfdwait = true;
        }
      }
      deadline(now() + 1000): {
        msg.msg = "test";
        msg.size = 4;
        chs(snd2, struct nn_mill_msg, msg);
      }
      end
    }
  }
  // stop
  // clean exit
  return 0;
}
