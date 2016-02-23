#include <stdio.h>
#include <assert.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include "nnmill.h"

static void coroutine senderex(chan ch) {
  struct nn_mill_msg msg;

  for(;;) {
    msg.msg = "test";
    msg.size = 4;
    chs(ch, struct nn_mill_msg, msg);
    msleep(now() + 1000);
  }
}

static void coroutine receiverex(chan ch)
{
  struct nn_mill_msg msg;
  for(;;) {
    msg = chr(ch, struct nn_mill_msg);
    printf("msg: %.*s\n", msg.size, msg.msg);
  }
}

int main(int argc, char *argv[])
{
  chan rcv = chmake(struct nn_mill_msg, 0);
  chan snd = chmake(struct nn_mill_msg, 0);
  chan rcv2 = chmake(struct nn_mill_msg, 0);
  chan snd2 = chmake(struct nn_mill_msg, 0);
  chan endch = chmake(int, 0);
  int s = nn_socket(AF_SP, NN_PAIR);
  int s2 = nn_socket(AF_SP, NN_PAIR);
  int rc;

  nn_bind(s, "tcp://127.0.0.1:7458");
  nn_connect(s2, "tcp://127.0.0.1:7458");

  rc = nn_mill_attach(s, rcv, snd);
  assert(rc == 0);
  rc = nn_mill_attach(s2, rcv2, snd2);
  assert(rc == 0);

  go(senderex(snd));
  go(receiverex(rcv2));

  // never exit
  chr(endch, int);

  return 0;
}