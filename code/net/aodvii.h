/*

Copyright 2003 Joel Branch, Gilbert (Gang) Chen, Boleslaw K. Szymanski 
and Rensselaer Polytechnic Institute. All worldwide rights reserved.  
A license to use, copy, modify and distribute this software for 
non-commercial research purposes only is hereby granted, provided that 
this copyright notice and accompanying disclaimer is not modified or 
removed from the software.

DISCLAIMER: The software is distributed "AS IS" without any
express or implied warranty, including but not limited to, any
implied warranties of merchantability or fitness for a particular
purpose or any warranty of non-infringement of any current or
pending patent rights. The authors of the software make no
representations about the suitability of this software for any
particular purpose. The entire risk as to the quality and
performance of the software is with the user. Should the software
prove defective, the user assumes the cost of all necessary
servicing, repair or correction. In particular, neither Rensselaer
Polytechnic Institute, nor the authors of the software are liable
for any indirect, special, consequential, or incidental damages
related to the software, to the maximum extent the law
permits.

*/

#include <map>
#include <list>
#include <vector>

#ifndef AODVII_H
#define AODVII_H

#define MAX_UNREACHABLE 5
template <class PLD>
struct AODVII_Struct 
{
 public:

    enum { RREQ=0,RREP,RERR,RACK,HELLO,DATA,RTABLE};
    enum { JOIN=0, REPAIR=1, GRATUITOUS=2, DESTINATION=3, UNKNOWN=4};
    enum { RREQ_SIZE=24, RREP_SIZE=20, HELLO_SIZE=4, RERR_SIZE=4, RACK_SIZE=2, DATA_SIZE=12};
    
    // used for control packet flags
    struct flag_struct
    {
	unsigned int join:1;
	unsigned int repair:1;
	unsigned int gratuitous:1;
	unsigned int destination:1;
	unsigned int unknown:1;
	unsigned int acknowledgement:1;
	unsigned int nodelete:1;
    };
  
    struct rreq_tuple_t
    {
    	int rreq_id;
	simtime_t rreq_time;
    };
  
    struct route_entry_t
    {
    	int dst_seq_num;
    	bool valid;
	int hop_count;
	bool in_local_repair;
	double local_repair_time;
	ether_addr_t src_addr;
	ether_addr_t next_hop;
    };
  
    struct hdr_struct
    {
	int type;
    	int TTL;
        flag_struct flags;
	int hop_count;
	int rreq_id;
	ether_addr_t prev_hop_addr;       // the current node that sends the packet

	ether_addr_t dst_addr;
	ether_addr_t src_addr;
	int dst_seq_num;
	int src_seq_num;
	unsigned int size;
	int data_packet_id;

	ether_addr_t unreachable_dsts[MAX_UNREACHABLE];
	int unreachable_seqs[MAX_UNREACHABLE];
	int unreachable_count;

	bool dump(std::string& str) const ;
    };

    typedef smart_packet_t<hdr_struct, PLD> packet_t;
    typedef PLD payload_t;

    static const int TimeToLive;
    static const simtime_t NodeTraversalTime;
    static const simtime_t HelloInterval;
    static const int AllowedHelloLoss;
    static const int MaxRepairTTL;
    static const int MinRepairTTL;
    static const int LocalAddTTL;
    static const simtime_t LocalRepairTime;
};

template <class PLD> const simtime_t AODVII_Struct<PLD>::LocalRepairTime = 5;
template <class PLD> const int AODVII_Struct<PLD>::TimeToLive = 15;
template <class PLD> const simtime_t AODVII_Struct<PLD>::NodeTraversalTime = 40;
template <class PLD> const simtime_t AODVII_Struct<PLD>::HelloInterval = 5.0;
template <class PLD> const int AODVII_Struct<PLD>::AllowedHelloLoss = 3;
template <class PLD> const int AODVII_Struct<PLD>::MaxRepairTTL = 5;
template <class PLD> const int AODVII_Struct<PLD>::MinRepairTTL = 2;
template <class PLD> const int AODVII_Struct<PLD>::LocalAddTTL = 2;

