/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* sv_libabc.c

*/
#include <stdio.h>

void
abc(void)
{
    void xyz(void);

    printf("abc() calling xyz()\n");
    xyz();
}

__asm__(".symver xyz,xyz@VER_1");
