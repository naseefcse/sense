/************************************************************************
 * @<title> Multiple M/M/1 Simulation </title>@
 *
 * @<h1> Multiple M/M/1 Simulation </h1>@ 
 * This example illustrates how flexible COST/CompC++ is in handling 
 * variable-size arrays of outports or components.  The sizes of these
 * arrays are controlled by some simulation parameters that can only
 * be determined during the runtime.  
 * 
 * @<center><img src=mmm1.png></center>@
 * 
 * As shown in the above picture, we will build a simulation consisting
 * of several multiple M/M/1 systems running independently. It employs 
 * same @Source@, @FIFO@, 
 * and @Sink@ as in the @<a href=mm1.cc.html>M/M/1</a> simulation. 
 * Only the server is different.
 *
 * The server we are going to build for this example is a more powerful 
 * server called @MultiServer@ that can be viewed as a composition of 
 * multiple servers that runs independently. It is capable of
 * supporting any number of FIFO queues. As a result, the Multiple 
 * M/M/1 system has the same number of sources, FIFO queues, and sinks, 
 * but only one super server. The number of sources, FIFO queues and 
 * sinks are configurable , i. e., this number can be exported as a 
 * simulation parameter which is to be assigned
 * when the system is being instantiated. 
 * 
 * @packet_t@, @Source@, @FIFO@, and @Sink@ 
 * are almost identical as those in the @<a href=mm1-cost.cpp>M/M/1</a>@ simulation, 
 * so we will skip them.  There is only one difference though.  Since there are
 * no inport arrays, packets arriving at the MultiServer must contain an integral
 * id to identify the source (and the FIFO queue) from which they are coming from.
 * This also explains why inport arrays are not essential: a single inport can act
 * as an inport array if we can find a way to distinguish different sources, based on
 * either one of the arguments or some special field contained in the data being passed.
 *
 * @<!--
 ************************************************************************/

#define queue_t SimpleQueue

#include "../../common/cost.h"


/* this defines the packets used in the simulation */
struct packet_t
{
    int seq_number;
    int index;
    double arrival_time,departure_time;
};

/* a source component will generate packets at random intervals*/

component Source : public TypeII
{
public:
    double interval;
    int index;
    outport void out (packet_t& p);
    MultiTimer <trigger_t> wait;

public:
    Source();
    virtual ~Source() {}
    void Start();
    inport void create(trigger_t& t, unsigned int);

private:
	packet_t m_packet;
    int m_seq_number;
};

Source::Source()
{
    connect wait.to_component,create;
}

void Source::Start()
{
    m_seq_number=0;
    wait.Set(Exponential(interval));
}

void Source::create(trigger_t&, unsigned int)
{
    wait.Set(SimTime()+Exponential(interval) );
    m_packet.seq_number=m_seq_number++;
    m_packet.arrival_time = SimTime();
    m_packet.index = index;
    out(m_packet);
    return;
}

/* A fifo queue provides a buffer for packets in case the server is busy */