template <class PLD>
component AODVII : public TypeII, public AODVII_Struct<PLD>
{
public:
    inport inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    inport inline void from_mac_data (packet_t* pkt, ether_addr_t& dst);
    inport inline void from_mac_ack (bool errflag);

    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();

    InfiTimer< AODVII_Struct<PLD>::packet_t* > to_mac_delay;
	
    outport void to_transport ( payload_t& pld );
    outport void to_mac (packet_t* pkt, ether_addr_t& dst, unsigned int size);
    bool SendPacket(packet_t* pkt);
    void CheckBuffer(ether_addr_t&);
    void BufferPacket(packet_t* pkt);
    void UpdateRoutingTable(packet_t* pkt);
    void RepairLink(const ether_addr_t& src_addr, const ether_addr_t& dst_addr, route_entry_t& route, int hop_count);	
    inport inline void to_mac_depart(packet_t* & p, unsigned int i);
    
    Timer < trigger_t> hello_timer;
    inport inline void HelloTimer(trigger_t&);
    InfiTimer < ether_addr_t> repair_timer;
    inport inline void RepairTimer(ether_addr_t&, unsigned int i);

    AODVII() 
    {
	connect to_mac_delay.to_component, to_mac_depart;  
	connect hello_timer.to_component, HelloTimer;
	connect repair_timer.to_component, RepairTimer;
    }

    virtual ~AODVII() {};
    void Start();				        // called when simulation starts
    void Stop();				        // called when simulation stops

    simtime_t ForwardDelay; 
    bool DumpPackets;
    ether_addr_t MyEtherAddr;

    int SentPackets, RecvPackets;
    int TotalHop;

 protected:
    void init_rreq(ether_addr_t, int, int);       // node initiates a route request  
    void receive_rreq(packet_t*);                 // node receives a rreq packet
    void forward_rreq(packet_t*,int);             
    void receive_rrep(packet_t*); 
    void receive_data(packet_t*);

    void init_rerr(ether_addr_t);
    void receive_rerr(packet_t*);
    void prepare_rerr(bool sent, ether_addr_t dst_addr = ether_addr_t::BROADCAST, int seq = 0);

    void SendDataPacket(packet_t*, ether_addr_t, route_entry_t&, unsigned int);
    
    typedef std::multimap<ether_addr_t, rreq_tuple_t, ether_addr_t::compare> rreq_cache_t;
    typedef std::multimap<ether_addr_t, int, ether_addr_t::compare> data_cache_t;
    typedef std::map<ether_addr_t, route_entry_t, ether_addr_t::compare> routing_table_t;
    typedef std::map<ether_addr_t, double, ether_addr_t::compare> neighbor_table_t;

    typedef std::list<packet_t*> packet_buffer_t;

    neighbor_table_t m_neighbor_table;
    routing_table_t m_routing_table;
    packet_buffer_t m_packet_buffer;
    rreq_cache_t m_rreq_cache;                   // for tracking received rreq's
    data_cache_t m_data_cache;
    int m_seq_num;                               // node's current sequence number
    int m_req_id;
    int m_data_packet_id;

    bool m_in_route;
    simtime_t m_last_hello_sent;
    simtime_t m_last_data_recv;

    typedef std::list<packet_t* > packet_queue_t;
    packet_queue_t m_mac_queue;
    packet_t* m_active_packet;
    bool m_mac_busy;

};

template <class PLD>
void AODVII<PLD>::Start()
{
    m_seq_num = 1;
    m_req_id = 0;
    m_data_packet_id = 1;
    SentPackets=RecvPackets=0;
    TotalHop=0;

    m_last_hello_sent = 0.0;
    m_last_data_recv = 0.0;
    m_in_route = true;
    //hello_timer.Set(Random(HelloInterval));
    m_mac_busy = false;
}

template <class PLD>
void AODVII<PLD>::Stop()
{
    if(!m_packet_buffer.empty())
    {
	Printf((DumpPackets, "%d AODVII: %d packet(s) remained\n", (int)MyEtherAddr,
	       (int)m_packet_buffer.size()));
	packet_buffer_t::iterator iter=m_packet_buffer.begin();
	for(;iter!=m_packet_buffer.end();iter++)
	    (*iter)->free();
    }
}

