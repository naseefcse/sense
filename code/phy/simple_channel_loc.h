
/*************************************************************************
*   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
*   Rensselaer Polytechnic Institute. All worldwide rights reserved.
*   A license to use, copy, modify and distribute this software for
*   non-commercial research purposes only is hereby granted, provided
*   that this copyright notice and accompanying disclaimer is not
*   modified or removed from the software.
*
*   DISCLAIMER: The software is distributed "AS IS" without any
*   express or implied warranty, including but not limited to, any
*   implied warranties of merchantability or fitness for a particular
*   purpose or any warranty of non-infringement of any current or
*   pending patent rights. The authors of the software make no
*   representations about the suitability of this software for any
*   particular purpose. The entire risk as to the quality and
*   performance of the software is with the user. Should the software
*   prove defective, the user assumes the cost of all necessary
*   servicing, repair or correction. In particular, neither Rensselaer
*   Polytechnic Institute, nor the authors of the software are liable
*   for any indirect, special, consequential, or incidental damages
*   related to the software, to the maximum extent the law permits.
*************************************************************************/

/*************************************************************************
*	simple_channel_loc.h
*	This is the same code as simple_channel.h. However, it enforces
*	node connectivity based on an adjacency matrix from an input file
*
*	Author: Joel Branch
*	Date: 6-24-2006
*************************************************************************/

#ifndef simple_channel_loc_h
#define simple_channel_loc_h

#include <algorithm>
#include <stdio.h>
#include "../libraries/parseConfig/inc/SimEvent.h"

template <class PACKET>

component SimpleChannelLoc : public TypeII
{
 public:
  bool		initSources( int numSources, bool oneSink);
  void		getSourcePair( int &src, int &dest);

  inport void		from_phy( PACKET *pkt, double power, int id);
  outport[] void	to_phy( PACKET *pkt, double power);
  inport void		pos_in( const coordinate_t& pos, int id);
	
  // data values: packet, power, destination
  InfiTimer< triple<PACKET*, double, int> > propagation_delay;
  inport void	depart( const triple<PACKET*, double, int> & data, unsigned int);

  // data values: node id, event
  InfiTimer< pair< int, SimEvent>>	nodeStateChange;
  inport void	doNodeStateChange( const pair< int, SimEvent> &data, unsigned int);

		SimpleChannelLoc();
  virtual	~SimpleChannelLoc() {}
  void		Start();
  void		Stop();
  void		Setup();
  void		setNumNodes( int n)		{ NumNodes = n; }
  void		setDumpPackets( bool flag)	{ DumpPackets = flag; }
  void		setCSThresh( double)		{}
  void		setRXThresh( double)		{}
  void		setWaveLength( double wl)	{ WaveLength= wl; }
  void		useFreeSpace()			{}
  void		useTwoRay()			{}
  void		setX( double x)			{ X = x; }
  void		setY( double y)			{ Y = y; }
  void		setGridEnabled( bool)		{}
  void		setMaxTXPower( double)		{}
  void		setDistanceThreshold( double)	{}

 private:    
  int		NumNodes;
  bool		DumpPackets;
  double	WaveLength;
  double	X;
  double	Y;
  int		nextSource;
  int		nextDestination;
  bool		sinkFlag;
  bool		forward( PACKET *pkt, int in, int out, bool flag);
  struct node_t : public coordinate_t
  {
    node_t() : coordinate_t( 0.0, 0.0), prev(NULL), next(NULL) {};
    node_t	*prev;
    node_t	*next;
  };
  std::vector< node_t > m_positions;
};

