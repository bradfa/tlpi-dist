/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* test_become_daemon.c

   Test our becomeDaemon() function.
*/
#include "become_daemon.h"
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    becomeDaemon(0);

    /* Normally a daemon would live forever; we just sleep for a while */

    sleep((argc > 1) ? getInt(argv[1], GN_GT_0, "sleep-time") : 20);

    exit(EXIT_SUCCESS);
}