template <class PLD>
void AODVII<PLD>::from_transport( payload_t& pld, ether_addr_t& dst_addr, unsigned int size)
{    
    packet_t* new_p = packet_t::alloc();
    new_p->hdr.type = DATA;
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.prev_hop_addr = MyEtherAddr; 
    new_p->hdr.dst_addr = dst_addr; 
    new_p->hdr.size = DATA_SIZE + size;
    new_p->hdr.hop_count = 0;
    new_p->hdr.data_packet_id = ++ m_data_packet_id;
    new_p->pld=pld;

    Printf((DumpPackets,"%d AODVII: %d->%d %s\n",(int)MyEtherAddr,(int)MyEtherAddr,
	    (int)dst_addr,new_p->dump().c_str()));
    
    // check if there's a route to the destination
    routing_table_t::iterator iter = m_routing_table.find(dst_addr);
    if(iter == m_routing_table.end() || iter->second.valid==false)
    {
	init_rreq(dst_addr,TimeToLive,0);
	// the payload packet gets dropped
	m_packet_buffer.push_back(new_p);
    }
    else
    {
	// send the packet out
	//SendDataPacket(new_p,dst_addr, iter->second,new_p->hdr.size);
	    BufferPacket(new_p);
    } 
} 

template <class PLD>
void AODVII<PLD>::from_mac_data (packet_t* packet, ether_addr_t& dst_addr)
{
    // update neighbor table
    neighbor_table_t::iterator iter = m_neighbor_table.find(packet->hdr.prev_hop_addr);
    if(iter == m_neighbor_table.end())
    {
	m_neighbor_table.insert(make_pair(packet->hdr.prev_hop_addr,SimTime()));
    }
    else
	(*iter).second=SimTime();

    if(dst_addr==MyEtherAddr||dst_addr==ether_addr_t::BROADCAST)
    {
	RecvPackets++;
        /*Printf((DumpPackets,"%d AODVII: %d %d %s\n",(int)MyEtherAddr,(int)packet->hdr.prev_hop_addr,
		(int)MyEtherAddr,packet->dump().c_str())); */

	switch(packet->hdr.type)
	{
	case RREQ:
	    receive_rreq(packet);
	    break;
	case RREP:
	    m_last_data_recv=SimTime();
	    receive_rrep(packet);
	    break;
	case DATA:
	    m_last_data_recv=SimTime();
	    receive_data(packet);
	    break; 
	case RERR:
	    receive_rerr(packet);
	    break;
	}
    }
    packet->free();
} 

template <class PLD>
void AODVII<PLD>::receive_data(packet_t* p)
{
    data_cache_t::iterator i=m_data_cache.begin();
    for(;i!=m_data_cache.end();i++)
	if(i->first==p->hdr.src_addr&&i->second==p->hdr.data_packet_id)
	    return;

    m_data_cache.insert(make_pair(p->hdr.src_addr,p->hdr.data_packet_id));  


    Printf((DumpPackets,"%d AODVII: %d %d %s\n", (int)MyEtherAddr, (int)p->hdr.prev_hop_addr,
	   (int)MyEtherAddr, p->dump().c_str()));
    if(p->hdr.dst_addr == MyEtherAddr)
    {
	TotalHop+=p->hdr.hop_count+1;
	p->inc_pld_ref();
	to_transport(p->pld);
	return;
    }

    // check if there's a route to the destination
    routing_table_t::iterator iter = m_routing_table.find(p->hdr.dst_addr);
    if(iter == m_routing_table.end())
    {
	Printf((DumpPackets,"%d AODVII: no route - %s\n",(int)MyEtherAddr, p->dump().c_str())); 
    }
    else
    {
	// route found - forward the data packet
	packet_t* new_p = packet_t::alloc();
	new_p->hdr.type = DATA;
	new_p->hdr.src_addr = p->hdr.src_addr;
	new_p->hdr.prev_hop_addr = MyEtherAddr;
	new_p->hdr.dst_addr = p->hdr.dst_addr;
	new_p->hdr.hop_count = p->hdr.hop_count + 1;
	new_p->hdr.size = p->hdr.size;
        new_p->hdr.data_packet_id = p->hdr.data_packet_id;
	
	p->inc_pld_ref();
	new_p->pld = p->pld;

	//SendDataPacket(new_p, p->hdr.dst_addr, iter->second, new_p->hdr.size);
	if(iter->second.valid==false)
	{
	    RepairLink(new_p->hdr.src_addr, iter->first,iter->second,iter->second.hop_count);
	    m_packet_buffer.push_back(new_p);
	}
	else
	    BufferPacket(new_p);
    }
} 
 
