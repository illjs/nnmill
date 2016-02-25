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

#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "nnmill.h"

struct endch_arr_item {
  int s; // nanomsg socket
  chan endch;
};

struct endch_arr {
  int count;
  int capacity;
  struct endch_arr_item *items;
} endch_arr = {
  .count = 0,
  .capacity = 0,
  .items = NULL
};

/* endch_arr's methods - start */

static int endch_arr_grow() {
  int ncapacity = endch_arr.capacity + 10;
  struct endch_arr_item *items = realloc(endch_arr.items, ncapacity * sizeof(struct endch_arr_item));

  if (items == NULL) {
    // reset endch_arr
    endch_arr.count = 0;
    endch_arr.capacity = 0;
    endch_arr.items = NULL;

    return ENOMEM;
  }

  if(endch_arr.items)
    memcpy(items, endch_arr.items, endch_arr.count * sizeof(struct endch_arr_item));

  endch_arr.capacity = ncapacity;
  endch_arr.items = items;

  return 0;
}

static int endch_arr_add(int s, chan endch) {
  int rc = 0;
  int index = endch_arr.count;

  if (endch_arr.count == endch_arr.capacity)
    rc = endch_arr_grow();

  if (rc != 0)
    return rc;

  endch_arr.items[index].s = s;
  endch_arr.items[index].endch = endch;

  endch_arr.count++;

  return 0;
}

static struct endch_arr_item *endch_arr_get(int index) {
  assert (index >= 0 && index < endch_arr.count);
  return &endch_arr.items[index];
}

static int endch_arr_get_index(int s) {
  int i = 0;
  for (; i < endch_arr.count; i++) {
    if (endch_arr.items[i].s == s)
      return i;
  }
  return -1;
}

static void endch_arr_del(int s) {
  int index = endch_arr_get_index(s);

  if (index == -1)
    return;

  int i = index;

  for(; i < endch_arr.count - 1; i++) {
    struct endch_arr_item *curr = &endch_arr.items[i];
    struct endch_arr_item *next = &endch_arr.items[i + 1];

    curr->s = next->s;
    curr->endch = next->endch;
  }

  endch_arr.count--;

  if (endch_arr.count == 0) {
    free(endch_arr.items);
    endch_arr.items = NULL;
  }
}

/* endch_arr's methods - end */

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

static void coroutine receiver(int s, int fd, chan in, int *endmsgrecvd, chan onend) {
  int rc;
  struct nn_pollfd pfd[1];
  struct nn_iovec iov;
  struct nn_msghdr hdr;

  while(1) {
    int events = fdwait(fd, FDW_IN, 100);

    if (*endmsgrecvd == true)
      break;

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

  fdclean(fd);
  chs(onend, int, 1);
}

static void coroutine poller(int s, chan in, chan out, int *result) {
  int fd = nn_mill_getfd(s);
  chan endch = chmake(chan, 1);
  chan rcvendch = chmake(int, 1);
  chan completech;
  int rc;
  int endmsgrecvd = false;

  if (fd < 0) {
    *result = -1;
    return;
  } else
    *result = 0;

  rc = endch_arr_add(s, endch);

  go(receiver(s, fd, in, &endmsgrecvd, rcvendch));

  struct nn_mill_msg msg;

  for(;;) {
    choose {
      in(out, struct nn_mill_msg, msg): {
        nn_send(s, msg.msg, msg.size, NN_DONTWAIT);
      }
      in(endch, chan, completech): {
        endmsgrecvd = true;
        chr(rcvendch, int);
        chs(completech, int, 1);
      }
      end
    }

    if (endmsgrecvd)
      break;
  }
}

int nn_mill_attach(int s, chan inch, chan outchan) {
  int result = 0;

  go(poller(s, inch, outchan, &result));

  return result;
}

int nn_mill_detach(int s) {
  int index = endch_arr_get_index(s);
  struct endch_arr_item *item;

  if (index < 0)
    return EINVAL;

  chan completech = chmake(int, 1);
  item = endch_arr_get(index);
  chs(item->endch, chan, completech);
  chr(completech, int);
  chclose(item->endch);
  chclose(completech);

  endch_arr_del(item->s);
}