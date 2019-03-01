#include <algorithm>
#include <map>
#include "SimInfo.h"
#include "usage.h"
#include "file.h"
#include "commonRtns.h"
#include <stdio.h>

using namespace std;

typedef multimap< int, int>	NodesByHC;

void	processSender( const int);

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

  printf( "Sender list (%d entries)...\n"
	  "Sorted within sender by increasing hop count\n", senderList.size());
  for_each( senderList.begin(), senderList.end(), processSender);

  return 0;
}

void processSender(
  const int	dest)
{
  // create new multimap (hc -> node id)
  NodesByHC	nodesByHC;
  NodeInfo	*node;

  // for each node:
  NodeList::iterator	nodeIter = nodeList.begin();
  for( ; nodeIter != nodeList.end(); nodeIter++)
  {
    int		hc;

    node = nodeIter->second;
    CTList::iterator	ctIter;
  //   look up sender in cost table
    ctIter = node->costTable.find( dest);
  //   if found add (hc, node) to multimap
    if( ctIter != node->costTable.end())
    {
      hc = ctIter->second->currentHC;
      nodesByHC.insert( make_pair( hc, nodeIter->first));
    }
  }

  // display multimap
  NodesByHC::iterator	hcIter = nodesByHC.begin();
  int		lastHC, count;

  printf( "\nSender: %3d\n", dest);
  for( lastHC = -1, count = -1; hcIter != nodesByHC.end(); hcIter++)
  {
    if( hcIter->first != lastHC)
    {
      if( count > 0)
	printf( "(%d)\n", count);
      count = 0;
      lastHC = hcIter->first;
      printf( "%2d: ", lastHC);
    }
    printf( "%3d ", hcIter->second);
    count++;
  }
  printf( "(%d)\n", count);
  return;
}

/* end file sortNodes.cpp */
