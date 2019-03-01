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

#ifndef AODVI_H
#define AODVI_H

template <class PLD>
struct AODVI_Struct 
{
 public:

    enum { RREQ=0,RREP,RERR,RACK,HELLO,DATA,RTABLE};
    enum { JOIN=0, REPAIR=1, GRATUITOUS=2, DESTINATION=3, UNKNOWN=4};
    enum { RREQ_SIZE=24, RREP_SIZE=20, RERR_SIZE=5, RACK_SIZE=2, DATA_SIZE=12};
    
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
	bool duplicate;
    };
  
    struct route_entry_t
    {
    	int dst_seq_num;
    	bool valid_dst_seq;
	    int hop_count;
	    ether_addr_t next_hop;
	    double lifetime;
    };
  
    struct hdr_struct
    {
	    int type;
    	int TTL;
        flag_struct flags;
	    int hop_count;
	    int rreq_id;
	    ether_addr_t dst_addr;
	    int dst_seq_num;
	    ether_addr_t src_addr;
	    ether_addr_t prev_hop_addr;
	    int src_seq_num;
	    double lifetime;
	    int prefix_size;
	    unsigned int size;

	    bool dump(std::string& str) const ;
    };

    typedef smart_packet_t<hdr_struct, PLD> packet_t;
    typedef PLD payload_t;

    static const int TimeToLive;
    static const simtime_t PathDiscoveryTime;
    static const simtime_t NetTraversalTime;
    static const simtime_t NodeTraversalTime;
    static const simtime_t ActiveRouteTimeout;
    static const simtime_t MyRouteTimeout;

};

template <class PLD> const int AODVI_Struct<PLD>::TimeToLive = 15;
template <class PLD> const simtime_t AODVI_Struct<PLD>::ActiveRouteTimeout = 3;
template <class PLD> const simtime_t AODVI_Struct<PLD>::NodeTraversalTime = 40;
template <class PLD> const simtime_t AODVI_Struct<PLD>::NetTraversalTime = 2 * NodeTraversalTime * 35;
template <class PLD> const simtime_t AODVI_Struct<PLD>::PathDiscoveryTime = 2 * NetTraversalTime;
template <class PLD> const simtime_t AODVI_Struct<PLD>::MyRouteTimeout = 2 * ActiveRouteTimeout; 


template <class PLD>
component AODVI : public TypeII, public AODVI_Struct<PLD>
{
public:
    inport inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    inport inline void from_mac_data (packet_t* pkt, ether_addr_t& dst);
    inport inline void from_mac_ack (bool errflag);

    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();

    InfiTimer<pair<AODVI_Struct<PLD>::packet_t*,ether_addr_t> > to_mac_delay;
	
    outport void to_transport ( payload_t& pld );
    outport void to_mac (packet_t* pkt, ether_addr_t& dst, unsigned int size);
	
    inport inline void to_mac_depart(const pair<packet_t*,ether_addr_t>& p, unsigned int i);
    
    AODVI() { connect to_mac_delay.to_component, to_mac_depart;  }
    virtual ~AODVI() {};
    void Start();				        // called when simulation starts
    void Stop();				        // called when simulation stops

    simtime_t ForwardDelay; 
    bool DumpPackets;
    ether_addr_t MyEtherAddr;

    int SentPackets, RecvPackets;
    int TotalHop;
 protected:
    void init_rreq(ether_addr_t);                 // node initiates a route request  
    void receive_rreq(packet_t*);                 // node receives a rreq packet
    void forward_rreq(packet_t*,int); 
    void receive_rrep(packet_t*); 
    void receive_data(packet_t*);
    

    typedef std::multimap<ether_addr_t, rreq_tuple_t, ether_addr_t::compare> rreq_cache_t;
    typedef std::map<ether_addr_t, route_entry_t, ether_addr_t::compare> routing_table_t;
    typedef std::list<packet_t*> packet_buffer_t;

    routing_table_t m_routing_table;
    packet_buffer_t m_packet_buffer;
    rreq_cache_t m_rreq_cache;                         // for tracking received rreq's
    int m_seq_num;                               // node's current sequence number
    int m_req_id;
};



