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

#ifndef mac80211_h
#define mac80211_h
#include <map>

template <class PLD>
struct MAC80211_Struct
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

    // Frame control fields
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
        frame_control    	dh_fc;
        uint16_t            dh_duration;
        ether_addr_t        dh_da;
        ether_addr_t        dh_sa;
        ether_addr_t        dh_bssid;
        uint16_t            dh_scontrol;

		// extra fields to keep track of certain information.
		// They should not be counted when calculating the length of the packet
		double tx_time;                              // packet transmission time
		double wave_length;                          // needed to calculate path loss
		unsigned int size;

        bool dump(std::string& str) const
        {
            std::string type;
            bool payload=false;
            switch(dh_fc.fc_type)
            {
            case MAC_Type_Control:
	            switch(dh_fc.fc_subtype)
	            {
	            case MAC_Subtype_RTS:
	                type="RTS";
	                break;
	            case MAC_Subtype_CTS:
	                type="CST";
	                break;
	            case MAC_Subtype_ACK:
	                type="ACK";
	                break;
	            default:
	                type="UNKNOWN";
	            }
	            break;
            case MAC_Type_Data:
	            if(dh_fc.fc_subtype == MAC_Subtype_Data)
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
    };	

    typedef smart_packet_t<hdr_struct,PLD> packet_t;
    typedef PLD payload_t;
    
    // physical layer parameters
    static int CWMin;
    static int CWMax;
    static double SlotTime;
    static double SIFSTime;

    // mac layer parameters
    static unsigned int RTSThreshold;
    static unsigned int ShortRetryLimit;
    static unsigned int LongRetryLimit;


    // lengths of various frames or headers
    static int PLCP_HDR_LEN;
    static int HDR_LEN;
    static int RTS_LEN;
    static int CTS_LEN;
    static int ACK_LEN;
    static int FCS_LEN;
    
    // (Gang) my understanding is that unicast data frames are sent at
    // DataRate, while all other frames are sent at BasicRate
    static double DataRate;
    static double BasicRate;
    static double CPThreshold;    
};

// define values for simulation related parameters
template <class PLD> int MAC80211_Struct<PLD>::CWMin=31;
template <class PLD> int MAC80211_Struct<PLD>::CWMax=1023;
template <class PLD> double MAC80211_Struct<PLD>::SlotTime=0.000020;
template <class PLD> double MAC80211_Struct<PLD>::SIFSTime=0.000010;

template <class PLD> unsigned int MAC80211_Struct<PLD>::RTSThreshold=3000;
template <class PLD> unsigned int MAC80211_Struct<PLD>::ShortRetryLimit=3;
template <class PLD> unsigned int MAC80211_Struct<PLD>::LongRetryLimit=4;

template <class PLD> int MAC80211_Struct<PLD>::PLCP_HDR_LEN = (144 + 48) >> 3;
template <class PLD> int MAC80211_Struct<PLD>::HDR_LEN = PLCP_HDR_LEN + FCS_LEN + 4*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::RTS_LEN = PLCP_HDR_LEN + FCS_LEN + 2*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::CTS_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::ACK_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::FCS_LEN = 4;

template <class PLD> double MAC80211_Struct<PLD>::DataRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::BasicRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::CPThreshold=10; 

/*
 * MAC80211 requires a template parameter PLD, which specifies
 * the type of payload that will be transmitted by the mac layer. In
 * particular, PLD could be a plain of data packet, or a pointer
 * to a data packet. The underlying packet management handles both
 * cases elegantly.
 */

