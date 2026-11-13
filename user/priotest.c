#include "kernel/types.h"
#include "kernel/schedinfo.h"
#include "user/user.h"

int
main(void)
{
  int start[2], done[2];
  int pids[3];
  char tokens[3] = {'A', 'B', 'C'};
  char token, result;
  int oldpolicy;

  printf("=== Priority Scheduling Test ===\n");

  oldpolicy = schedpolicy(-1);

  if (pipe(start) < 0 || pipe(done) < 0) {
    printf("Pipe creation failed\n");
    exit(1);
  }

  sched_setaffinity(0, 0x1);
  schedpolicy(1);

  for (int i = 0; i < 3; i++) {
    pids[i] = fork();

    if (pids[i] < 0) {
      printf("Fork failed\n");
      exit(1);
    }

    if (pids[i] == 0) {
      close(start[1]);
      close(done[0]);

      read(start[0], &token, 1);
      close(start[0]);

      int end = uptime() + 10;
      while (uptime() < end)
        ;

      result = tokens[i];
      write(done[1], &result, 1);
      close(done[1]);

      exit(0);
    }

    if (setprio(pids[i], i) < 0) {
      printf("setprio failed for PID %d\n", pids[i]);
      exit(1);
    }
  }

  close(start[0]);
  close(done[1]);

  // Release all children only after assigning priorities.
  write(start[1], tokens, 3);
  close(start[1]);

  printf("Completion order: ");

  for (int i = 0; i < 3; i++) {
    if (read(done[0], &result, 1) == 1)
      printf("%c ", result);
  }

  printf("\n");

  close(done[0]);

  for (int i = 0; i < 3; i++)
    wait(0);

  schedpolicy(oldpolicy);
  sched_setaffinity(0, 0x7);

  printf("Expected order: A B C (HIGH, NORMAL, LOW)\n");
  printf("Priority test completed.\n");

  exit(0);
}
