#include <algorithm>
#include "SimInfo.h"
#include "usage.h"
#include "file.h"
#include "commonRtns.h"
#include <stdio.h>

using namespace std;

void	processNode( const NodeList::value_type info);

const char	usage[] = {"<statusFileName>\n"};
int		totalNeighbors = 0;

int main(
  int		argc,
  char		*argv[])
{

  if( argc != 2)
    printUsage( argv[0], usage);

  // parse file
  openFile( argv[1]);
  parse();
  closeFile();

  if( nodeList.size() == 0)
    return 1;

  printf( "Node\t# Neighbors\n");
  for_each( nodeList.begin(), nodeList.end(), processNode);
  printf( "Average # neighbors: %f\n", (float) totalNeighbors/nodeList.size());
  return 0;
}

void processNode(
  const NodeList::value_type	info)
{
  NodeInfo	*nodeInfo = info.second;
  int		numNeighbors = nodeInfo->neighborList.size();

  printf( "%3d\t%3d\n", info.first, numNeighbors);
  totalNeighbors += numNeighbors;
  return;
}

/* end file findNeighbors.cpp */

