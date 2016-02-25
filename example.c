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

#include "nnmill.c"
#include "pair.h"
#include "inproc.h"

chan endch;

static void coroutine senderex(int s, chan ch) {
  struct nn_mill_msg msg;
  int count = 0;

  for(;;) {
    msg.msg = "test";
    msg.size = 4;
    chs(ch, struct nn_mill_msg, msg);

    count++;

    if (count == 5)
      break;

    msleep(now() + 1000);
  }

  nn_mill_detach(s);
  chs(endch, int, 1);
}

static void coroutine receiverex(int s, chan ch)
{
  int complete = false;
  struct nn_mill_msg msg;
  for(;;) {
    choose {
      in(ch, struct nn_mill_msg, msg): {
        printf("msg: %.*s\n", msg.size, msg.msg);
      }
      deadline(now() + 3000): {
        nn_mill_detach(s);
        chs(endch, int, 1);
        complete = true;
      }
      end
    }

    if (complete)
      break;
  }
}

int main(int argc, char *argv[])
{
  chan rcv = chmake(struct nn_mill_msg, 0);
  chan snd = chmake(struct nn_mill_msg, 0);
  chan rcv2 = chmake(struct nn_mill_msg, 0);
  chan snd2 = chmake(struct nn_mill_msg, 0);
  endch = chmake(int, 0);

  int s = nn_socket(AF_SP, NN_PAIR);
  int s2 = nn_socket(AF_SP, NN_PAIR);
  int rc;

  nn_bind(s, "tcp://127.0.0.1:7458");
  nn_connect(s2, "tcp://127.0.0.1:7458");

  rc = nn_mill_attach(s, rcv, snd);
  assert(rc == 0);
  rc = nn_mill_attach(s2, rcv2, snd2);
  assert(rc == 0);

  go(senderex(s, snd));
  go(receiverex(s2, rcv2));

  // never exit
  chr(endch, int);
  chr(endch, int);

  return 0;
}
