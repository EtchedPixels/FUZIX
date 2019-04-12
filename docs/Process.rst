Process Management
==================

Each task in Fuzix consists of two structures. The first is the process
table entry (struct p_tab, ptptr). This holds the elements of the
process information that are required all of the time. The remaining
data is stored in a per process structure called udata structure (struct
u_data). This is assumed to be mapped at a fixed address in memory
although this can be changed (see advanced topics). The udata fields are
only required when this process is being executed. This allows them to
be swapped or banked out of the address space when the process is not
running. The udata also usually holds the kernel and any interrupt
stacks for the process. This allows them to be swapped out or banked
out. Note that this means references to objects on the kernel stacks of
a process must never be used from another process or interrupt.

The fixed address mapping of udata allows the compilers to generate much
better code on most 8bit processors than would be the case if pointer
indirections where used.

In the normal mode of operation the kernel multi-tasks by switching
between processes when they block, when they exceed their available CPU
time when running application code, or at the end of a system call which
has exceeded the available time. Tasks are not pre-empted during a
system call unless they block, nor is any real time scheduling system
implemented.

Scheduling
----------

Fuzix implements a fairly simple task scheduler designed to work
efficiently with a small number of processes while using the minimum
amount of memory. Runnable tasks execute in strict FIFO order with lower
priority tasks being given less CPU time. The scheduler does not
currently punish CPU heavy tasks. There is no idle task, instead the
kernel repeatedly invokes platform_idle() when there is no work to be
done. This function can execute platform specific device polling or CPU
idle and halt instructions as appropriate.

If the kernel is compiled in single tasking mode only the most recent
task is ever scheduled. When a fork() occurs the child runs to
completion before the parent is woken.

Task States
-----------

The Fuzix task states roughly match those of most Unix like systems.
They implement the expected Unix semantics of zombie processes and
wait().

+-----------------------------------+-----------------------------------+
| P_EMPTY                           | A slot in the task array that is  |
|                                   | not being used                    |
+-----------------------------------+-----------------------------------+
| P_RUNNING                         | The current executing task        |
+-----------------------------------+-----------------------------------+
| P_READY                           | A task which can be run but is    |
|                                   | not assigned the CPU              |
+-----------------------------------+-----------------------------------+
| P_SLEEP                           | A task which is waiting for an    |
|                                   | event or a signal                 |
+-----------------------------------+-----------------------------------+
| P_IOWAIT                          | A task which is waiting for an    |
|                                   | event, but will not be woken by a |
|                                   | signal                            |
+-----------------------------------+-----------------------------------+
| P_STOPPED                         | A task that is suspended and not  |
|                                   | runnable until it receives a      |
|                                   | waking signal (SIGCONT)           |
+-----------------------------------+-----------------------------------+
| P_FORKING                         | A task that is currently being    |
|                                   | forked                            |
+-----------------------------------+-----------------------------------+
| P_ZOMBIE                          | A task that has exited. Only the  |
|                                   | process table entry remains       |
|                                   | waiting for wait() to reap the    |
|                                   | process                           |
+-----------------------------------+-----------------------------------+
| P_NOSLEEP                         | An internal state where sleeping  |
|                                   | is forbidden.                     |
+-----------------------------------+-----------------------------------+

Sleeping And Waking
-------------------

Processes sleep by calling psleep() and passing a cookie which is the
address of an object. In general you should choose a representative
address such as the pointer to the object being waited upon, or a field
within it. Waiting on NULL is a wait which will not be woken by an
event.

Processes waiting for an event will be woken by a called to wakeup()
with the same event cookie. It is important to note that a wakeup wakes
all processes that are waiting on the event. On a small machine with
little memory it is very important to wake only those processes which
need to be woken. Decisions about which task to wake should be made by
the waker not by the processes being woken.

Where it is necessary to explicitly wake a specific process because the
wake queues are not sufficient to implement the desired precise wake
semantics it is also possible to call pwake() to wake a process
explicitly.

Kernel Facilities
-----------------

This documents only those facilities intended to be used by driver and
platform code. There are many more process manipulation functions but
they should not be needed by platform or drivers.

**void psleep(void \*event)**

Move the running process from P_RUNNING into P_SLEEP if an event cookie
is passed, or into P_PAUSE if not. Execution will continue with another
task or the platform_idle code, until the task is woken, whereupon it
will return from psleep.

