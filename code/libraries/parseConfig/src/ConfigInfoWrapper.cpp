/*************************************************************************
 *   @<title> ConfigInfoWrapper.cpp </title>@
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

void parseConfigInfo(
  const char	*fileName)
{
  ConfigInfo::instantiate( fileName);
  return;
}

bool isNeighbor(
  int		from,
  int		to)
{
  return ConfigInfo::getInstance()->isNeighbor( from, to);
}

bool getLocation(
  int		id,
  double	&x,
  double	&y)
{
  return ConfigInfo::getInstance()->getLocation( id, x, y);
}

void dumpConnections(
  double	maxX,
  double	maxY)
{
  return ConfigInfo::getInstance()->dumpConnections( maxX, maxY);
}

int getNumNodes()
{
  return ConfigInfo::getInstance()->getNumNodes();
}

bool getNodeState(
  int		id,
  SimEvent	&event)
{
  return ConfigInfo::getInstance()->getNodeState( id, event);
}

bool bringNodeUp(
  int		id)
{
  return ConfigInfo::getInstance()->bringNodeUp( id);
}

bool bringNodeDown(
  int		id)
{
  return ConfigInfo::getInstance()->bringNodeDown( id);
}

bool toggleNodeState(
  int		id)
{
  return ConfigInfo::getInstance()->toggleNodeState( id);
}

bool firstEvent(
  double	&time,
  int		&node,
  SimEvent	&event)
{
  return ConfigInfo::getInstance()->firstEvent( time, node, event);
}

bool nextEvent(
  double	&time,
  int		&node,
  SimEvent	&event)
{
  return ConfigInfo::getInstance()->nextEvent( time, node, event);
}

const char *eventToString(
  SimEvent	event)
{
  const char	*str;

  switch( event)
  {
    case NodeUp:
      str = "up";
      break;
    case NodeDown:
      str = "down";
      break;
    default:
      str = "unknown";
      break;
  }
  return str;
}

/* end file ConfigInfoWrapper.cpp */
