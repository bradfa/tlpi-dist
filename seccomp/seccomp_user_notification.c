/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2019.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* seccomp_user_notification.c

   Demonstrate the seccomp notification-to-user-space feature added in
   Linux 5.0.

   This program creates two child processes, the "target" and the "tracer".

   The target process uses seccomp(2) to install a BPF filter using the
   SECCOMP_FILTER_FLAG_NEW_LISTENER flag. This flag causes seccomp(2) to return
   a file descriptor that can be used to receive notifications when the filter
   performs a return with the action SECCOMP_RET_USER_NOTIF; the BPF filter
   employed in this example performs such a return when the target process
   calls mkdir(2).

   The target process passes the notification file descriptor returned by
   seccomp(2) to the tracer process via a UNIX domain socket.

   The target process then performs a series of mkdir(2) calls using the
   pathnames supplied as command-line arguments to the program. The effect
   of each SECCOMP_RET_USER_NOTIF action triggered by these system calls is:

   (a) the mkdir(2) system call in the target process is *not* executed;
   (b) a notification is generated on the notification file descriptor;
   (c) the target process remains blocked in the mkdir(2) system call until
       a response is sent on the notification file descriptor (this response
       will include information for a "faked" return value for the mkdir(2)
       call--either a success return value, or a -1 error return with a value
       to be assigned to 'errno').

   The tracer process receives the notification file descriptor that was sent
   by the target process over the UNIX domain socket. It then waits for
   notifications using the SECCOMP_IOCTL_NOTIF_RECV ioctl(2) operation. Each
   of these notifications returns a structure that includes the PID of the
   target process, and information (the same 'struct seccomp_data' that a
   seccomp BPF filter receives) describing the target process's system call.
   In this example program, these notifications will relate to the mkdir(2)
   calls made by the target process.

   From user space, the tracer is able to do some things that an in-kernel
   seccomp BPF filter can't do; in particular, it can inspect the target
   process's memory (via /proc/PID/mem) in order to find out the values
   referred to by pointer arguments (e.g., the pathname argument of mkdir(2)).
   The tracer then makes a decision based on the pathname, creating the
   specified directory on behalf of the target process only if the pathname
   starts with ("/tmp/").

   The tracer then performs a SECCOMP_IOCTL_NOTIF_SEND ioctl(2) operation,
   which provides a response for the target process's system call. This
   response can specify either a success return value for the system call, or
   an error return, including a value that will be placed in 'errno' in the
   target process.
*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <stddef.h>
#include <stdbool.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "scm_functions.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(__NR_seccomp, operation, flags, args);
}

/* Values from command-line options */

struct cmdLineOpts {
    int  delaySecs;     /* Delay time for responding to notifications */
    int  secondFilter;  /* Install a second BPF filter? */
    bool killTracer;    /* Kill tracer when target has died? */
};

/* The following is the x86-64-specific BPF boilerplate code for checking that
   the BPF program is running on the right architecture + ABI. At completion
   of these instructions, the accumulator contains the system call number. */

/* For the x32 ABI, all system call numbers have bit 30 set */

#define X32_SYSCALL_BIT         0x40000000

#define X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, arch))), \
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 2), \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                 (offsetof(struct seccomp_data, nr))), \
        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, X32_SYSCALL_BIT, 0, 1), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS)

/* installNotifyFilter() installs a seccomp filter that generates user-space
   notifications (SECCOMP_RET_USER_NOTIF) when the process calls mkdir(2); the
   filter allows all other system calls.

   The function return value is a file descriptor from which the user-space
   notifications can be fetched. */

