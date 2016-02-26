#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

static void do_segv() {
  /* lets do some stuff that would segfault on any system */

  int *segv;
  segv = 0; /* malloc(a_huge_amount); */
  *segv = 1;

  /* also try to deref the null pointer wrong, should cause a segfault too */
  int* ptr = NULL;
  int i = *(int*)0;
}

sigjmp_buf point;

static void handler(int sig, siginfo_t *dont_care, void *dont_care_either) {
  longjmp(point, 1);
}

int main(){
  struct sigaction sa;
  memset(&sa, 0, sizeof(sigaction));
  sigemptyset(&sa.sa_mask);

  sa.sa_flags     = SA_NODEFER;
  sa.sa_sigaction = handler;

  sigaction(SIGSEGV, &sa, NULL); /* ignore whether it works or not */

  if (setjmp(point) == 0){
    printf("lets do a segfault\n");
    do_segv();
  } else {
    fprintf(stderr, "rather unexpected error\n");
  }

  return 0; /* program exits clean with no error */
}
