/*

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

#include <libmill.h>
#include <sodium.h>

#define MAX_INPUT_LEN 4096
#define noncehex "e500906852935c3fa50a82ba5deed1cb36c3eb038da7838c"

/*
 * print_hex() is a wrapper around sodium_bin2hex() which allocates
 * temporary memory then immediately prints the result followed by \n
 */
static void print_hex(const void *bin, const size_t bin_len) {
  char   *hex;
  size_t  hex_size;

  if ( bin_len >= SIZE_MAX / 2 )
    abort();

  hex_size = bin_len * 2 + 1;
  if ((hex = malloc(hex_size)) == NULL)
    abort();

  /* the library supplies a few utility functions like the one below */
  if (sodium_bin2hex(hex, hex_size, bin, bin_len) == NULL)
    abort();

  puts(hex);
  free(hex);
}

size_t reinterpret_msg (const char *normalchar, char *input) {
  size_t actual_input_len = strlen(normalchar);
  memcpy(input, normalchar, actual_input_len);
  return actual_input_len;
}

void whoswho(int z, char *alice, char *bob) {
  if (z) {
    strncpy(alice, "alice", 5);
    strncpy(bob, "bob", 3);
  } else {
    strncpy(alice, "bob", 3);
    strncpy(bob, "alice", 5);
  }
}


static coroutine void alice_and_bob ( const char *original_msg,
                                      chan ch,
                                      unixsock s,
                                      unsigned char *n,
                                      unsigned char *n2,
                                      int a) {

  size_t sz =   crypto_box_PUBLICKEYBYTES * 2 + 1; /* HEX size of pub keys */
  char          hex[sz];
  unsigned char pk[crypto_box_PUBLICKEYBYTES]; /* public key */
  unsigned char ppk[crypto_box_PUBLICKEYBYTES]; /* inbound pub key */
  unsigned char sk[crypto_box_SECRETKEYBYTES]; /* secret key */

  /* hold the ciphertext */
  unsigned char ciphertext[crypto_box_MACBYTES + MAX_INPUT_LEN];
  unsigned char cphrtxtin[crypto_box_MACBYTES + MAX_INPUT_LEN];
  size_t        cphr_len;
  size_t msz    = sizeof ciphertext * 2 + 1;

  unsigned char msg[MAX_INPUT_LEN]; /* used with msg_tmp for outbound */
  unsigned char msgrcv[MAX_INPUT_LEN]; /* used for inbound decipher */

  /* generate public and private keys */
  crypto_box_keypair(pk, sk);

  /* public key exchange over unix sock */
  sodium_bin2hex(hex, sz, pk, sizeof pk);
  unixsend(s, hex, sizeof hex, -1);
  unixflush(s, -1);

  char buf[sz];
  size_t nbytes = unixrecv(s, buf, sizeof hex, -1);
  buf[nbytes] = '\0';

  char whoami[6];
  char whoarethey[6];
  whoswho(a, whoami, whoarethey);

  printf("-----\n%s public key: %s\n%s public key: %s\n",
    whoami, hex, whoarethey, buf);
  sodium_hex2bin(ppk, MAX_INPUT_LEN, buf, nbytes, NULL, NULL, NULL);

  size_t len = reinterpret_msg(original_msg, (char*)msg);
  assert (crypto_box_easy(ciphertext, msg, len, n, ppk, sk) == 0);

  char msgout[msz];
  char msgin[msz];

  cphr_len = crypto_box_MACBYTES + len;
  sodium_bin2hex(msgout, msz, ciphertext, cphr_len);

  unixsend(s, msgout, sizeof msgout, -1);
  unixflush(s, -1);
  nbytes = unixrecv(s, msgin, sizeof msgout, -1);
  msgin[nbytes] = '\0';

  sodium_hex2bin(cphrtxtin, MAX_INPUT_LEN, msgin, nbytes, NULL, &cphr_len, NULL);

  printf("-----\n%s cipher: %s\n%s cipher: ", whoami, msgout, whoarethey);
  print_hex(cphrtxtin, cphr_len);

  assert (crypto_box_open_easy(msgrcv, cphrtxtin, cphr_len, n2, ppk, sk) == 0);

  printf("%s's recv'd plaintext: ", whoami);
  fwrite(msgrcv, 1U, len, stdout);
  putchar('\n');

  unixclose(s);
  chs(ch, int, 1);
}

int main (const int argc, const char **argv) {

  unsigned char nonce[crypto_box_NONCEBYTES];

  unixsock a, b;
  unixpair(&a, &b);

  if (sodium_init() == -1)
    abort();

  printf("Using libsodium %s\nGenerating nonces..\n", sodium_version_string());
  sodium_hex2bin(nonce, 4096, noncehex, sizeof nonce, NULL, NULL, NULL);
  print_hex(nonce, crypto_box_NONCEBYTES);
  print_hex(nonce + 1, crypto_box_NONCEBYTES);

  printf("Encrypting and authenticating with %s\n", crypto_box_primitive());

  chan ch     = chmake(int, 0);

  go(alice_and_bob("encrypt this", ch, a, nonce, nonce + 1, 1));
  go(alice_and_bob("encrypt that", ch, b, nonce + 1, nonce, 0));

  chr(ch, int);
  chr(ch, int);
  chclose(ch);

  printf("-----\n");

  return 0;
}
