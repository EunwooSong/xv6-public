#include "types.h"
#include "stat.h"
#include "user.h"

// Test for aging in the priority scheduler.
// Each worker burns CPU until a common deadline and counts how many
// ticks it got to run before the deadline. Without aging, a worker
// with a large nice value never runs while workers with smaller nice
// values are RUNNABLE, so its count is 0.

#define DURATION 300    // ticks
#define NWORKER 3

// Burn CPU until the deadline and report, through fd, the number of
// distinct ticks during which this process was running.
void
worker(int nice, int deadline, int fd)
{
  int count = 0, last = -1, now;

  setnice(getpid(), nice);
  while((now = uptime()) < deadline){
    if(now != last){
      count++;
      last = now;
    }
  }
  write(fd, &count, sizeof(count));
  exit();
}

// Run 3 workers with the given nice values for DURATION ticks and
// store the tick count of each worker in count[].
void
run_workers(int *nice, int *count)
{
  int fd[NWORKER][2], deadline, i;

  deadline = uptime() + DURATION;
  for(i = 0; i < NWORKER; i++){
    pipe(fd[i]);
    if(fork() == 0)
      worker(nice[i], deadline, fd[i][1]);
  }
  for(i = 0; i < NWORKER; i++){
    read(fd[i][0], &count[i], sizeof(count[i]));
    close(fd[i][0]);
    close(fd[i][1]);
  }
  for(i = 0; i < NWORKER; i++)
    wait();
}

// Print the CPU share of each worker as a percentage with one decimal.
void
report(int id, int *nice, int *count, int ok)
{
  int i, sum = 0, share;

  for(i = 0; i < NWORKER; i++)
    sum += count[i];
  printf(1, "case %d. nice %d %d %d -> cpu", id, nice[0], nice[1], nice[2]);
  for(i = 0; i < NWORKER; i++){
    share = sum > 0 ? count[i] * 1000 / sum : 0;
    printf(1, " %d.%d%%", share / 10, share % 10);
  }
  printf(1, ": %s\n", ok ? "OK" : "WRONG");
}

int
main(int argc, char **argv)
{
  int nice[NWORKER], count[NWORKER], sum, ok, i;

  // Keep the parent at the highest priority so that it can create
  // the workers and collect the results in time.
  setnice(getpid(), 0);

  // Case 1: the nice 10 process is not starved, but still runs less.
  nice[0] = 0; nice[1] = 0; nice[2] = 10;
  run_workers(nice, count);
  ok = count[2] > 0 && count[0] > count[2] && count[1] > count[2];
  report(1, nice, count, ok);

  // Case 2: every process runs, and a smaller nice gets more CPU.
  nice[0] = 1; nice[1] = 5; nice[2] = 9;
  run_workers(nice, count);
  ok = count[2] > 0 && count[0] > count[1] && count[1] > count[2];
  report(2, nice, count, ok);

  // Case 3: equal priorities share CPU fairly
  // (each gets 1/2 ~ 3/2 of the average).
  nice[0] = 3; nice[1] = 3; nice[2] = 3;
  run_workers(nice, count);
  sum = count[0] + count[1] + count[2];
  ok = sum > 0;
  for(i = 0; i < NWORKER; i++)
    if(count[i] * NWORKER * 2 < sum || count[i] * NWORKER * 2 > sum * 3)
      ok = 0;
  report(3, nice, count, ok);

  exit();
}