template <class PLD>
void AODVII<PLD>::init_rreq(ether_addr_t dst, int ttl, int seq_num)
{
    packet_t* new_p = packet_t::alloc();
 
    // NOTE - the "lifetime" field is not used in RREQ's 
    new_p->hdr.type = RREQ;
    new_p->hdr.TTL = ttl; 
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.dst_addr = dst;
    new_p->hdr.prev_hop_addr = MyEtherAddr; 
    new_p->hdr.hop_count = 0;
    new_p->hdr.rreq_id = ++ m_req_id; 
    new_p->hdr.src_seq_num = ++ m_seq_num;
    new_p->hdr.size= RREQ_SIZE;
    new_p->hdr.dst_seq_num = seq_num;

    // buffer the RREQ (source and ID)
    rreq_tuple_t tuple;
    tuple.rreq_id = m_req_id;
    tuple.rreq_time = SimTime();
    m_rreq_cache.insert(make_pair(MyEtherAddr,tuple));
 
    Printf((DumpPackets,"%d AODVII: %d->%d %s\n",(int)MyEtherAddr,(int)MyEtherAddr,
	    (int)dst,new_p->dump().c_str()));
    BufferPacket(new_p);
} 

template <class PLD>
void AODVII<PLD>::receive_rreq(packet_t* p)
{
    ether_addr_t src = p->hdr.src_addr;
    int id = p->hdr.rreq_id; 
    int max_seq_num = p->hdr.dst_seq_num;

    // check if RREQ with same source IP and RREQ ID was received 
    // within at least last PathDiscoveryTime
    rreq_cache_t::iterator iter; 
    for(iter = m_rreq_cache.begin(); iter != m_rreq_cache.end(); iter++)
    {
	if((iter->first == src) && ((iter->second).rreq_id == id))
	{
	    /*Printf((DumpPackets,"%d AODVII: duplicate RREQ received\n",(int)MyEtherAddr));*/ 
	    return;
	}
    }

    // store the RREQ info in rreq cache
    rreq_tuple_t tuple;
    tuple.rreq_id = id;
    tuple.rreq_time = SimTime();
    m_rreq_cache.insert(make_pair(src,tuple));

    UpdateRoutingTable(p);
  
    // ALL prior operation was for processing the reverse routing table entry.
    // Now we decide whether or not to forward on the RREQ.
 
    ether_addr_t dst = p->hdr.dst_addr; 

    if(MyEtherAddr == dst)
    {
        /*Printf((DumpPackets, "aodv%d received rreq at destination from %d with hop %d\n", 
	  (int)MyEtherAddr, (int)p->hdr.prev_hop_addr,p->hdr.hop_count));*/

	// current node is the destination 
	packet_t* new_p = packet_t::alloc();
	
	if(m_seq_num <  p->hdr.dst_seq_num)
	    m_seq_num = p->hdr.dst_seq_num;
	m_seq_num ++;

	new_p->hdr.src_addr = MyEtherAddr;
	new_p->hdr.prev_hop_addr = MyEtherAddr; 
	new_p->hdr.type = RREP;
	new_p->hdr.dst_seq_num = m_seq_num;
	new_p->hdr.hop_count = 0;
	new_p->hdr.dst_addr = p->hdr.src_addr;
	new_p->hdr.size = RREP_SIZE; 
	
	// unicast the RREP back to the source via next hop node
	BufferPacket(new_p);
    }
    else
    {

	bool reply = false;
	routing_table_t::iterator fw_iter = m_routing_table.find(dst);
	
	// confirm condition under which an RREP may be sent 
	if(fw_iter != m_routing_table.end())
	{
	    if( fw_iter->second.dst_seq_num >= p->hdr.dst_seq_num )
	    {
		max_seq_num = fw_iter->second.dst_seq_num;
		if(fw_iter->second.valid)
		    reply = true;
	    }
	}

	if(reply)
	{
	    // current node is an intermediate node 
	    packet_t* new_p = packet_t::alloc();

	    // copy current node's known dest. seq num into RREP dest. seq num
	    new_p->hdr.dst_seq_num = fw_iter->second.dst_seq_num;
	    new_p->hdr.hop_count = fw_iter->second.hop_count+1; 
	    new_p->hdr.src_addr = fw_iter->first;
	    new_p->hdr.prev_hop_addr = MyEtherAddr; 
	    new_p->hdr.dst_addr = p->hdr.src_addr;
	    new_p->hdr.type = RREP;
	    new_p->hdr.size = RREP_SIZE; 

	    // send packet
	    to_mac_delay.Set( new_p, SimTime()+Random(ForwardDelay));
	}
	else
	{
	    // must forward the rreq
	    forward_rreq(p,max_seq_num);
	}
    }
}

