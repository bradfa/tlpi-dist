/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* sv_prog_abc.c

*/
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    void xyz(void), abc(void);

    printf("main() calling xyz()\n");
    xyz();

    abc();

    exit(EXIT_SUCCESS);
}
