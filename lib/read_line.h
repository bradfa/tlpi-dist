/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* read_line.h

   Header file for read_line.c.
*/
#ifndef READ_LINE_H
#define READ_LINE_H

#include <sys/types.h>

ssize_t readLine(int fd, void *buffer, size_t n);

#endif