Take particular care when testing an event that is set in an interrupt.
You should disable interrupts before testing and calling psleep and then
restore interrupts on return. The kernel core will deal with re-enabling
interrupts on the task switch.

**void wakeup(void \*event)**

Wake all processes that are sleeping on the passed event cookie. The
current process will remain P_RUNNING, while those woken will move to
P_READY and will be scheduled when the waking task pre-empts them. This
function may also be called from an interrupt, in which case the same
will occur, or if the system is executing platform_idle it will begin
running a task.

**void pwake(ptptr p)**

Explicitly wake a task. The task will enter P_READY state if not already
ready or running. Tasks in P_FORKING or P_ZOMBIE will not be
affected. This function can be called from interrupts but care must be
taken that the interrupt wake is disabled before the process exits.
Where possible use wakeup and events.

**int need_resched(void)**

Returns true if the current process has exceeded its time allocation.
This is useful in busy waiting or bitbanging applications where the
driver needs to know when to sleep and when to poll.

**p->p_timeout**

This field holds a timeout. If this is set to 1 plus the number of
centiseconds to wait then the process will be woken after the timeout.
The process can then check if this value is 1 indicating a timeout. Each
system call this field is cleared.

Signals
-------

Fuzix implements all of the classic V7 signals and a subset of the newer
ones. The kernel provides signal handling functionality based upon the
System 5 API rather than the BSD interface. This is because the kernel
parts of the System 5 interface are smaller and cleaner, whilst a C
library can implement the POSIX API on either.

The following signals are provided

