/*************************************************************************
 *   @<title> ConfigInfo.cpp </title>@
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

#include <cstdio>
#include <assert.h>
#include "ConfigInfo.h"
#include "file.h"
#include "ether_addr.h"		// from sense, required by Visualizer
#include "path.h"		// from sense, required by Visualizer
#include "Visualizer.h"

extern "C" void parse();

ConfigInfo	*ConfigInfo::instance = NULL;

ConfigInfo *ConfigInfo::instantiate(
  const char	*fileName)
{
  if( instance == NULL)
    instance = new ConfigInfo();
  instance->parse( fileName);
  return instance;
}

ConfigInfo *ConfigInfo::getInstance()
{
  return instance;
}

ConfigInfo::ConfigInfo() 
    : callFirst( false)
{
  nIter = nodeList.end();
  return;
}

void ConfigInfo::parse(
  const char	*fileName)
{
  if( openFile( fileName) == false)
    assert( 0);
  ::parse();
  closeFile();
  return;
}

ConfigInfo::~ConfigInfo()
{
  NodeList::iterator	iter;

  for( iter = nodeList.begin(); iter != nodeList.end(); iter++)
  {
    delete (*iter).second;
    nodeList.erase( iter);
  }
  return;
}

NodeInfo *ConfigInfo::getNode(
  int		id)
{
  // look id up in map
  NodeList::iterator	nodeIter = nodeList.find( id);
  // if not found, return false
  if( nodeIter == nodeList.end())
    return NULL;
  return (*nodeIter).second;
}

bool ConfigInfo::getNeighbor(
  int		id,
  int		&neighbor)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  return node->nextNeighbor( neighbor);
}

void ConfigInfo::addNode(
  NodeInfo	*node)
{
  nodeList.insert( make_pair( node->getId(), node));
  return;
}

bool ConfigInfo::isNeighbor(
  int		from,
  int		to)
{
  NodeInfo	*node = getNode( from);
  if( node == NULL)
    return false;
  return node->isNeighbor( to);
}

bool ConfigInfo::getLocation(
  int		id,
  double	&x,
  double	&y)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  node->getLocation( x, y);
  return true;
}

int ConfigInfo::getNumNodes()
{
  return nodeList.size();
}

void ConfigInfo::dumpConnections(
  double	maxX,
  double	maxY)
{
  NodeList::iterator	iter = nodeList.begin();
  Visualizer		*vis = Visualizer::instantiate();

  vis->startConnectFile( maxX, maxY);
  for( ; iter != nodeList.end(); iter++)
    ((*iter).second)->dumpConnections();
  vis->endConnectFile();
  return;
}

bool ConfigInfo::getNodeState(
  int		id,
  SimEvent	&event)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  event = node->getState();
  return true;
}

bool ConfigInfo::bringNodeUp(
  int		id)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  node->bringUp();
  return true;
}

bool ConfigInfo::bringNodeDown( int id)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  node->bringDown();
  return true;
}

bool ConfigInfo::toggleNodeState( int id)
{
  NodeInfo	*node = getNode( id);
  if( node == NULL)
    return false;
  node->toggleState();
  return true;
}

bool ConfigInfo::firstEvent(
  double	&time,
  int		&node,
  SimEvent	&event)
{
  nIter = nodeList.begin();
  callFirst = true;

  return nextEvent( time, node, event);
}

bool ConfigInfo::nextEvent(
  double	&time,
  int		&id,
  SimEvent	&event)
{
  NodeInfo	*node;
  bool		status =  false;	// by default, event not found

  for( ;;)
  {
    // if end of list, end loop
    if( nIter == nodeList.end())
      break;
    // get node from iterator
    node = nIter->second;
    if( callFirst == true)
      status = node->firstEvent( time, event);
    else
      status = node->nextEvent( time, event);
    if( status == true)
      break;		// got event, end loop
    // this node has no more events, so try next node
    nIter++;
    callFirst = true;
  }
  // was an event found?
  if( status == false)
    return false;
  // found an event
  callFirst = false;	// there may be another event associated with this node
  id = node->getId();
  return true;
}

/* end file ConfigInfo.cpp */
