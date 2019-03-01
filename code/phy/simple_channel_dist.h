
/*************************************************************************
*   Copyright 2006 Mark Lisee, Boleslaw K. Szymanski and
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
* simple_channel_dist.h
* This component is a idealization of a radio. If two nodes are within a
* specific distance, then the packet arrives with full power. Beyond this
* distance, the packet does not arrive.
*
* Author: Mark Lisee
* Date: 6-28-2006
*************************************************************************/

#ifndef simple_channel_dist_h
#define simple_channel_dist_h

#include <algorithm>
#include <stdio.h>

template <class PACKET>

component SimpleChannelDist : public TypeII
{
 public:
  bool		initSources( int numSources, bool oneSink);
  void		getSourcePair( int &src, int &dest);

  inport void		from_phy( PACKET *pkt, double power, int id);
  outport[] void	to_phy( PACKET *pkt, double power);
  inport void		pos_in( const coordinate_t& pos, int id);
	
  InfiTimer< triple<PACKET*, double, int> > propagation_delay;
  inport void	depart( const triple<PACKET*, double, int> & data, unsigned int );

		SimpleChannelDist();
  virtual	~SimpleChannelDist() {}
  void		Start();
  void		Stop();
  void		Setup();

  void		setNumNodes( int n)		{ NumNodes = n; }
  void		setDumpPackets( bool flag)	{ DumpPackets = flag; }
  void		setCSThresh( double thresh)	{}
  void		setRXThresh( double thresh)	{}
  void		setWaveLength( double wl)	{ WaveLength= wl; }
  void		useFreeSpace()			{}
  void		useTwoRay()			{}
  void		setX( double x)			{ X = x; }
  void		setY( double y)			{ Y = y; }
  void		setGridEnabled( bool flag)	{}
  void		setMaxTXPower( double pwr)	{}
  void		setDistanceThreshold( double thresh)
					{ distanceThreshold = thresh; }

 private:    
  int		NumNodes;
  bool		DumpPackets;
  double	WaveLength;
  double	X;
  double	Y;
  double	distanceThreshold;
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
SimpleChannelDist<PACKET>::SimpleChannelDist()
{
  connect propagation_delay.to_component, depart;
  return;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::Setup()
{
  m_positions.reserve( NumNodes);
  for(int i = 0; i < NumNodes; i++)
    m_positions.push_back( node_t() );
  to_phy.SetSize( NumNodes);
    
  return;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::Start()
{
  return;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::Stop()
{
  return;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::pos_in(const coordinate_t & pos, int id)
{
  m_positions[id].x = pos.x;
  m_positions[id].y = pos.y;
  return;
}

template <class PACKET>
bool SimpleChannelDist<PACKET>::forward(
  PACKET	*pkt,
  int		in,
  int		out,
  bool		flag)
{
  double	dx, dy, delay, distance;
  double	now = SimTime();

  // calculate time it would take packet to travel to other node
  dx = m_positions[in].x - m_positions[out].x;
  dy = m_positions[in].y - m_positions[out].y;
  distance = sqrt( dx*dx + dy*dy);
  delay = distance / speed_of_light;
   
  // if too far, don't forward
  if( distance > distanceThreshold)
    return false;

  // if testing connectivity, don't forward
  if( flag == false)
    return true;

  // send packet to physical layer; power is hard coded to 1.0
  pkt->inc_ref();
  Printf((DumpPackets,"transmits from %f %f to %f %f\n",
	  m_positions[in].x, m_positions[in].y,
	  m_positions[out].x,m_positions[out].y));
  propagation_delay.Set( make_triple( pkt, 1.0, out), now + delay);
  return true;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::from_phy(
  PACKET	*packet,
  double	,	// power
  int		in)
{   
  for(int out = 0; out < NumNodes; out++)
    if( out != in)
      forward( packet, in, out, true);
  packet->free();
  return;
}

template <class PACKET>
void SimpleChannelDist<PACKET>::depart(
  const triple<PACKET*, double, int>	&data,
  unsigned int	index)
{
  to_phy[ data.third]( data.first, data.second);
  return;
}

template <class PACKET>
bool SimpleChannelDist<PACKET>::initSources(
  int		,	// numSources
  bool		)	// oneSink
{
  return true;		// let channel determine source nodes
}

template <class PACKET>
void SimpleChannelDist<PACKET>::getSourcePair(
  int		&,
  int		&)
{
  return;
}

#endif // simple_channel_dist_h