+-----------------------+-----------------------+-----------------------+
| SIGHUP                | Yes                   | Sent to a process     |
|                       |                       | whose controlling tty |
|                       |                       | hangs up.             |
+-----------------------+-----------------------+-----------------------+
| SIGINT                | Yes                   | Sent when the user    |
|                       |                       | types the interrupt   |
|                       |                       | character (usually    |
|                       |                       | ^C).                  |
+-----------------------+-----------------------+-----------------------+
| SIGQUIT               | Yes                   | Sent when the user    |
|                       |                       | types the quit        |
|                       |                       | character (usually    |
|                       |                       | ^\\).                 |
+-----------------------+-----------------------+-----------------------+
| SIGILL                | Platform              | Sent when an illegal  |
|                       |                       | instruction is        |
|                       |                       | executed. Most 8bit   |
|                       |                       | platforms do not      |
|                       |                       | support illegal       |
|                       |                       | instruction           |
|                       |                       | reporting.            |
+-----------------------+-----------------------+-----------------------+
| SIGTRAP               | Platform              | Sent when a trap      |
|                       |                       | occurs if the         |
|                       |                       | platform implements   |
|                       |                       | traps.                |
+-----------------------+-----------------------+-----------------------+
| SIGABORT              | Yes                   | Sent from userspace   |
|                       |                       | when the abort()      |
|                       |                       | library function is   |
|                       |                       | called.               |
+-----------------------+-----------------------+-----------------------+
| SIGIOT                | Platform              | I/O trap. Not         |
|                       |                       | implemented by        |
|                       |                       | current platforms.    |
+-----------------------+-----------------------+-----------------------+
| SIGBUS                | Platform              | Sent when the         |
|                       |                       | platform receives a   |
|                       |                       | bus error.            |
+-----------------------+-----------------------+-----------------------+
| SIGFPE                | Platform              | Sent when a platform  |
|                       |                       | receives a floating   |
|                       |                       | point exception. In   |
|                       |                       | some cases also sent  |
|                       |                       | when division by zero |
|                       |                       | is attempted.         |
+-----------------------+-----------------------+-----------------------+
| SIGKILL               | Yes                   | Sent by one process   |
|                       |                       | to another to kill a  |
|                       |                       | process. Cannot be    |
|                       |                       | blocked or deferred.  |
+-----------------------+-----------------------+-----------------------+
| SIGUSR1               | Yes                   | Reserved for user.    |
+-----------------------+-----------------------+-----------------------+
| SIGSEGV               | Platform              | A segmentation fault  |
|                       |                       | trap was taken by the |
|                       |                       | hardware.             |
+-----------------------+-----------------------+-----------------------+
| SIGUSR2               | Yes                   | Reserved for user.    |
+-----------------------+-----------------------+-----------------------+
| SIGPIPE               | Yes                   | A write was attempted |
|                       |                       | to a pipe with no     |
|                       |                       | readers.              |
+-----------------------+-----------------------+-----------------------+
| SIGALRM               | Yes                   | An alarm timed out.   |
+-----------------------+-----------------------+-----------------------+
| SIGTERM               | Yes                   | Sent by one process   |
|                       |                       | to another to request |
|                       |                       | termination. Can be   |
|                       |                       | blocked or caught.    |
+-----------------------+-----------------------+-----------------------+
| SIGSTKFLT             | Platform              | Sent when the         |
|                       |                       | platform detects a    |
|                       |                       | stack fault.          |
+-----------------------+-----------------------+-----------------------+
| SIGCHLD               | Yes                   | Sent to the parent    |
|                       |                       | when the child exits. |
+-----------------------+-----------------------+-----------------------+
| SIGCONT               | Yes                   | Continue a process    |
|                       |                       | stopped with SIGSTOP  |
|                       |                       | or SIGTSTP.           |
+-----------------------+-----------------------+-----------------------+
| SIGSTOP               | Yes                   | Freeze a process and  |
|                       |                       | prevent further       |
|                       |                       | execution until       |
|                       |                       | continued. Cannot be  |
|                       |                       | blocked or deferred.  |
+-----------------------+-----------------------+-----------------------+
| SIGTSTP               | No                    | Sent when the stop    |
|                       |                       | character is used to  |
|                       |                       | halt a process.       |
+-----------------------+-----------------------+-----------------------+
| SIGTTIN               | No                    | Job control.          |
+-----------------------+-----------------------+-----------------------+
| SIGTTOU               | No                    | Job control.          |
+-----------------------+-----------------------+-----------------------+
| SIGURG                | No                    | Sent when an urgent   |
|                       |                       | event occurs.         |
|                       |                       | Normally used for     |
|                       |                       | networking. Available |
|                       |                       | for platform specific |
|                       |                       | uses.                 |
+-----------------------+-----------------------+-----------------------+
| SIGXCPU               | No                    | CPU ulimit exceeded.  |
+-----------------------+-----------------------+-----------------------+
| SIGXFSZ               | Yes                   | File size exceeded.   |
+-----------------------+-----------------------+-----------------------+
| SIGVTALRM             | No                    | Virtual alarm timed   |
|                       |                       | out.                  |
+-----------------------+-----------------------+-----------------------+
| SIGPROF               | No                    | Used for profiling.   |
+-----------------------+-----------------------+-----------------------+
| SIGWINCH              | Platform              | The console window    |
|                       |                       | has changed size. No  |
|                       |                       | current console       |
|                       |                       | supports window size  |
|                       |                       | changes.              |
+-----------------------+-----------------------+-----------------------+
| SIGIO/SIGPOLL         | No                    | Used for asynchronous |
|                       |                       | I/O notifications.    |
+-----------------------+-----------------------+-----------------------+
| SIGPWR                | Platform              | Sent when power is    |
|                       |                       | restored so that      |
|                       |                       | processes may refresh |
|                       |                       | their display.        |
+-----------------------+-----------------------+-----------------------+
| SIGSYS                | No                    | Sent when an invalid  |
|                       |                       | system call is made.  |
+-----------------------+-----------------------+-----------------------+

Signals may be sent either to processes or to process groups. The kernel
implements kill(), signal() and a private sigdisp() function which is
used by the C library to provide sighold() and sigrelse(). The SIG_IGN
and SIG_DFL behaviours are implemented.

The kernel provides signal helper methods to platform and driver code.
These include methods for delivering signals and for implementing the
platform or CPU specific signal processing logic.

**void ssig(ptrptr p, uint_fast8_t sig)**

Sends a signal to the specified process. A signal delivered to the
current process will be handled asynchronously when the task returns to
user space. For synchronous delivery see the Advanced Topics section.

**void sgrpsig(uint16_t pgrp, uint_fast8_t sig)**

Sends a signal to the specified process group. The signal will be
delivered and handled asynchronously in the same way as ssig().

**uint_fast8_t chksigs(void)**

