#include "../../common/cost.h"

/* this defines the packets used in the simulation */
struct packet_t
{
    int seq_number;
    double arrival_time,departure_time;
};

/* a source component will generate packets at random intervals*/

component Source : public TypeII
{
public:
    double interval;
    outport void out (packet_t&);
    Timer <trigger_t> wait;

public:
    Source();
    virtual ~Source() {}
    void Start();
    inport inline void create(trigger_t& t);

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

void Source::create(trigger_t&)
{
    wait.Set(SimTime()+Exponential(interval) );
    m_packet.seq_number=m_seq_number++;
    m_packet.arrival_time = SimTime();
    out(m_packet);
    return;
}

/* A FIFOServer queue provides a buffer for packets in case the server is busy */

template < class DATATYPE >
component FIFOServer : public TypeII
{
public:
    FIFOServer();
    virtual ~FIFOServer();
    void Start();

    unsigned int queue_length;
    double service_time;

    inport inline void in(DATATYPE&);
    outport void out(DATATYPE&);

    inport inline void depart(trigger_t&);
    Timer<trigger_t> wait;

private:
    bool m_busy;
    std::deque<DATATYPE> m_queue;
    DATATYPE m_packet;
};

template < class DATATYPE >
FIFOServer <DATATYPE> :: FIFOServer()
{
	connect wait.to_component,depart;
}

template < class DATATYPE >
FIFOServer <DATATYPE> :: ~FIFOServer()
{
}

template < class DATATYPE >
void FIFOServer <DATATYPE> :: Start()
{
    m_busy=false;
}

template < class DATATYPE >
void FIFOServer<DATATYPE>::in(DATATYPE& packet)
{
    if (!m_busy)
    { // server is free, we can pass it through
	    m_packet=packet;
    	wait.Set(SimTime()+Exponential(service_time));
        m_busy=true;
    } 
    else if (m_queue.size()<queue_length)
    { // queue is not full
        	m_queue.push_back(packet);
    }
    return;
}

template <class DATATYPE>
void FIFOServer <DATATYPE> :: depart(trigger_t&)
{
    out(m_packet);
    if (m_queue.size()>0)
    {
        m_packet=m_queue.front();
    	wait.Set(SimTime()+Exponential(service_time));
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

/* an M/M/1 simulation consists of the above four components */

component MM1 : public CostSimEng
{
public:
    void Setup();

    double interval;
    int queue_length;
    double service_time;

private:

    Source source;
    FIFOServer <packet_t> fifo_server;
    Sink sink;
};

void MM1::Setup()
{
    source.interval=interval;
    fifo_server.queue_length=queue_length;
    fifo_server.service_time=service_time;

    connect source.out,fifo_server.in;
    connect fifo_server.out,sink.in;
}

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



