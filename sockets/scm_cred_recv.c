/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* scm_cred_recv.c

   Used in conjunction with scm_cred_send.c to demonstrate passing of
   process credentials via a UNIX domain socket.

   This program receives credentials sent to a UNIX domain socket.

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
    struct ucred *ucredp, ucred;
    int data, lfd, sfd, optval, opt;
    ssize_t nr;
    Boolean useDatagramSocket;
    union {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(struct ucred))];
                        /* Space large enough to hold a ucred structure */
    } control_un;
    struct cmsghdr *cmhp;
    socklen_t len;

    /* Parse command-line arguments */

    useDatagramSocket = FALSE;

    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = TRUE;
            break;

        default:
            usageErr("%s [-d]\n"
                    "        -d    use datagram socket\n", argv[0]);
        }
    }

    /* Create socket bound to well-known address */

    if (remove(SOCK_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SOCK_PATH);

    if (useDatagramSocket) {
        printf("Receiving via datagram socket\n");
        sfd = unixBind(SOCK_PATH, SOCK_DGRAM);
        if (sfd == -1)
            errExit("unixBind");

    } else {
        printf("Receiving via stream socket\n");
        lfd = unixListen(SOCK_PATH, 5);
        if (lfd == -1)
            errExit("unixListen");

        sfd = accept(lfd, NULL, NULL);
        if (sfd == -1)
            errExit("accept");
    }

    /* We must set the SO_PASSCRED socket option in order to receive
       credentials */

    optval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) == -1)
        errExit("setsockopt");

    /* Set 'control_un' to describe ancillary data that we want to receive */

    control_un.cmh.cmsg_len = CMSG_LEN(sizeof(struct ucred));
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_CREDENTIALS;

    /* Set 'msgh' fields to describe 'control_un' */

    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    /* Set fields of 'msgh' to point to buffer used to receive (real)
       data read by recvmsg() */

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    msgh.msg_name = NULL;               /* We don't need address of peer */
    msgh.msg_namelen = 0;

    /* Receive real plus ancillary data */

    nr = recvmsg(sfd, &msgh, 0);
    if (nr == -1)
        errExit("recvmsg");
    printf("recvmsg() returned %ld\n", (long) nr);

    if (nr > 0)
        printf("Received data = %d\n", data);

    /* Extract credentials information from received ancillary data */

    cmhp = CMSG_FIRSTHDR(&msgh);
    if (cmhp == NULL || cmhp->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
        fatal("bad cmsg header / message length");
    if (cmhp->cmsg_level != SOL_SOCKET)
        fatal("cmsg_level != SOL_SOCKET");
    if (cmhp->cmsg_type != SCM_CREDENTIALS)
        fatal("cmsg_type != SCM_CREDENTIALS");

    ucredp = (struct ucred *) CMSG_DATA(cmhp);
    printf("Received credentials pid=%ld, uid=%ld, gid=%ld\n",
                (long) ucredp->pid, (long) ucredp->uid, (long) ucredp->gid);

    /* The Linux-specific, read-only SO_PEERCRED socket option returns
       credential information about the peer, as described in socket(7) */

    len = sizeof(struct ucred);
    if (getsockopt(sfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        errExit("getsockopt");

    printf("Credentials from SO_PEERCRED: pid=%ld, euid=%ld, egid=%ld\n",
            (long) ucred.pid, (long) ucred.uid, (long) ucred.gid);

    exit(EXIT_SUCCESS);
}