template <class PLD>
void AODVII<PLD>::forward_rreq(packet_t* p, int max_seq_num)
{
    if(p->hdr.TTL > 1) 
    {
	packet_t* new_p = packet_t::alloc();
	new_p->hdr.type = RREQ; 
	new_p->hdr.TTL = p->hdr.TTL - 1; 
	new_p->hdr.src_addr = p->hdr.src_addr;
	new_p->hdr.dst_addr = p->hdr.dst_addr;
	new_p->hdr.prev_hop_addr = MyEtherAddr;
	new_p->hdr.hop_count = p->hdr.hop_count + 1;
	new_p->hdr.rreq_id = p->hdr.rreq_id;
	new_p->hdr.size = RREQ_SIZE; 
	new_p->hdr.flags=p->hdr.flags;
	new_p->hdr.dst_seq_num = max_seq_num;
	 
	to_mac_delay.Set(new_p,SimTime()+Random(ForwardDelay));
    }
}

template <class PLD>
void AODVII<PLD>::receive_rrep(packet_t* p)
{
    UpdateRoutingTable(p);

    if(MyEtherAddr != p->hdr.dst_addr)
    {
	// unicast the RREP back to the source via next hop node
	// look up next hop in reverse routing table
	routing_table_t::iterator rv_iter = m_routing_table.find(p->hdr.dst_addr);
	if(rv_iter == m_routing_table.end())
	{
	    printf("[%f] net%d reverse routing error: no route back to the source!\n", SimTime(), (int)MyEtherAddr);
	    return;
	}
	else
	{   
	    packet_t* new_p = packet_t::alloc();
	    
	    new_p->hdr.src_addr = p->hdr.src_addr;
	    new_p->hdr.prev_hop_addr = MyEtherAddr;
	    new_p->hdr.dst_addr = p->hdr.dst_addr; 
	    new_p->hdr.type = RREP;
	    new_p->hdr.dst_seq_num = p->hdr.dst_seq_num;
	    new_p->hdr.hop_count = p->hdr.hop_count + 1;
	    new_p->hdr.size = RREP_SIZE;
	    new_p->hdr.flags.repair=0;
	    new_p->hdr.flags.acknowledgement=0;
	    /*Printf((DumpPackets,"aodv%d sending %s\n",(int)MyEtherAddr,new_p->dump().c_str()));*/

	    if(rv_iter->second.valid==false)
	    {
		RepairLink(p->hdr.src_addr, rv_iter->first, rv_iter->second,rv_iter->second.hop_count);
		m_packet_buffer.push_back(new_p);
	    }
	    else
		BufferPacket(new_p);
	}

    }
    else
    {
	// RREP reaches the source. 
	/*Printf((DumpPackets,"%d AODVII:: Route %d->%d established\n", (int)MyEtherAddr,
	       (int)MyEtherAddr, (int)p->hdr.src_addr));*/
    }
}

template <class PLD>
void AODVII<PLD>::to_mac_depart(packet_t*&p, unsigned int)
{
    BufferPacket(p);
}

