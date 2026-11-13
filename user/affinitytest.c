#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int cpu, mask;

  printf("Initial CPU: %d\n", getcpu());
  printf("Initial affinity: 0x%x\n", sched_getaffinity(0));

  printf("\nSetting affinity to hart 1...\n");
  if (sched_setaffinity(0, 0x2) < 0) {
    printf("sched_setaffinity failed\n");
    exit(1);
  }

  cpu = getcpu();
  mask = sched_getaffinity(0);
  printf("Current CPU: %d, affinity: 0x%x\n", cpu, mask);

  printf("\nSetting affinity to hart 2...\n");
  if (sched_setaffinity(0, 0x4) < 0) {
    printf("sched_setaffinity failed\n");
    exit(1);
  }

  cpu = getcpu();
  mask = sched_getaffinity(0);
  printf("Current CPU: %d, affinity: 0x%x\n", cpu, mask);

  printf("\nRestoring affinity to all harts...\n");
  sched_setaffinity(0, 0x7);

  printf("Current CPU: %d, affinity: 0x%x\n",
         getcpu(), sched_getaffinity(0));

  printf("\nAffinity test completed.\n");
  exit(0);
}
