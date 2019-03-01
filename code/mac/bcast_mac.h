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


#ifndef broadcast_mac_h
#define broadcast_mac_h
#include <map>

template <class PLD>
struct BcastMAC_Struct
{
    // for frame format
    enum 
    {
    	MAC_ProtocolVersion=0x00,
	    MAC_Type_Management=0x00,
	    MAC_Type_Control=0x01,
	    MAC_Type_Data=0x02,
	    MAC_Type_Reserved=0x03,   
	    MAC_Subtype_RTS=0x0b,
	    MAC_Subtype_CTS=0x0c,
	    MAC_Subtype_ACK=0x0d,
	    MAC_Subtype_Data=0x00
    };

    struct frame_control 
    {
        u_char          fc_subtype              : 4;
        u_char          fc_type                 : 2;
        u_char          fc_protocol_version     : 2;

        u_char          fc_order                : 1;
        u_char          fc_wep                  : 1;
        u_char          fc_more_data            : 1;
        u_char          fc_pwr_mgt              : 1;
        u_char          fc_retry                : 1;
        u_char          fc_more_frag            : 1;
        u_char          fc_from_ds              : 1;
        u_char          fc_to_ds                : 1;
    };

    struct hdr_struct
    {
	// fields defined in 802.11 standard
        struct frame_control    dh_fc;
        u_int16_t               dh_duration;
        ether_addr_t            dh_da;
        ether_addr_t            dh_sa;
        ether_addr_t            dh_bssid;
        u_int16_t               dh_scontrol;

	// extra fields to keep track of certain information.
	// They should not be counted when calculating the length of the packet
	double tx_time;                              // packet transmission time
	double wave_length;                          // needed to calculate path loss
	unsigned int size;

        bool dump(std::string& str) const;
    };

    typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

    // physical layer parameters
    static double SlotTime;
    static double SIFSTime;
    //static int PreambleLength;
    //static int PLCPHeaderLength;


    // lengths of various frames or headers
    static int PLCP_HDR_LEN;
    static int HDR_LEN;
    static int FCS_LEN;
    
    // (Gang) my understanding is that unicast data frames are sent at
    // DataRate, while all other frames are sent at BasicRate
    static double DataRate;
    static double BasicRate;
    static double CPThreshold;

};

// define values for simulation related parameters
template <class PLD> double BcastMAC_Struct<PLD>::SlotTime=0.000020;
//template <class PLD> int BcastMAC_Struct<PLD>::PreambleLength=144;
//template <class PLD> int BcastMAC_Struct<PLD>::PLCPHeaderLength=48;

template <class PLD> int BcastMAC_Struct<PLD>::PLCP_HDR_LEN = (144+48) >> 3;
template <class PLD> int BcastMAC_Struct<PLD>::HDR_LEN = PLCP_HDR_LEN + FCS_LEN + 4*ether_addr_t::LENGTH + 6;
template <class PLD> int BcastMAC_Struct<PLD>::FCS_LEN = 4;

template <class PLD> double BcastMAC_Struct<PLD>::DataRate=1.0e6/8;
template <class PLD> double BcastMAC_Struct<PLD>::BasicRate=1.0e6/8;
template <class PLD> double BcastMAC_Struct<PLD>::CPThreshold=10;