template < class DATATYPE >
component FIFO : public TypeII
{
public:
    FIFO();
    virtual ~FIFO();
    void Start();

    unsigned int queue_length;

    inport void in(DATATYPE& p);
    outport void out(DATATYPE& p);
    inport void next();

private:
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

template < class DATATYPE >
void FIFO <DATATYPE> :: Start()
{
    m_busy=false;
}

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

/* a sink collects some statistics */

component Sink : public TypeII
{
public:
    inport void in(packet_t&);

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
 * -->@  
 * @<h2>MultiServer</h2>@
 * There are several differences between a @MultiServer@ and a simple @Server@.
 * First, the outport are declared as an array, so an outport
 * array is actually a collection of almost identical outports, except
 * each of which is assigned a unique index. Second, we
 * must allocate enough memory for holding multiple packets being serviced.
 * Since the number of included sub-servers is controlled by the parameter
 * @n_servers@, which can only be determined after the component
 * instance has been created, we must allocate the required space right
 * before simulation within the @Start()@ function, and, correspondingly, destroy 
 * it when simulation ends within the @Stop()@ function.
 *
 * As mentioned before, the inport @in@ is actually connected to multiple
 * outports from different FIFO queues.  So to determined from which
 * the incoming packet arrive, @MultiServer@ must look at the field @index@, which
 * contains the id of the Source component on which the packet was created.
 ************************************************************************/

template <class DATATYPE>
component MultiServer : public TypeII
{
public:
    MultiServer();
	virtual ~MultiServer() ;
	void Setup();
	
    double service_time;
    int size;

    inport void in(DATATYPE& p);
    outport[] void out(DATATYPE& p);
    outport[] void next();
    inport void depart(trigger_t&, unsigned int);

/************************************************************************
 * Notice the declaration of the timer @wait@.  It is a @MultiTimer@, which is
 * a timer that can, unlike the simple timer used in the M/M/1 simulation, 
 * schedule multiple events.  Please refer to the @<a href=manual.html#MultiTimer>manual</a>@
 * for more details.
 ************************************************************************/

    MultiTimer<trigger_t> wait;

private:
    DATATYPE* m_packets;
};

template <class DATATYPE>
MultiServer<DATATYPE>::MultiServer()
{
    connect wait.to_component,depart;
}

template <class DATATYPE>
MultiServer<DATATYPE>::~MultiServer()
{
    delete[] m_packets;
}

/************************************************************************
 * Now we need a @Setup@ function.  Remeber the name of this function becomes
 * merely conventional in COST/CompC++, so you can use any other name.
 * In this function we must allocate enough space for holding packets according
 * to the value of the parameter @size@.  @SetSize()@ is a predefined function
 * for outport arrays which must be called before they can ever be used.
 ************************************************************************/

template <class DATATYPE>
void MultiServer<DATATYPE>::Setup()
{
    m_packets = new DATATYPE [ size ];
    out.SetSize(size);
    next.SetSize(size);
}

/************************************************************************
 * The field @index@ in the packet is very important.  This index is used 
 * to tag the event scheduled. We
 * can think of a timer as a timerarray in which each imaginary timer
 * is identified by a unique index. 
 ************************************************************************/

template <class DATATYPE>
void MultiServer<DATATYPE> :: in(DATATYPE& packet)
{
	int index = packet.index;
    m_packets[index]=packet;
    wait.Set(SimTime()+Exponential(service_time),index);
    return;
}

/************************************************************************
 * Once a departure event is activated, the index of that event is returned
 * as the second argument.
 ************************************************************************/

template <class DATATYPE>
void MultiServer <DATATYPE> :: depart(trigger_t&, unsigned int i)
{
    out[i](m_packets[i]);
    next[i]();
    return;
}


/************************************************************************
 * @<h2>Building the Duplicate M/M/1 System</h2>@
 * @source@, @fifo@, and @sink@ are now defined as component arrays.
 ************************************************************************/

component MMM1 : public CostSimEng
{
public:
    void Setup();

	int size;
    double interval;
    int queue_length;
    double service_time;

private:

    Source[] source;
    FIFO <packet_t> [] fifo;
    MultiServer <packet_t> server;
    Sink[] sink;
};

/************************************************************************
 * We must always first set the size of the component arrays, by calling
 * there predefined @SetSize()@ functions.  We then assign values to
 * the parameters of every component in each component array (we cann't
 * do it at once).
 ************************************************************************/

void MMM1::Setup()
{
	int i;
	
	source.SetSize(size);
	fifo.SetSize(size);
	sink.SetSize(size);
	
	for(i=0;i<size;i++)
	{
    	source[i].interval=interval;
    	source[i].index=i;
	    fifo[i].queue_length=queue_length;
	}
	
    server.service_time=service_time;
    server.size=size;
    server.Setup();

	for(i=0;i<size;i++)
	{
	    connect source[i].out,fifo[i].in;
    	connect fifo[i].out,server.in;
    	connect server.next[i],fifo[i].next;
    	connect server.out[i],sink[i].in;
    }
}

/************************************************************************
 * @<h2>Running the Duplicate M/M/1 Simulation</h2>@
 * Running the simulation is no different. 
 *
 * To run the simulation, type in:
 *
 * dmm1-cost [number of servers] [stop time] [random seed]
 ************************************************************************/

int main(int argc, char* argv[])
{
    MMM1 mmm1;

	mmm1.size=4;
    mmm1.interval=1;
    mmm1.queue_length=100;
    mmm1.service_time=0.5;
    mmm1.StopTime( 100000.0);
    mmm1.Seed=10;

    if (argc>=2)
      mmm1.StopTime( atof(argv[1]));
    if (argc>=3)
      mmm1.Seed=atoi(argv[2]);

    mmm1.Setup(); // must be called first
    mmm1.Run(); // run the simulation

    return 0;
}



