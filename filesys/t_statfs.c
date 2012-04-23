/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* t_statfs.c

   Demonstrate the use of statfs() to retrieve information about
   a mounted file system.

   This program is Linux-specific.

   See also t_statvfs.c.
*/
#include <sys/statfs.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct statfs sfs;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s path\n", argv[0]);

    if (statfs(argv[1], &sfs) == -1)
        errExit("statfs");

    printf("File system type:              %#x\n", (unsigned int) sfs.f_type);
    printf("Optimal I/O block size:        %ld\n", (long) sfs.f_bsize);
    printf("Total data blocks:             %ld\n", (long) sfs.f_blocks);
    printf("Free data blocks:              %ld\n", (long) sfs.f_bfree);
    printf("Free blocks for nonsuperuser:  %ld\n", (long) sfs.f_bavail);
    printf("Total i-nodes:                 %ld\n", (long) sfs.f_files);
    printf("File system ID:                %ld, %ld\n",
            (long) sfs.f_fsid.__val[0],  (long) sfs.f_fsid.__val[1]);
    printf("Free i-nodes:                  %ld\n", (long) sfs.f_ffree);
    printf("Maximum file name length:      %ld\n", (long) sfs.f_namelen);

    exit(EXIT_SUCCESS);
}
