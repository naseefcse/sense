/*************************************************************************
 *   @<title> parseRtns.h </title>@
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

#ifndef	_parseRtns_h_
#define	_parseRtns_h_

#include "misc.h"

#ifdef	__cplusplus
extern "C"
{
#endif	// __cplusplus
  void	initSimInfo();

  void	setNumNodes( int);
  void	setBackOff( BackOffType);
  void	setContinuous( int boolVal);
  void	setRouteRepair( int boolVal);
  void	setHopCounts( int);
  void	setSlotWidth( int);
  void	setTransitionTime( float);
  void	setMaxCounter( int);

  void	startSenders();
  void	endSenders();

  void	initNode();
  void	saveNode();

  void	setLocX( float);
  void	setLocY( float);
  void	setAddr( int);
  void	setSeqNumber( int);
  void	setForwardDelay( float);
  void	setRXThresh( float);
  void	setAckWindow( float);
  void	setMaxResend( int);
  void	setTimeToLive( int);
  void	setAdditionalHop( int);
  void	setTotalDelay( float);
  void	setTotalSamples( int);
  void	setTotalHop( int);
  void	setSentPkts( int);
  void	setCanceledPkts(int);
  void	setRecvPkts( int);
  void	setRecvUniPkts( int);
  void	setRecvDataPkts( int);
  void	setSentSubopt(int);
  void	setCanceledSubopt(int);

  void	initCTEntry( int);
  void	saveCTEntry();
  void	setCTCurrentHC( int);
  void	setCTState( CTState);
  void	setCTPendingHC( int);
  void	setCTUpdateCtr( int);

  void	startNeighborList();
  void	endNeighborList();
  void	gotInteger( int);
#ifdef	__cplusplus
}
#endif	// __cplusplus

#endif	// _parseRtns_h_