template <class PLD>
component BcastMAC : public TypeII, public BcastMAC_Struct<PLD>
{
 public:
    enum MAC_STATE {
      MAC_IDLE        = 0x0000,   // nothing to do
      MAC_DEFER       = 0x0001,   // has a packet to send
      MAC_SEND        = 0x0002,   // is sending a packet
    };

    ether_addr_t	MyEtherAddr;
    bool	DumpPackets;
    bool	Promiscuity;
    double	IFS;
    long	SentPackets;
    long	RecvPackets;
    long	rrCollisions;

    // Frame control fields

    // this is the real declaration of the packet format.  hdr_struct
    // is the header added by the 802.11 protocols, and PLD is the
    // payload needed to be transmitted.

    typedef triple<PLD,unsigned int,double> net_inf_t;

    // ports and timers
    // packets from higher layer
    inport void from_network( PLD const& pld, unsigned int size, double backoff);
    inport void cancel ();
    outport void to_network_data ( PLD pld, double receive_power );
    outport void to_network_ack ( bool ack);
    // the cxx compiler doesn't support #ifdef
    outport void to_network_recv_recv_coll( PLD pld1, PLD pld2);
    
    inport void from_phy (packet_t* pkt, bool errflag, double power);
    outport void to_phy (packet_t* pkt);

    Timer <trigger_t> defer_timer;                   // for backoff
    Timer <trigger_t> recv_timer;                    // for receiving a packet
    Timer <trigger_t> send_timer;
    Timer <bool> ack_timer;

    BcastMAC();
    virtual ~BcastMAC();
        
    void	Start();		// called when simulation starts
    void	Stop();			// called when simulation stops
    void	clearStats() { SentPackets = 0; RecvPackets = 0; rrCollisions = 0;}

    // functions to be bound to timers. Just from the name
    // you can tell which timer it is bound to.
    inport void DeferTimer(trigger_t&);
    inport void RecvTimer(trigger_t&);
    inport void SendTimer(trigger_t&);
    inport void AckTimer(bool&);

 private:

    void RecvData(packet_t* p);

    void           StartDefer();
    inline void    ResumeDefer();

    // drop a packet and tell why
    void DropPacket(packet_t* p, const char*);

    MAC_STATE m_state;       // MAC's current state

    struct {
        payload_t pld;
        unsigned int size;
        double delay;
    }m_pldinf;    // containing info about the pld to be transmitted
    simtime_t m_tx_end;   // the end tx time of last packet
    struct {
        packet_t* packet;
        bool errflag;
        double power;
    } m_rxinf;// information about the packet being received
    u_int16_t m_seq_no;      // sequence number
    simtime_t m_defer_start;
    bool m_sr_collision;

    // a STL container containing pairs of address and sequence
    // number.  It is used to remove duplicate data packet.  The
    // container is sorted so the address has to be comparable
    typedef std::map<ether_addr_t, u_int16_t, ether_addr_t::compare> cache_t;
    cache_t m_recv_cache;
};

template <class PLD>
BcastMAC<PLD>::BcastMAC()
{
  connect defer_timer.to_component, DeferTimer;
  connect recv_timer.to_component, RecvTimer;
  connect send_timer.to_component, SendTimer;
  connect ack_timer.to_component, AckTimer;
}

template <class PLD>
BcastMAC<PLD>::~BcastMAC()
{
}

template <class PLD>
void BcastMAC<PLD>::Start()
{
  // perform some initialization tasks
  m_state=MAC_IDLE;
  m_tx_end=0.0;
  m_seq_no=0;
    
  SentPackets=0l;
  RecvPackets=0l;
  rrCollisions = 0L;
}

template <class PLD>
void BcastMAC<PLD>::Stop()
{
}

template <class PLD>
void BcastMAC<PLD>::from_network(
  PLD const	&pld,
  unsigned int	size,
  double	backoff_delay)
{
  Printf( (DumpPackets, "mac%d backoff delay: %f\n", (int) MyEtherAddr,
	   backoff_delay));

  // a new payload packet to be transmitted arrived from the higher layer
  if(m_state!=MAC_IDLE||m_tx_end>SimTime())
  {
    packet_t::free_pld(pld);
    // only when mac is in MAC_IDLE can it transmit a payload packet. The
    // below is to tell the higher layer that transmission has
    // failed. In all cases an ack must be sent, whether it is a
    // success or a failure
    assert(ack_timer.Active()==false);
    ack_timer.Set( false, SimTime());
    return;
  }
    
  m_pldinf.pld=pld;
  m_pldinf.size=size;
  m_pldinf.delay=backoff_delay;
  m_seq_no++;

  StartDefer();
}

template <class PLD>
void BcastMAC<PLD>::DeferTimer(
  trigger_t	&)
{
  // deferral timer expires. It is now time to send out the packet

  if(m_state!=MAC_DEFER)
  {
    Printf((DumpPackets,"mac%d not in defer state \n",(int)MyEtherAddr));
  }
  assert(m_state==MAC_DEFER);
  Printf((DumpPackets,"mac%d deferal timer fires \n",(int)MyEtherAddr));
  packet_t * np = packet_t::alloc();              // creates a new packet

  // simply send out the packet without an RTS
  np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
  np->hdr.dh_fc.fc_type             = MAC_Type_Data;
  np->hdr.dh_fc.fc_subtype          = MAC_Subtype_Data;
  np->hdr.dh_da                     = ether_addr_t::BROADCAST;
  np->hdr.dh_sa                     = MyEtherAddr;
  np->hdr.dh_scontrol               = m_seq_no;

  np->hdr.size = m_pldinf.size + HDR_LEN;
  np->hdr.tx_time = np->hdr.size / DataRate;
  np->pld = m_pldinf.pld;

  // it is a broadcast packet. just send it out and no need for ack
  np->hdr.tx_time = np->hdr.size /BasicRate;
  np->hdr.dh_duration = 0;
  // send an ack to network layer, after a delay equal to the transmission time
  m_tx_end=SimTime() + np->hdr.tx_time;
  m_sr_collision=false;
  send_timer.Set(m_tx_end);
  m_state=MAC_SEND;

  Printf((DumpPackets,"mac%d sends %s until %f\n",(int)MyEtherAddr,np->dump().c_str(),m_tx_end));
  to_phy(np);
  SentPackets++;
}

