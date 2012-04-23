/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* mod3.c */

#include <stdio.h>
#include <unistd.h>
int test3[10000] = { 1, 2, 3 };

#ifndef VERSION
#define VERSION ""
#endif

void
x3(void) {
    printf("Called mod3-x3 " VERSION "\n");
}
