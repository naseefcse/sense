/*************************************************************************
*   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
*   Rensselaer Polytechnic Institute. All worldwide rights reserved.
*   A license to use, copy, modify and distribute this software for
*   non-commercial research purposes only is hereby granted, provided
*   that this copyright notice and accompanying disclaimer is not
*   modified or removed from the software.
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
*   related to the software, to the maximum extent the law permits.
*************************************************************************/

#include <deque>

template < class T1, class T2, class T3 >
component FIFOACK3 : public TypeII
{
public:

    bool NoBuffer;

    FIFOACK3() {};
    virtual ~FIFOACK3() {};

    struct data_t 
    {
        data_t ( T1 const& _t1, T2 const& _t2, T3 const& _t3): t1(_t1), t2(_t2), t3(_t3) {};
        T1 t1; T2 t2; T3 t3; 
    };

    void Start();

    inport void in (T1 const& t1, T2 const& t2, T3 const& t3);
    inport void next (const bool&);
    outport void out (T1 t1, T2 t2, T3 t3);
    outport void ack (bool);

private:
    bool m_busy; // if the connected server is busy
    std::deque <data_t> m_queue; // store packets here
};

/************************************************************************
 * It makes sense to assume the connected server is available for
 * immediate service, so we need to initialize the status here.
 ************************************************************************/

template < class T1, class T2, class T3 >
void FIFOACK3 <T1,T2,T3> :: Start()
{
    m_busy=false;
}

/************************************************************************
 * The @Arrive()@ function is called when a packet arrives.
 * Packets are passed by reference to avoid variable copying overhead.
 * @m_busy@ remember the status of the connected server. If the
 * server is free, simply forward the packet to the outport and update
 * @m_busy@ accordingly. Otherwise, append the packet to the tail of
 * the internal queue, if the queue is not full.
 ************************************************************************/

template < class T1, class T2, class T3 >
void FIFOACK3 <T1,T2,T3>::in (T1 const& t1, T2 const& t2, T3 const& t3)
{
    if(!NoBuffer)
    {
	if (!m_busy)
	{ // server is free, we can pass it through
	    m_busy=true;
	    out(t1,t2,t3);
	} 
	else
	{
	    data_t d (t1,t2,t3);
	    m_queue.push_back(d);
	}
	ack(true);
    }
    else
    {
	out(t1,t2,t3);
    }
    return;
}

/************************************************************************
 * When asked for the next packet, give the first one in the queue. If
 * queue is empty, change the value of @m_busy@ to indicate the server
 * is now free so that the next arriving packet will
 * directly go through.
 ************************************************************************/

template <  class T1, class T2, class T3  >
void FIFOACK3 <T1,T2,T3> :: next (const bool& flag)
{
    if(!NoBuffer)
    {
	if (m_queue.size()>0)
	{
	    data_t data = m_queue.front();
	    m_queue.pop_front();
	    out(data.t1,data.t2,data.t3);
	} 
	else
	{
	    m_busy=false;
	}
    }
    else
    {
	ack(flag);
    }
    return;
}
