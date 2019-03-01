/*************************************************************************
*   @<title> A Constant Bit Rate Component </title>@
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
*   permits. -->@
*
*   @<h1> A Constant Bit Rate (CBR) Component </h1>@
*
*   CBR is an applicate layer component that generates constant
*   traffic during the simulation.  This picture shows the inport and
*   outport of this component.
*
*   @<center><img src=cbr.gif></center>@
*************************************************************************/

#ifndef cbr_h
#define cbr_h
#include <vector>
#include <set>

/*******************************************************************
 * Before building the CBR component, we must first have a structure that
 * contains all data structures and types used by the CBR component.
 * We have to use a separate class @CBR_Struct for this purpose, because
 * @<a href=../compc++>CompC++</a>@ doesn't allow any intenally defined classes and types to be
 * accessed from outside.  For instance, if @CBR@ is a component that
 * defines a packet type @packet_t@, we cannot use @CBR::packet_t@ to
 * refer to @packet_t@.
 *******************************************************************/

struct CBR_Struct
{
/*******************************************************************
 * Here we define the packet we will be using in the application layer.
 * Each packet contains the source address, the destination address,
 * the size of the packet, and the time the packet is sent by its source.
 *******************************************************************/

	struct packet_t
	{
		ether_addr_t src_addr;
		ether_addr_t dst_addr;
		int data_size;
		simtime_t send_time;

/*******************************************************************
 * The @dump()@ function prints out the detailed information about
 * the packet.  Two versions are required, one taking no argument and
 * the other taking a string.  The former is called within the application
 * layer, while the latter is called from other layers. 
 *******************************************************************/

		std::string dump() const 
		{
		    char buffer[50];
		    sprintf(buffer,"%d %d %d",(int)src_addr,(int)dst_addr,data_size);
	    	return buffer;
		}
		void dump(std::string& str) const { str=dump();}
	};


/*******************************************************************
 * A connection is specified by the destination address, the size
 * of CBR packets, and the interval between two consecutive
 * sends.  A CBR component may have multiple connections, so we use a vector
 * to store them.
 *******************************************************************/
	
    typedef triple<ether_addr_t,int,double> connection_t;
    std::vector<connection_t> Connections;
};

/*******************************************************************
 * Now we can build the CBR component.  It must be derived from @TypeII@,
 * as it is a time-aware component in our simulation component classification.
 * It must also be derived from @CBR_Struct@, as it is dependent on 
 * data strutures and types defined in @CBR_Struct@.
 * Notice that both @TypeII@ and @CBR_Struct@ are classes, not components.
 * In fact, in the current version of @<a href=../compc++>CompC++</a>@ a 
 * component cannot be subclassed
 * from any other component.
 *******************************************************************/

component CBR : public TypeII, public CBR_Struct
{
public:

/*******************************************************************
 * Inports and outports are declared here.  Although from the names of ports
 * you may think that a CBR component must be linked to a transport layer component,
 * it is not absolutely necessary.  The CBR component can be linked to any
 * lower layer component, provided their inports and outport are compatible.
 *
 * To send out a packet, the CBR component must tell the transport layer component (or any
 * layer below) the content of the packet, the destination, and the size.  The receive a packet,
 * only the packet content is needed.
 * 
 *******************************************************************/

	outport void to_transport (packet_t & packet, ether_addr_t & dst, unsigned int size);
	inport inline void from_transport ( packet_t & pkt);

/*******************************************************************
 * The CBR component needs to periodically schedule events at specified interval to sent out
 * packets, so we must use a timer.  Although each CBR component may have multiple connections,
 * we just need to have one @MultiTimer@, since a @<a href=manual.html#MultiTimer>MultiTimer</a>@ 
 * can schedule multiple events.
 * @trigger_t@ is specified as the type of the timer, since we don't need to store anything
 * into the scheduled events.
 *******************************************************************/

    MultiTimer < trigger_t > connection_timer;

/*******************************************************************
 * This inport is to be connected with an outport, named @to_component@, of the @MultiTimer@.
 * The second argument returns the slot as which the event was scheduled
 *******************************************************************/
    inport inline void create(trigger_t& t, int i);
    
    
/*******************************************************************
 * Here we declare several component parameters. @MyEtherAddr@ is the
 * ethernet address of this component.  @DumpPackets@ indicates
 * whether or not the packet content is dumped if a macro 
 * @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@
 * is defined.  @FinishTime@ specifies a time after which the CBR component
 * won't be active, i.e., it won't create more packets; this gives the
 * network a chance to clean up packets in transit, forcing packets sent 
 * earlier to reach their destinations.
 *******************************************************************/

    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    double FinishTime;

/*******************************************************************
 * These variables keep track of the numbers of packets sent and
 * received, and the total delay, respectively.
 *******************************************************************/

    long SentPackets, RecvPackets;
    double TotalDelay;
    void	clearStats() { SentPackets = RecvPackets = 0; TotalDelay = 0; }

/*******************************************************************
 * These three functions are required by every components. @Start()@
 * will be called when the simulation is started, @Stop()@ when the
 * simulation is stopped, @Setup()@ when the simulation is being set
 * up.
 *******************************************************************/
    CBR();
    virtual ~CBR();
    void Start();
    void Stop();

/*******************************************************************
 * This set stores all the nodes from which this component receives packets
 *******************************************************************/

    typedef std::set<ether_addr_t, ether_addr_t::compare> addr_table_t;
    addr_table_t m_sources;

};

