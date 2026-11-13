#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid;

  printf("Parent: setting affinity to hart 1 (0x2)\n");

  if (sched_setaffinity(0, 0x2) < 0) {
    printf("Parent: sched_setaffinity failed\n");
    exit(1);
  }

  printf("Parent: CPU %d, affinity 0x%x\n",
         getcpu(), sched_getaffinity(0));

  pid = fork();

  if (pid < 0) {
    printf("Fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    printf("Child: CPU %d, inherited affinity 0x%x\n",
           getcpu(), sched_getaffinity(0));
    exit(0);
  }

  wait(0);

  sched_setaffinity(0, 0x7);
  printf("Parent: restored affinity to 0x%x\n",
         sched_getaffinity(0));

  printf("Fork inheritance test completed.\n");
  exit(0);
}
