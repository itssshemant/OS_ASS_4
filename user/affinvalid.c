#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("Testing zero mask: ");
  printf("%d (expected -1)\n", sched_setaffinity(0, 0x0));

  printf("Testing unavailable hart: ");
  printf("%d (expected -1)\n", sched_setaffinity(0, 0x8));

  printf("Testing valid mask: ");
  printf("%d (expected 0)\n", sched_setaffinity(0, 0x2));

  sched_setaffinity(0, 0x7);
  exit(0);
}
