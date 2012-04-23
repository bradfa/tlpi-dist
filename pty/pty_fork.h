/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* pty_fork.h

   Header file for pty_fork.c.
*/
#ifndef FORK_PTY_H
#define FORK_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS);

#endif
