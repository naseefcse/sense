/************************************************************************
 * @<title> M/M/1 Simulation written in COST/COMPC++ </title>@
 *
 * @<h1> M/M/1 Simulation </h1>@
 * To help users understand how COST facilitates the development of 
 * simulation models, we will describe in detail the process of building an 
 * M/M/1 system. 

 * In an M/M/1 system, packets(or customers) arrive according 
 * to a Poisson distribution and compete for the service in an FIFO
 * (First-In-First-Out) manner. The service time is also drawn from a 
 * Poisson distribution. In practice, M/M/1 systems are useful because many
 * complex systems can be abstracted as compositions of simple M/M/1 systems. 
 * Theoretically, M/M/1 system has an accurate mathematical solution
 * with respect to the mean arrival rate and the mean service rate. As a 
 * result, it is well suited for validating simulation results. 
 *
 * An M/M/1 system built using COST/COMPC++ is composed of four components, namely, 
 * Source, FIFO, Server and Sink. Packets are created by Source, queued 
 * in FIFO, served by Server, and dispatched from Sink. 
 *
 * @<center><img src=mm1.png></center>@
 *
 * @<h2> Using COST </h2>@
 * To use COST, simply include the header file @cost.h@. The other file,
 * @deque@, is required by the FIFO component which we will describe momentarily.
 ************************************************************************/

#include "../../common/cost.h"
#include <deque>

/************************************************************************
 * @<h2> New Data Type </h2>@
 * A new data type, @packet_t@, is created to represent the packets 
 * that flow through the M/M/1 system. We are interested in measuring the 
 * delay of packets, which is equal to the time spent in the 
 * FIFO queue plus the service time. For this purpose, @packet_t@ contains 
 * two fields, @arrival_time@ and @departure_time@, which 
 * record the arrival time and the departure time, respectively. In 
 * addition, to identify each packet, another field @seq_number@ is 
 * declared to hold the sequence number (these fields are not all used
 * in the sample simulation, but they may be essential in other cases).
 ************************************************************************/
 
struct packet_t
{
    int seq_number;
    double arrival_time,departure_time;
};

/************************************************************************
 * @<h2> Source </h2>@
 * Now let us build our first component class @Source@, derived 
 * from @TypeII@. @TypeII@ is the base class for all simulation components in COST.
 * All components in COST must be derived from @TypeII@. And the reason
 * it is called TypeII is that in our simulation component classification
 * they are Time-Aware, meaning they are aware of the existence of
 * simulated time but have no control over it.
 ***********************************************************************/

component Source : public TypeII
{


public:

/************************************************************************
 * The @Source@ component creates packets randomly at an average rate 
 * specified by the parameter @interval@. It has an outport with
 * an argument type @packet_t@ and return type @void@ (nothing to return), 
 * and a timer @wait@ for scheduling the event to deliver the next packet.
************************************************************************/

    double interval;
    outport void out (packet_t&);
    Timer <trigger_t> wait;
    
/************************************************************************
 * A timer is defined a component.  It has an outport called @to_component@
 * which will be invoked once the event associated with the timer is activated.
 * Therefore, any component that embeds a timer component must also connect 
 * the outport of the timer to one of its inports.  The inport @create@ is declared
 * for this purpose, and its return type must be @void@, and it (only) argument must be
 * consistent with the template parameter of the timer (@trigger_t@ in this case).
************************************************************************/
    
    inport inline void create(trigger_t& t);

    Source();
    virtual ~Source() {}
    void Start();

private:
	packet_t m_packet; // for temporarily holding the outgoing packet
    int m_seq_number; // each packet has an unique sequence number
};
/************************************************************************
 * Here is where we connect the outport of the timer component to the @create()@ inport
 ************************************************************************/
Source::Source()
{
    connect wait.to_component,create;
}

/************************************************************************
 * The @Start()@ function, invoked the moment the simulation just
 * gets started, i.e., at the simulation time zero, is where a component
 * can perform initialization of variables and schedule initial events
 * by calling the @Set()@ method of the desired timer. 
 * @Exponential()@ is an function declared in the @TypeII@
 * class to generate a Poisson distribution.
 ************************************************************************/

void Source::Start()
{
    m_seq_number=0;
    wait.Set(Exponential(interval));
}

