/*

  Copyright (c) 2016 Fatih Kaya
  Copyright (c) 2016 Bent Cardan

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

#include "nnmill.h"

static int nn_mill_getfd (int s) {
  int rc, fd;
  size_t fdsz = sizeof fd;

  if ( nn_getsockopt (s, NN_SOL_SOCKET, NN_RCVFD, &fd, &fdsz) != 0 )
    return -1;

  /* TODO: we might as well return both FDs, NN_SNDFD too */
  return fd;
}

/*  looks like fdwait can not be used in choose statement. So I have to create
    events channel and add dummy(or useful) data to it when fd has an event.
    By this way fdwait can be used in choose statement.
    Note: fn1 can send fdwait's result to the channel

    Example:
      void fn1(int fd, chan ch)
      {
        for(;;) {
          fdwait(fd ...)
          chs(ch, int, 1)
        }
      }

      void fn2(int fd)
      {
        chan fdevents = chmake(int, 0);
        go(fn1(fd, fdevents));

        choose {
          in(fdevents): ...
          in(otherchan): ...
          in(endchan):
            fdclean(fd);
            chdone(fdevents);
        }
      }
    */

static void coroutine receiver(int s, chan in) {
  int rc;
  int fd;
  struct nn_pollfd pfd[1];
  struct nn_iovec iov;
  struct nn_msghdr hdr;

  fd = nn_mill_getfd(s);

  if (fd < 0)
    return;

  while(1) {
    int events = fdwait(fd, FDW_IN, -1);
    if (!(events & FDW_IN))
      break;

    pfd[0] = (struct nn_pollfd) {
      .fd = s,
      .events = NN_POLLIN
    };
    nn_poll (pfd, 1, -1);

    if (!(pfd[0].revents & NN_POLLIN))
      break;

    for (;;) {
      char *buf;
      struct nn_mill_msg msg;
      rc = nn_recv (s, &buf, NN_MSG, NN_DONTWAIT);

      if (rc <= 0)
        break;

      msg.size = rc;
      msg.msg = buf;

      chs(in, struct nn_mill_msg, msg);
    }
  }
}

static void coroutine poller(int s, chan in, chan out, int *result) {
  int fd = nn_mill_getfd(s);

  if (fd < 0) {
    *result = -1;
    return;
  } else
    *result = 0;

  go(receiver(s, in));

  struct nn_mill_msg msg;

  for(;;) {
    choose {
      in(out, struct nn_mill_msg, msg): {
        nn_send(s, msg.msg, msg.size, NN_DONTWAIT);
      }
      end
    }
  }
}

int nn_mill_attach(int s, chan inch, chan outchan) {
  int result = 0;

  go(poller(s, inch, outchan, &result));

  return result;
}
