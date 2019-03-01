#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

extern FILE	*yyin;
extern FILE	*yyout;

/*
****************************************
**
** Function: ::openFile
**
** Opens the given file for reading and stores the file pointer in source.
**
** Return value:
**	True if the file was opened, False otherwise.
*/
bool openFile(
  const char	*fileName)
{
  struct stat	statBuf;

  if( stat( fileName, &statBuf) < 0)
  {
    fprintf( stderr, "fstat failed on file '%s': %s\n", fileName,
	     strerror( errno));
    return false;
  }
  if( (statBuf.st_mode & S_IFREG) == 0)
  {
    fprintf( stderr, "File '%s' is not a regular file\n", fileName);
    return false;
  }
  yyin = fopen( fileName, "r");
  if( yyin == 0)
  {
    fprintf( stderr, "Unable to open file '%s': %s\n", fileName,
	     strerror( errno));
    return false;
  }
  yyout = stderr;
  return true;
} /* end function ::openFile */


/*
****************************************
**
** Function: ::closeFile
**
** Closes the currently open status file.
**
** Return value:
**	none
*/
void closeFile()
{
  fclose( yyin);
  return;
} /* end function ::closeFile */
