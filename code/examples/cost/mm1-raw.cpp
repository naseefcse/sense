#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include <iostream>
#include <iomanip>
#include <deque>
using namespace std;

#include "../../common/priority_q.h"

#ifndef N
#define N 1
#endif 

struct EVENT
{
    enum TYPE { ARRIVAL, DEPARTURE };
    double time;
    EVENT* next;
    EVENT* prev;
    TYPE type;
    double arrival_time;
    int unused[N];
};

struct MSG
{
    MSG(double at): arrival_time(at){};
    double arrival_time;
    int unused[N];
};

double Exponential(double mean, double r)
{
    return -mean*log(r);
}

bool Run(double stoptime, double interval, double service_time)
{

    SimpleQueue<EVENT> event_list;
    std::deque<MSG> arrival_queue;

    EVENT arrival_event,departure_event;
    EVENT *event;
    int departures=0;
    double total_delay=0.0;

    struct timeval start_time;
    gettimeofday(&start_time,NULL);

    double clock=0.0;
    long events_processed=0l;
    bool server_busy=false;

    srand48(1234);

    arrival_event.time=clock+Exponential(interval,drand48());
    arrival_event.type=EVENT::ARRIVAL;
    event_list.EnQueue(&arrival_event);
    
    while(true)
    {
	event=event_list.DeQueue();
	if(event==NULL||event->time>stoptime)
	    break;
	clock=event->time;
	//Cout << "clock : " << clock << endl;
	events_processed++;
	if(event->type==EVENT::ARRIVAL)
	{
	    if(server_busy)
		arrival_queue.push_back(MSG(clock));
	    else
	    {
		departure_event.time=clock+Exponential(service_time,drand48());
		departure_event.type=EVENT::DEPARTURE;
		departure_event.arrival_time=clock;
		event_list.EnQueue(&departure_event);
		server_busy=true;
	    }

	    arrival_event.time=clock+Exponential(interval,drand48());
	    arrival_event.type=EVENT::ARRIVAL;
	    event_list.EnQueue(&arrival_event);
	    
	}
	else
	{
	    departures++;
	    total_delay+=clock-event->arrival_time;
	    if(arrival_queue.empty())
		server_busy=false;
	    else
	    {
		departure_event.time=clock+Exponential(service_time,drand48());
		departure_event.type=EVENT::DEPARTURE;
		departure_event.arrival_time=arrival_queue.front().arrival_time;
		arrival_queue.pop_front();
		event_list.EnQueue(&departure_event);
	    }
	}
    }
    
    struct timeval stop_time;
    gettimeofday(&stop_time,NULL);
    double running_time = stop_time.tv_sec-start_time.tv_sec
                          + (stop_time.tv_usec-start_time.tv_usec)/1000000.0;
    double processing_rate=events_processed/running_time;
    
    cout << "Average delay is " << total_delay/departures << " over " << departures 
	 << " packets " << endl;
    cout << "-----------------------------------------" << endl;
    cout << "  stopped at time " << stoptime << endl;
    cout << "Total events processed: " << events_processed 
	 << ",  running time: " << setprecision(3) << running_time << endl;
    cout <<  "processing rate: " << setprecision(0) << processing_rate << endl;

    return true;
}

int main(int argc, char* argv[])
{
    double StopTime=1000000.0;
    if (argc>=2)
        StopTime=atof(argv[1]);

    Run(StopTime,1,0.5);

    return 0;
}