template <class PLD>
component MAC80211 : public TypeII, public MAC80211_Struct<PLD>
{
 public:

    int SentPackets,RecvPackets;

    enum MAC_STATE {
        MAC_IDLE        = 0x0000,   // nothing to do
        MAC_DEFER       = 0x0001,   // has a packet to send
        MAC_CTS         = 0x0002,   // waiting for CTS
        MAC_ACK         = 0x0004,   // waiting for ACK
    };

    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    bool Promiscuity;

 	inport void from_network ( payload_t const & pld, const ether_addr_t& dst, unsigned int size );
 	outport void to_network_data ( payload_t& pld, const ether_addr_t & dst);
 	outport void to_network_ack ( bool errflag );
 	
 	inport void from_phy ( packet_t* pkt, bool errflag, double  power );
 	outport void to_phy (packet_t* pkt);
    
    Timer <trigger_t> defer_timer;                   // for backoff
    Timer <trigger_t> recv_timer;                    // for receiving a packet
    Timer <MAC80211_Struct<PLD>::packet_t*> cts_timer;                     // for CTSTimeout
    Timer <MAC80211_Struct<PLD>::packet_t*> ack_timer;                     // for ACKTimeout
    Timer <trigger_t> nav_timer;                     // for NAV
    
    Timer <bool> network_ack_timer;
    inport void NetworkAckTimer(bool&);
    Timer <MAC80211_Struct<PLD>::packet_t*> phy_timer;
    inport void PhyTimer(packet_t*&);
    
    void Start();                                    // called when simulation starts
    void Stop();                                     // called when simulation stops
    MAC80211();
	virtual ~MAC80211();

    // functions to be bound to inports and timers. Just from the name
    // you can tell which inport or timer it is bound to.
    inport void DeferTimer(trigger_t&);
    inport void RecvTimer(trigger_t&);
    inport void CTSTimer(packet_t*&);
    inport void ACKTimer(packet_t*&);
    inport void NAVTimer(trigger_t&);

 private:

    // functions to be called by others
    void RecvRTS(packet_t* p);
    void RecvCTS(packet_t* p);
    void RecvData(packet_t* p);
    void RecvACK(packet_t* p);

    // We have two TxPacket functions: one starts transmission
    // immediately, the other will wait for a specified delay
    inline void TxPacket(packet_t* p, simtime_t tx_time);
    inline void TxPacket(packet_t* p, simtime_t start_time, simtime_t tx_time);

    // In StartDefer(), the mac state changes from MAC_IDLE to
    // MAC_DEFER; a new backoff time must be sampled. In
    // ResumeDefer(), the mac state should already be MAC_DEFER
    // (otherwise the function just exits); the old value of backoff
    // time will be used.
    void           StartDefer(bool backoff);
    inline void    ResumeDefer();

    // drop a packet and tells why
    void DropPacket(packet_t* p, const char*);
    // convert from sec to usec; copied from ns2
    uint16_t usec(simtime_t t) { return (uint16_t)floor((t *= 1e6) + 0.5); }

    MAC_STATE m_state;       // MAC's current state
    double   m_nav;          // Network Allocation Vector
    int      m_cw;           // Contention Window
    double   m_backoff_time; // Backoff Time

    unsigned int m_ssrc;         // STA Short Retry Count
    unsigned int m_slrc;         // STA Long Retry Count
    double   m_sifs;         // Short Interframe Space
    double   m_difs;         // DCF Interframe Space
    double   m_eifs;         // Extended Interframe Space

    double   m_ifs;          // equal to either m_difs or m_eifs
    double   m_defer_start;  // when deferral started

    bool     m_in_session;   // if involved in an RTS/CTS session
    ether_addr_t m_session_peer; // who initiated the session

	struct {
		payload_t pld;
		ether_addr_t dst_addr;
		unsigned int size;
	} m_pldinfo;
	struct {
		packet_t * packet;
		bool error;
		double power;
	} m_rxinfo; 
	
    bool     m_tx_failed;    // if last transmission succeeded
    bool     m_long_pld;     // if the length of current pld >= RTSThreshold
    simtime_t m_last_send;   // the end tx time of last packet
    uint16_t m_seq_no;      // sequence number

    // a STL container containing pairs of address and sequence
    // number.  It is used to remove duplicate data packet.  The
    // container is sorted so the address has to be comparable
    typedef std::map<ether_addr_t, uint16_t, ether_addr_t::compare> cache_t;
    cache_t m_recv_cache;
};

