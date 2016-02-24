#ifndef NNMILL_H
#define NNMILL_H

#include <stdio.h>
#include <assert.h>

#include "nn.h"
#include "libmill.h"

struct nn_mill_msg {
  int size;
  char *msg;
};

int nn_mill_attach(int s, chan inch, chan outchan);

#endif
