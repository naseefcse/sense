/*******************************************************************
 * @<title> A Simple MAC Layer Component </title>@
 *
 * @<!-- Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
 * Rensselaer Polytechnic Institute. All worldwide rights reserved.  A
 * license to use, copy, modify and distribute this software for
 * non-commercial research purposes only is hereby granted, provided
 * that this copyright notice and accompanying disclaimer is not
 * modified or removed from the software.
 *
 * DISCLAIMER: The software is distributed "AS IS" without any express
 * or implied warranty, including but not limited to, any implied
 * warranties of merchantability or fitness for a particular purpose
 * or any warranty of non-infringement of any current or pending
 * patent rights. The authors of the software make no representations
 * about the suitability of this software for any particular
 * purpose. The entire risk as to the quality and performance of the
 * software is with the user. Should the software prove defective, the
 * user assumes the cost of all necessary servicing, repair or
 * correction. In particular, neither Rensselaer Polytechnic
 * Institute, nor the authors of the software are liable for any
 * indirect, special, consequential, or incidental damages related to
 * the software, to the maximum extent the law permits. -->@ 
 *
 * @<h1> A Simple MAC Layer Component </h1>@
 *
 * NullMAC is a simple mac layer component which shows how to build
 * an intermediate layer component in SENSE. This component is
 * unrealistic because it assumes the transmission of any packet is
 * instantaneous; as a result collisions never occur. This picture shows
 * the inports and outports of a NullMAC component.
 *
 * @<center><img src=simple_mac.gif></center>@
 *******************************************************************/

#ifndef simple_mac_h
#define simple_mac_h

/*******************************************************************
 * Before building the flooding component, we must first have a structure that
 * contains all data structures and types used by the flooding component.
 * We have to use a separate class @NullMAC_Struct for this purpose, because
 * @<a href=../compc++>CompC++</a>@ doesn't allow any intenally defined classes and types to be
 * accessed from outside.  For instance, if @NullMAC@ is a component that
 * defines a packet type @packet_t@, we cannot use @NullMAC::packet_t@ to
 * refer to @packet_t@.
 *
 * @NullMAC_Struct@ is defined as a template class and the template parameter
 * is the type of the payload packets.  The NullMAC component will pack
 * the payload packets into the network layer packets.
 *******************************************************************/

template <class PLD>
struct NullMAC_Struct
{
    
/*******************************************************************
 * Here we define the mac layer packet header
 *******************************************************************/
    
	struct hdr_t
	{
		ether_addr_t dst_addr;
		ether_addr_t src_addr;
		unsigned int size;