template <class PLD>
MAC80211<PLD>::MAC80211()
{
	connect defer_timer.to_component, DeferTimer;
	connect recv_timer.to_component, RecvTimer;
	connect cts_timer.to_component, CTSTimer;
	connect ack_timer.to_component, ACKTimer;
	connect nav_timer.to_component, NAVTimer;
	
	connect network_ack_timer.to_component, NetworkAckTimer;
	connect phy_timer.to_component, PhyTimer;
}

template <class PLD>
MAC80211<PLD>::~MAC80211()
{
	
}

template <class PLD>
void MAC80211<PLD>::Start()
{
    // perform some initialization tasks
    m_state=MAC_IDLE;
    m_nav=0.0;
    m_cw=CWMin;

    m_last_send=0.0;

    m_ssrc=0;
    m_slrc=0;
    m_sifs=SIFSTime;
    m_difs=m_sifs+SlotTime;
    m_eifs=m_sifs+ACK_LEN/DataRate+m_difs;

    m_tx_failed=false;
    m_seq_no=0;

    SentPackets=RecvPackets=0;
}

template <class PLD>
void MAC80211<PLD>::Stop()
{
    //printf("%s: sent %d, recv %d\n",GetName(),SentPackets,RecvPackets);
}

template <class PLD>
void MAC80211<PLD>::from_network ( payload_t const& pld, const ether_addr_t& dst_addr, unsigned int size )
{
    // a new payload packet to be transmitted arrived from the higher layer
    if(m_state!=MAC_IDLE)
    {
		Printf((DumpPackets,"mac%d state: %d, m_last_send: %f, simtime: %f\n",(int)MyEtherAddr,
			m_state,m_last_send,SimTime()));
		packet_t::free_pld(pld);
		// only when mac is in MAC_IDLE can it transmit a payload packet. The
		// below is to tell the higher layer that transmission has
		// failed. In all cases an ack must be sent, whether it is a
		// success or a failure
		to_network_ack(false);
		return;
    }
    if(size < RTSThreshold || dst_addr == ether_addr_t::BROADCAST )
    {
		m_long_pld=false;
		m_ssrc=0;
    }
    else
    {
		m_long_pld=true;
		m_slrc=0;
    }
    
    m_pldinfo.pld=pld;
    m_pldinfo.dst_addr=dst_addr;
    m_pldinfo.size=size;
    m_seq_no++;

    // start the defer timer without backoff. This is because
    // if the medium is still free after difs or eifs then
    // the packet can be sent immediately without backoff
    StartDefer(false);
}
template <class PLD>
void MAC80211<PLD>::DeferTimer(trigger_t&)
{
    // deferral timer expires. It is now time to send out the packet

    assert(m_state==MAC_DEFER);
    Printf((DumpPackets,"mac%d deferal timer fires \n", (int)MyEtherAddr));
    packet_t * np = packet_t::alloc();              // creates a new packet
    if(m_long_pld==false)
    {
		// simply send out the packet without an RTS
		np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
		np->hdr.dh_fc.fc_type             = MAC_Type_Data;
		np->hdr.dh_fc.fc_subtype          = MAC_Subtype_Data;
		np->hdr.dh_da                     = m_pldinfo.dst_addr;
		np->hdr.dh_sa                     = MyEtherAddr;
		np->hdr.dh_scontrol               = m_seq_no;

    	np->hdr.size = m_pldinfo.size + HDR_LEN;
		np->hdr.tx_time = np->hdr.size / DataRate;
		np->pld = m_pldinfo.pld;
		if( np->hdr.dh_da != ether_addr_t::BROADCAST)
		{
	    	// this function increases the reference count of the packet by 1,
	    	// virtually meaning this mac component partly owns the packet (together
	    	// with any other components that will receive this packet, because of
	    	// the use of reference counting. This mac component will regain the packet
	    	// when the ack_timer expires (an ACKTimeout event).
	    	np->inc_ref();
	    	np->hdr.dh_duration = usec (m_sifs+ACK_LEN/BasicRate + np->hdr.size/DataRate);
	    	// this function sets the ack time. Notice the first argument which is used
	    	// to the packet that will be sent out. When the timer expires, whatever stored
	    	// here will be returned
	    	ack_timer.Set(np, np->hdr.dh_duration*1e-6 + m_difs + SimTime() );
	    	m_state = MAC_ACK;          // waiting for ack
		}
		else
		{
	    	// it is a broadcast packet. just send it out and no need for ack
	    	np->hdr.tx_time = np->hdr.size /BasicRate;
	    	np->hdr.dh_duration = 0;
	    	// send an ack to network layer, after a delay equal to the transmission time
	    	assert(!network_ack_timer.Active());
	    	network_ack_timer.Set(true,SimTime()+np->hdr.tx_time);
	    	m_state = MAC_IDLE;
		}
    }
    else
    {
		// np is now an RTS packet. Again, this mac component partly owns the packet
		np->inc_ref();
		np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
		np->hdr.dh_fc.fc_type             = MAC_Type_Control;
		np->hdr.dh_fc.fc_subtype          = MAC_Subtype_RTS;
		np->hdr.dh_da                     = m_pldinfo.dst_addr;
		np->hdr.dh_sa                     = MyEtherAddr;

		np->hdr.size                      = RTS_LEN;                     
		np->hdr.tx_time                   = np->hdr.size/BasicRate;
		np->hdr.dh_duration = usec ( 3*m_sifs + CTS_LEN/BasicRate + 
				     (m_pldinfo.size + HDR_LEN)/DataRate 
				     + ACK_LEN/BasicRate );
		// set the cts timer and change state to MAC_CTS
		cts_timer.Set(np,np->hdr.dh_duration*1e-6 + m_sifs + SimTime());
		m_state = MAC_CTS;
    }

    // send out the packet immediately. The second argument is the transmission time
    TxPacket(np,np->hdr.tx_time);
}