/*******************************************************************
 * Here we connect the timer with the inport @create@.
 *******************************************************************/

CBR::CBR()
{
	connect connection_timer.to_component, create;
}

CBR::~CBR()
{
}

/*******************************************************************
 * In this function, variables are initialized and initial events
 * are scheduled.  A @MultiTimer@ can schedule multiple events, distinguished
 * by the slot index (this second argument).
 *******************************************************************/

void CBR::Start()
{
    SentPackets=RecvPackets=0l;
    TotalDelay=0.0;
    for(unsigned int i=0;i<Connections.size();i++)
    {
#ifdef COST_DEBUG
	printf("connection: %d -> %d, packet size: %d, interval: %f\n",
	(int)MyEtherAddr, (int)Connections[i].first, Connections[i].second, Connections[i].third); 
#endif
		connection_timer.Set(Random(Connections[i].third),i);
    }
}

/*******************************************************************
 * This function collects and/or prints statistics.
 *******************************************************************/

void CBR::Stop()
{
#ifdef COST_DEBUG
    if(SentPackets!=0)
    {
        printf("%d sent %ld packets to ",(int)MyEtherAddr,SentPackets);
        for(unsigned int i=0;i<Connections.size();i++)
        {
		    printf("%d ",(int)Connections[i].first);
        }
        printf("\n");
    }
    if(RecvPackets!=0) 
    {
        //printf("%s : average delay: %f\n",GetName(), TotalDelay/RecvPackets);
        addr_table_t::iterator iter;
        printf("                       %d received %ld packets from ", (int)MyEtherAddr, RecvPackets);
        for(iter=m_sources.begin();iter!=m_sources.end();iter++)
        {
            printf("%d ", (int)(*iter));
        }
        printf("\n");
    }
#endif
}

/*******************************************************************
 * A new packet arrives from the transport layer.
 *******************************************************************/

void CBR::from_transport(packet_t& p)
{

/*******************************************************************
 * The @Printf@ statement will be executed only if a macro,
 * @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@, 
 * is defined, and if the first argument, @DumpPackets@,
 * is true. If @COST_DEBUG@ is not defined, this statement will not
 * appear in the executable.
 *******************************************************************/

    Printf((DumpPackets,"receives %s\n",p.dump().c_str()));

    if(p.dst_addr==MyEtherAddr)
    {
		RecvPackets++;
		TotalDelay += SimTime() - p.send_time;
		m_sources.insert(p.src_addr);
    }
    else
    {
		printf("cbr %d received a packet destined for %d\n",
	       (int)MyEtherAddr,(int)p.dst_addr);
    }
}

/*******************************************************************
 * It is time to send out a packet.  The second argument of this
 * function tells the slot index of the active event. This slot
 * index can also identify which connection is associated with
 * the active event. We must remember to schedule the next
 * event, using the same slot index.
 *******************************************************************/

void CBR::create(trigger_t&, int index)
{
    if(SimTime()>FinishTime) return;

    packet_t p;
    p.data_size=Connections[index].second + 2*ether_addr_t::LENGTH;
    p.dst_addr=Connections[index].first;
    p.src_addr=MyEtherAddr;
    p.send_time=SimTime();

    Printf((DumpPackets,"sends %s\n",p.dump().c_str()));

    to_transport(p,p.dst_addr,p.data_size);
    SentPackets++;
    connection_timer.Set(SimTime()+Connections[index].third,index);
}

#endif /* cbr_h*/
