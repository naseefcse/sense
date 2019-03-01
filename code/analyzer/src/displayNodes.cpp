#include <algorithm>
#include "SimInfo.h"
#include "usage.h"
#include "file.h"
#include "commonRtns.h"
#include <stdio.h>

using namespace std;

const char		usage[] = {"<statusFileName>\n"};

int main(
  int		argc,
  char		*argv[])
{
  if( argc != 2)
    printUsage( argv[0], usage);
  openFile( argv[1]);
  parse();
  closeFile();

  printf( "Sender list (%d)...\n", senderList.size());
  for_each( senderList.begin(), senderList.end(), printInt);
  printf( "\n");

  printf( "Node list (%d)...\n", nodeList.size());
  for_each( nodeList.begin(), nodeList.end(), printNodeInfo);

  return 0;
}
