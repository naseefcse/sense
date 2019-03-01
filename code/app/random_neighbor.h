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

#ifndef random_neighbor_h
#define random_neighbor_h

struct RandomNeighbor_Struct
{
    struct packet_t
    {
	    ether_addr_t src_addr;
    	ether_addr_t dst_addr;
	    int data_size;
	    std::string dump() const 
	    {
	        char buffer[50];
	        sprintf(buffer,"%d %d %d",(int)src_addr,(int)dst_addr,data_size);
	        return buffer;
	    }
	    void dump(std::string& str) const { str=dump();}
    };
};

component RandomNeighbor : public TypeII, public RandomNeighbor_Struct
{
public:
    
 	outport void to_transport ( packet_t& pld, const ether_addr_t& dst, unsigned int size );
 	inport void from_transport_data ( packet_t& pld, const ether_addr_t & dst);
 	inport void from_transport_ack ( bool flag);
    
    
    Timer < trigger_t > broadcast_timer;
    Timer < trigger_t > unicast_timer;

    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    ether_addr_t MyEtherAddr;
    double MaxX, MaxY, MaxPacketSize;
    bool DumpPackets;

    int SentPackets, RecvPackets;
    int SendFailures;

    RandomNeighbor();
    virtual ~RandomNeighbor() ;
    void Start();
    void Stop();

    inport void BroadcastTimer(trigger_t&);
    inport void UnicastTimer(trigger_t&);

 private:
    
    std::vector<ether_addr_t> m_neighbors;
    bool m_transport_busy;

};

RandomNeighbor::RandomNeighbor()
{
    connect broadcast_timer.to_component, BroadcastTimer;
    connect unicast_timer.to_component, UnicastTimer;
}
RandomNeighbor::~RandomNeighbor()
{
}
void RandomNeighbor::Start()
{
    m_transport_busy=false;
    SentPackets=0;
    RecvPackets=0;
    SendFailures=0;
    broadcast_timer.Set(Random(BroadcastInterval));
    unicast_timer.Set(Random(UnicastInterval));
}
void RandomNeighbor::Stop()
{
    /*printf("%s sent %d/%d packets, received %d packets\n",GetName(),
      SentPackets,SendFailures,RecvPackets);*/
}
void RandomNeighbor::from_transport_ack ( bool flag)
{
    if(flag==false)SendFailures++;
    m_transport_busy=false;
}
void RandomNeighbor::from_transport_data ( packet_t& pld, const ether_addr_t & dst)
{
    const packet_t & p = pld;

    Printf((DumpPackets,"receives %s\n",p.dump().c_str()));

    unsigned int i;
    if(p.dst_addr==ether_addr_t::BROADCAST)
    {
	    for(i=0;i<m_neighbors.size();i++)
	    {
	        if(m_neighbors[i]==p.src_addr)break;
	    }
	    if(i>=m_neighbors.size())
	    {
	        Printf((DumpPackets,"detects a new neighbor station%d\n",int(p.src_addr)));
	        m_neighbors.push_back(p.src_addr);
	    }
    }
    else
    {
	    if(p.dst_addr==MyEtherAddr)
	    {
	        RecvPackets++;
	    }
	    else
	    {
	        printf("rn%d received a packet with wrong destination!\n",(int)MyEtherAddr);
	    }
    }
}
void RandomNeighbor::BroadcastTimer(trigger_t&)
{
    broadcast_timer.Set(Random(BroadcastInterval)+SimTime());
    if(m_transport_busy==true) return;
    packet_t p;
    p.src_addr=MyEtherAddr;
    p.dst_addr=ether_addr_t::BROADCAST;
    p.data_size=(int)Random(MaxPacketSize);
    m_transport_busy=true;
    Printf((DumpPackets,"broadcasts %s\n",p.dump().c_str()));
    to_transport(p,p.dst_addr,p.data_size+2*ether_addr_t::LENGTH);
}
void RandomNeighbor::UnicastTimer(trigger_t&)
{
    unicast_timer.Set(Random(UnicastInterval)+SimTime());
    if(m_transport_busy==true||m_neighbors.size()==0) return;
    packet_t p;
    p.src_addr=MyEtherAddr;
    p.dst_addr=m_neighbors[(int)Random((double)m_neighbors.size())];
    p.data_size=(int)Random(MaxPacketSize);
    m_transport_busy=true;
    Printf((DumpPackets,"unicasts %s\n",p.dump().c_str()));
    SentPackets++;
    to_transport(p,p.dst_addr,p.data_size+2*ether_addr_t::LENGTH);
}


#endif /* random_neighbor_h*/
