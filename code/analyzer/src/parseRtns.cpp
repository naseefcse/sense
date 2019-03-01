/*************************************************************************
 *   @<title> parseRtns.cpp </title>@
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

#include "parseRtns.h"
#include "SimInfo.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

const int	BadAddr = -999;

SimInfo		simInfo;
NodeInfo	*workingNode = NULL;
int		workingNodeAddr = BadAddr;
CTInfo		*workingCTEntry = NULL;
int		workingCTAddr = BadAddr;
SenderList	senderList;
NodeList	nodeList;

extern "C"
void initSimInfo()
{
  simInfo.numNodes = 0;
  simInfo.backOff = BOT_SSR;
  simInfo.continuous = false;
  simInfo.routeRepair = false;
  simInfo.hopCounts = 0;
  simInfo.slotWidth = 0.0;
  simInfo.transitionTime = 0.0;
  simInfo.maxCounter = 0;
  return;
}

/*
** Functions for setting SimInfo values.
*/

extern "C"
void setNumNodes(
  int		n)
{
  simInfo.numNodes = n;
  return;
}

extern "C"
void setBackOff(
  BackOffType	bot)
{
  simInfo.backOff = bot;
  return;
}

extern "C"
void setContinuous(
  int		boolVal)
{
  simInfo.continuous = (boolVal == 1);
  return;
}

extern "C"
void setRouteRepair(
  int		boolVal)
{
  simInfo.routeRepair = (boolVal == 1);
  return;
}

extern "C"
void setHopCounts(
  int		hc)
{
  simInfo.hopCounts = hc;
  return;
}

extern "C"
void setSlotWidth(
  int		sw)
{
  simInfo.slotWidth = sw;
  return;
}

extern "C"
void setTransitionTime(
  float		tt)
{
  simInfo.transitionTime = tt;
  return;
}

extern "C"
void setMaxCounter(
  int		mc)
{
  simInfo.maxCounter = mc;
  return;
}

/*
** Functions to start and finish the processing of a node.
*/

extern "C"
void initNode()
{
  assert( workingNode == NULL);
  workingNode = new NodeInfo();
  return;
}

extern "C"
void saveNode()
{
  assert( workingNode != NULL);
  assert( workingNodeAddr != BadAddr);
  nodeList[ workingNodeAddr] = workingNode;
  workingNode = NULL;
  workingNodeAddr = BadAddr;
  return;
}

extern "C"
void setLocX(
  float)
{
  return;
}

extern "C"
void setLocY(
  float)
{
  return;
}

extern "C"
void setAddr(
  int		val)
{
  assert( workingNodeAddr == BadAddr);
  workingNodeAddr = val;
  return;
}

extern "C"
void setSeqNumber(
  int)
{
  return;
}

extern "C"
void setForwardDelay(
  float)
{
  return;
}

extern "C"
void setRXThresh(
  float)
{
  return;
}

extern "C"
void setAckWindow(
  float)
{
  return;
}

extern "C"
void setMaxResend(
  int)
{
  return;
}

extern "C"
void setTimeToLive(
  int)
{
  return;
}

extern "C"
void setAdditionalHop(
  int)
{
  return;
}

extern "C"
void setTotalDelay(
  float)
{
  return;
}

extern "C"
void setTotalSamples(
  int)
{
  return;
}

extern "C"
void setTotalHop(
  int)
{
  return;
}

extern "C"
void setSentPkts(
  int)
{
  return;
}

extern "C"
void setRecvPkts(
  int)
{
  return;
}

extern "C"
void setRecvUniPkts(
  int)
{
  return;
}

extern "C"
void setRecvDataPkts(
  int)
{
  return;
}

extern "C"
void setSentSubopt(
  int)
{
  return;
}

extern "C"
void setCanceledPkts(
  int)
{
  return;
}

extern "C"
void setCanceledSubopt(
  int)
{
  return;
}

extern "C"
void initCTEntry(
  int		addr)
{
  assert( workingCTEntry == NULL);
  assert( workingNode != NULL);
  workingCTEntry = new CTInfo();
  workingCTAddr = addr;
  return;
}

extern "C"
void saveCTEntry()
{
  assert( workingCTEntry != NULL);
  assert( workingCTAddr != BadAddr);
  assert( workingNode != NULL);
  workingNode->costTable[ workingCTAddr] = workingCTEntry;
  workingCTEntry = NULL;
  workingCTAddr = BadAddr;
  return;
}

extern "C"
void setCTCurrentHC(
  int		hc)
{
  assert( workingCTEntry != NULL);
  workingCTEntry->currentHC = hc;
  return;
}

extern "C"
void setCTState(
  CTState	state)
{
  assert( workingCTEntry != NULL);
  workingCTEntry->state = state;
  return;
}

extern "C"
void setCTPendingHC(
  int		hc)
{
  assert( workingCTEntry != NULL);
  workingCTEntry->pendingHC = hc;
  return;
}

extern "C"
void setCTUpdateCtr(
  int		ctr)
{
  assert( workingCTEntry != NULL);
  workingCTEntry->updateCtr = ctr;
  return;
}

/*
** Start and stop processing of a list of integers
*/

enum ListType
{
  LT_Undefined,
  LT_Senders,
  LT_Neighbors
};

ListType	whichList = LT_Undefined;


extern "C"
void printIntListError(
  const char	*str)
{
  const char	*sstr;

  switch( whichList)
  {
    case LT_Undefined:
      sstr = "Undefined";
      break;
    case LT_Senders:
      sstr = "Senders";
      break;
    case LT_Neighbors:
      sstr = "Neighbors";
      break;
    default:			// not used, but keeps gcc quiet
      sstr = "unused";
      break;
  }      
  printf( "Bad Integer List state (%s) when attempting to change %s", sstr,
	  str);
  return;
}
  
extern "C"
void startSenders()
{
  if( whichList != LT_Undefined)
  {
    printIntListError( "to Senders");
    assert( 0);
  }
  whichList = LT_Senders;
  return;
}

extern "C"
void endSenders()
{
  if( whichList != LT_Senders)
  {
    printIntListError( "from Senders");
    assert( 0);
  }
  whichList = LT_Undefined;
  return;
}

extern "C"
void startNeighborList()
{
  if( whichList != LT_Undefined)
  {
    printIntListError( "to Neighbors");
    assert( 0);
  }
  whichList = LT_Neighbors;
  return;
}

extern "C"
void endNeighborList()
{
  if( whichList != LT_Neighbors)
  {
    printIntListError( "from Neighbors");
    assert( 0);
  }
  whichList = LT_Undefined;
  return;
}

extern "C"
void gotInteger(
  int		val)
{
  switch( whichList)
  {
    case LT_Undefined:
      printf( "Got an integer, but list not defined\n");
      assert( 0);
      break;
    case LT_Senders:
      senderList.insert( val);
      break;
    case LT_Neighbors:
      workingNode->neighborList.insert( val);
      break;
  }
  return;
}