template <class PLD>
void BcastMAC<PLD>::SendTimer(trigger_t&)
{
  m_state=MAC_IDLE;
  to_network_ack(!m_sr_collision);
}

template <class PLD>
void BcastMAC<PLD>::AckTimer(
  bool		&flag)
{
  m_state=MAC_IDLE;
  to_network_ack(flag);
  return;
}


template <class PLD>
void BcastMAC<PLD>::cancel()
{
  if(m_state==MAC_DEFER)
  {
    defer_timer.Cancel();
    m_state=MAC_IDLE;
    packet_t::free_pld(m_pldinf.pld);
    assert(ack_timer.Active()==false);
    ack_timer.Set(true,SimTime());
  }
}

template <class PLD>
void BcastMAC<PLD>::from_phy(
  packet_t	*pkt,
  bool		errflag,
  double	power)
{
  RecvPackets++;
  m_sr_collision=true;

  // A packet has arrived from the physical layer. At this time, the
  // first bit of the packet is just starting transmission

  // if the recv timer is active then another receive is in progress
  if( recv_timer.Active() )
  {
    packet_t* rx_packet = m_rxinf.packet;
	
    // if the power of the new packet is high enough, discard the old packet
    if( power > m_rxinf.power * CPThreshold)
    {
      DropPacket(m_rxinf.packet,"power too weak");
      m_rxinf.packet=pkt;
      m_rxinf.errflag=errflag;
      m_rxinf.power=power;
      recv_timer.Set(SimTime()+pkt->hdr.tx_time);
    }
    // otherwise, if the old packet was powerful enough, drop the new packet
    else if (m_rxinf.power > power * CPThreshold)
    {
      DropPacket(pkt, "power too weak");
    }
    // otherwise, power levels are too close, so drop both packets
    else
    {
      packet_t	*p1 = pkt, *p2 = m_rxinf.packet;
      simtime_t end_time = SimTime() + pkt->hdr.tx_time;
      // to decide which lasts longer. The longer one takes the recv timer.
      if( end_time > recv_timer.GetTime() )
      {
	recv_timer.Set(end_time);
	DropPacket(rx_packet, "receive-receive collision");
	m_rxinf.packet=pkt;
	m_rxinf.errflag=errflag;
	m_rxinf.power=power;
      }
      else
      {
	DropPacket(pkt,"receive-receive collision");
      }
      rrCollisions++;
      m_rxinf.errflag = true;		// set the error flag to true
      to_network_recv_recv_coll( p1->pld, p2->pld);	// pass collision to routing layer
//      to_network_recv_recv_coll( pkt->pld, m_rxinf.packet->pld);	// pass collision to routing layer
    }
  }
  else
  {
    m_rxinf.packet=pkt;
    m_rxinf.errflag=errflag;
    m_rxinf.power=power;
    // the recv timer expires as soon as the receive is done
    recv_timer.Set(pkt->hdr.tx_time + SimTime());
    if(m_state==MAC_DEFER)
    {
      // if it is in the MAC_DEFER state, defer_timer must be
      // active. Have to defer it.
      assert(defer_timer.Active());
      // pt is the elapsed time after deferral starts
      defer_timer.Cancel();
      m_pldinf.delay -= SimTime() - m_defer_start;
    }
  }
}

template <class PLD>
void BcastMAC<PLD>::RecvTimer(
  trigger_t	&)
{
  // recv timer expires, meaning a packet has just been completely
  // received.  Previously in the FromPhy function if there are
  // outgoing backoff activity then it had been suspended
  // (deferred). Therefore on every exit of this function, it must
  // be checked by calling ResumeDefer() whether there is any
  // outgoing payload packet to be sent

  packet_t* p=m_rxinf.packet;

  Printf((DumpPackets,"mac%d receives %s (%e,%d)\n",(int)MyEtherAddr,p->dump().c_str(),m_rxinf.power,m_rxinf.errflag));

  if(m_rxinf.errflag==1)  // if there is any error
  {  
    DropPacket(p, "frame error");
    ResumeDefer();
    return ;
  }
  simtime_t now = SimTime();
  // m_tx_end is the time last packet had been sent
  if( now - p->hdr.tx_time < m_tx_end )
  {
    // receive-send collision
    DropPacket(p,"receive-send collision");
    ResumeDefer();
    return;
  }

  u_int8_t type = p->hdr.dh_fc.fc_type;
  u_int8_t subtype = p->hdr.dh_fc.fc_subtype;  

  switch(type)
  {
    case MAC_Type_Management:
      DropPacket(p, "unknown MAC packet");
      break;
    case MAC_Type_Control:
      DropPacket(p, "not a DATA packet");
      break;
    case MAC_Type_Data:
      switch(subtype)
      {
        case MAC_Subtype_Data:
	  ResumeDefer();
	  RecvData(p);
	  return;
        default:
	  fprintf(stderr, "mac%d recv_timer(2):Invalid MAC Data Subtype %x\n",(int)MyEtherAddr,
		  subtype);
	  break;
      }
      break;
    default:
      fprintf(stderr, "mac%d recv_timer(3):Invalid MAC Type %x\n", (int)MyEtherAddr,subtype);
      break;
  }
  ResumeDefer();
}

