/*************************************************************************
*   @<title> A Flooding Routing Component </title>@
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
*   @<h1> A Flooding Routing Component </h1>@
*
*   Flooding is a simple network layer component which
*   rebroadcasts every received packet not destined to itself. This
*   picture shows the inports and outports of this component.
*
*   @<center><img src=flooding_routing.gif></center>@
*************************************************************************/

#ifndef flooding_h
#define flooding_h
#include <map>

/*******************************************************************
 * Before building the flooding component, we must first have a structure that
 * contains all data structures and types used by the flooding component.
 * We have to use a separate class @Flooding_Struct for this purpose, because
 * @<a href=../compc++>CompC++</a>@ doesn't allow any intenally defined classes and types to be
 * accessed from outside.  For instance, if @Flooding@ is a component that
 * defines a packet type @packet_t@, we cannot use @Flooding::packet_t@ to
 * refer to @packet_t@.
 *
 * @Flooding_Struct@ is defined as a template class and the template parameter
 * is the type of the payload packets.  The Flooding component will pack
 * the payload packets into the network layer packets.
 *******************************************************************/

template <class PLD>
struct Flooding_Struct
{

/*******************************************************************
 * Here we define the network layer packet header, which includes the sequence number,
 * the size of the header, and the time the packet is sent by the network
 * layer.
 *******************************************************************/

    struct hdr_struct
    {
		unsigned int seq_number;
		unsigned int size;
		double send_time;

/*******************************************************************
 * The function @dump()@ is used to display the
 * content of the packet. The value it returns indicates whether the
 * payload is valid or not. If the payload is valid, the content of
 * the payload will be printed out automatically, and otherwise
 * ignored. In this example, this payload filed is always valid, so
 * a true value is returned every time @dump()@ is called.
 *******************************************************************/
		
		bool dump(std::string& str) const 
		{ 
	    	char buffer[30];
	    	sprintf(buffer,"%d %d",seq_number,size); 
	    	str=buffer;
	    	return true;
		}
    };

/*******************************************************************
 * Here we define the packet types.  @payload_t@ is the type of the
 * payload, which could be a pointer to the payload packet or a 
 * reference to the payload packet.  We don't need to distinguish these
 * two cases, as the SENSE library handles this automatically for us.
 * @packet_t@ is the type of the network layer packet, with @hdr_struct@
 * as the header and @PLD@ as the payload.
 *******************************************************************/

	typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

};


/*******************************************************************
 * Now we can build the Flooding component.  It must be derived from @TypeII@,
 * as it is a time-aware component in our simulation component classification.
 * It must also be derived from @Flooding_Struct@, as it is dependent on 
 * data strutures and types defined in @Flooding_Struct@.
 * Notice that both @TypeII@ and @Flooding_Struct@ are classes, not components.
 * In fact, in the current version of @<a href=../compc++>CompC++</a>@ a 
 * component cannot be subclassed
 * from any other component.
 *******************************************************************/

