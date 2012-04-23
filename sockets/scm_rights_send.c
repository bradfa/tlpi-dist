/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* scm_rights_send.c

   Used in conjunction with scm_rights_recv.c to demonstrate passing of
   file descriptors via a UNIX domain socket.

   This program sends a file descriptor to a UNIX domain socket.

   Usage is as shown in the usageErr() call below.

   File descriptors can exchanged over stream or datagram sockets. This
   program uses stream sockets by default; the "-d" command-line option
   specifies that datagram sockets should be used instead.

   This program is Linux-specific.
*/
#include "scm_rights.h"

int
main(int argc, char *argv[])
{
    struct msghdr msgh;
    struct iovec iov;
    int data, sfd, opt, fd;
    ssize_t ns;
    Boolean useDatagramSocket;
    union {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(int))];
                        /* Space large enough to hold an 'int' */
    } control_un;
    struct cmsghdr *cmhp;

    /* Parse command-line arguments */

    useDatagramSocket = FALSE;

    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = TRUE;
            break;

        default:
            usageErr("%s [-d] file\n"
                     "        -d    use datagram socket\n", argv[0]);
        }
    }

    if (argc != optind + 1)
        usageErr("%s [-d] file\n", argv[0]);

    /* Open the file named on the command line */

    fd = open(argv[optind], O_RDONLY);
    if (fd == -1)
        errExit("open");

    /* On Linux, we must transmit at least 1 byte of real data in
       order to send ancillary data */

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);
    data = 12345;

    /* We don't need to specify destination address, because we use
       connect() below */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    fprintf(stderr, "Sending fd %d\n", fd);

    /* Set message header to describe ancillary data that we want to send */

    cmhp = CMSG_FIRSTHDR(&msgh);
    cmhp->cmsg_len = CMSG_LEN(sizeof(int));
    cmhp->cmsg_level = SOL_SOCKET;
    cmhp->cmsg_type = SCM_RIGHTS;
    *((int *) CMSG_DATA(cmhp)) = fd;

    /*
    We could rewrite the preceding lines as:

    control_un.cmh.cmsg_len = CMSG_LEN(sizeof(int));
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_RIGHTS;
    *((int *) CMSG_DATA(CMSG_FIRSTHDR(&msgh))) = fd;
    */

    /* Do the actual send */

    sfd = unixConnect(SOCK_PATH, useDatagramSocket ? SOCK_DGRAM : SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    ns = sendmsg(sfd, &msgh, 0);
    if (ns == -1)
        errExit("sendmsg");

    fprintf(stderr, "sendmsg() returned %ld\n", (long) ns);
    exit(EXIT_SUCCESS);
}
