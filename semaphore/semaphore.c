#include "semaphore.h"

int compare_semaphores(void *d1, void *d2) {
  Semaphore *p1 = (Semaphore *)d1;

  char d11;
  d11 = p1->name;

  char *d22 = (char *)d2;
  char d23 = *d22;

  if (d11 == d23) {
    return 1;
  } else {
    return 0;
  }
}