/*
 *	Implement strsignal and the C sys_siglist
 */

#include <string.h>
#include <unistd.h>

const char *sys_siglist[NSIGS] = {
/* 0 */
  "Unknown signal",
  "Hangup",
  "Interrupt",
  "Quit",
  "Illegal instruction",

  "BPT trace/trap",
  "I/O trap/ABORT instruction",
  "Bus error",
  "Floating point exception",
  "Killed",

 /* 10 */
  "User signal 1",
  "Segmentation fault",
  "User signal 2",
  "Broken pipe",
  "Alarm clock",

  "Terminated",
  "Stack fault",
  "Child death or stop",
  "Continue",
  "Stopped",

 /* 20 */
  "Stopped (signal)",
  "Stopped (tty input)",
  "Stopped (tty output)",
  "Urgent IO condition",
  "CPU limit",

  "File limit",
  "Alarm (virtual)",
  "Alarm (profile)",
  "Window changed",
  "I/O ready",

/* 30 */
  "Power",
  "Bad system call"
};

const char *strsignal(int s)
{
  if (s < 1 || s >= NSIGS)
    s = 0;
  return sys_siglist[s];
}