template <class PLD>
void MAC80211<PLD>::from_phy ( packet_t* pkt, bool errflag, double  power )

{
    // A packet has arrived from the physical layer. At this time, the first 
    // bit of the packet is just starting transmission
    // printf("[%f] mac%d receiving %s (%e %d)\n",SimTime(),(int)MyEtherAddr,pkt->dump().c_str(),power,errflag);

    Printf((DumpPackets,"mac%d start receiving %s (%e,%d)\n",(int)MyEtherAddr,pkt->dump().c_str(),power,errflag));

    // if the recv timer is active then another receive is in progress
    if( recv_timer.Active() )
    {
		packet_t* rx_packet = m_rxinfo.packet;
	
		if( power > m_rxinfo.power * CPThreshold)
		{
	    	DropPacket(m_rxinfo.packet,"power too weak");
	    	m_rxinfo.packet=pkt;
	    	m_rxinfo.error=errflag;
	    	m_rxinfo.power=power;
	    	recv_timer.Set(SimTime()+pkt->hdr.tx_time);
		}
		else if (m_rxinfo.power > power * CPThreshold)
		{
	    	DropPacket(pkt, "power too weak");
		}
		else
		{
	    	simtime_t end_time = SimTime() + pkt->hdr.tx_time;
	    	// to decide which lasts longer. The longer one takes the recv timer.
	    	if( end_time > recv_timer.GetTime() )
	    	{
				recv_timer.Set(end_time);
				DropPacket(rx_packet, "receive-receive collision");
		    	m_rxinfo.packet=pkt;
		    	m_rxinfo.error=errflag;
	    		m_rxinfo.power=power;
	    	}
	    	else
	    	{
				DropPacket(pkt,"receive-receive collision");
	    	}
	    	m_rxinfo.error=1;   // set the error flag to 1
		}
    }
    else
    {
    	m_rxinfo.packet=pkt;
    	m_rxinfo.error=errflag;
   		m_rxinfo.power=power;
		// the recv timer expires as soon as the receive is done
		recv_timer.Set(pkt->hdr.tx_time + SimTime());
	    if(m_state==MAC_DEFER)
	    {
	        // if it is in the MAC_DEFER state, defer_timer must be
	        // active. Have to defer it.
	        assert(defer_timer.Active());
	        // pt is the elapsed time after deferral starts
	        double pt=SimTime()-m_defer_start;
	        if(pt>m_ifs)
	        {
		        // pt-m_ifs is the time we have already spent
		        // in the backoff period. Need to adjust
		        // m_backoff_time accordingly
		        m_backoff_time-= pt-m_ifs;
		        assert(m_backoff_time>=0);
    	    }
	        defer_timer.Cancel();
	        Printf((DumpPackets,"mac%d deferral suspended\n",(int)MyEtherAddr));
	    }
    }
}

