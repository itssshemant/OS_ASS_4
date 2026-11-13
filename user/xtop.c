#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/schedinfo.h"
#include "user/user.h"

#define MAXP 64
#define MAX_HARTS 8

struct row {
  struct schedinfo si;
  int cpu;
};

// Static storage: do not put these large arrays on the xv6 user stack.
static int prev_pid[MAXP];
static int prev_ticks[MAXP];
static int pids[MAXP];
static int hart_busy[MAX_HARTS];
static struct row rows[MAXP];
static struct schedinfo si;

char*
state_text(int state)
{
  switch(state) {
  case PS_RUNNING: return "RUN";
  case PS_RUNNABLE: return "RUNNABLE";
  case PS_SLEEPING: return "SLEEP";
  case PS_ZOMBIE: return "ZOMBIE";
  case PS_USED: return "USED";
  default: return "UNKNOWN";
  }
}

int
main(int argc, char *argv[])
{
  int delay = 10;
  int frames = 0;
  int i, j, f;
  int have_prev = 0;
  int prev_count = 0;
  int count, runnable, policy, nrows, delta;

  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
      delay = atoi(argv[++i]);
      if(delay <= 0) {
        printf("xtop: delay must be positive\n");
        exit(1);
      }
    } else if(strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      frames = atoi(argv[++i]);
      if(frames < 0) {
        printf("xtop: invalid frame count\n");
        exit(1);
      }
    } else {
      printf("Usage: xtop [-d N] [-n F]\n");
      exit(1);
    }
  }

  for(f = 0; frames == 0 || f < frames; f++) {
    count = getpids(pids, MAXP);
    if(count < 0)
      count = 0;
    if(count > MAXP)
      count = MAXP;

    nrows = 0;
    runnable = 0;

    for(i = 0; i < MAX_HARTS; i++)
      hart_busy[i] = 0;

    for(i = 0; i < count; i++) {
      if(schedinfo(pids[i], &si) < 0)
        continue;

      if(si.state == PS_RUNNABLE)
        runnable++;

      delta = 0;
      if(have_prev) {
        for(j = 0; j < prev_count; j++) {
          if(prev_pid[j] == si.pid) {
            delta = si.rticks - prev_ticks[j];
            if(delta < 0)
              delta = 0;
            break;
          }
        }
      }

      if(nrows >= MAXP)
        break;

      rows[nrows].si = si;
      rows[nrows].cpu = (delta * 100) / delay;
      nrows++;

      if(have_prev && si.lastcpu >= 0 && si.lastcpu < MAX_HARTS)
        hart_busy[si.lastcpu] += delta;
    }

    // Sort processes by CPU usage, highest first.
    for(i = 0; i < nrows; i++) {
      for(j = i + 1; j < nrows; j++) {
        if(rows[j].cpu > rows[i].cpu) {
          struct row temp = rows[i];
          rows[i] = rows[j];
          rows[j] = temp;
        }
      }
    }

    printf("\x1b[H\x1b[2J");
    policy = schedpolicy(-1);

    printf("xv6 TOP | uptime: %d ticks | runnable: %d | policy: %s\n",
           uptime(), runnable,
           policy == POLICY_PRIO ? "PRIO" : "RR");

    printf("Hart busy (approx): ");
    for(i = 0; i < 3; i++)
      printf("H%d %d%%  ", i,
             have_prev ? (hart_busy[i] * 100) / delay : 0);
    printf("\n\n");

    printf("PID NAME STATE PRIO CPU MASK SLICE CPU%%\n");

    for(i = 0; i < nrows; i++) {
      printf("%d %s %s %d %d 0x%x %d %d%%\n",
             rows[i].si.pid,
             rows[i].si.name,
             state_text(rows[i].si.state),
             rows[i].si.prio,
             rows[i].si.lastcpu,
             rows[i].si.affinity,
             rows[i].si.slice,
             rows[i].cpu);
    }

    // Save current counters for the next refresh.
    prev_count = nrows;
    for(i = 0; i < nrows; i++) {
      prev_pid[i] = rows[i].si.pid;
      prev_ticks[i] = rows[i].si.rticks;
    }

    have_prev = 1;

    if(frames == 0 || f + 1 < frames)
      pause(delay);
  }

  exit(0);
}