static int
installNotifyFilter(void)
{
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR,

        /* mkdir() triggers notification to user-space tracer */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mkdir, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = (unsigned short) (sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    int notifyFd;

    /* Install the filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER flag; as
       a result, seccomp() returns a notification file descriptor. */

    notifyFd = seccomp(SECCOMP_SET_MODE_FILTER,
                        SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (notifyFd == -1)
        errExit("seccomp-install-notify-filter");

    return notifyFd;
}

/* installFilter2() optionally installs a second BPF filter in order to allow
   experiments with the precedence of SECCOMP_RET_USER_NOTIF relative to other
   filter return values. As with the other filter, this filter performs special
   treatment of mkdir(2) and allows all other system calls. */

static void
installFilter2(struct cmdLineOpts *opts)
{
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR,

        /* Treat mkdir() system calls specially */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mkdir, 1, 0),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* The last entry in the BPF program will be replaced by a "return"
           instruction; see below */

        { 0, 0, 0, 0 },
    };

    struct sock_fprog prog = {
        .len = (unsigned short) (sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    /* Depending on the value of the "-f" command-line option, place either
       a SECCOMP_RET_ERRNO instruction in the BPF program, or otherwise a
       SECCOMP_RET_TRACE instruction. This can be used to illustrate that
       SECCOMP_RET_ERRNO has higher precedence than the SECCOMP_RET_USER_NOTIF
       returned by the other filter, with the result that the user-space
       notification will not occur. By contrast, SECCOMP_RET_TRACE has lower
       precedence (so that the user-space notification does occur). */

    const struct sock_filter retTrace = BPF_STMT(BPF_RET + BPF_K,
                                              SECCOMP_RET_TRACE);
    const struct sock_filter retErrno = BPF_STMT(BPF_RET + BPF_K,
                                              SECCOMP_RET_ERRNO | ENOTSUP);

    filter[prog.len - 1] = (opts->secondFilter == SECCOMP_RET_ERRNO) ?
                                        retErrno : retTrace;

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog) == -1)
        errExit("seccomp-install-filter-2");
}

/* Handler for the SIGINT signal in the target process */

static void
handler(int sig)
{
    /* UNSAFE: This handler uses non-async-signal-safe functions
       (printf(); see TLPI Section 21.1.2) */

    printf("Target process: received signal\n");
}

/* Close a pair of sockets created by socketpair() */

static void
closeSocketPair(int sockPair[2])
{
    if (close(sockPair[0]) == -1)
        errExit("closeSocketPair-close-0");
    if (close(sockPair[1]) == -1)
        errExit("closeSocketPair-close-1");
}

/* Implementation of the target process; create a child process that:

   (1) installs a seccomp filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER
       flag;
   (2) writes the seccomp notification file descriptor returned from the
       previous step onto the UNIX domain socket, 'sockPair[0]';
   (3) calls mkdir(2) for each element of 'argv'.

   The function return value is the PID of the child process. */

static pid_t
targetProcess(int sockPair[2], char *argv[], struct cmdLineOpts *opts)
{
    pid_t targetPid;
    int notifyFd;
    struct sigaction sa;
    int s;

    targetPid = fork();
    if (targetPid == -1)
        errExit("fork");

    if (targetPid > 0)          /* In parent, return PID of child */
        return targetPid;

    /* Child falls through to here */

    printf("Target process: PID = %ld\n", (long) getpid());

    /* Install a handler for the SIGINT signal */

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    /* Install seccomp filter(s) */

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        errExit("prctl");

    notifyFd = installNotifyFilter();

    if (opts->secondFilter != -1)
        installFilter2(opts);

    /* Pass the notification file descriptor to the tracing process over
       a UNIX domain socket */

    if (sendfd(sockPair[0], notifyFd) == -1)
        errExit("sendfd");

    /* Notification and socket FDs are no longer needed in target process */

    if (close(notifyFd) == -1)
        errExit("close-target-notify-fd");

    closeSocketPair(sockPair);

    /* Perform a mkdir() call for each of the command-line arguments */

    for (char **ap = argv; *ap != NULL; ap++) {
        printf("\nTarget process: about to make directory \"%s\"\n", *ap);
        s = mkdir(*ap, 0600);
        if (s == -1)
            perror("Target process: mkdir");
        else
            printf("Target process: SUCCESS: mkdir(2) returned = %d\n", s);
    }

    printf("Target process: terminating\n");
    exit(EXIT_SUCCESS);
}

/* Check that the notification ID provided by a SECCOMP_IOCTL_NOTIF_RECV
   operation is still valid. It will no longer be valid if the process has
   terminated. This operation can be used when accessing /proc/PID files in
   the target process in order to avoid TOCTOU race conditions where the
   PID that is returned by SECCOMP_IOCTL_NOTIF_RECV terminates and is
   reused by another process. */

static void
checkNotificationIdIsValid(int notifyFd, __u64 id, char *tag)
{
    if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_ID_VALID, &id) == -1) {
        fprintf(stderr, "Tracer: notification ID check (%s): "
                "target has died!!!!!!!!!!!\n", tag);
    }
}

/* Handle notifications that arrive via SECCOMP_RET_USER_NOTIF file
   descriptor, 'notifyFd'. */

