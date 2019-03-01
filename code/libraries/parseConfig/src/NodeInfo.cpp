/*************************************************************************
 *   @<title> NodeInfo.cpp </title>@
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

#include "NodeInfo.h"
#include "ConfigInfo.h"
#include "ether_addr.h"		// from sense, required by Visualizer
#include "path.h"		// from sense, required by Visualizer
#include "Visualizer.h"

NodeInfo::NodeInfo() 
    : id( -999), x( 100000.0), y( 100000.0), state( NodeUp)
{
  nIter = neighborList.end();
  eIter = eventList.end();
  return;
}

NodeInfo::~NodeInfo()
{
  neighborList.clear();
  eventList.clear();
  return;
}

void NodeInfo::addNeighbor(
  int		n)
{
  neighborList.insert( n);
  return;
}

void NodeInfo::addEvent(
  double	time,
  SimEvent	event)
{
  eventList.insert( make_pair( time, event));
  return;
}

bool NodeInfo::firstNeighbor(
  int		&neighbor)
{
  nIter = neighborList.begin();
  return nextNeighbor( neighbor);
}

bool NodeInfo::nextNeighbor(
  int		&neighbor)
{
  // if end of list, return false
  if( nIter == neighborList.end())
    return false;

  // get neighbor from iterator (set)
  neighbor = *nIter;
  nIter++;	// point to next neighbor
  return true;
}

bool NodeInfo::firstEvent(
  double	&time,
  SimEvent	&event)
{
  eIter = eventList.begin();
  return nextEvent( time, event);
}

bool NodeInfo::nextEvent(
  double	&time,
  SimEvent	&event)
{
  // if end of list, return false
  if( eIter == eventList.end())
    return false;

  // get event from iterator
  time = eIter->first;
  event = eIter->second;
  eIter++;	// point to next event
  return true;
}

bool NodeInfo::isNeighbor(
  int		n)
{
  return neighborList.find( n) != neighborList.end();
}

/*
** For each node in the neighbor list, tell the visualizer to write a line
** that will draw an arrow from this node's location to the neighbor's
** location.
*/
void NodeInfo::dumpConnections()
{
  NeighborList::iterator	iter = neighborList.begin();
  Visualizer		*vis = Visualizer::instantiate();
  ConfigInfo		*ci = ConfigInfo::getInstance();
  double		destX, destY;

  for( ; iter != neighborList.end(); iter++)
    if( ci->getLocation( *iter, destX, destY) == true)
      vis->addConnection( x, y, destX, destY);
  return;
}

/* end file NodeInfo.cpp */
