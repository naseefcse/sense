/*************************************************************************
*   DSR Routing Component
*
*   dsr.h
*   Lijuan Zhu, Eugene Brevdo
*   Jan, 2004
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
*   permits.
   
*************************************************************************/

#ifndef dsri_h
#define dsri_h

#include <map>
#include <list>

using std::list;
using std::map;

#ifndef DSRI_TIMETOLIVE
#define DSRI_TIMETOLIVE 10
#endif

template <class PLD>
struct DSRI_Struct
{
 public:
    enum { RREQ=0,RREP=1,DATA=2};
    enum { RREQ_SIZE=68, RREP_SIZE=64, DATA_SIZE=64}; 

    struct path_t
    {
    	ether_addr_t addrs[DSRI_TIMETOLIVE+1];
    	int get_hops() const 
	    {
	        return hops;
	    }
	    int get_size() const
	    {
	        return hops*ether_addr_t::LENGTH+1;
	    }
	    ether_addr_t get_addr(int pos) const 
	    {
    	    assert(hops>pos);
	        return addrs[pos];
	    }
	    void clear()
	    {
	        hops=0;
	    }
	    void append(const ether_addr_t&);
	    void copy(const path_t&);
	    void shift_copy(const path_t&);  
	    int  truncate_copy(const path_t&, const ether_addr_t&);
	    bool contain(const ether_addr_t&);  //if this address is in the path?
     private:
	    int hops;   
    };

    struct hdr_struct
    {
    	int type;
    	int TTL;
    	ether_addr_t dst_addr;
	    ether_addr_t src_addr;
	    path_t srcrt;    // soure route, rreq route collected till now, rrep route
	    int rreq_id;
	    unsigned int size;
	    bool dump (std::string& str) const;
    };

    typedef smart_packet_t<hdr_struct,PLD> packet_t;
    typedef PLD payload_t;
};


template <class PLD>
component DSRI : public TypeII, public DSRI_Struct<PLD>
{
public:
    inport inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    inport inline void from_mac_data (packet_t* pkt, ether_addr_t& dst);
    inport inline void from_mac_ack (bool errflag);

    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();

    InfiTimer<pair<DSRI_Struct<PLD>::packet_t*,ether_addr_t> > to_mac_delay;
	
    outport void to_transport ( payload_t& pld );
    outport void to_mac (packet_t* pkt, ether_addr_t& dst, unsigned int size);
	
    inport inline void to_mac_depart(const pair<packet_t*,ether_addr_t>& p, unsigned int i);

    DSRI() { connect to_mac_delay.to_component, to_mac_depart;  }
    virtual ~DSRI() {};
    void Start();
    void Stop();
  
    ether_addr_t MyEtherAddr;
    simtime_t ForwardDelay;
    bool DumpPackets;

    int SentPackets, RecvPackets;
    int TotalHop;
protected:
    
    void init_rreq(ether_addr_t);
    void init_rrep(packet_t*);
    void receive_rreq(packet_t*);
    void receive_rrep(packet_t*);
    void receive_data(packet_t*);
  
    typedef std::list<path_t> path_list_t;
    typedef std::map<ether_addr_t,int,ether_addr_t::compare> rreq_table_t; 
    typedef std::list<packet_t*> packet_buffer_t;
    typedef std::map<ether_addr_t,path_t,ether_addr_t::compare> cache_table_t;
    packet_buffer_t packet_buffer;   // send buffer
    rreq_table_t rreq_table; //request table, for tracking received rreq's
    cache_table_t m_cache;  //route cache, store a shortest route for each known destination
    int m_rreq_id;
};
 
template <class PLD>
void DSRI<PLD>::Start() 
{	
    m_rreq_id=0;
    SentPackets=RecvPackets=0;
}

template <class PLD>
void DSRI<PLD>::Stop() 
{
    //clear the send buffer, free all the packets left. 	
    packet_buffer_t::iterator iter=packet_buffer.begin();
    for(;iter!=packet_buffer.end();iter++)
        (*iter)->free();
}

template <class PLD>
void DSRI<PLD>::from_transport( payload_t& pld, ether_addr_t& dst_addr, unsigned int size)
{
    packet_t* new_p=packet_t::alloc();
    new_p->pld=pld;
    new_p->hdr.type = DATA;
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.dst_addr = dst_addr; 
    new_p->hdr.size = DATA_SIZE + size;  

    //try to find a cached route to destination
    ether_addr_t dst = dst_addr;
    if (m_cache.find(dst)!=m_cache.end())
    //route found, send out the packet using the source route
    {
	    new_p->hdr.srcrt.copy(m_cache[dst]);
	    new_p->hdr.size += new_p->hdr.srcrt.get_size();
    	SentPackets++;
	    to_mac(new_p, new_p->hdr.srcrt.get_addr(0), new_p->hdr.size);
    }  
    else 
    {
	    //no route cached, then initiate a route discovery and send to buffer 	
	    init_rreq(dst_addr);
	    packet_buffer.push_back(new_p);	
    }     
}    