template <class PLD>
void AODVI<PLD>::Start()
{
    m_seq_num = 0;
    m_req_id = 0;
    SentPackets=RecvPackets=0;
    TotalHop=0;
}

template <class PLD>
void AODVI<PLD>::Stop()
{
    packet_buffer_t::iterator iter=m_packet_buffer.begin();
    for(;iter!=m_packet_buffer.end();iter++)
        (*iter)->free();
}

template <class PLD>
void AODVI<PLD>::from_transport( payload_t& pld, ether_addr_t& dst_addr, unsigned int size)
{    
    packet_t* new_p = packet_t::alloc();
    new_p->hdr.type = DATA;
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.prev_hop_addr = MyEtherAddr; 
    new_p->hdr.dst_addr = dst_addr; 
    new_p->hdr.size = DATA_SIZE + size;
    new_p->hdr.hop_count = 0;
    new_p->pld=pld;

    Printf((DumpPackets,"%d AODVI: %d->%d %s\n",(int)MyEtherAddr,(int)MyEtherAddr,
	    (int)dst_addr,new_p->dump().c_str()));
    
    // check if there's a route to the destination
    routing_table_t::iterator iter = m_routing_table.find(dst_addr);
    if(iter == m_routing_table.end())
    {
	Printf((DumpPackets,"%d AODVI: no route \n",(int)MyEtherAddr));
	init_rreq(dst_addr);
	// the payload packet gets dropped
	m_packet_buffer.push_back(new_p);
    }
    else
    {
	// route found - send packet 
	    SentPackets++;
	    to_mac(new_p, (iter->second).next_hop, new_p->hdr.size); 
    } 
} 

template <class PLD>
void AODVI<PLD>::from_mac_ack (bool)
{
}

template <class PLD>
void AODVI<PLD>::from_mac_data (packet_t* packet, ether_addr_t& dst_addr)
{
    if(dst_addr==MyEtherAddr||dst_addr==ether_addr_t::BROADCAST)
    {
	RecvPackets++;
	Printf((DumpPackets,"%d AODVI: %d %d %s\n",(int)MyEtherAddr,(int)packet->hdr.prev_hop_addr,
		(int)MyEtherAddr,packet->dump().c_str()));

	    switch(packet->hdr.type)
	    {
	    case RREQ:
	        receive_rreq(packet);
	        break;
	    case RREP:
		receive_rrep(packet);
	        break;
	    case DATA:
		receive_data(packet);
	        break; 
	    }
    }
    packet->free();
}

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
template <class PLD>
void AODVI<PLD>::from_pm_node_up()
{
  return;
}

template <class PLD>
void AODVI<PLD>::receive_data(packet_t* p)
{
    if(p->hdr.dst_addr == MyEtherAddr)
    {
	TotalHop+= p->hdr.hop_count+1;
	p->inc_pld_ref();
	to_transport(p->pld);
	return;
    }

    // check if there's a route to the destination
    routing_table_t::iterator iter = m_routing_table.find(p->hdr.dst_addr);
    if(iter == m_routing_table.end())
    {
	    Printf((DumpPackets,"aodv%d no route - %s\n",(int)MyEtherAddr, p->dump().c_str()));        
    }
    else
    {
	    // route found - forward the data packet
	    packet_t* new_p = packet_t::alloc();
	    new_p->hdr.type = DATA;
	    new_p->hdr.src_addr = p->hdr.src_addr;
	    new_p->hdr.prev_hop_addr = MyEtherAddr;
	    new_p->hdr.dst_addr = p->hdr.dst_addr;
	    new_p->hdr.size = p->hdr.size;
	    new_p->hdr.hop_count = p->hdr.hop_count + 1;
	
	    p->inc_pld_ref();
	    new_p->pld = p->pld;

	    SentPackets++;
	    to_mac(new_p, (iter->second).next_hop, new_p->hdr.size);
    }
} 
 
