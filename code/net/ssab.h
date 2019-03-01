/*************************************************************************
*   @<title> A SSAB Component </title>@
*
*   @<!-- Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski
*   and Rensselaer Polytechnic Institute. All worldwide rights
*   reserved.  A license to use, copy, modify and distribute this
*   software for non-commercial research purposes only is hereby
*   granted, provided that this copyright notice and accompanying
*   disclaimer is not modified or removed from the software.
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
*   related to the software, to the maximum extent the law
*   permits.-->@
*
*************************************************************************/

#ifndef SSAB_h
#define SSAB_h
#include <map>
#include <list>

template <class PLD>
struct SSAB_Struct
{
    struct hdr_struct
    {
	unsigned int seq_number;
	unsigned int size;
	double send_time;
	unsigned int hop;
	
	bool dump(std::string& str) const 
	{ 
	    char buffer[30];
	    sprintf(buffer,"%d %d",seq_number,size); 
	    str=buffer;
	    return true;
	}
    };

    typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

};



template <class PLD>
component SSAB : public TypeII, public SSAB_Struct<PLD>
{
 public:

    ether_addr_t MyEtherAddr;
    simtime_t ForwardDelay;
    double RXThresh;
    bool DumpPackets;

    int SentPackets;
    int RecvPackets;
    int RecvUniPackets;
    double TotalDelay;
    int TotalSamples;
    int TotalHop;
    

    inport inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    inport inline void from_mac_data (packet_t* pkt, double power);
    inport inline void from_mac_ack (bool errflag);
    inport inline void	from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2);

    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();

    outport void cancel ();
    outport void to_transport ( payload_t& pld );
    outport void to_mac (packet_t* pkt, unsigned int size, double backoff);

    void Start();
    void Stop();
    SSAB();
    virtual ~SSAB();
	
 protected:

    bool m_mac_busy;                  // if the mac layer is busy
    unsigned int m_seq_number;        // current sequence number
    typedef std::list<pair<packet_t*,double> > packet_buffer_t;
    packet_buffer_t m_packet_buffer;
    packet_t* m_active_packet;

    class seq_number_t
    {
    public:
	seq_number_t () : current(0),bits(0u) {}
	bool check(int n)
	{
	    if( n + (int)sizeof(unsigned long) <= current ) return true;
	    if ( n > current )
	    {
		bits = bits << (n - current);
		current = n;
	    }

	    unsigned long flag = 1 << (current - n);
	    unsigned long r = flag & bits;
	    bits |= flag;
	    return r;
	}
    private:
	int current;
	uint32_t bits;
    };
	

/*******************************************************************
 * For each source, we must maintain a @seq_number_t@ object, so we better
 * keep them in a map.
 *******************************************************************/

    typedef std::map<ether_addr_t, seq_number_t, ether_addr_t::compare> cache_t;
    cache_t m_seq_cache;

};

template <class PLD>
SSAB<PLD>::SSAB()
{

}

template <class PLD>
SSAB<PLD>::~SSAB()
{
}

template <class PLD>
void SSAB<PLD>::Start() 
{
    m_mac_busy=false;
    m_seq_number=1u;
    m_seq_cache.insert(make_pair(MyEtherAddr,seq_number_t()));

    SentPackets=RecvPackets=RecvUniPackets=0l;
    TotalDelay=0.0;
    TotalHop=0;
    TotalSamples=0;
}
template <class PLD>
void SSAB<PLD>::Stop() 
{
    //printf("%s: sent %d, recv %d\n",GetName(),SentPackets,RecvPackets);
    //printf("%s: %f \n", GetName(), (double)RecvPackets/RecvUniPackets);
}

template <class PLD>
void SSAB<PLD>::from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size)
{
    packet_t* p=packet_t::alloc();
    p->pld=pld;
    m_seq_number++;
    m_seq_cache[MyEtherAddr].check(m_seq_number);
    p->hdr.seq_number=m_seq_number;
    p->hdr.size=size+2*sizeof(unsigned int);
    p->hdr.hop=0;
    SentPackets++;

    Printf((DumpPackets,"ssab%d creates %s\n",(int)MyEtherAddr,p->dump().c_str()));

    p->hdr.send_time=SimTime();
    if(!m_mac_busy)
    {
	m_mac_busy=true;
	m_active_packet=p;
	to_mac(p,size,0.0);
    }
    else
    {
	packet_buffer_t::iterator iter=m_packet_buffer.begin();
	while(iter!=m_packet_buffer.end()&&(*iter).second<=0.0)iter++;
	m_packet_buffer.insert(iter,make_pair(p,0.0));
    }

}