template <class PLD>
void DSRI<PLD>::from_mac_ack (bool)
{
  return;
}	

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
template <class PLD>
void DSRI<PLD>::from_pm_node_up()
{
  return;
}

template <class PLD>
void DSRI<PLD>::from_mac_data (packet_t* packet, ether_addr_t& dst_addr)
{
    if(dst_addr==MyEtherAddr||dst_addr==ether_addr_t::BROADCAST)
    {
	    RecvPackets++;
        Printf((DumpPackets,"received %s\n",packet->dump().c_str()));

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

template <class PLD>
void DSRI<PLD>::receive_data(packet_t* p)
{
    if(p->hdr.dst_addr == MyEtherAddr)  //this packet is for me
    {
	    p->inc_pld_ref();
	    to_transport(p->pld);
	return;
    }

    // not for me, forward this packet using the source route
    packet_t* new_p = packet_t::alloc();
    new_p->hdr.type = p->hdr.type;
    new_p->hdr.src_addr = p->hdr.src_addr;
    new_p->hdr.dst_addr = p->hdr.dst_addr;
    new_p->hdr.size = p->hdr.size - ether_addr_t::LENGTH;
    new_p->hdr.srcrt.shift_copy(p->hdr.srcrt); //remove myself from the path
    p->inc_pld_ref();
    new_p->pld = p->pld;
    SentPackets++;
    to_mac(new_p, new_p->hdr.srcrt.get_addr(0), new_p->hdr.size);
}

template <class PLD>
void DSRI<PLD>::init_rreq(ether_addr_t dst)
{
    packet_t* new_p = packet_t::alloc();
    new_p->hdr.type = RREQ;
    new_p->hdr.TTL = DSRI_TIMETOLIVE-1;   
    new_p->hdr.src_addr = MyEtherAddr;
    new_p->hdr.dst_addr = dst;
    new_p->hdr.srcrt.clear();  //initiate the route length to 0
    new_p->hdr.srcrt.append(MyEtherAddr);
    new_p->hdr.size= RREQ_SIZE + new_p->hdr.srcrt.get_size();
    new_p->hdr.rreq_id=++m_rreq_id;  //increase m_rreq_id by 1 for each init_rreq
    rreq_table[MyEtherAddr]=m_rreq_id;  //insert this rreq to rreq_table
    SentPackets++;
    to_mac(new_p, ether_addr_t::BROADCAST,new_p->hdr.size);
}


template <class PLD>
void DSRI<PLD>::receive_rreq(packet_t* p)
{
    ether_addr_t src=p->hdr.src_addr;
    ether_addr_t dst=p->hdr.dst_addr;
    ether_addr_t myaddr=MyEtherAddr;
    int id=p->hdr.rreq_id;

    rreq_table_t::iterator iter = rreq_table.find(src);
    if(iter==rreq_table.end())
    {
        //has not received rreq from this source before, set the rreq_id be 0*/
	    iter=rreq_table.insert(make_pair(src,0)).first;
    }
    if(iter->second>=id ||(p->hdr.srcrt.contain(myaddr)))
    { 
    	//this rreq has been received before or myaddr has been in the path already
        Printf((DumpPackets,"duplicated RREQ received\n"));
        return;
    }  	
    //this rreq is new to me
    rreq_table[src]=id; 
    if (dst==myaddr)
    {
	    // this is the destination, initiate rrep
	    Printf((DumpPackets,"received RREQ at destination form %d\n",(int)p->hdr.src_addr));
	    init_rrep(p);
    }  
    else
    {
	    if(p->hdr.TTL>0)  //forward this rreq further
	    {
	        packet_t* new_p = packet_t::alloc();	
	        new_p->hdr.srcrt.copy(p->hdr.srcrt);  //copy the route
	        new_p->hdr.srcrt.append(myaddr); //add myself at the end of the path
	        new_p->hdr.type = RREQ;
	        new_p->hdr.TTL = p->hdr.TTL-1;        //TTL-1
	        new_p->hdr.src_addr = p->hdr.src_addr;
	        new_p->hdr.dst_addr = p->hdr.dst_addr;
	        new_p->hdr.rreq_id = p->hdr.rreq_id;
	        new_p->hdr.size = p->hdr.size + ether_addr_t::LENGTH;
	        SentPackets++;
	        to_mac_delay.Set(make_pair(new_p, (ether_addr_t)ether_addr_t::BROADCAST), SimTime()+Random(ForwardDelay));
        }      
    }	
}

template <class PLD>
void DSRI<PLD>::init_rrep(packet_t* p)
{
    packet_t* new_p = packet_t::alloc();
    new_p->hdr.type = RREP;
    new_p->hdr.src_addr = p->hdr.dst_addr;
    new_p->hdr.dst_addr = p->hdr.src_addr;
    new_p->hdr.srcrt.copy(p->hdr.srcrt);
    new_p->hdr.srcrt.append(MyEtherAddr);
    new_p->hdr.size= RREP_SIZE + new_p->hdr.srcrt.get_size();
    //unicast the rrep back to source using the route collected 
    SentPackets++;
    to_mac(new_p, new_p->hdr.srcrt.get_addr(p->hdr.srcrt.get_hops()-1),new_p->hdr.size); 
}

template <class PLD>
void DSRI<PLD>::receive_rrep(packet_t* p)
{
    ether_addr_t myaddr=MyEtherAddr;
    ether_addr_t dst=p->hdr.src_addr;
    path_t path;   
    
    // truncate_copy returns the position where MyEtherAddr is found
    // It copies from this position (exclusive) to the end
    int index=path.truncate_copy(p->hdr.srcrt,MyEtherAddr);
    if(myaddr!=p->hdr.dst_addr) //this is an intermediate node
    {
	    //forward this rrep further
	    packet_t* new_p = packet_t::alloc();
	    new_p->hdr.type = p->hdr.type;
	    new_p->hdr.src_addr = p->hdr.src_addr;
	    new_p->hdr.dst_addr = p->hdr.dst_addr;
	    new_p->hdr.srcrt.copy(p->hdr.srcrt);
	    new_p->hdr.size= p->hdr.size;  
	    assert(index>0);
	    SentPackets++;
	    to_mac(new_p, new_p->hdr.srcrt.get_addr(index-1), new_p->hdr.size);
    }    
    else
    {
	    assert(index==0);
    }

    //for all the nodes, update the cache_table
    cache_table_t::iterator citer=m_cache.find(dst);  
    if((citer==m_cache.end())||(citer->second.get_hops()>path.get_hops()))
    {
        //there is no route for this detination, or this new path is shorter than the old one   
	    m_cache[dst]=path;  //store this new path in cache table
	    if(citer!=m_cache.end()) return; // no need to empty buffer if a old path exists

	    //new path can be used to send out the packets in the buffer
	    packet_buffer_t ::iterator iter,curr;
	    iter=packet_buffer.begin();
	    while(iter!=packet_buffer.end())
	    {//keep checking the packet buffer to see if there is any packet can be sent out
	        curr=iter;
	        iter++;
	        if((*curr)->hdr.dst_addr==p->hdr.src_addr)
	        {//found one, send out this packet
		        packet_t* new_p=*curr;
		        new_p->hdr.srcrt.copy(m_cache[dst]); //use the source route to the destination
		        new_p->hdr.size += new_p->hdr.srcrt.get_size();
		        SentPackets++;
		        to_mac(new_p, new_p->hdr.srcrt.get_addr(0), new_p->hdr.size); 
		        packet_buffer.erase(curr);  //delete this packet from the buffer 
	        }
	    }	
    }
}     

template <class PLD>
void DSRI<PLD>::to_mac_depart(const pair<packet_t*,ether_addr_t>&p, unsigned int i)
{
	to_mac(p.first,p.second,p.first->hdr.size);
}


template <class PLD>
bool DSRI_Struct<PLD>::hdr_struct::dump(std::string& str) const
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
    }
    sprintf(buffer,"%s %d %d %d",t.c_str(),TTL,(int)src_addr,(int)dst_addr);
    str=buffer;
    return type==DATA;
}        