template <class PLD>
void AODVI<PLD>::init_rreq(ether_addr_t dst)
{
    packet_t* new_p = packet_t::alloc();
 
    // NOTE - the "lifetime" field is not used in RREQ's 
    new_p->hdr.type = RREQ;
    new_p->hdr.TTL = TimeToLive; 
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.dst_addr = dst;
    new_p->hdr.prev_hop_addr = MyEtherAddr; 
    new_p->hdr.hop_count = 0;
    new_p->hdr.rreq_id = ++ m_req_id; 
    new_p->hdr.src_seq_num = ++ m_seq_num;
    new_p->hdr.size= RREQ_SIZE;

    new_p->hdr.flags.join=0;
    new_p->hdr.flags.repair=0;
    new_p->hdr.flags.gratuitous=0;
    new_p->hdr.flags.destination=0;
    new_p->hdr.flags.unknown=0;

    // buffer the RREQ (source and ID) for PathDiscoveryTime 
    rreq_tuple_t tuple;
    tuple.rreq_id = m_req_id;
    tuple.rreq_time = SimTime();
    tuple.duplicate = false;
    m_rreq_cache.insert(make_pair(MyEtherAddr,tuple));
 
    Printf((DumpPackets,"%d AODVI: %d->%d %s\n",(int)MyEtherAddr,(int)MyEtherAddr,
	    (int)dst,new_p->dump().c_str()));

    // see if route to dest is known - then we can obtain dst seq. num.
    routing_table_t::iterator iter = m_routing_table.find(dst);
    if(iter != m_routing_table.end())
	    new_p->hdr.dst_seq_num = iter->second.dst_seq_num;
    else
	    new_p->hdr.flags.unknown = true;

    SentPackets++;
        to_mac(new_p, ether_addr_t::BROADCAST, new_p->hdr.size);
} 