		double tx_time;       // transmission time
		double wave_length;   // used for calculating path loss

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
	    	char buffer[50];
	    	sprintf(buffer,"%d %d %d",int(src_addr),int(dst_addr),size); 
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
	typedef smart_packet_t<hdr_t, PLD> packet_t;

};

/*******************************************************************
 * Now we can build the NullMAC component.  It must be derived from @TypeII@,
 * as it is a time-aware component in our simulation component classification.
 * It must also be derived from @NullMAC_Struct@, as it is dependent on 
 * data strutures and types defined in @NullMAC_Struct@.
 * Notice that both @TypeII@ and @NullMAC_Struct@ are classes, not components.
 * In fact, in the current version of @<a href=../compc++>CompC++</a>@ a 
 * component cannot be subclassed from any other component.
 *******************************************************************/

template <class PLD>
component NullMAC : public TypeII, public NullMAC_Struct <PLD>
{
 public:

/*******************************************************************
 * Here we declare several component parameters. @MyEtherAddr@ denotes
 * the ethernet address of this component. @RTSThreshold@ is not used
 * in this component; it is merely to make this component compatible
 * with the 802.11 component. @DumpPackets@ indicates whether or
 * not the packet content is dumped if a macro @COST_DEBUG@ is
 * defined.  @Promiscuity@ determines if packets with wrong
 * destination addresses will be picked and forwarded to the upper
 * layer.
 *******************************************************************/

    static unsigned int RTSThreshold;
    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    bool Promiscuity;

	int SentPackets, RecvPackets;

    
/*******************************************************************
 * The outport
 * @to_network_ack@ is used by the mac layer component to notify the
 * network component that the transmission of a packet sent upon request has
 * been completed.  Whether or not the transmission is successful is
 * indicated by the argument of the inport.
 * The outport @to_network_data@ sends a payload packet to the network component,
 * The inport @from_network@ is activated when there is a payload packet
 * from the transport layer or any layer above.
 *******************************************************************/
 
 	inport inline void from_network ( payload_t& pld, const ether_addr_t& dst, unsigned int size );
 	outport void to_network_data ( payload_t& pld, const ether_addr_t & dst);
 	outport void to_network_ack ( bool errflag );

/*******************************************************************
 * The inport @from_phy@ is activated when a packet from the physical layer arrives.
 * The outport @to_phy@ is to send a packet to the physical layer.
 *******************************************************************/
 	
 	inport inline void from_phy ( packet_t* pkt, bool errflag, double  power );
 	outport void to_phy (packet_t* pkt);
    
    void Start();                                    // called when simulation starts
    void Stop();                                     // called when simulation stops

};
template <class PLD>
unsigned int NullMAC<PLD>::RTSThreshold=0;

template <class PLD>
void NullMAC<PLD>::Start()
{
	SentPackets=RecvPackets=0;
}

template <class PLD>
void NullMAC<PLD>::Stop()
{
}


/*******************************************************************
 * This function first creates a new mac layer packet. Mac layer
 * packets are always passed by pointers. Since @packet_t@ is of type
 * @smart_packet_t@ in which @hdr@ and @pld@ are just two fields, we
 * can access the header by @p->hdr@ and the payload by @p->pld@.
 * However, we can not access the header of @p->pld@ in the same way,
 * because we don't know whether @p->pld@ is a pointer or a reference. It
 * is possible to use some functions provides in SENSE to do so, but
 * fortunately we don't need to access the header of @m@.
 *
 * This is because all the information we need regarding what to do
 * with the payload packet provided by the arguments.  @dst@ is 
 * the destination address of the payload packet, and @size@ is the
 * size.
 *
 * The reference count of a smart packet is always 1 upon creation.
 * When the packet is transmitted to the physical layer, so is the
 * ownership of the packet. If this component needs to keep track of
 * the packet, it would have to call @p->inc_ref()@ before it forwards
 * the packet to the physical layer. Later when it no long needs the
 * packet it can call @p->free()@ to attempt to release the packet.
 * In this example, we don't need to keep the sent packet, so @p->inc_ref()@
 * isn't called.
 *
 * Note the @Printf@ statement and its double parentheses.  This
 * statement will be executed only if a macro,
 * @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@, is
 * defined, and if the first argument, @DumpPackets@, is true. If
 * @COST_DEBUG@ is not defined, this statement will not appear
 * in the executable.
 *
 *******************************************************************/

template <class PLD>
void NullMAC<PLD>::from_network(payload_t& pld, const ether_addr_t& dst, unsigned int size)
{
    packet_t* p     = packet_t::alloc();
    p->hdr.dst_addr = dst;
    p->hdr.src_addr = MyEtherAddr;
    p->pld          = pld;
    p->hdr.size     = size + sizeof(hdr_t);

    Printf((DumpPackets,"creates %s\n",p->dump().c_str()));

    to_phy(p);                 // send the packet pointer to the physical layer
    to_network_ack(true);      // the transmission is accomplished
    SentPackets++;
}

/*******************************************************************
 * This function receives a packet pointer as well as an error flag and the receive power
 * from the physical layer. The only useful field is the packet pointer, 
 * It then looks at the destination
 * address of the packet (this destination address is not always the
 * same as that of the payload packet, although in this example they
 * are the same) to see if the packet arrives at the correct receiver.
 *******************************************************************/

template <class PLD>
void NullMAC<PLD>::from_phy(packet_t* pkt, bool errflag, double  power)
{
	RecvPackets++;
    Printf((DumpPackets,"receives %s with power %f\n",pkt->dump().c_str(),power));

    if(pkt->hdr.dst_addr==MyEtherAddr||pkt->hdr.dst_addr==ether_addr_t::BROADCAST||Promiscuity)
    {
/*******************************************************************
 * Before forwarding the encapsulated payload packet, @p->pld@, to the higher
 * layer, it must increase the reference count of the payload packet by calling
 * @p->inc_pld_ref()@. This makes sure that when the packet @p@ is being
 * released by @p->free()@, the stripped payload packet remains unaffected.
 *******************************************************************/
		pkt->inc_pld_ref();
		to_network_data(pkt->pld,pkt->hdr.dst_addr);
    }
/*******************************************************************
 * @p->free()@ has to be called because when a component receives 
 * a packet, it also automatically receives the ownership of the packet.
 *******************************************************************/
    pkt->free();
}


#endif /* simple_mac_h */