template <class PLD>
void MAC80211<PLD>::RecvTimer(trigger_t&)
{
    // recv timer expires, meaning a packet has just been completely
    // received.  Previously in the FromPhy function if there are
    // outgoing backoff activity then it had been suspended
    // (deferred). Therefore on every exit of this function, it must
    // be checked by calling ResumeDefer() whether there is any
    // outgoing payload packet to be sent

    packet_t* p=m_rxinfo.packet;

    Printf((DumpPackets,"mac%d received %s (%e,%d)\n",(int)MyEtherAddr,p->dump().c_str(),m_rxinfo.power,m_rxinfo.error));
    // printf("[%f] mac%d received %s (%e,%d)\n",SimTime(),(int)MyEtherAddr,p->dump().c_str(),m_rxinfo.power,m_rxinfo.error);

    if(m_rxinfo.error==1)  // if there is any error
    {  
	    DropPacket(p, "frame error");
	    m_tx_failed = true;
	    ResumeDefer();
	    return ;
    }
    simtime_t now = SimTime();
    // m_last_send is the time last packet had been sent
    if( now - p->hdr.tx_time < m_last_send )
    {
	    // receive-send collision
	    DropPacket(p,"frame error");
	    m_tx_failed = true;
        ResumeDefer();
	    return;
    }

    ether_addr_t dst = p->hdr.dh_da;
    uint8_t type = p->hdr.dh_fc.fc_type;
    uint8_t subtype = p->hdr.dh_fc.fc_subtype;  

    // set the nav value
    simtime_t nav = p->hdr.dh_duration*1e-6 + SimTime();
    if(nav>m_nav) m_nav=nav;
    RecvPackets++;
    //printf(" %d %d\n",(int)p->hdr.dh_sa, (int)MyEtherAddr);

    if( dst != MyEtherAddr && dst != ether_addr_t::BROADCAST)
    {
	    if( !Promiscuity || type!= MAC_Type_Data)
  	    { 
            DropPacket(p,"wrong destination");
	        m_tx_failed=false;
	        ResumeDefer();
            return;
	    }
    }

    m_tx_failed=false;  // a transmission has been successful

    switch(type)
    {
    case MAC_Type_Management:
        DropPacket(p, "unknown MAC packet");
        break;
    case MAC_Type_Control:
        switch(subtype)
        {
        case MAC_Subtype_RTS:
            RecvRTS(p);
            ResumeDefer();
            break;
        case MAC_Subtype_CTS:
            RecvCTS(p);
            ResumeDefer();
            break;
        case MAC_Subtype_ACK:
            ResumeDefer();
            RecvACK(p);
	    break;
        default:
            fprintf(stderr, "recv_timer(1):Invalid MAC Control Subtype %x\n",
                    subtype);
            break;
        }
        break;
    case MAC_Type_Data:
        switch(subtype)
        {
        case MAC_Subtype_Data:
            RecvData(p);
	    break;
        default:
            fprintf(stderr, "recv_timer(2):Invalid MAC Data Subtype %x\n",
                    subtype);
            break;
        }
        break;
    default:
        fprintf(stderr, "recv_timer(3):Invalid MAC Type %x\n", subtype);
        break;
    }
}