template <class PACKET>
SimpleChannelLoc<PACKET>::SimpleChannelLoc()
{
  NumNodes = 0;
  DumpPackets = false;
  WaveLength = 0;
  X = 0;
  Y = 0;
  nextSource = 0;
  nextDestination = 0;
  sinkFlag = false;
  connect propagation_delay.to_component, depart;
  connect nodeStateChange.to_component, doNodeStateChange;
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::Setup()
{
  bool		status;
  int		node;
  double	time;
  SimEvent	event;

  m_positions.reserve( NumNodes);
  for(int i = 0; i < NumNodes; i++)
    m_positions.push_back( node_t());
  to_phy.SetSize( NumNodes);

  // process events listed in configuration file
  for( status = firstEvent( time, node, event); status == true;
       status = nextEvent( time, node, event))
  {
    printf( "SCL::Setup <%f, %d, %s>\n", time, node, eventToString( event));
    nodeStateChange.Set( make_pair( node, event), time);
  }
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::Start()
{
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::Stop()
{
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::pos_in(const coordinate_t & pos, int id)
{
  m_positions[id].x = pos.x;
  m_positions[id].y = pos.y;
  return;
}

template <class PACKET>
bool SimpleChannelLoc<PACKET>::forward(
  PACKET	*pkt,
  int		in,
  int		out,
  bool		justTesting)
{
  SimEvent	event;

  assert( getNodeState( out, event) == true);
  if( event != NodeUp)
  {
//    printf( "SCL::f: ea %d; node can't receive %08X \n", out, (unsigned int) pkt);
    return false;
  }
  if( isNeighbor( in, out) == false)
    return false;
  if( justTesting == false)
    return true;		// don't actually forward

  double	dx, dy, delay;
  double	now = SimTime();

  // calculate time it would take packet to travel to other node
  dx = m_positions[in].x - m_positions[out].x;
  dy = m_positions[in].y - m_positions[out].y;
  delay = sqrt( dx*dx + dy*dy) / speed_of_light;
   
  // send packet to physical layer; power is hard coded to 1.0
  pkt->inc_ref();
  Printf((DumpPackets,"transmits from %f %f to %f %f\n",
	  m_positions[in].x, m_positions[in].y,
	  m_positions[out].x, m_positions[out].y));
  propagation_delay.Set( make_triple( pkt, 1.0, out), now + delay);
  return true;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::from_phy(
  PACKET	*packet,
  double	,	// power
  int		in)
{   
  SimEvent	event;

  assert( getNodeState( in, event) == true);
  if( event == NodeUp)
  {
    for(int out = 0; out < NumNodes; out++)
      if( out != in)
	forward( packet, in, out, true);
  }
  else
    printf( "SCL::fp: ea %d; node can't send\n", in);
  packet->free();
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::depart(
  const triple<PACKET*, double, int>	&data,
  unsigned int	index)
{
  to_phy[ data.third]( data.first, data.second);
  return;
}

/*
** This method is called by the top level component to notify the channel that
** the master wants the ids of the source and destination nodes.
*/
template <class PACKET>
bool SimpleChannelLoc<PACKET>::initSources(
  int		numSources,
  bool		oneSink)
{
  if( (oneSink == true && numSources + 1 > NumNodes) ||
      (oneSink == false && numSources * 2 > NumNodes))
  {
    printf( "***** There are more sources than nodes\n");
    assert( 0);
  }
  nextSource = 0;
  nextDestination = numSources;
  sinkFlag = oneSink;
  return false;		// component will determine source nodes
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::getSourcePair(
  int		&src,
  int		&dest)
{
  src = nextSource++;
  dest = nextDestination;
  if( sinkFlag == false)
    nextDestination++;
  return;
}

template <class PACKET>
void SimpleChannelLoc<PACKET>::doNodeStateChange(
  const pair<int, SimEvent>	&data,
  unsigned int	index)
{
  int		id = data.first;
  SimEvent	event = data.second;

  switch( event)
  {
    case NodeUp:
      printf( "SCL::dNSC: ea %3d; node coming up @ %f\n", id, SimTime());
      bringNodeUp( id);		// see ConfigInfoWrapper.cpp
      break;
    case NodeDown:
      printf( "SCL::dNSC: ea %3d; node going down @ %f\n", id, SimTime());
      bringNodeDown( id);
      break;
  }
  return;
}

#endif /* simple_channel_loc_h */
