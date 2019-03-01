/*************************************************************************
 *   @<title> ConfigInfo </title>@
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

#ifndef	_ConfigInfo_h_
#define _ConfigInfo_h_

#include <map>
#include "NodeInfo.h"

using namespace std;

typedef map<int, NodeInfo *>	NodeList;

class ConfigInfo
{
 public:
  static ConfigInfo	*instantiate( const char *fileName);
  static ConfigInfo	*getInstance();
		~ConfigInfo();
  bool		getLocation( int id, double &x, double &y);
  bool		getNeighbor( int id, int &neighbor);
//  bool		getEvent( int id, double &time, SimEvent &eType);
  void		addNode( NodeInfo *);
  bool		isNeighbor( int from, int to);
  void		dumpConnections( double maxX, double maxY);
  int		getNumNodes();
  bool		getNodeState( int id, SimEvent &event);
  bool		bringNodeUp( int id);
  bool		bringNodeDown( int id);
  bool		toggleNodeState( int id);
  bool		firstEvent( double &time, int &node, SimEvent &event);
  bool		nextEvent( double &time, int &node, SimEvent &event);
 private:
		ConfigInfo();
  void		parse( const char *fileName);
  NodeInfo	*getNode( int id);
  static ConfigInfo	*instance;
  NodeList	nodeList;
  NodeList::iterator	nIter;
  bool		callFirst;
};

#endif	// _ConfigInfo_h_

/* end file ConfigInfo.h */
