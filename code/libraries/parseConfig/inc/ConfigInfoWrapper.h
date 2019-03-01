/*************************************************************************
 *   @<title> ConfigInfoWrapper.h </title>@
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

#ifndef	_ConfigInfoWrapper_h_
#define	_ConfigInfoWrapper_h_

#include "SimEvent.h"

bool		isNeighbor( int from, int to);
void		parseConfigInfo( const char *fileName);
bool		getLocation( int id, double &x, double &y);
int		getNumNodes();
void		dumpConnections( double maxX, double maxY);
bool		getNodeState( int id, SimEvent &event);
bool		bringNodeUp( int id);
bool		bringNodeDown( int id);
bool		toggleNodeState( int id);
// The returned events are not sorted by time
bool		firstEvent( double &time, int &node, SimEvent &event);
bool		nextEvent( double &time, int &node, SimEvent &event);
const char	*eventToString( SimEvent);

#endif	// _ConfigInfoWrapper_h_

/* end file ConfigInfoWrapper.h */
