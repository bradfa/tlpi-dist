/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* ugid_functions.h

   Header file for ugid_functions.c.
*/
#ifndef UGID_FUNCTIONS_H
#define UGID_FUNCTIONS_H

#include "tlpi_hdr.h"

char *userNameFromId(uid_t uid);

uid_t userIdFromName(const char *name);

char *groupNameFromId(gid_t gid);

gid_t groupIdFromName(const char *name);

#endif
