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
*       transceiver_dist.h
*       This code determines node connectivity using only using distance;
*       not the radio model
*
*       Altered by Joel Branch
*       June 28, 2006
*************************************************************************/

#ifndef transceiver_h
#define transceiver_h


/************************************************

  Class: DuplexTransciever
  Template Parameter: PACKET

*************************************************/
template <class PACKET>
component DuplexTransceiver  : public TypeII
{
 public:
    
    //double TXConsumed;        // power consumption for transmission (W)
    //double RXConsumed;        // power consumption for reception (W)
    //double IdleConsumed;      // idle power consumption (W)
    //double SleepConsumed;     // sleep power consumption (W)
     
    double TXPower;
    double TXGain;
    double RXGain;
    double Frequency;

    double RXThresh;       // receive power threshold (W)
    double CSThresh;       // carrier sense threshold (W)
    int ID;
   
    // added by Joel Branch - 6/28/2006
    double distance_threshold;

	inport inline void from_mac (PACKET* p);    
	outport void to_mac (PACKET* p, bool errflag, unsigned int size);
    
    inport inline void from_channel (PACKET* p, double power);
    outport void to_channel (PACKET* p, double power, int id);

	outport double to_power_switch ( int state, double time);
    
    void Start();
    void Stop();

};

template <class PACKET>
void DuplexTransceiver<PACKET>::Start()
{
}

template <class PACKET>
void DuplexTransceiver<PACKET>::Stop()
{
}

template <class PACKET>
void DuplexTransceiver<PACKET>::from_mac(PACKET* p)
{
	double now = SimTime();
	if(to_power_switch(PM_Struct::TX,now)<=0.0)  //switch the state to TX
    {
    	p->free();
		return;
    }	 
   
    if(to_power_switch(PM_Struct::IDLE,now+p->hdr.tx_time)<=0.0)
    {
    	p->free();
    	return;
    } 
    p->hdr.wave_length=speed_of_light/Frequency;
    to_channel(p,TXPower * TXGain, ID);
}

template <class PACKET>
void DuplexTransceiver<PACKET>::from_channel(PACKET* p, double power)
{
	double now = SimTime();
    if(to_power_switch(PM_Struct::RX,now)<= 0.0)  //switch the state to RX
    {
       p->free();
       return;
    }
          
    if(to_power_switch(PM_Struct::IDLE, now+p->hdr.tx_time)<=0.0)
    {
		p->free();
		return;
    } 
  
    double recv_power = power * RXGain;
    // outer if-else statement added by Joel Branch - 6-28-2006
    if(enable_distane_threshold == false)
    {
    	if( recv_power < CSThresh )
    	{
    		p->free(); // cannot detect. drop it.
    	}
    	else
    	{
		bool error = ( recv_power < RXThresh ) ;
		to_mac( p, error, recv_power );
    	}
    }
    else
    {
	to_mac(p, false, recv_power);
    }
}

#endif /* transceiver_h */


