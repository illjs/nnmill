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

#include <nanomsg/nn.h>
#include <libmill.h>
#include "nnmill.h"

static int nn_mill_getfd (int s, int fdtype) {
  int rc, fd;
  size_t fdsz = sizeof fd;

  if ( nn_getsockopt (s, NN_SOL_SOCKET, fdtype, &fd, &fdsz) != 0 )
    return -1;

  return fd;
}

int nm_send (int s, const void *buf, size_t len, int flags, int64_t deadline) {
  int fd = nn_mill_getfd(s, NN_SNDFD);
  int events;
  int rc;

  if (flags == NN_DONTWAIT)
    return nn_send(s, buf, len, flags);

  events = fdwait(fd, FDW_OUT, deadline);

  if (!(events & FDW_OUT))
    return EAGAIN;

  rc = nn_send(s, buf, len, 0);

  return rc;
}

int nm_recv (int s, void *buf, size_t len, int flags, int64_t deadline) {
  int fd = nn_mill_getfd(s, NN_RCVFD);
  int events;
  int rc;

  if (flags == NN_DONTWAIT)
    return nn_recv(s, buf, len, flags);

  events = fdwait(fd, FDW_IN, deadline);

  if (!(events & FDW_IN))
    return EAGAIN;

  rc = nn_recv(s, buf, len, 0);

  return rc;
}
