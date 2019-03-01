/*************************************************************************
 *   @<title> NodeInfo.h </title>@
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

#ifndef	_NodeInfo_h_
#define	_NodeInfo_h_

#include <map>
#include <set>
#include "SimEvent.h"

using namespace std;

typedef set<int>	NeighborList;

// map time to event
typedef	map<double, SimEvent>	EventList;

class NodeInfo
{
 public:
  NodeInfo();
  ~NodeInfo();
  void		setId( int ID)	{ id = ID; }
  int		getId() const	{ return id; }
  void		setLocation( double xin, double yin)	{ x = xin; y = yin; }
  void		addNeighbor( int n);
  void		addEvent( double time, SimEvent event);

  void		getLocation( double &xout, double &yout) const
			{ xout = x; yout = y; return; }
  bool		firstNeighbor( int &neighbor);
  bool		nextNeighbor( int &neighbor);
  bool		firstEvent( double &time, SimEvent &event);
  bool		nextEvent( double &time, SimEvent &event);
  bool		isNeighbor( int id);
  void		dumpConnections();
  SimEvent	getState() const	{ return state; }
  void		bringUp()		{ state = NodeUp; }
  void		bringDown()		{ state = NodeDown; }
  void		toggleState()		{ if( state == NodeUp) state = NodeDown; else state = NodeUp; }
 private:
  int		id;
  double	x;
  double	y;
  SimEvent	state;
  NeighborList	neighborList;
  NeighborList::iterator	nIter;
  EventList	eventList;
  EventList::iterator		eIter;
};

#endif	// _NodeInfo_h_

/* end file NodeInfo.h */