template <class PLD>
void AODVII<PLD>::prepare_rerr(bool send, ether_addr_t dst_addr, int dst_seq)
{
    static packet_t* p = NULL;

    if(send==true)
    {
        if(p!=NULL)
	{
	    to_mac_delay.Set(p, SimTime()+Random(ForwardDelay));		
	    p=NULL;
	}
	return;
    }

    if(p==NULL)
    {
	p = packet_t::alloc();
	p->hdr.unreachable_count = 0;
	p->hdr.type = RERR;
	p->hdr.prev_hop_addr = MyEtherAddr;
	p->hdr.size = RERR_SIZE;
    }

    p->hdr.unreachable_dsts[p->hdr.unreachable_count] = dst_addr;
    p->hdr.unreachable_seqs[p->hdr.unreachable_count] = dst_seq;
    p->hdr.size += ether_addr_t::LENGTH + sizeof(int);
    p->hdr.unreachable_count++;
    if(p->hdr.unreachable_count>=MAX_UNREACHABLE)
    {
	to_mac_delay.Set(p, SimTime()+Random(ForwardDelay));		
	p=NULL;
    }
}


template <class PLD>
void AODVII<PLD>::init_rerr(ether_addr_t neighbor)
{
    Printf((DumpPackets,"%d AODVII: INIT RERR %d -> %d \n", (int)MyEtherAddr, (int)MyEtherAddr, (int)neighbor));
    routing_table_t::iterator iter;
    for(iter=m_routing_table.begin();iter!=m_routing_table.end();iter++)
    {
	if( (iter->second).next_hop==neighbor)
	{
	    iter->second.dst_seq_num++;
	    prepare_rerr(false, iter->first, iter->second.dst_seq_num);
	    iter->second.valid = false;
	}
    }
    prepare_rerr(true);
}

template <class PLD>
void AODVII<PLD>::receive_rerr(packet_t* p)
{
    routing_table_t::iterator iter;
    for(int i=0;i<p->hdr.unreachable_count;i++)
    {
	iter=m_routing_table.find(p->hdr.unreachable_dsts[i]);
	if(iter!=m_routing_table.end()&&p->hdr.prev_hop_addr==iter->second.next_hop&&iter->second.valid)
	{
	    iter->second.valid = false;
	    iter->second.dst_seq_num = p->hdr.unreachable_seqs[i];
	    if(iter->second.src_addr==MyEtherAddr)
		init_rreq(iter->first,TimeToLive,iter->second.dst_seq_num); 
	    else
		prepare_rerr(false, iter->first, iter->second.dst_seq_num);
	}
    }
    prepare_rerr(true);
}

template <class PLD>
void AODVII<PLD>::SendDataPacket(packet_t* p, ether_addr_t dst, route_entry_t& route, unsigned int size)
{
    neighbor_table_t::iterator iter = m_neighbor_table.find(route.next_hop);
    if(iter == m_neighbor_table.end())
    {
	//printf("%f: AODVII%d has never heard from AODVII%d\n", SimTime(), (int)MyEtherAddr, (int)route.next_hop);	
	p->free();
    }
    else if ( iter->second < SimTime() - AllowedHelloLoss * HelloInterval )
    {
	/*printf("%f: AODVII%d hasn't heard from AODVII%d since %f\n", SimTime(), (int)MyEtherAddr,
	       (int)route.next_hop,iter->second); */
	m_packet_buffer.push_back(p);

	if(route.in_local_repair)
	{
	    if(route.local_repair_time > SimTime() - LocalRepairTime)
		return;
	}

	if( !route.in_local_repair && p->hdr.hop_count < MaxRepairTTL )
	{
	    // local repair
	    int ttl = MinRepairTTL>2*p->hdr.hop_count?MinRepairTTL:p->hdr.hop_count/2+LocalAddTTL;
	    init_rreq(dst, ttl, route.dst_seq_num);
	    route.in_local_repair=true;
	    route.local_repair_time=SimTime();
	}
	else
	{
	    init_rerr(route.next_hop);
	}
    }
    else
    {
	BufferPacket(p);
    }
}

template <class PLD>
void AODVII<PLD>::HelloTimer(trigger_t& )
{
    simtime_t now = SimTime();
    hello_timer.Set(HelloInterval+now);
    if(m_last_hello_sent<now-HelloInterval && ( m_last_data_recv > now - HelloInterval*AllowedHelloLoss ) )
    {
	packet_t* p = packet_t::alloc();
	p->hdr.prev_hop_addr = MyEtherAddr;
	p->hdr.type = HELLO;
	p->hdr.size = HELLO_SIZE;
	//printf("%f: AODVII%d sends a hello message\n",SimTime(),(int)MyEtherAddr);
	BufferPacket(p);
    }
    //else
	//printf("[%f]: %d AODVII: avoids a hello message: %f %f\n",SimTime(), (int)MyEtherAddr,m_last_hello_sent,m_last_data_recv);
}

