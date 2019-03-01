/*
** File: usage.cpp
**
** Copyright 2006 Mark Lisee
**
*/

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include "misc.h"

/*
****************************************
**
** Function: printUsage
**
** Print an error message indicating the correct usage and exits.
**
** Returns:	exits
*/
void printUsage(
  const char	*pName,
  const char	*usageString)
{
  const char	*str, *ptr;

  for( str = ptr = pName; *ptr != 0; ptr++)
    if( *ptr == '/')
      str = ptr+1;
  printf( "Usage:\n%s %s\n", str, usageString);
  exit( EINVAL);
}

/* end file usage.cpp */