template <class PLD>
void DSRI_Struct<PLD>::path_t::append(const ether_addr_t& addr)
{
    assert(hops<DSRI_TIMETOLIVE+1);
    addrs[hops]=addr;
    hops++;
} 

template <class PLD>
void DSRI_Struct<PLD>::path_t::copy(const path_t& path) 
{ 
    for(int i=0;i<path.hops;i++)
	addrs[i]=path.addrs[i];
    hops=path.hops;
}

template <class PLD>
void DSRI_Struct<PLD>::path_t::shift_copy(const path_t& path)
{
    assert(path.hops>0);
    for(int i=0;i<path.hops-1;i++)
	addrs[i]=path.addrs[i+1];
    hops=path.hops-1;
}    	

template <class PLD>
int DSRI_Struct<PLD>::path_t::truncate_copy(const path_t& path, const ether_addr_t& addr)
{
    int i=0;
    while(i<path.hops)
    {
        if (addr==path.addrs[i])
	{
	    for(int j=i+1;j<path.hops;j++)
		addrs[j-i-1]=path.addrs[j];
	    hops=path.hops-i;
	    return i;
	}
	i++;
    }
    return -1;
}

template <class PLD> 
bool DSRI_Struct<PLD>::path_t::contain(const ether_addr_t& addr)
{
    for(int i=0;i<hops;i++)
	if(addr==addrs[i])
	    return true;
    return false;
}      	    	

#endif /*dsri_h*/