static void
watchForNotifications(int notifyFd, struct cmdLineOpts *opts)
{
    struct seccomp_notif *req;
    struct seccomp_notif_resp *resp;
    struct seccomp_notif_sizes sizes;
    char path[PATH_MAX];
    int procMem;        /* FD for /proc/PID/mem of target process */

    /* Discover the sizes of the structures that are used to receive
       notifications and send notification responses, and allocate
       buffers of those sizes. */

    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, &sizes) == -1)
        errExit("Tracer: seccomp-SECCOMP_GET_NOTIF_SIZES");

    req = malloc(sizes.seccomp_notif);
    if (req == NULL)
        errExit("Tracer: malloc");

    resp = malloc(sizes.seccomp_notif_resp);
    if (resp == NULL)
        errExit("Tracer: malloc");

    /* Loop handling notifications */

    for (;;) {

        /* Wait for next notification, returning info in '*req' */

        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1)
            errExit("Tracer: ioctlSECCOMP_IOCTL_NOTIF_RECV");

        printf("Tracer: got notification for PID %d; ID is %llx\n",
                req->pid, req->id);

        /* If a delay interval was specified on the command line, then delay
           for the specified number of seconds. This can be used to demonstrate
           the following:

           (1) The target process is blocked until the tracer sends a response.
           (2) If the blocked system call is interrupted by a signal handler,
               then the SECCOMP_IOCTL_NOTIF_SEND operation fails with the error
               ENOENT.
           (3) If the target process terminates, then we can discover this
               using the SECCOMP_IOCTL_NOTIF_ID_VALID operation (which is
               employed by checkNotificationIdIsValid()). */

        if (opts->delaySecs > 0) {
            printf("Tracer: delaying for %d seconds:", opts->delaySecs);
            checkNotificationIdIsValid(notifyFd, req->id, "pre-delay");

            for (int d = opts->delaySecs; d > 0; d--) {
                printf(" %d", d);
                sleep(1);
            }
            printf("\n");

            checkNotificationIdIsValid(notifyFd, req->id, "post-delay");
        }

        /* Access the memory of the target process in order to discover
           the pathname that was given to mkdir() */

        snprintf(path, sizeof(path), "/proc/%d/mem", req->pid);

        procMem = open(path, O_RDONLY);
        if (procMem == -1)
            errExit("Tracer: open");

        /* Check that the process whose info we are accessing is still alive */

        checkNotificationIdIsValid(notifyFd, req->id, "post-open");

        /* Since, the SECCOMP_IOCTL_NOTIF_ID_VALID operation (performed in
           checkNotificationIdIsValid()) succeeded, we know that the
           /proc/PID/mem file descriptor that we opened corresponded to the
           process for which we received a notification. If that process
           subsequently terminates, then read() on that file descriptor will
           return 0 (EOF). This can be tested by (1) uncommenting the sleep()
           call below (and rebuilding the program); (2) running the program
           with flags to ensure that the tracer is not killed if the target
           dies; and (3) killing the target process during the sleep(). */

        // printf("About to sleep in target\n");
        // sleep(15);

        /* Seek to the location containing the pathname argument (i.e., the
           first argument) of the mkdir(2) call and read that pathname */

        if (lseek(procMem, req->data.args[0], SEEK_SET) == -1)
            errExit("Tracer: lseek");

        ssize_t s = read(procMem, path, sizeof(path));
        if (s == -1)
            errExit("read");
        else if (s == 0) {
            fprintf(stderr, "Tracer: read returned EOF\n");
            exit(EXIT_FAILURE);
        }

        printf("Tracer: mkdir(\"%s\", %llo)\n", path, req->data.args[1]);

        if (close(procMem) == -1)
            errExit("close-/proc/PID/mem");

        /* The response to the notification includes the notification ID */

        resp->id = req->id;
        resp->flags = 0;        /* Must be zero as at Linux 5.0 */

        /* Success return value is the length of the pathname given to
           mkdir() */

        resp->val = strlen(path);

        /* If the directory is in /tmp, then create it on behalf of the tracer;
           give an error for a directory pathname in any other location. */

        if (strncmp(path, "/tmp/", strlen("/tmp/")) == 0) {
            mkdir(path, req->data.args[1]);
            resp->error = 0;
        } else {
            resp->error = -EPERM;
        }

        /* Provide a response to the target process */

        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            if (errno == ENOENT)
                printf("Tracer: response failed with ENOENT; perhaps target "
                        "process's syscall was interrupted by signal?\n");
            else
                perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
        }

        /* If the pathname is just "/bye", then the tracer terminates. This
           allows us to see what happens if the target process makes further
           calls to mkdir(2). */

        if (strcmp(path, "/bye") == 0) {
            printf("Tracer: terminating <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* Implementation of the tracer process; create a child process that:

   (1) obtains the seccomp notification file descriptor from 'sockPair[1]';
   (2) handles notifications that arrive on that file descriptor.

   The function return value is the PID of the child process. */

static pid_t
tracerProcess(int sockPair[2], struct cmdLineOpts *opts)
{
    pid_t tracerPid;

    tracerPid = fork();
    if (tracerPid == -1)
        errExit("fork");

    if (tracerPid > 0)          /* In parent, return PID of child */
        return tracerPid;

    /* Child falls through to here */

    printf("Tracer: PID = %ld\n", (long) getpid());

    /* Receive the notification file descriptor from the target process */

    int notifyFd = recvfd(sockPair[1]);
    if (notifyFd == -1)
        errExit("recvfd");

    closeSocketPair(sockPair);  /* We no longer need the socket pair */

    /* Handle notifications */

    watchForNotifications(notifyFd, opts);

    exit(EXIT_SUCCESS);         /* NOTREACHED */
}

/* Diagnose an error in command-line option or argument usage */

static void
usageError(char *msg, char *pname)
{
    if (msg != NULL)
        fprintf(stderr, "%s\n", msg);

#define fpe(msg) fprintf(stderr, "      " msg);
    fprintf(stderr, "Usage: %s [options] <dir> <dir>...\n", pname);
    fpe("Options\n");
    fpe("-d <nsecs>    Tracer delays 'nsecs' before inspecting target\n");
    fpe("-f <val>      Install second filter whose return value is:\n");
    fpe("              'e' - SECCOMP_RET_ERRNO\n");
    fpe("              't' - SECCOMP_RET_TRACE\n");
    fpe("-K            Don't kill tracer on termination of target process\n");
    exit(EXIT_FAILURE);
}

/* Parse command-line options, returning option info in 'opts' */

static void
parseCommandLineOptions(int argc, char *argv[], struct cmdLineOpts *opts)
{
    int opt;

    opts->secondFilter = -1;
    opts->delaySecs = 0;
    opts->killTracer = true;

    while ((opt = getopt(argc, argv, "d:Kf:")) != -1) {
        switch (opt) {

        case 'K':       /* Don't kill tracer when target process terminates */
            opts->killTracer = false;
            break;

        case 'f':       /* Install a second BPF filter */
            if (optarg[0] == 'e')
                opts->secondFilter = SECCOMP_RET_ERRNO;
            else if (optarg[0] == 't')
                opts->secondFilter = SECCOMP_RET_TRACE;
            else
                usageError("Bad value for -f", argv[0]);
            break;

        case 'd':       /* Delay time before sending notification response */
            opts->delaySecs = atoi(optarg);
            break;

        default:
            usageError("Bad option", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* There should be at least one command-line argument after the options */

    if (optind >= argc)
        usageError("At least one pathname argument should be supplied",
                argv[0]);
}

int
main(int argc, char *argv[])
{
    pid_t targetPid, tracerPid;
    int sockPair[2];
    struct cmdLineOpts opts;

    setbuf(stdout, NULL);

    parseCommandLineOptions(argc, argv, &opts);

    /* Create a UNIX domain socket that is used to pass the seccomp
       notification file descriptor from the target process to the tracer
       process. */

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockPair) == -1)
        errExit("socketpair");

    /* Create a child process--the "target"--that installs seccomp filtering.
       The target process writes the seccomp notification file descriptor
       onto 'sockPair[0]' and then calls mkdir(2) for each directory in the
       command-line arguments. */

    targetPid = targetProcess(sockPair, &argv[optind], &opts);

    /* Create the "tracer" as another child process. This allows the parent to
       wait on the target process and then either kill or wait on the tracer
       when the target terminates. The tracer reads the seccomp notification
       file descriptor from 'sockPair[1]' and then handles the notifications
       that arrive on that file descriptor. */

    tracerPid = tracerProcess(sockPair, &opts);

    /* The parent process does not need the socket pair */

    closeSocketPair(sockPair);

    /* Wait for the target process to terminate */

    waitpid(targetPid, NULL, 0);
    printf("Parent: target process has terminated\n");

    /* After the target process has terminated, either kill or wait for
       the tracer process */

    if (opts.killTracer) {
        printf("Parent: killing tracer\n");
        kill(tracerPid, SIGTERM);
    } else {
        waitpid(tracerPid, NULL, 0);
    }

    exit(EXIT_SUCCESS);
}