Recompute the pending signals for the process and place the next signal
to be delivered into udata.u_cursig. This is used by the CPU and
platform specific syscall exit paths in order to identify signal
delivery events.

Platform and CPU Provided Methods
---------------------------------

The required methods for each platform are provided by the platform
layer, but may well be included from library code, or in most cases will
be similar for systems. Most implementations are based upon the existing
examples. As some methods such as switchout are very tricky it is
strongly recommended any new implementation is closely based upon an
existing one or uses the library implementations when possible.

The defines for process management are as follows

**PTABSIZE**

This gives the maximum number of processes that can exist at a time. It
defaults to 15 which is actually plenty for a small single user micro.

The methods that are required for process management are as follows

**void platform_switchout(void)**

Save the current state of the process onto the process kernel stack or
into udata then call getproc and switchin in order to execute a new
task. Switchin() and switchout() co-operate such that the task appears
to return from switchout() when and if it resumes execution. This
function may be called with interrupts disabled. Do not assume a process
that calls switchout() will ever switchin() again. Process exit is
implemented by doing a switchin() of the next task and later freeing the
process table resources of the exiting task.

**int switchin(ptptr p)**

Resume execution of the process p as if it had returned from
switchout(). Upon return interrupts should either be re-enabled or
restored to the state at the time of switchout().

**int dofork(ptptr new)**

Fork the current process into the proces new. At the point of entry new
is not fully set up. The dofork function is expected to copy the udata,
copy the process memory image, set up the stack such that both the
parent and child will seem to return from the dofork() call, the parent
with the child pid, the child with pid 0, call newproc with the new
ptptr and then return in the context of the child process.

**void map_init(void)**

The init process is special as it is created out of thin air rather than
forked from an existing process. The map_init function allows the
platform to set up data for init. At the point of entry init_process is
a ptptr to the init task, and udata is the udata of the newly created
task. The kernel is effectively running as if it was executing system
call for init. It then completes startup by executing an execve system
call on behalf of the new init process. On most platforms map_init()
will either be an empty function, or set up some memory management
fields.

Advanced Topics
---------------

The udata is normally a fixed address object. For some processors such
as the 68000 this does not really make sense. On these platforms udata
can be defined to be (&udata_ptr) and udata_ptr maintained by the
platform code to point to the current udata, either as a static variable
or a register global. If udata is defined then a udata_ptr is made
available for the platform/CPU code in each process structure.

The fixed location udata poses a problem in some situations because it
needs to be mapped in whenever the kernel is executing code for this
process, but also needs to be banked with the process. In some cases it
may be necessary to implement udata copying in preference to a udata
pointer. Z80 platforms do this as the cost of udata being indirectly
referenced exceeds the pain of copying.

In some situations a signal needs to be delivered synchronously, e.g. an
unrecoverable alignment error. In this case it is sufficient to call
ssig() as the process will take the signal handler when the code
prepares to return to user space. The library code may also wish to
stack a call to \_exit on the user stack before the signal is takem so
that a signal handler that returns exits the process.

When an exception requires an immediate exit of the process to avoid
further harm a SIGKILL should be delivered. This will always exit the
process.

Future Directions
-----------------

More of the signals will become available and implemented. In particular
SIGCHLD and SIGSTOP behaviour. Other areas such as job control are
probably too complex to fit in a small 8bit system, but will be more
relevant to a 16/32bit system.

Many of the current processors do not have a trap
interface or a clear kernel and user mode. As a result their current
lowlevel-$(cpu) implementations are a bit fuzzy about kernel and user
space divides when delivering signals and do not deliver any kind of
signal information frame on the stack with the signal call.

There may be a need for a sigreturn system call and path on bigger
machines to get the semantics right.

Process And Udata Fields And Meaning
------------------------------------

Process Table
~~~~~~~~~~~~~