/************************************************************************
 * The @create()@ function is bound to the outport of the timer @wait@,
 * so it is invoked every time the timer becomes activated. Upon activation,
 * it needs to schedule the next timer event whose timestamp represents the time
 * the next packet is to be generated, and deliver the current packet
 * to the outport @out@.
 *
 * @SimTime()@, a member function declared in the @TypeII@ class, returns
 * the current simulated time. 
 ************************************************************************/

void Source::create(trigger_t&)
{
    wait.Set(SimTime()+Exponential(interval) );
    m_packet.seq_number=m_seq_number++;
    m_packet.arrival_time = SimTime();
    out(m_packet);
    return;
}

/************************************************************************
 * @<h2>FIFO Queue</h2>@
 * The @FIFO@ component is declared as a template class with a template
 * parameter specifying the type of packets that the queue can hold. When
 * instantiated with different packet types, this component can be 
 * reused to handle packets of any type.
 ************************************************************************/
 
template < class DATATYPE >
component FIFO : public TypeII
{
public:
    FIFO();
    virtual ~FIFO();
    void Start();

/************************************************************************
 * The parameter @queue_length@ indicates the maximum number of packets
 * that can be held in the queue.
 ************************************************************************/

    unsigned int queue_length;

/************************************************************************
 * This component contains an inport and outport, to receive
 * and sent packets, of type @DATATYPE@, the template parameter,
 * as the parameter type, and @void@ as the return type. 
 * Another port, named @next@, is an inport
 * indicating that it is time to send out the next packet. Its parameter
 * type is @trigger_t@, a predefined empty class.
 ************************************************************************/

    inport inline void in(DATATYPE&);
    outport void out(DATATYPE&);
    inport inline void next();

private:

/************************************************************************
 * A FIFO queue is usually linked to a server, and it needs to know
 * whether the server is busy or not, since when it receives a new packet
 * it must determine if it needs to pass on the packet directly or save
 * it in an internal queue.  @m_busy@ indicates the status of the server,
 * and @m_queue@ is the internal queue.
 ************************************************************************/

    bool m_busy;
    std::deque<DATATYPE> m_queue;
};

template < class DATATYPE >
FIFO <DATATYPE> :: FIFO()
{
}

template < class DATATYPE >
FIFO <DATATYPE> :: ~FIFO()
{
}

/************************************************************************
 * @m_busy@ must be set to false at the beginning, since the server
 * is free at time 0.
 ************************************************************************/
template < class DATATYPE >
void FIFO <DATATYPE> :: Start()
{
    m_busy=false;
}

/************************************************************************
 * The inport @in@ is invoked when a new packet arrives.
 * Packets are passed by reference to avoid copying overhead.
 * @m_busy@ remembers the status of the connected server. If the
 * server is free, simply forward the packet to the outport and update
 * @m_busy@ accordingly. Otherwise, append the packet to the tail of
 * the internal queue, if the queue is not full.
 ************************************************************************/
template < class DATATYPE >
void FIFO<DATATYPE>::in(DATATYPE& packet)
{
    if (!m_busy)
    { // server is free, we can pass it through
        out(packet);
        m_busy=true;
    } 
    else if (m_queue.size()<queue_length)
    { // queue is not full
        	m_queue.push_back(packet);
    }
    return;
}

/************************************************************************
 * When asked for the next packet, give the first one in the queue. If the
 * queue is empty, change the value of @m_busy@ to indicate the server
 * is now free so that the next arriving packet will
 * directly go through without staying in the queue first.
 ************************************************************************/


template < class DATATYPE >
void FIFO <DATATYPE> :: next()
{
    if (m_queue.size()>0)
    {
        out(m_queue.front());
        m_queue.pop_front();
    } 
    else
    {
        m_busy=false;
    }
    return;
}

/************************************************************************
 * @<h2>Server</h2>@
 * @Sever@ is a template component too. It simulates the
 * service for each incoming packet. Ports are similar to those of
 * the @FIFO@ component, except that the @next@ port is an
 * outport, which is to be connected with the @next@ port of the
 * @FIFO@ component. The timer @wait@ is used to schedule the event representing
 * the completion of the service.
 ************************************************************************/