template <class PLD>
void MAC80211<PLD>::RecvRTS(packet_t* p)
{
    if( m_state != MAC_IDLE && m_state != MAC_DEFER )
    {
	    // sorry, I am waiting for a CTS or an ACK. Clearly something
	    // is wrong. A is sending me an RTS while I am communicating
	    // with B. I just ignore A's request since the communication
	    // with B will perhaps be successful in spite of A's presence
	    DropPacket(p,"mac is busy");
	    return;
    }

    // if nav_timer is active then another session (which may or may
    // not involve this address) is already in the middle, so we have
    // to ignore this packet
    if( !nav_timer.Active() )
    {
	    // prepare a CTS packet
	    packet_t* np = packet_t::alloc();

	    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
	    np->hdr.dh_fc.fc_type       = MAC_Type_Control;
	    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_CTS;

	    np->hdr.size=CTS_LEN;
	    np->hdr.dh_da = p->hdr.dh_sa ;
	    np->hdr.dh_sa=MyEtherAddr;
	    np->hdr.tx_time = np->hdr.size/BasicRate;
	    np->hdr.dh_duration = p->hdr.dh_duration - usec(m_sifs + RTS_LEN/BasicRate);

	    // send out the packet at time SimTime()+m_sifs
	    TxPacket(np,m_sifs,np->hdr.tx_time);
	
	    // record session info
	    m_in_session=true;
	    m_session_peer=p->hdr.dh_sa;
	    assert(m_nav>=SimTime());
	    nav_timer.Set(m_nav);

	    // discard the RTS packet. Other mac components may be still processing
	    // this packet so here only the reference count is decreased by one
	    p->free();  
    }
    else
    {
	    DropPacket(p,"unexpected RTS");
	    return;
    }
}

template <class PLD>
void MAC80211<PLD>::RecvCTS(packet_t* p)
{
    if(m_state != MAC_CTS)
    {
	    // sorry I don't need a CTS. Simply drop the packet and
	    // do nothing.
	    DropPacket(p, "CTS without RTS");
	    return;
    }
    
    // prepare the data packet. Please refer to the same code in
    // Defer() function.
    packet_t* np = packet_t::alloc();

    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
    np->hdr.dh_fc.fc_type       = MAC_Type_Data;
    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_Data;
    np->hdr.dh_da=p->hdr.dh_sa;;
    np->hdr.dh_sa=MyEtherAddr;
    np->hdr.dh_scontrol=m_seq_no;
	
    np->hdr.size=m_pldinfo.size+HDR_LEN;
    np->pld=m_pldinfo.pld;
    np->hdr.tx_time = np->hdr.size/DataRate;
    np->hdr.dh_duration = usec(2*m_sifs+ACK_LEN/BasicRate + np->hdr.size/DataRate);
    m_state = MAC_ACK;
    ack_timer.Set(np, np->hdr.dh_duration*1e-6 + m_difs + SimTime());

    np->inc_ref();  // I need to keep a hand on this packet
    TxPacket(np,m_sifs,np->hdr.tx_time);

    // discard the CTS packet
    p->free();
    // cts timer still stores the RTS packet. Now need to discard it too
    cts_timer.GetData()->free();
    cts_timer.Cancel();  // the CTS packet has been received, so cancels
                         // the CTSTimeout event

    // (Gang) Actually cts timer doesn't need to store the RTS packet.
    // The RTS packet can be immediately freed when the CTS is sent,
    // for it is not used in the CTSTimer() function. The code has
    // been so because of my initial misunderstanding of the protocol.
    // I believed RTS must be retransmitted when CTSTimeout fires. Now
    // I know deferral has to be restarted when this happens (am I
    // right?)

}

