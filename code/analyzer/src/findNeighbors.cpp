/*************************************************************************
 *   @<title> findNeighbors.cpp </title>@
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

#include <algorithm>
#include "SimInfo.h"
#include "usage.h"
#include "file.h"
#include "commonRtns.h"
#include <stdio.h>

using namespace std;

typedef multimap< int, int>	NodesByHC;

void	processNode( const NodeList::value_type info);
void	processSender( const int);
void	displayNeighbors( const int node, const int dest, bool);

const char	usage[] = {"<n|s> <statusFileName>\n"
			   "n = Use node as major index, sender as minor index\n"
			   "s = Use sender as major index, node as minor index\n"};

int main(
  int		argc,
  char		*argv[])
{
  bool		byNode = true;

  if( argc != 3)
    printUsage( argv[0], usage);
  if( *argv[1] == 'n')
    byNode = true;
  else if( *argv[1] == 's')
    byNode = false;
  else
    printUsage( argv[0], usage);

  // parse file
  openFile( argv[2]);
  parse();
  closeFile();

  if( byNode == true)
  {
    printf( "Node list (%d entries)...\nSorted within node by sender\n"
	    "Sorted within sender by increasing hop count\n", nodeList.size());
    for_each( nodeList.begin(), nodeList.end(), processNode);
  }
  else
  {
    printf( "Sender list (%d entries)...\nSorted within sender by node\n"
	    "Sorted within node by increasing hop count\n", senderList.size());
    for_each( senderList.begin(), senderList.end(), processSender);
  }
  return 0;
}

void processNode(
  const NodeList::value_type	info)
{
  SenderList::iterator	iter = senderList.begin();
  printf( "---------- Node %3d ----------\n", info.first);
  for( ; iter != senderList.end(); iter++)
    displayNeighbors( info.first, *iter, true);
  return;
}

void processSender(
  const int	dest)
{
  NodeList::iterator	iter = nodeList.begin();
  printf( "---------- Sender %3d ----------\n", dest);
  for( ; iter != nodeList.end(); iter++)
    displayNeighbors( iter->first, dest, false);
  return;
}

void displayNeighbors(
  const int	node,
  const int	dest,
  bool		sender)
{
  // create new multimap (hc -> node id)
  NodesByHC	nodesByHC;
  NodeInfo	*nodeInfo, *neighborInfo;
  int		nodeHC;
  CTList::iterator	ctIter;

  nodeInfo = (nodeList.find( node))->second;
  ctIter = nodeInfo->costTable.find( dest);
  if( ctIter == nodeInfo->costTable.end())
    return;
  nodeHC = nodeInfo->costTable.find( dest)->second->currentHC;
  if( sender == true)
    printf( "----- Sender %3d -----\n", dest);
  else
    printf( "----- Node %3d -----\n", node);
    
  printf( "Hop Count %d\n", nodeHC);

  // for each neighbor:
  NeighborList::iterator	nIter = nodeInfo->neighborList.begin();
  for( ; nIter != nodeInfo->neighborList.end(); nIter++)
  {
    int		hc, neighbor;

    neighbor = *nIter;
  //   look up sender in neighbor's cost table
    neighborInfo = nodeList.find( neighbor)->second;
    ctIter = neighborInfo->costTable.find( dest);
  //   if found add (hc, node) to multimap
    if( ctIter != neighborInfo->costTable.end())
    {
      hc = ctIter->second->currentHC;
      nodesByHC.insert( make_pair( hc, neighbor));
    }
    else
      nodesByHC.insert( make_pair( 999, neighbor));
  }

  // display multimap
  NodesByHC::iterator	hcIter = nodesByHC.begin();
  int		lastHC, count;

  for( lastHC = -1, count = -1; hcIter != nodesByHC.end(); hcIter++)
  {
    if( hcIter->first != lastHC)
    {
      if( count > 0)
	printf( "(%d)\n", count);
      count = 0;
      lastHC = hcIter->first;
      printf( "%3d: ", lastHC);
    }
    printf( "%3d ", hcIter->second);
    count++;
  }
  printf( "(%d)\n", count);
  return;
}

/* end file findNeighbors.cpp */