template <class DATATYPE>
component Server : public TypeII
{
public:
	virtual ~Server() {}
    double service_time;  // the average serice time

/************************************************************************
 * Packets arrive at the inport @in@, after a random service time, depart
 * from @out@. Accompanying the departure of an event, a signal must also be
 * sent out through the outport @next@ to indicate that the server is now
 * ready to receive the next packet.
 ************************************************************************/

    inport inline void in(DATATYPE&);
    outport void out(DATATYPE&);
    outport void next();
    inport inline void depart(trigger_t&);

    Timer<trigger_t> wait;

    Server();

private:
    DATATYPE m_packet; // 
};
/************************************************************************
 * The outport @to_component@ of the timer component must be connected to
 * the @depart()@ inport.
 ************************************************************************/

template <class DATATYPE>
Server<DATATYPE>::Server()
{
    connect wait.to_component,depart;
}

/************************************************************************
 * When a packet comes, schedule the service completion event.
 ************************************************************************/
template <class DATATYPE>
void Server<DATATYPE> :: in(DATATYPE& packet)
{
    m_packet=packet;
    wait.Set(SimTime()+Exponential(service_time));
    return;
}

/************************************************************************
 * When it is time for the packet to depart (the service completion event arrives), 
 * write it to the outport @out@, and at the same time send a trigger signal to the
 * outport @next@.
 ************************************************************************/
template <class DATATYPE>
void Server <DATATYPE> :: depart(trigger_t&)
{
    out(m_packet);
    next();
    return;
}

/************************************************************************
 * @<h2>Sink</h2>@
 * In the @Sink@ component, we collect the time that each packet spent in
 * the @FIFO@ queue and the server. It only has one inport and no timer.
 * What is new here is the @Stop()@ function, which
 * is called when the simulation reaches the preset end time.
 ************************************************************************/

component Sink : public TypeII
{
public:
    inport inline void in(packet_t&);

    void Start()
    {
        m_total=0.0;
        m_number=0;
    }
    Sink() {}
    void Stop()
    {
        printf("Average packet delay is: %f (%d packets) \n",
	       m_total/m_number,m_number);
    }
private:
    double m_total;
    int m_number;
    packet_t m_packet;
};

void Sink::in(packet_t &packet)
{
    m_packet=packet;
    m_packet.departure_time=SimTime();
    m_total+=m_packet.departure_time-m_packet.arrival_time;
    m_number++;
    return;
}

/************************************************************************
 * @<h2>Constructing the Simulation</h2>@
 * The simulation component is derived from the @CostSimEng@ class. 
 Components 
 * are instantiated as public members.
 ************************************************************************/

component MM1 : public CostSimEng
{
public:
    void Setup();

/************************************************************************
 * Several simulation parameters.  
 ************************************************************************/

    double interval;
    int queue_length;
    double service_time;

/************************************************************************
 * Components are instantiated as public members.
 ************************************************************************/

private:
    Source source;
    FIFO <packet_t> fifo;
    Server <packet_t> server;
    Sink sink;
};

/************************************************************************
 * The simulation has a @Setup()@ function which must be called before
 * the simulation can be run.  The reason we don't do this in the constructor
 * is that we must give the simulation an opportunity to assign values to
 * its parameters after the simulation component has been instantiated. 
 *  The @Setup()@ function, which you can rename, first maps component
 * parameters to corresponding simulation parameters (for instance, assign
 * the value of the simulation parameter @interval@ to the component parameter
 * @source.interval@).  It then connects pairs of inport and outports. 
 ************************************************************************/

void MM1::Setup()
{
    source.interval=interval;
    fifo.queue_length=queue_length;
    server.service_time=service_time;

    connect source.out,fifo.in;
    connect fifo.out,server.in;
    connect server.next,fifo.next;
    connect server.out,sink.in;
}

/************************************************************************
 * @<h2>Running the Simulation</h2>@
 * To run the M/M/1 simulation, first we need to create an M/M/1
 * simulation object. Several default simulation parameters must be determined. 
 * @StopTime@ denotes the ending time of the simulation.
 * @Seed@ is the initial seed of the random number generator used
 * by the simulation.
 *
 * To run the simulation, simply type in:
 *
 * mm1-cost [stop time] [random seed]
 * 
 ************************************************************************/

int main(int argc, char* argv[])
{
    MM1 mm1;

    mm1.interval=1;
    mm1.queue_length=100;
    mm1.service_time=0.5;
    mm1.StopTime( 1000000.0);
    mm1.Seed=10;

    if (argc>=2)
      mm1.StopTime( atof(argv[1]));
    if (argc>=3)
      mm1.Seed=atoi(argv[2]);

    mm1.Setup(); // must be called first
    mm1.Run(); // run the simulation

    return 0;
}



