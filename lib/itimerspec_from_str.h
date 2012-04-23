/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* itimerspec_from_str.h

   Header file for itimerspec_from_str.c.
*/
#ifndef ITIMERSPEC_FROM_STR_H
#define ITIMERSPEC_FROM_STR_H

#include <time.h>

void itimerspecFromStr(char *str, struct itimerspec *tsp);

#endif
