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

#ifndef transceiver_loc_h
#define transceiver_loc_h


/************************************************

  Class: DuplexTransceiverLoc
  Template Parameter: PACKET

*************************************************/

template <class PACKET>
component DuplexTransceiverLoc : public TypeII
{
 public:
  void		setTXPower( double)	{}
  double	getTXPower()		{ return 1.0; }
  void		setTXGain( double)	{}
  void		setRXGain( double)	{}
  void		setRXThresh( double)	{}
  double	getRXThresh()		{ return 1.0; }
  void		setCSThresh( double)	{}
  double	getCSThresh()		{ return 1.0; }
  void		setFrequency( double freq)	{ Frequency = freq; }
  double	getFrequency()		{ return Frequency; }
  void		setID( int id)		{ ID = id; }

  inport inline void	from_mac( PACKET *pkt);    
  outport void		to_mac( PACKET *pkt, bool errflag, unsigned int size);
  inport inline void	from_channel( PACKET *pkt, double power);
  outport void		to_channel( PACKET *pkt, double power, int id);
  outport double	to_power_switch( int state, double time);
    
  void		Start();
  void		Stop();
 private:
  double	Frequency;
  int		ID;
};

template <class PACKET>
void DuplexTransceiverLoc<PACKET>::Start()
{
  return;
}

template <class PACKET>
void DuplexTransceiverLoc<PACKET>::Stop()
{
  return;
}

template <class PACKET>
void DuplexTransceiverLoc<PACKET>::from_mac(
  PACKET	*pkt)
{
  double now = SimTime();
  if( to_power_switch( PM_Struct::TX, now) <= 0.0)  //switch the state to TX
  {
    pkt->free();
    return;
  }	 
   
  if( to_power_switch( PM_Struct::IDLE, now + pkt->hdr.tx_time) <= 0.0)
  {
    pkt->free();
    return;
  } 
  pkt->hdr.wave_length = speed_of_light / Frequency;
  // transmit power set to 1.0
  to_channel( pkt, 1.0, ID);
  return;
}

template <class PACKET>
void DuplexTransceiverLoc<PACKET>::from_channel(
  PACKET	*pkt,
  double	power)
{
  double now = SimTime();

  if( to_power_switch( PM_Struct::RX, now) <= 0.0)  //switch the state to RX
  {
    pkt->free();
    return;
  }
          
  if( to_power_switch( PM_Struct::IDLE, now + pkt->hdr.tx_time) <= 0.0)
  {
    pkt->free();
    return;
  } 
  
  // error set to false and power set to 1.0
  to_mac( pkt, false, 1.0);
  return;
}

#endif /* transceiver_loc_h */
