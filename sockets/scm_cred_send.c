/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* scm_cred_send.c

   Used in conjunction with scm_cred_recv.c to demonstrate passing of
   process credentials via a UNIX domain socket.

   This program sends credentials to a UNIX domain socket.

   Usage is as shown in the usageErr() call below.

   Credentials can exchanged over stream or datagram sockets. This program
   uses stream sockets by default; the "-d" command-line option specifies
   that datagram sockets should be used instead.

   This program is Linux-specific.
*/
#include "scm_cred.h"

int
main(int argc, char *argv[])
{
    struct msghdr msgh;
    struct iovec iov;
    int data, sfd, opt;
    ssize_t ns;
    Boolean useDatagramSocket, noExplicit;
    union {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(struct ucred))];
                        /* Space large enough to hold a ucred structure */
    } control_un;
    struct cmsghdr *cmhp;

    /* Parse command-line arguments */

    useDatagramSocket = FALSE;
    noExplicit = FALSE;

    while ((opt = getopt(argc, argv, "dn")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = TRUE;
            break;

        case 'n':
            noExplicit = TRUE;
            break;

        default:
            usageErr("%s [-d] [-n] [data [PID [UID [GID]]]]\n"
                    "        -d    use datagram socket\n"
                    "        -n    don't construct explicit "
                                  "credentials structure\n", argv[0]);
        }
    }

    /* On Linux, we must transmit at least 1 byte of real data in
       order to send ancillary data */

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    /* Data is optionally taken from command line */

    data = (argc > optind) ? atoi(argv[optind]) : 12345;

    /* Don't need to specify destination address, because we use
       connect() below */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    if (noExplicit) {

        /* Don't construct an explicit credentials structure. (It
           is not necessary to do so, if we just want the receiver to
           receive our real credentials.) */

        printf("Not explicitly sending a credentials structure\n");
        msgh.msg_control = NULL;
        msgh.msg_controllen = 0;

    } else {
        struct ucred *ucp;

        /* Set 'msgh' fields to describe 'control_un' */

        msgh.msg_control = control_un.control;
        msgh.msg_controllen = sizeof(control_un.control);

        /* Set message header to describe ancillary data that we want to send */

        cmhp = CMSG_FIRSTHDR(&msgh);
        cmhp->cmsg_len = CMSG_LEN(sizeof(struct ucred));
        cmhp->cmsg_level = SOL_SOCKET;
        cmhp->cmsg_type = SCM_CREDENTIALS;
        ucp = (struct ucred *) CMSG_DATA(cmhp);

        /*
        We could rewrite the preceding as:

        control_un.cmh.cmsg_len = CMSG_LEN(sizeof(struct ucred));
        control_un.cmh.cmsg_level = SOL_SOCKET;
        control_un.cmh.cmsg_type = SCM_CREDENTIALS;
        ucp = (struct ucred *) CMSG_DATA(CMSG_FIRSTHDR(&msgh));
        */

        /* Use sender's own PID, real UID, and real GID, unless
           alternate values were supplied on the command line */

        ucp->pid = (argc > optind + 1 && strcmp(argv[optind + 1], "-") != 0) ?
                        atoi(argv[optind + 1]) : getpid();
        ucp->uid = (argc > optind + 2 && strcmp(argv[optind + 2], "-") != 0) ?
                        atoi(argv[optind + 2]) : getuid();
        ucp->gid = (argc > optind + 3 && strcmp(argv[optind + 3], "-") != 0) ?
                        atoi(argv[optind + 3]) : getgid();

        printf("Send credentials pid=%ld, uid=%ld, gid=%ld\n",
                (long) ucp->pid, (long) ucp->uid, (long) ucp->gid);
    }

    sfd = unixConnect(SOCK_PATH, useDatagramSocket ? SOCK_DGRAM : SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    ns = sendmsg(sfd, &msgh, 0);
    if (ns == -1)
        errExit("sendmsg");
    printf("sendmsg() returned %ld\n", (long) ns);

    exit(EXIT_SUCCESS);
}