template <class PLD>
void AODVII<PLD>::UpdateRoutingTable(packet_t* pkt)
{
    // check if forward route for destination exists and update -
    // if not, create route
    routing_table_t::iterator iter = m_routing_table.find(pkt->hdr.src_addr);
    if(iter == m_routing_table.end())
    {
	// create new entry
	route_entry_t t;
	t.dst_seq_num = pkt->hdr.dst_seq_num;
	t.valid = true;
	t.hop_count = pkt->hdr.hop_count;
	t.next_hop = pkt->hdr.prev_hop_addr;
	t.src_addr = pkt->hdr.dst_addr;
	
	iter=m_routing_table.insert(make_pair(pkt->hdr.src_addr, t)).first;

	CheckBuffer(pkt->hdr.src_addr);
    }
    else
    {
	if( !iter->second.valid || 
	    pkt->hdr.dst_seq_num > iter->second.dst_seq_num || 
	    ( pkt->hdr.dst_seq_num == iter->second.dst_seq_num &&
		 pkt->hdr.hop_count < iter->second.hop_count ) )
	{
	    // update entry
	    iter->second.dst_seq_num = pkt->hdr.dst_seq_num;
	    iter->second.valid = true;
	    iter->second.hop_count = pkt->hdr.hop_count ;
	    iter->second.next_hop = pkt->hdr.prev_hop_addr;
	    iter->second.in_local_repair = false;
	    CheckBuffer(pkt->hdr.src_addr);
	}
    }
}

template <class PLD>
void AODVII<PLD>::CheckBuffer(ether_addr_t& dst_addr)
{
    std::vector<packet_t*> packets;
    packet_buffer_t::iterator iter,curr;
    iter=m_packet_buffer.begin();
    while(iter!=m_packet_buffer.end())
    {
	curr = iter;
	iter ++;
	if((*curr)->hdr.dst_addr==dst_addr)
	{
	    /*Printf((DumpPackets,"aodv%d sending buffered packet to %d %s\n",(int)MyEtherAddr,
	      (int)fw_iter->second.next_hop,(*curr)->dump().c_str()));*/
	    //SendDataPacket(*curr, p->hdr.src_addr, fw_iter->second, (*curr)->hdr.size);
	    packets.push_back(*curr);
	    m_packet_buffer.erase(curr);
	}
    }

    for(unsigned int i=0;i<packets.size();i++)
	BufferPacket(packets[i]);
}

template <class PLD>
bool AODVII<PLD>::SendPacket(packet_t* pkt)
{
    if(pkt->hdr.type==RREP||pkt->hdr.type==DATA)
    {
	routing_table_t::iterator iter = m_routing_table.find(pkt->hdr.dst_addr);
	if(iter == m_routing_table.end())
	{
	    printf("[%f] %d AODVII: no route - %s\n",SimTime(),(int)MyEtherAddr, pkt->dump().c_str());
	    pkt->free();
	    return false;
	}
	else
	{
	    if(iter->second.valid==false)
	    {
		m_packet_buffer.push_back(pkt);
		return false;
	    }
	    else
	    {
		SentPackets++;
		m_last_hello_sent = SimTime();
		pkt->inc_ref();
		m_active_packet=pkt;
		to_mac(pkt,iter->second.next_hop,pkt->hdr.size);
	    }
	}
    }
    else
    {
	SentPackets++;
	m_last_hello_sent = SimTime();
	pkt->inc_ref();
	m_active_packet=pkt;
	to_mac(pkt,ether_addr_t::BROADCAST,pkt->hdr.size);
    }
    return true;

}
template <class PLD>
void AODVII<PLD>::BufferPacket(packet_t* pkt)
{
    if(m_mac_busy==false)
    {
	m_active_packet=pkt;
	m_mac_busy=true;
	if(!SendPacket(pkt))
	{
	    m_mac_busy=false;
	}
    }
    else
    {
	m_mac_queue.push_back(pkt);
    }
}


