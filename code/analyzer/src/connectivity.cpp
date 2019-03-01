/*************************************************************************
 *   @<title> connectivity.cpp </title>@
 *
 *   @<!-- Copyright 2006 Mark Lisee, Boleslaw K. Szymanski and Rensselaer
 *   Polytechnic Institute. All worldwide rights reserved.  A license to use,
 *   copy, modify and distribute this software for non-commercial research
 *   purposes only is hereby granted, provided that this copyright notice and
 *   accompanying disclaimer is not modified or removed from the software.
 *
 *   DISCLAIMER: The software is distributed "AS IS" without any express or
 *   implied warranty, including but not limited to, any implied warranties of
 *   merchantability or fitness for a particular purpose or any warranty of
 *   non-infringement of any current or pending patent rights. The authors of
 *   the software make no representations about the suitability of this
 *   software for any particular purpose. The entire risk as to the quality
 *   and performance of the software is with the user. Should the software
 *   prove defective, the user assumes the cost of all necessary servicing,
 *   repair or correction. In particular, neither Rensselaer Polytechnic
 *   Institute, nor the authors of the software are liable for any indirect,
 *   special, consequential, or incidental damages related to the software,
 *   to the maximum extent the law permits.-->@
 *
 *************************************************************************/

/*
** Generate, one per line, a pair of node ids that show the connectivity in
** topology. The format of the output is suitable for input to another utility.
**
** pseudo code:
** parse file
** for each node
**   for each neighbor
**     print nodeId neighborId
*/
#include <algorithm>
#include "SimInfo.h"
#include "usage.h"
#include "file.h"
#include "commonRtns.h"
#include <stdio.h>

using namespace std;

void	processNode( const NodeList::value_type info);

const char	usage[] = {"<statusFileName> <nodesFileName> <topoFileName>\n"};

struct location
{
  float	x, y;
};

typedef	map<int, location *>	locInfo;

locInfo		locations;
FILE		*topoFile;

int main(
  int		argc,
  char		*argv[])
{

  if( argc != 4)
    printUsage( argv[0], usage);

  // parse status file
  openFile( argv[1]);
  parse();
  closeFile();

  if( nodeList.size() == 0)
    return 1;

  // Use nodes file to create mapping of node Id to location
  FILE		*fp = fopen( argv[2], "r");
  if( fp == 0)
  {
    printf( "Unable to open file %s\n", argv[2]);
    return 1;
  }
  char		line[200];
  int		id;
  location	*lp;
  while( fgets( line, sizeof( line)-1, fp) != 0)
  {
    if( line[0] == '#')
      continue;
    lp = new location;
    if( lp == NULL)
    {
      printf( "out of memory\n");
      return 1;
    }
    sscanf( line, "%d %f %f", &id, &lp->x, &lp->y);
    locations.insert( make_pair(id, lp));
  }
  fclose( fp);

  // process each node
  topoFile = fopen( argv[3], "w");
  if( topoFile == NULL)
  {
    printf( "Unable to open file %s\n", argv[2]);
    return 1;
  }
  for_each( nodeList.begin(), nodeList.end(), processNode);
  fclose( topoFile);
  // good programming style says to free memory here
  return 0;
}

void processNode(
  const NodeList::value_type	info)
{
  int		nodeId = info.first;
  NodeInfo	*nodeInfo = info.second;
  NeighborList::iterator	nIter;
  locInfo::iterator		src, dest;
  float		srcX, srcY;
  int		srcId;

  src = locations.find( nodeId);
  srcX = (src->second)->x;
  srcY = (src->second)->y;
  srcId = src->first;

  // for each neighbor:
  nIter = nodeInfo->neighborList.begin();
  for( ; nIter != nodeInfo->neighborList.end(); nIter++)
  {
    dest = locations.find( *nIter);
//    fprintf( topoFile, "%3d -> %3d; ", srcId, *nIter);
    fprintf( topoFile, "set arrow from %f, %f to %f, %f nohead lt 5\n", srcX, srcY,
	     (dest->second)->x, (dest->second)->y);
  }
  return;
}

/* end file findNeighbors.cpp */