template <class PLD>
component Flooding : public TypeII, public Flooding_Struct<PLD>
{
 public:

/*******************************************************************
 * Here we declare several component parameters. @MyEtherAddr@ is the
 * ethernet address of this component. @ForwardDelay@ is the period of
 * time that must elapse before this component can sent a packet to
 * the mac layer if the packet is to be rebroadcastm in order to avoid
 * collision caused by simultaneous transmissions.  
 * @DumpPackets@ indicates whether or not packet
 * content is dumped if the macro 
 * @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@ is defined.
 *******************************************************************/

    ether_addr_t MyEtherAddr;
    simtime_t ForwardDelay;
    bool DumpPackets;

/*******************************************************************
 * These are variables to record statistics.
 *******************************************************************/

    int SentPackets;
    int RecvPackets;
    int RecvUniPackets;
    double TotalDelay;
    int TotalHop;

/*******************************************************************
 * The inport
 * @from_mac_ack@ is used by the mac layer component to notify this
 * component that the transmission of a packet sent upon request has
 * been completed.  Whether or not the transmission is successful is
 * indicated by the argument of the inport.
 * The inport @from_transport@ is activated when there is a payload packet
 * from the transport layer or any layer above.
 * The inport @from_mac_data@ is activated when there is a packet
 * arriving from the mac layer.
 *******************************************************************/
    
    inport inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    inport inline void from_mac_data (packet_t* pkt, ether_addr_t& dst);
    inport inline void from_mac_ack (bool errflag);

    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();

/*******************************************************************
 *  There are two outports, one to send the payload packet to the
 * transport layer and the other to send the packet to the mac layer
 *******************************************************************/

    outport void to_transport ( payload_t& pld );
    outport void to_mac (packet_t* pkt, ether_addr_t& dst, unsigned int size);

/*******************************************************************
 *  This component needs a timer to delay packet retranmission to avoid possible
 *  collisions.  Since we don't know the number
 *  of events that could coexist (there is virtually no upper bound),
 *  so we can @<a href=manual.html#InfiTimer>InfiTimer</a>@.  The type of
 *  the @InfiTimer@ is just @packet_t@, but due to the implementation of 
 *  @<a href=../compc++>CompC++</a>@, the complete name including the
 *  class is required here.
 *  The @depart()@ inport will receive the pointer to the delayed packet 
 *  and the slot number which will be ignored.  This inport must be
 *  connected to the timer.
 *******************************************************************/
	InfiTimer<Flooding_Struct<PLD>::packet_t*> delay;
	inport inline void depart(packet_t* p, unsigned int i);
	
/*******************************************************************
 * @Start()@ will be called when the simulation is started, @Stop()@ when the
 * simulation is stopped.
 *******************************************************************/

    void Start();
    void Stop();
    Flooding();
    virtual ~Flooding();
	
 protected:

    bool m_mac_busy;                  // if the mac layer is busy
    unsigned int m_seq_number;        // current sequence number

/*******************************************************************
 * In the original flooding protocol, whenever a node recieves a packet
 * it will rebroadcast it.  However, this would result in an exponential
 * increase on the number of packets being trasmitted.  Here we adopt
 * an optimization which would rebroadcast the received packet only when
 * it has never been seen before.
 *
 * One subsequent problem is how to know if a packet has been received
 * before.  An easy solution is to use an unique sequence number when
 * the source initiates a packet.  However, we don't want to store
 * the sequence numbers for all packets received so far.  @seq_number_t@
 * is used to remember a few (32 more precisely) most recent packets.
 * The details of this class are not revelant.  Just remember that
 * the only function @check()@ will tell if a sequence number has
 * seen before.
 *******************************************************************/

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
Flooding<PLD>::Flooding()
{
	connect delay.to_component, depart;
}
template <class PLD>
Flooding<PLD>::~Flooding()
{
}

template <class PLD>
void Flooding<PLD>::Start() 
{
    m_mac_busy=false;
    m_seq_number=1u;
    m_seq_cache.insert(make_pair(MyEtherAddr,seq_number_t()));

    SentPackets=RecvPackets=RecvUniPackets=0l;
    TotalDelay=0.0;
}
template <class PLD>
void Flooding<PLD>::Stop() 
{
    //printf("%s: sent %d, recv %d\n",GetName(),SentPackets,RecvPackets);
    //printf("%s: %f \n", GetName(), (double)RecvPackets/RecvUniPackets);
}

/*******************************************************************
 * This function is the member function bound to the inport
 * @from_transport@.  When there is a payload packet to be
 * transmitted, a new network layer packet will be created, with the
 * payload packet stored in the payload field.
 *******************************************************************/

template <class PLD>
void Flooding<PLD>::from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size)
{

/*******************************************************************
 *  If the mac layer is busy transmitting another packet, we have to
 *  ignore this new packet.  When doing so, we must not forgot to
 *  release the packet.  Since @PLD@ may be a packet structure or a
 *  packet pointer, the best way is to call the @free()@ function via
 *  the helper funciton @free_pld@.
 *******************************************************************/

    if(m_mac_busy)
    {
		packet_t::free_pld(pld);
		return;               
    }

    packet_t* p=packet_t::alloc();
    p->pld=pld;
    m_seq_number++;
    m_seq_cache[MyEtherAddr].check(m_seq_number);
    p->hdr.seq_number=m_seq_number;
    p->hdr.size=size+2*sizeof(unsigned int);
    m_mac_busy=true;
    SentPackets++;

/*******************************************************************
 * The @Printf@ statement will be executed only if a macro,
 * @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@, 
 * is defined, and if the first argument, @DumpPackets@,
 * is true. If @COST_DEBUG@ is not defined, this statement will have
 * no effect.
 *******************************************************************/

    Printf((DumpPackets,"creates %s\n",p->dump().c_str()));

    p->hdr.send_time=SimTime();
    to_mac(p,ether_addr_t::BROADCAST,size);
}

/*******************************************************************
 * We are now receiving a packet from the mac layer.
 *******************************************************************/

template <class PLD>
void Flooding<PLD>::from_mac_data (packet_t* pkt, ether_addr_t& dst)
{
  ether_addr_t src;
  src=pkt->access_pld().src_addr;
  RecvPackets++;
  TotalDelay+=SimTime()-pkt->hdr.send_time;

  Printf((DumpPackets,"receives %s\n",pkt->dump().c_str()));
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
    }
    else  
    {
      // rebroadcast needed
      if(m_mac_busy==false)
      {
	Printf((DumpPackets,"forwards %s\n",pkt->dump().c_str()));
/*******************************************************************
 * Note that this @Write()@ function takes two arguments. We don't
 * want the mac layer to send the packet immediately, so the second
 * argument is used to tell the mac layer when to do so.
 *
 * We actually have a bug here. We don't set the flag @m_mac_busy@ to
 * @true@ after writing the packet to the outport. The reason is we
 * don't have a simple way to modify a variable at a certain future
 * time. The consequence is that @m_mac_busy@ doesn't reflect the
 * status of the mac layer correctly. However, it doesn't matter
 * because packets that go through may still be dropped by the mac
 * layer.
 * 
 *******************************************************************/

	SentPackets++;
	pkt->hdr.send_time=SimTime();
	delay.Set(pkt,SimTime()+Random(ForwardDelay));
	return;
      }
    }
  }
  pkt->free();
}

/*******************************************************************
 * If an ack is recevied, the mac layer must be free, not matter
 * whether the previous transmission is successful or not.
 *******************************************************************/

template <class PLD>
void Flooding<PLD>::from_mac_ack(bool)
{
    m_mac_busy=false;
}

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
template <class PLD>
void Flooding<PLD>::from_pm_node_up()
{
  return;
}

template <class PLD>
void Flooding<PLD>::depart(packet_t* p, unsigned int i)
{
	m_mac_busy=true;
	to_mac(p,ether_addr_t::BROADCAST,p->hdr.size);
}
#endif /*flooding_h*/