template <class PLD>
void AODVII<PLD>::RepairLink(const ether_addr_t& src_addr, const ether_addr_t& dst_addr, route_entry_t& route, int hop_count)
{
    Printf((DumpPackets, "%d AODVII: link -> %d -> %d broken \n", (int)MyEtherAddr,
	   (int)route.next_hop, (int)dst_addr));

	bool repair_needed = (!route.in_local_repair) || (route.local_repair_time < SimTime() - LocalRepairTime);

	if( ( hop_count < MaxRepairTTL || src_addr==MyEtherAddr ) && repair_needed ) 
	{
	    Printf((DumpPackets, "[%f] %d AODVII: local repair -> %d -> %d \n", (int)MyEtherAddr,
		   (int)route.next_hop, (int)dst_addr));
	    // local repair
	    int ttl = MinRepairTTL>2*hop_count?MinRepairTTL:hop_count/2+LocalAddTTL;
	    if(src_addr==MyEtherAddr) ttl = route.hop_count + LocalAddTTL;
  	    route.dst_seq_num ++;	    
	    init_rreq(dst_addr, ttl, route.dst_seq_num);
	    route.in_local_repair=true;
	    route.local_repair_time=SimTime();
	    route.valid=false;
	}
	else
	{
	    if( repair_needed )
	    {
		route.valid=false;
		route.in_local_repair=false;
		init_rerr(route.next_hop);
	    }
	}

	repair_timer.Set(dst_addr,SimTime()+LocalRepairTime);
	
}
template <class PLD>
void AODVII<PLD>::RepairTimer(ether_addr_t& dst_addr, unsigned int)
{
    Printf((DumpPackets,"%d AODVII: repair link to %d\n", (int)MyEtherAddr, (int)dst_addr));
    
    packet_buffer_t::iterator pb_iter=m_packet_buffer.begin();
    for(;pb_iter!=m_packet_buffer.end();pb_iter++)
    {
	if( (*pb_iter)->hdr.dst_addr==dst_addr && (*pb_iter)->hdr.type==DATA )
	{
	    routing_table_t::iterator iter = m_routing_table.find(dst_addr);
	    assert(iter!=m_routing_table.end());

	    if(iter->second.valid==false)
	    {
		RepairLink(MyEtherAddr,dst_addr,iter->second,0);
		return;
	    }
	}
    }    
}

template <class PLD>
void AODVII<PLD>::from_mac_ack (bool success)
{
    if(!success)
    {
	assert(m_active_packet!=NULL);
	assert(m_active_packet->hdr.type==DATA||m_active_packet->hdr.type==RREP);
	routing_table_t::iterator iter = m_routing_table.find(m_active_packet->hdr.dst_addr);
	assert(iter!=m_routing_table.end());

	RepairLink(m_active_packet->hdr.src_addr, iter->first,iter->second,iter->second.hop_count);
	m_packet_buffer.push_back(m_active_packet);
    }
    else
    {
	
	m_active_packet->free();
    }

    while(!m_mac_queue.empty())
    {
	m_active_packet=m_mac_queue.front();
	m_mac_queue.pop_front();
	if(SendPacket(m_active_packet))
	    return;
    }
    
    if(m_mac_queue.empty())
    {
	m_mac_busy=false;
    }
}

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
template <class PLD>
void AODVII<PLD>::from_pm_node_up()
{
  return;
}

template <class PLD>
bool AODVII_Struct<PLD>::hdr_struct::dump(std::string& str) const 
{ 
    char buffer[100];
    std::string t;
    switch(type)
    {
    case RREQ:
        t="REQUEST";
	break;    
    case RREP:
        t="REPLY";
	break;
    case DATA:
        t="DATA";
	break;
    case HELLO:
        t="HELLO";
	break;
    case RERR:
	t="ERR";
	break;
    default:
	t="UNKNOWN";
	break;
    }
    sprintf(buffer,"%s %d %d %d %d %d->%d %d (%d %d) ",
	    t.c_str(),TTL,hop_count, rreq_id, (int)prev_hop_addr,
	    (int)src_addr,(int)dst_addr,data_packet_id, src_seq_num,dst_seq_num);
    str=buffer;
    return type==DATA;
}


#endif /* AODVII_H */