+-----------------------------------+-----------------------------------+
| p_status                          | Execution state of the process    |
|                                   | (see Task States)                 |
+-----------------------------------+-----------------------------------+
| p_tty                             | The minor number of the           |
|                                   | controlling tty for this process  |
+-----------------------------------+-----------------------------------+
| p_pid                             | The process identifier            |
+-----------------------------------+-----------------------------------+
| p_uid                             | The real uid for this process     |
+-----------------------------------+-----------------------------------+
| p_pptr                            | Pointer to our parent process.    |
+-----------------------------------+-----------------------------------+
| p_alarm                           | Centiseconds until alarm occurs   |
+-----------------------------------+-----------------------------------+
| p_exitval                         | Exit code for the process (set    |
|                                   | when it terminates)               |
+-----------------------------------+-----------------------------------+
| p_wait                            | Cookie used in psleep/pwait       |
+-----------------------------------+-----------------------------------+
| p_page                            | If zero no memory is allocated to |
|                                   | this process and p_page2 is swap  |
|                                   | data, otherwise platform specific |
|                                   | memory management information     |
+-----------------------------------+-----------------------------------+
| p_page2                           | Either swap location, or if in    |
|                                   | memory further platform specific  |
|                                   | memory data                       |
+-----------------------------------+-----------------------------------+
| p_udata                           | Pointer to the process udata (not |
|                                   | present for fixed address udata)  |
+-----------------------------------+-----------------------------------+
| p_priority                        | Process priority                  |
+-----------------------------------+-----------------------------------+
| p_sig                             | Signal structures                 |
+-----------------------------------+-----------------------------------+
| p_waitno                          | Wait counter value, used to work  |
|                                   | out what to swap                  |
+-----------------------------------+-----------------------------------+
| p_timeout                         | 0 – no timeout, 1 – timed out, 2+ |
|                                   | number of centiseconds -1 left    |
|                                   | before timeout                    |
+-----------------------------------+-----------------------------------+
| p_name                            | First 8 bytes of the executable   |
|                                   | name                              |
+-----------------------------------+-----------------------------------+
| p_time                            | Time used by this process         |
+-----------------------------------+-----------------------------------+
| p_utime                           | User time used by this process    |
+-----------------------------------+-----------------------------------+
| p_stime                           | System time used by this process  |
+-----------------------------------+-----------------------------------+
| p_cutime                          | User time used by this process    |
|                                   | and completed children            |
+-----------------------------------+-----------------------------------+
| p_cstime                          | System time used by this process  |
|                                   | and completed children            |
+-----------------------------------+-----------------------------------+
| p_pgrp                            | The process group this process    |
|                                   | belongs to                        |
+-----------------------------------+-----------------------------------+
| p_nice                            | The nice value for this process   |
+-----------------------------------+-----------------------------------+
| p_event                           | Process events (for job control)  |
+-----------------------------------+-----------------------------------+
| p_top                             | Copy of u_top for swapping        |
+-----------------------------------+-----------------------------------+
| p_flags                           | Process bit flags                 |
+-----------------------------------+-----------------------------------+
| p_session                         | Job control session (level 2)     |
+-----------------------------------+-----------------------------------+
| p_profscale                       | Profiling scaling shift           |
+-----------------------------------+-----------------------------------+
| p_profbuf                         | Profiling buffer base             |
+-----------------------------------+-----------------------------------+
| p_profsize                        | Profiling size                    |
+-----------------------------------+-----------------------------------+
| p_profoff                         | Offset of memory to profile       |
+-----------------------------------+-----------------------------------+

When a process enters P_ZOMBIE state prior to exiting the values from
p_priority down are overlaid with the exit state and data

User Data Structure
~~~~~~~~~~~~~~~~~~~