template <class PLD>
void AODVI<PLD>::receive_rreq(packet_t* p)
{
    ether_addr_t src = p->hdr.src_addr;
    int id = p->hdr.rreq_id; 
    int max_seq_num = p->hdr.dst_seq_num;

    // check if RREQ with same source IP and RREQ ID was received 
    // within at least last PathDiscoveryTime
    rreq_cache_t::iterator iter; 
    for(iter = m_rreq_cache.begin(); iter != m_rreq_cache.end(); iter++)
    {
	    if((iter->first == src) && ((iter->second).rreq_id == id) /*&& 
	        ((SimTime() - (iter->second).rreq_time) <= PathDiscoveryTime)*/ )
	    {
		Printf((DumpPackets,"%d AODVI: duplicate RREQ received\n",(int)MyEtherAddr)); 
	        iter->second.duplicate=true;
		return;
	    }
    }
 
    // store the RREQ info in rreq cache
    rreq_tuple_t tuple;
    tuple.rreq_id = id;
    tuple.rreq_time = SimTime();
    tuple.duplicate = false;
    m_rreq_cache.insert(make_pair(src,tuple));
 
    // search for reverse route to source node
    routing_table_t::iterator rv_iter = m_routing_table.find(src);
    if(rv_iter == m_routing_table.end())
    {
	    // create new entry
	    route_entry_t t;
	    t.dst_seq_num = p->hdr.src_seq_num; 
	    t.valid_dst_seq = true;
	    t.hop_count = p->hdr.hop_count; 
	    t.next_hop = p->hdr.prev_hop_addr;
	    t.lifetime = SimTime() + 2*NetTraversalTime - 2*p->hdr.hop_count*NodeTraversalTime;

    	rv_iter=m_routing_table.insert(make_pair(src, t)).first;
    }
    else
    {
	// update entry
	if(p->hdr.src_seq_num > rv_iter->second.dst_seq_num || 
	   p->hdr.hop_count < rv_iter->second.hop_count )
	{
	    rv_iter->second.dst_seq_num = p->hdr.src_seq_num;
	    rv_iter->second.hop_count = p->hdr.hop_count;
	    rv_iter->second.next_hop = p->hdr.prev_hop_addr;
	    rv_iter->second.valid_dst_seq = true;
	}

	simtime_t  lt = SimTime() + 2*NetTraversalTime - 2*p->hdr.hop_count*NodeTraversalTime;
	if( lt > rv_iter->second.lifetime)
	        rv_iter->second.lifetime = lt;  
    }

    // ALL prior operation was for processing the reverse routing table entry.
    // Now we decide whether or not to forward on the RREQ.
 
    ether_addr_t dst = p->hdr.dst_addr; 

    if(MyEtherAddr == dst)
    {
        /*Printf((DumpPackets, "aodv%d received rreq at destination from %d with hop %d\n", (int)MyEtherAddr, (int)p->hdr.prev_hop_addr,p->hdr.hop_count));*/
	    // current node is the destination 
	    packet_t* new_p = packet_t::alloc();

	    if(m_seq_num <  p->hdr.dst_seq_num)
	        m_seq_num = p->hdr.dst_seq_num;

	    new_p->hdr.src_addr = MyEtherAddr;
	    new_p->hdr.prev_hop_addr = MyEtherAddr; 
	    new_p->hdr.type = RREP;
	    new_p->hdr.dst_seq_num = m_seq_num;
	    new_p->hdr.hop_count = 0;
	    new_p->hdr.lifetime = SimTime() + MyRouteTimeout;
	    new_p->hdr.dst_addr = p->hdr.src_addr;
	    new_p->hdr.size = RREP_SIZE; 
	    new_p->hdr.flags.repair = 0;
	    new_p->hdr.flags.acknowledgement = 0;
	
	    // unicast the RREP back to the source via next hop node
	    SentPackets++;
	    to_mac(new_p, rv_iter->second.next_hop, new_p->hdr.size);
    }
    else
    {

	    bool reply = false;
	    routing_table_t::iterator fw_iter = m_routing_table.find(dst);
 
	    // confirm condition under which an RREP may be sent 
	    if(fw_iter != m_routing_table.end())
	    {
	        if( fw_iter->second.valid_dst_seq == true && 
		    fw_iter->second.dst_seq_num >= p->hdr.dst_seq_num )
	        {
		        max_seq_num = fw_iter->second.dst_seq_num;
		        if(p->hdr.flags.destination == false)
		            reply = true;
	        }
	    }

	    if(reply==true)
	    {
	        // current node is an intermediate node 
	        packet_t* new_p = packet_t::alloc();

	        // copy current node's known dest. seq num into RREP dest. seq num
	        new_p->hdr.dst_seq_num = fw_iter->second.dst_seq_num;
	        new_p->hdr.hop_count = fw_iter->second.hop_count; 
	        new_p->hdr.lifetime = fw_iter->second.lifetime;
	        new_p->hdr.src_addr = fw_iter->first;
	        new_p->hdr.prev_hop_addr = MyEtherAddr; 
	        new_p->hdr.type = RREP;
	        new_p->hdr.dst_addr = p->hdr.src_addr;
	        new_p->hdr.size = RREP_SIZE; 
	        new_p->hdr.flags.repair = 0;
	        new_p->hdr.flags.acknowledgement = 0;

	        // send packet
	        //Printf((DumpPackets,"aodv%d sending %s\n",(int)MyEtherAddr,new_p->dump().c_str()));
	        SentPackets++;
	        to_mac_delay.Set(make_pair(new_p, rv_iter->second.next_hop), SimTime()+Random(ForwardDelay));
	    }
	    else
	    {
	        // must forward the rreq
	        forward_rreq(p,max_seq_num);
	    }
    }
}

template <class PLD>
void AODVI<PLD>::forward_rreq(packet_t* p, int max_seq_num)
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
	 
	    SentPackets++;
	    to_mac_delay.Set(make_pair(new_p, (ether_addr_t)ether_addr_t::BROADCAST),SimTime()+Random(ForwardDelay));
    }
}