template <class PLD>
void MAC80211<PLD>::RecvData(packet_t* p)
{
    if( m_state != MAC_IDLE && m_state != MAC_DEFER )
    {
	    // sorry, I am waiting for a CTS or an ACK
	    DropPacket(p,"mac is busy");
	    return;
    }

    if(p->hdr.dh_da==MyEtherAddr )//  ack is needed
    {
	    // if nav_timer is not active, we can ack the data packet.
	    // Otherwise, we need to check if the current session involves
	    // this address (by looking at m_in_session) and if the sender of
	    // the data packet is the session peer. Only when both conditions
	    // are satisfied can we reply this data packet.
	    if( nav_timer.Active() && !(m_in_session&&p->hdr.dh_sa==m_session_peer) )
	    {
            ResumeDefer();
	        DropPacket(p,"unexpected DATA");
	        return;
	    }
	    
	    packet_t* np = packet_t::alloc();

	    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
	    np->hdr.dh_fc.fc_type       = MAC_Type_Control;
	    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_ACK;

	    np->hdr.dh_da=p->hdr.dh_sa;
	    np->hdr.dh_sa=MyEtherAddr;

	    np->hdr.size = ACK_LEN;
	    np->hdr.tx_time = np->hdr.size /BasicRate;
	    np->hdr.dh_duration = 0;

	    TxPacket(np,m_sifs,np->hdr.tx_time);
    }

    ResumeDefer();

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
    to_network_data(p->pld,p->hdr.dh_da);
    p->free(); // discard the data packet;
}
template <class PLD>
void MAC80211<PLD>::RecvACK(packet_t* p)
{
    if(m_state != MAC_ACK)
    {
	    DropPacket(p, "ACK without DATA");
	    return;
    }
    p->free();                   // discard the ack packet
    ack_timer.GetData()->free(); // discard the data packet
    ack_timer.Cancel(); 
    m_state=MAC_IDLE;            
    m_cw=CWMin;                  // reset the contention window
    to_network_ack(true);  // inform the higher layer that
                                 // the transmission has been successful
}
template <class PLD>
void MAC80211<PLD>::CTSTimer(packet_t*& p)
{
    // discard the RTS packet. It was stored in the cts timer when the cts
    // timer was set. Now it is returned as the first argument of the CTSTimer
    // function
    p->free();                   
    assert(m_state==MAC_CTS);

    // QUESTION FOR YOU: should I increase ssrc or slrc?
    m_ssrc++;
    if(m_ssrc< ShortRetryLimit ) 
    {
	    if(m_cw<CWMax)m_cw=2*m_cw+1;   // nearly double the contention window
	    StartDefer(true);              // as name implies, must use backoff
    }
    else
    {
	    m_state=MAC_IDLE;              
	    m_cw=CWMin;                    // reset the contention window
	    to_network_ack(false);   // say sorry to the higher layer
    }
}
template <class PLD>
void MAC80211<PLD>::ACKTimer(packet_t*& p)
{
    assert(m_state==MAC_ACK);
    bool retry=false;
    if(m_long_pld)
    { 
	    m_slrc++;
	    if(m_slrc < LongRetryLimit)
	        retry=true;
    }
    else
    {
	    m_ssrc++;
	    if(m_ssrc < ShortRetryLimit)
	        retry=true;
    }

    if(retry)                          // same as above
    {
	    if(m_cw<CWMax)m_cw=2*m_cw+1;
	    StartDefer(true);
	    p->inc_pld_ref();
    }
    else
    {
	    m_state=MAC_IDLE;
	    m_cw=CWMin;
      	to_network_ack(false);
    } 
    p->free();
}

