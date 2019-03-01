/*************************************************************************
 *   @<title> CIParseRtns.cpp </title>@
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

#include "ConfigInfo.h"
#include "CIParseRtns.h"
#include <assert.h>

NodeInfo	*workingNode = NULL;
bool		gotID = false;
bool		gotLocation = false;

extern "C"
void initNode()
{
  assert( workingNode == NULL);
  workingNode = new NodeInfo();
  gotID = false;
  gotLocation = false;
  return;
}

extern "C"
void saveNode()
{
  assert( workingNode != NULL);
  assert( gotID == true);
  assert( gotLocation == true);
  ConfigInfo::getInstance()->addNode( workingNode);
  workingNode = NULL;
  return;
}

extern "C"
void setId(
  int		id)
{
  assert( workingNode != NULL);
  assert( gotID == false);
  workingNode->setId( id);
  gotID = true;
  return;
}

extern "C"
void setLocation(
  double	x,
  double	y)
{
  assert( workingNode != NULL);
  assert( gotLocation == false);
  workingNode->setLocation( x, y);
  gotLocation = true;
  return;
}

extern "C"
void startNeighborList()
{
  return;
}

extern "C"
void endNeighborList()
{
  return;
}

extern "C"
void gotInteger(
  int		val)
{
  assert( workingNode != NULL);
  workingNode->addNeighbor( val);
  return;
}

extern "C"
void startEventList()
{
  return;
}

extern "C"
void endEventList()
{
  return;
}

extern "C"
void gotEvent(
  double	time,
  SimEvent	event)
{
  assert( workingNode != NULL);
  workingNode->addEvent( time, event);
  return;
}

/* end CIParseRtns.cpp */