template <class PLD>
void AODVI<PLD>::receive_rrep(packet_t* p)
{
    // check if forward route for destination exists and update -
    // if not, create route
    routing_table_t::iterator fw_iter = m_routing_table.find(p->hdr.src_addr);
    if(fw_iter == m_routing_table.end())
    {
	    // create new entry
	    route_entry_t t;
	    t.dst_seq_num = p->hdr.dst_seq_num;
	    t.valid_dst_seq = true;
	    t.hop_count = p->hdr.hop_count + 1;
	    t.next_hop = p->hdr.prev_hop_addr;
	    t.lifetime = SimTime() + p->hdr.lifetime;
	
	    fw_iter=m_routing_table.insert(make_pair(p->hdr.src_addr, t)).first;

	    packet_buffer_t::iterator iter,curr;
	    iter=m_packet_buffer.begin();
	    while(iter!=m_packet_buffer.end())
	    {
	        curr = iter;
	        iter ++;
	        if((*curr)->hdr.dst_addr==p->hdr.src_addr)
	        {
		    /*Printf((DumpPackets,"aodv%d sending buffered packet to %d %s\n",(int)MyEtherAddr,
		      (int)fw_iter->second.next_hop,(*curr)->dump().c_str()));*/
		        SentPackets++;
	            to_mac(*curr, fw_iter->second.next_hop, (*curr)->hdr.size);
	            m_packet_buffer.erase(curr);
 	        }
	    }
    }
    else
    {
	    bool update = false;
	
	    if( fw_iter->second.valid_dst_seq == false)
	        update = true;
	    else if ( p->hdr.dst_seq_num > fw_iter->second.dst_seq_num)
	        update = true;
	    else if( p->hdr.dst_seq_num == fw_iter->second.dst_seq_num && 
		     p->hdr.hop_count < fw_iter->second.hop_count )
	        update = true;
	
	    if(update == true)
	    {
	        // update entry
	        (fw_iter->second).dst_seq_num = p->hdr.dst_seq_num;
	        (fw_iter->second).valid_dst_seq = true;
	        (fw_iter->second).hop_count = p->hdr.hop_count + 1;
	        (fw_iter->second).next_hop = p->hdr.prev_hop_addr;
	        (fw_iter->second).lifetime = p->hdr.lifetime;
	    }
    }

    // ALL prior operation was for processing the forward routing table entry.
    // Now we decide whether or not to forward on the RREP.

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
	        new_p->hdr.lifetime = p->hdr.lifetime;
	        new_p->hdr.size = RREP_SIZE;
	        new_p->hdr.flags.repair=0;
	        new_p->hdr.flags.acknowledgement=0;
	        //Printf((DumpPackets,"aodv%d sending %s\n",(int)MyEtherAddr,new_p->dump().c_str()));
	        SentPackets++;
	        to_mac(new_p, rv_iter->second.next_hop, new_p->hdr.size);
	    }

	    // update lifetime entry for reverse route table
	    if((rv_iter->second).lifetime < (SimTime() + ActiveRouteTimeout))
	        (rv_iter->second).lifetime = SimTime() + ActiveRouteTimeout; 
    }
    else
    {

    }
}

template <class PLD>
void AODVI<PLD>::to_mac_depart(const pair<packet_t*,ether_addr_t>&p, unsigned int i)
{
    /*rreq_cache_t::iterator iter; 
    for(iter = m_rreq_cache.begin(); iter != m_rreq_cache.end(); iter++)
    {
	    if((iter->first == p.first->hdr.src_addr) && ((iter->second).rreq_id == p.first->hdr.rreq_id) ) 
	    {
		if(iter->second.duplicate)
		    p.first->free();
		else
		    to_mac(p.first,p.second,p.first->hdr.size);
		return;
	    }
    }*/
	to_mac(p.first,p.second,p.first->hdr.size);
}


template <class PLD>
bool AODVI_Struct<PLD>::hdr_struct::dump(std::string& str) const 
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
    }
    sprintf(buffer,"%s %d %d %d %d->%d->%d (%d %d) ",
	    t.c_str(),TTL,hop_count, rreq_id, (int)prev_hop_addr,
	    (int)src_addr,(int)dst_addr,src_seq_num,dst_seq_num);
    str=buffer;
    return type==DATA;
}


#endif /* AODVI_H */