template <class PLD>
void MAC80211<PLD>::NAVTimer(trigger_t&)
{
    // the current session should terminate at this time
    m_in_session=false;
}

template <class PLD>
void MAC80211<PLD>::StartDefer(bool backoff)
{
    // if the state is MAC_DEFER then ResumeDefer instead of this function must be called
    assert(m_state!=MAC_DEFER);
    assert(defer_timer.Active()==false);
    m_backoff_time = Random( m_cw * SlotTime);  // select a backoff time

    Printf((DumpPackets, "mac%d deferring (%d)\n",(int)MyEtherAddr,m_state));
    // if the recv timer is active then do nothing except changing 
    // the state to MAC_DEFER, because it is guaranteed that at the
    // end of RecvTimer, backoff must be resumed.
    if(!recv_timer.Active())                    
    {
	    // if last transmission failed, eifs must be used
	    if(m_tx_failed) m_ifs=m_eifs;
	    else m_ifs=m_difs;
	
	    double pt = m_ifs;
	    if(backoff) pt+=m_backoff_time; // may not need backoff
	    double now=SimTime();
	    if(now<m_last_send)now=m_last_send;
	    if(m_nav>now)
	        m_defer_start=m_nav;
	    else
	        m_defer_start=now;
	    defer_timer.Set(m_defer_start+pt);
    }
    m_state=MAC_DEFER;
}


template <class PLD>
void MAC80211<PLD>::ResumeDefer()
{
    // backoff can only be resumed when it is already in this state
    if(m_state!=MAC_DEFER)return;

    // don't need to select a new backoff time
    if(defer_timer.Active())printf("Mac%d\n",(int)MyEtherAddr);
    assert(defer_timer.Active()==false);
    if(m_tx_failed) m_ifs=m_eifs;
    else m_ifs=m_difs;
    double now=SimTime();
    if(now<m_last_send)now=m_last_send;
    if(m_nav>now)
	    m_defer_start=m_nav;
    else
	    m_defer_start=now;
    defer_timer.Set(m_defer_start+m_ifs+m_backoff_time);  
    Printf((DumpPackets, "mac%d resumes deferring\n", (int)MyEtherAddr));
}

template <class PLD>
void MAC80211<PLD>::TxPacket(packet_t* p, simtime_t tx_time)
{
    // sends out a packet immediately
    double now=SimTime();
    Printf((DumpPackets,"mac%d sends %s until %f\n",(int)MyEtherAddr, p->dump().c_str(),now+tx_time));
    if(now<m_last_send)
    {
	    printf("two sends overlap\n");
	    return;
    }
    m_last_send=now+tx_time;
    to_phy(p);
    SentPackets++;
}
template <class PLD>
void MAC80211<PLD>::TxPacket(packet_t* p, simtime_t start_time, simtime_t tx_time)
{
    // sends out a packet with certain delay
    double start=SimTime()+start_time;
    Printf((DumpPackets,"mac%d sends %s from %f to %f\n",(int)MyEtherAddr,p->dump().c_str(),start,start+tx_time));
    if(start<m_last_send)
    {
	    printf("two sends overlap\n");
	    return;
    }
    m_last_send=start+tx_time;
    assert(!phy_timer.Active());
    phy_timer.Set(p,start);
    SentPackets++;
}

template <class PLD>
void MAC80211<PLD>::NetworkAckTimer(bool& ack)
{
    to_network_ack(ack);
}

template <class PLD>
void MAC80211<PLD>::PhyTimer(packet_t* &p)
{
    to_phy(p);
}


template <class PLD>
void MAC80211<PLD>::DropPacket(packet_t* p, const char* reason)
{
    Printf((DumpPackets,"mac%d drops %s (%s)\n",(int)MyEtherAddr,p->dump().c_str(),reason));
    p->free();
}

#endif /* mac80211_h */