/*******************************************************************
 * We are now receiving a packet from the mac layer.
 *******************************************************************/

template <class PLD>
void SSAB<PLD>::from_mac_data (packet_t* pkt, double power)
{
    ether_addr_t src;
    src=pkt->access_pld().src_addr;
    RecvPackets++;

    Printf((DumpPackets,"ssab%d receives %s \n",(int)MyEtherAddr,pkt->dump().c_str()));
    cache_t::iterator iter=m_seq_cache.find(src); 
    if(iter==m_seq_cache.end())
    {
	// couldn't find the source address in the cache.
	// so we create a new record
	iter=(m_seq_cache.insert(make_pair(src,seq_number_t()))).first;
    }
    if( !iter->second.check(pkt->hdr.seq_number) )
    {
	// the received packet is new
	RecvUniPackets++;
	if(pkt->access_pld().dst_addr==MyEtherAddr||pkt->access_pld().dst_addr==ether_addr_t::BROADCAST)
	{
	    // the packets arrivs at its destination
	    pkt->inc_pld_ref();
	    to_transport(pkt->pld);
	    TotalHop+=pkt->hdr.hop;
	}
	else  
	{
	    double delay;
#ifdef USING_SSAB
	    delay=Random();
	    delay=ForwardDelay*(1-RXThresh/power);
#else
	    delay=Random(ForwardDelay);
#endif
	    TotalDelay+=delay;
	    TotalSamples++;
	    // rebroadcast needed

	    packet_t* np = pkt->copy();
	    np->hdr.hop++;
	    pkt->free();


	    if(m_mac_busy==false)
	    {
		Printf((DumpPackets,"ssab%d forwards %s\n",(int)MyEtherAddr,np->dump().c_str()));

		SentPackets++;
		np->hdr.send_time=SimTime();
		m_mac_busy=true;
		m_active_packet=np;
		to_mac(np,np->hdr.size,delay);
	    }
	    else
	    {
                packet_buffer_t::iterator iter=m_packet_buffer.begin();
                while(iter!=m_packet_buffer.end()&&(*iter).second<=delay)iter++;
                m_packet_buffer.insert(iter,make_pair(np,delay));
	    }
	    return;
	}
    }
    else
    {
        if(m_mac_busy==true&&m_active_packet->access_pld().src_addr==pkt->access_pld().src_addr&&
	   m_active_packet->hdr.seq_number==pkt->hdr.seq_number)
        {
	    Printf((DumpPackets,"ssab%d cancels the current packet\n",(int)MyEtherAddr));
            cancel();
        }
        else
        {
            packet_buffer_t::iterator iter;
            for(iter=m_packet_buffer.begin();iter!=m_packet_buffer.end();iter++)
            {
                if((*iter).first->access_pld().src_addr==pkt->access_pld().src_addr&&
                    (*iter).first->hdr.seq_number==pkt->hdr.seq_number)
                {
		    Printf((DumpPackets,"ssab%d cancels a buffered packet\n",(int)MyEtherAddr));fflush(stdout);
                    (*iter).first->free();
                    m_packet_buffer.erase(iter);
                    break;
                }
            }
        }
    }
    pkt->free();
}

/*
** Receive notification of receive-receive collisions. This must be defined
** since the cxx compiler doesn't support #ifdef.
*/
template <class PLD>
void SSAB<PLD>::from_mac_recv_recv_coll(
  packet_t	*pkt1,
  packet_t	*pkt2)
{
  return;
}

template <class PLD>
void SSAB<PLD>::from_mac_ack(bool)
{
    Printf((DumpPackets,"ssab%d recevies an ack\n",(int)MyEtherAddr));
    if(m_packet_buffer.size())
    {
        packet_t* packet=m_packet_buffer.front().first;
        double delay=m_packet_buffer.front().second;
        m_packet_buffer.pop_front();
    	Printf((DumpPackets,"ssab%d forwards %s from buffer\n",(int)MyEtherAddr,packet->dump().c_str()));
    	m_active_packet=packet;
        to_mac(packet,packet->hdr.size,delay);
    }
    else
        m_mac_busy=false;
    return;
}

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
template <class PLD>
void SSAB<PLD>::from_pm_node_up()
{
  return;
}

#endif /*SSAB_h*/