+-----------------------------------+-----------------------------------+
| u_ptab                            | Pointer to the process table      |
|                                   | entry for this process            |
+-----------------------------------+-----------------------------------+
| u_page                            | Copy of p_page, during fork the   |
|                                   | two may differ temporarily        |
+-----------------------------------+-----------------------------------+
| u_page2                           | Copy of p_page2, during fork the  |
|                                   | two may differ temporarily        |
+-----------------------------------+-----------------------------------+
| u_insys                           | True if in the kernel             |
+-----------------------------------+-----------------------------------+
| u_callno                          | Current system call number being  |
|                                   | executed                          |
+-----------------------------------+-----------------------------------+
| u_syscall_sp                      | User stack point when syscall     |
|                                   | occurred                          |
+-----------------------------------+-----------------------------------+
| u_retval                          | Return value from syscall         |
+-----------------------------------+-----------------------------------+
| u_error                           | Last error code from syscall      |
+-----------------------------------+-----------------------------------+
| u_sp                              | Stack pointer saved when task     |
|                                   | switching                         |
+-----------------------------------+-----------------------------------+
| u_ininterrupt                     | True if running on our interrupt  |
|                                   | stack                             |
+-----------------------------------+-----------------------------------+
| u_cursig                          | Current signal to deliver         |
|                                   | (computed by chksigs())           |
+-----------------------------------+-----------------------------------+
| u_argn                            | System call argument              |
+-----------------------------------+-----------------------------------+
| u_argn1                           | Second system call argument       |
+-----------------------------------+-----------------------------------+
| u_argn2                           | Third system call argument        |
+-----------------------------------+-----------------------------------+
| u_argn3                           | Fourth system call argument       |
+-----------------------------------+-----------------------------------+
| u_isp                             | Initial stack pointer to use      |
|                                   | during doexec() when entering     |
|                                   | user space                        |
+-----------------------------------+-----------------------------------+
| u_top                             | Top of memory for this task (will |
|                                   | be moving to process table)       |
+-----------------------------------+-----------------------------------+
| u_break                           | Top of data for this task         |
+-----------------------------------+-----------------------------------+
| u_codebase                        | Load address of binary (32bit)    |
+-----------------------------------+-----------------------------------+
| u_sigvec                          | Array of 32 user mode function    |
|                                   | pointers for signal handlers      |
+-----------------------------------+-----------------------------------+
| u_base                            | Source or destination for         |
|                                   | character style I/O               |
+-----------------------------------+-----------------------------------+
| u_count                           | Number of bytes for character     |
|                                   | style I/O                         |
+-----------------------------------+-----------------------------------+
| u_offset                          | Offset for I/O                    |
+-----------------------------------+-----------------------------------+
| u_buf                             | Pointer to the buffer to use for  |
|                                   | block I/O                         |
+-----------------------------------+-----------------------------------+
| u_sysio                           | True if the character I/O is to   |
|                                   | system space (special case for    |
|                                   | tty)                              |
+-----------------------------------+-----------------------------------+
| u_mask                            | Umask of the process              |
+-----------------------------------+-----------------------------------+
| u_guid                            | Group id of the process           |
+-----------------------------------+-----------------------------------+
| u_euid                            | Effective uid of the process      |
+-----------------------------------+-----------------------------------+
| u_egid                            | Effective gid of the process      |
+-----------------------------------+-----------------------------------+
| u_name                            | Copy of p_name                    |
+-----------------------------------+-----------------------------------+
| u_files                           | Array of indexes into the file    |
|                                   | handle table for each file that   |
|                                   | is open                           |
+-----------------------------------+-----------------------------------+
| u_cloexec                         | Bit mask of files to close on     |
|                                   | execv (may change to allow more   |
|                                   | open files)                       |
+-----------------------------------+-----------------------------------+
| u_cwd                             | Inode of the current working      |
|                                   | directory of the process          |
+-----------------------------------+-----------------------------------+
| u_root                            | Inode of the current root         |
|                                   | directory of the process          |
+-----------------------------------+-----------------------------------+
| u_rename                          | Inode to match during rename      |
|                                   | processing                        |
+-----------------------------------+-----------------------------------+
| u_ctty                            | Minor number of controlling       |
|                                   | terminal                          |
+-----------------------------------+-----------------------------------+
| u_block                           | Block number for disk I/O         |
|                                   | terminal                          |
+-----------------------------------+-----------------------------------+
| u_blkoff                          | Offset into block for disk I/O    |
|                                   | terminal                          |
+-----------------------------------+-----------------------------------+
| u_nblock                          | Number of blocks for disk I/O     |
+-----------------------------------+-----------------------------------+
| u_dptr                            | Pointer to the data buffer for an |
|                                   | I/O (can be kernel or user)       |
+-----------------------------------+-----------------------------------+
| u_done                            | I/O counter for driver methods    |
+-----------------------------------+-----------------------------------+

Level 2 systems also have

+-----------------------------------+-----------------------------------+
| u_groups                          | Supplementary group identifers    |
+-----------------------------------+-----------------------------------+
| u_ngroup                          | Number of supplementary group     |
|                                   | identifiers present               |
+-----------------------------------+-----------------------------------+
| u_flags                           | Additional flags                  |
+-----------------------------------+-----------------------------------+
| u_rlimit                          | Resource limits                   |
+-----------------------------------+-----------------------------------+