template <class PLD>
void BcastMAC<PLD>::RecvData(
  packet_t	*p)
{
  // maintain a database of latest sequence numbers received
  // from each neighbor
  cache_t::iterator src=m_recv_cache.find(p->hdr.dh_sa);
  if(src!=m_recv_cache.end())
  {
    // src->second is the latest sequence number
    // recently seen from the same source
    if(src->second==p->hdr.dh_scontrol)
    {
      // duplicate data packets. It seems that the source
      // did not receive the ack in the last round of 
      // transmissions
      DropPacket(p,"duplicated DATA packet");
      return;
    }
    else
    {
      // update the sequence number
      src->second=p->hdr.dh_scontrol;
    }
  }
  else
  {
    // source not found in the neighbor list, so add it to the cache
    m_recv_cache.insert(make_pair(p->hdr.dh_sa,p->hdr.dh_scontrol));
  }

  // increase the reference count of the payload by one. This is to
  // claim that I now partly own the payload. The ownership will be
  // transferred to the hight layer.
  p->inc_pld_ref();
  // forward the payload contained in the data packet to higher layer.
  to_network_data(p->pld,m_rxinf.power);
  p->free(); // discard the data packet;
}

template <class PLD>
void BcastMAC<PLD>::StartDefer()
{
  // if the state is MAC_DEFER then ResumeDefer instead of this function must be called
  assert(m_state==MAC_IDLE);
  assert(defer_timer.Active()==false);

  Printf((DumpPackets, "mac%d deferring\n",(int)MyEtherAddr));
  // if the recv timer is active then do nothing except changing 
  // the state to MAC_DEFER, because it is guaranteed that at the
  // end of RecvTimer, backoff must be resumed.
  if(!recv_timer.Active())                    
  {
    m_defer_start = SimTime();
    defer_timer.Set( m_defer_start+m_pldinf.delay);
  }
  m_state=MAC_DEFER;
}

template <class PLD>
void BcastMAC<PLD>::ResumeDefer()
{
  if(m_state!=MAC_DEFER)return;

  assert(defer_timer.Active()==false);
  m_defer_start=SimTime();
  defer_timer.Set(m_defer_start+m_pldinf.delay+IFS);
  Printf((DumpPackets, "mac%d resumes deferring: %f \n",(int)MyEtherAddr,m_pldinf.delay));
}

template <class PLD>
void BcastMAC<PLD>::DropPacket(
  packet_t	*p,
  const char	*reason)
{
  Printf((DumpPackets,"mac%d drops %s (%s)\n",(int)MyEtherAddr,p->dump().c_str(),reason));
  p->free();
}

template <class PLD>
bool BcastMAC_Struct<PLD>::hdr_struct::dump(
  std::string	&str) const
{
  std::string type;
  bool payload=false;
  switch(dh_fc.fc_type)
  {
    case BcastMAC_Struct<PLD>::MAC_Type_Control:
      switch(dh_fc.fc_subtype)
      {
	case BcastMAC_Struct<PLD>::MAC_Subtype_RTS:
	  type="RTS";
	  break;
	case BcastMAC_Struct<PLD>::MAC_Subtype_CTS:
	  type="CST";
	  break;
	case BcastMAC_Struct<PLD>::MAC_Subtype_ACK:
	  type="ACK";
	  break;
	default:
	  type="UNKNOWN";
      }
      break;
    case BcastMAC_Struct<PLD>::MAC_Type_Data:
      if(dh_fc.fc_subtype == BcastMAC_Struct<PLD>::MAC_Subtype_Data)
      {
	payload=true;
	if(dh_da!=ether_addr_t::BROADCAST)
	  type="DATA";
	else
	  type="BROADCAST";
      }
      else
	type="UNKNOWN";
      break;
    default:
      type="UNKNOWN";
      break;
  }
  char buffer[100];
  sprintf(buffer,"%f %s %d -> %d", tx_time, type.c_str(), (int)(dh_sa), (int)(dh_da));
  str=buffer;
  return payload;
}

#endif /* broadcast_mac_h */
