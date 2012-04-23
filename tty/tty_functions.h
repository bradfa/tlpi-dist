/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* tty_functions.h

   Header file for tty_functions.c.
*/
#ifndef TTY_FUNCTIONS_H
#define TTY_FUNCTIONS_H

#include <termios.h>

int ttySetCbreak(int fd, struct termios *prevTermios);

int ttySetRaw(int fd, struct termios *prevTermios);

#endif
