/*************************************************************************
 *   @<title> CIParseRtns.h </title>@
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

#ifndef	_CIParseRtns_h_
#define	_CIParseRtns_h_

#include "SimEvent.h"

#ifdef	__cplusplus
extern "C"
{
#endif	// __cplusplus

  void	initNode();
  void	saveNode();

  void	setId( int id);
  void	setLocation( double x, double y);

  void	startNeighborList();
  void	endNeighborList();
  void	gotInteger( int val);

  void	startEventList();
  void	endEventList();
  void	gotEvent( double time, SimEvent event);

#ifdef	__cplusplus
}
#endif	// __cplusplus

#endif	// _CIParseRtns_h_

/* end CIParseRtns.h */
