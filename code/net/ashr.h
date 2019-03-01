/*************************************************************************
 *   @<title> Autonomic Self-Healing Routing </title>@
 *
 *   @<!-- Copyright 2006 Mark Lisee, Joel Branch, Gilbert (Gang) Chen,
 *   Boleslaw K. Szymanski and Rensselaer Polytechnic Institute.
 *   
 *   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski
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
 *************************************************************************/

/*
** Modifications from rr.h
** - replaced lambda by slotting
** - Added T1, T2 & T3 time periods
**   - code to gather statistics
**   - code to set # slots per period from command line
** - Added handling of collision packet
**   - new packet type & field in header changed from int to enum
**   - Transmission is scheduled when a collision is detected
** - Added # slots to packet header
**   - I added this when I thought it would be required for v2. It no longer
**     is, but it's staying
** - Sender may modify its hop count based on slot during which response is
**   sent.
*/
#ifndef SSRv02_h
#define SSRv02_h
#include <map>
#include <list>

#ifdef	MWL_PRINT
//#define mwlDoPrint	(SimTime() > 1400. && SimTime() < 1500.)
//#define	mwlPrint(x)	 if( mwlDoPrint) printf x
#define mwlDoPrint	(true)
#define	mwlPrint(x)	 printf x
#else	//MWL_PRINT
#define	mwlPrint(x)
#define	mwlDoPrint	(false)
#endif	//MWL_PRINT

template <class PLD>
struct SSRv02_Struct
{
  enum PktType { REQUEST, REPLY, DATA, ACK, COLL, HELLO, BACKUP};

  // REQUEST: type + src_addr + dst_addr + seq_number + hop
  enum { REQUEST_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // REPLY: type + src_addr + dst_addr + seq_number + 2 * hop + flag
  enum { REPLY_SIZE = 4 * sizeof(int) + 4 * ether_addr_t::LENGTH + sizeof( bool) };
  // DATA: type + src_addr + dst_addr + seq_number + 2 * hop + flag
  enum { DATA_SIZE = 4 * sizeof(int) + 4 * ether_addr_t::LENGTH + sizeof( bool) };
  // ACK: type + src_addr + dst_addr + seq_number
  enum { ACK_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // COLL: type + src_addr + dst_addr + seq_number + slot number
  enum { COLL_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // BACKUP: type + src_addr + dst_addr + seq_number
  enum { BACKUP_SIZE = 2 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // HELLO: type + src_addr + dst_addr + seq_number + hop count
  enum { HELLO_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
    
  struct hdr_struct
  {
    PktType	type;
    ether_addr_t src_addr;
    ether_addr_t dst_addr;
    ether_addr_t cur_addr;
    ether_addr_t pre_addr;
    unsigned int seq_number;
    unsigned int size;
    double	send_time;	// time received from transport
    double	xmit_start_time;	// time sent to mac layer
    double	xmit_end_time;		// time sent to mac layer
    unsigned int actual_hop;
    unsigned int expected_hop;
    unsigned int max_hop;
    double	delay;
    int		t1Index;	// id of endT1Timer
    int		t2Index;	// id of endT2Timer
//mwl    int		t3Index;	// id of endT3Timer
    int		collIndex;	// id of endCollTimer
    bool	canceled;
    int		resend;
    int		numSlots;	// k, the number of slots in T2
    int		slotNumber;
    bool	collDetected;
    bool	collPktSent;
    bool	t3Flag;		// if true, nodes may forward this pkt in T3
#ifdef VISUAL_ROUTE
#ifndef VR_SIZE
#define VR_SIZE 20
#endif
    path_t<VR_SIZE> path;
#endif

    bool dump(std::string& str) const;
  };
  typedef PLD payload_t;
  typedef smart_packet_t<hdr_struct,PLD> packet_t;
};

#define	HCArraySize		(20+1)
#define	MaxT2Slots		100
#define	DefaultNumT2Slots	3
#define MaxT3Slots		10
#define	DefaultNumT3Slots	2

/*
** The AckWindow is a period of time during which only Acks from intermediate
** nodes are sent. This prevents the Acks from colliding with packets that are
** being forwarded. Note that the destination sends its Ack as a normal packet,
** i.e. after the AckWindow. This is because when node N forwards a packet, it
** causes node N-1 to send an Ack (during the AckWindow) and N+1 to forward the
** packet (after the AckWindow).
*/
template <class PLD>
component SSRv02 : public TypeII, public SSRv02_Struct<PLD>
{
 public:

  ether_addr_t	MyEtherAddr;
  simtime_t	ForwardDelay;
  double	RXThresh;
  bool		DumpPackets;
  double	AckWindow;	// see comment above
  int		MaxResend;
  unsigned int	TimeToLive;
  unsigned int	AdditionalHop;
    
  int		SentPackets;
  int		RecvPackets;
  int		RecvUniPackets;
  double	TotalDelay;
  int		TotalSamples;
  int		TotalHop;
  int		RecvDataPackets;
  int		HopCounts[ HCArraySize];
  int		T1PktsSent;
  int		T2PktsSent[ MaxT2Slots];
  int		T3PktsSent[ MaxT3Slots];
  int		T3AbnormalPktsSent;
  int		DropPkts;		// # times h_table > h_pkt+n
  int		T1Collisions;
  int		T2Collisions[ MaxT2Slots];
  int		T3Collisions[ MaxT3Slots];
  int		IgnoreCollisions;	// collision after end of T3
  int		T1PktsRcvd[2];
  int		T2PktsRcvd[2][ MaxT2Slots];
  int		T3PktsRcvd[2][ MaxT3Slots];
  int		T4PktsRcvd[2];
//mwl  int		OtherPktsRcvd[2];
  int		CanceledPktsRcvd[2];
  int		BadAddrPktsRcvd[2];
  int		WrongHopPktsRcvd[2];
  static const char	*pktRcvdName[2];

  typedef std::list<packet_t* > packet_queue_t;

/*
** Fields that support the transition timer.
** This timer simulates the amount of time it takes the hardware to transition
** from sensing the carrier to actually transmitting a packet.
*/
  Timer <trigger_t> transitionTimer;
  packet_t	*transitionPkt;		// pkt to be sent when timer expires
  inport void	TransitionTimer( trigger_t &);

/*
** When a COLL packet is sent, the original packet is added to the
** m_collPktsSent_queue. A list of collision packets is required to properly
** implement BACKUP messages.
**
** In some situations, a timer is required to remove packets from the queue.
** For example, N1 sends a packet. N2 forwards it, but N1 detects it as a
** collision when there is no interference at N2. (N1 could be located between
** the two nodes.) In this case, N1 will schedule a COLL. If the COLL
** is received by N2 after N3 has forwarded the packet, the COLL packet will be
** ignored. In this case, the original packet needs to be removed from N1.
*/
  InfiTimer <SSRv02_Struct<PLD>::packet_t*> endCollTimer;
  inport void	EndCollTimer( packet_t *pkt, unsigned int index);
  packet_queue_t	m_collPktsSent_queue;	// pkts for which a COLL packet has been sent

/*
** Fields that support the timers that expire at the ends of T1, T2 & T3.
*/
  InfiTimer <SSRv02_Struct<PLD>::packet_t*> endT1Timer;
  InfiTimer <SSRv02_Struct<PLD>::packet_t*> endT2Timer;
//mwl  InfiTimer <SSRv02_Struct<PLD>::packet_t*> endT3Timer;
  inport void	EndT1Timer( packet_t *pkt, unsigned int index);
  inport void	EndT2Timer( packet_t *pkt, unsigned int index);
//mwl  inport void	EndT3Timer( packet_t *pkt, unsigned int index);
  packet_queue_t	m_T3_queue;	// packets w/ t3 timer pending

/*
** Connections to transport (layer above) and mac (layer below)
*/
  inport inline void	from_transport( payload_t& pld, ether_addr_t& dst,
					unsigned int size);
  inport inline void	from_mac_data( packet_t* pkt, double power);
  inport inline void	from_mac_ack( bool errflag);
  inport inline void	from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2);

  outport void		cancel();
  outport void		to_transport( payload_t& pld );
  outport void		to_mac( packet_t* pkt, unsigned int size,
				double backoff);

  void		Start();
  void		Stop();
		SSRv02();
  virtual	~SSRv02();
  int		mwlGetHC( int addr);

/*
** Added static members for slot width, transition timer and back off
*/
  static void		setSlotWidth( double);
  static double		getSlotWidth();
  static void		setTransitionTime( double);
  static double		getTransitionTime();
  static void		setNumT2Slots( int);
  static int		getNumT2Slots();
  static void		setNumT3Slots( int);
  static int		getNumT3Slots();
	
 private:
  void		updateHopCountInCache( ether_addr_t src,
				       unsigned int hopCount, bool always,
				       const char *, packet_t *pkt);
 protected:
  static double		slotWidth;
  static double		transitionTime;
  static int		numT2Slots;
  static int		numT3Slots;
  void		CancelPacket( ether_addr_t& src, unsigned int seq_number);
  void		CancelRetransmission( ether_addr_t& src, unsigned int seq);
  void		SendPacket( packet_t *p, bool new_seq = true);
  void		ForwardPacket( packet_t *p);
  void		SendToMac( packet_t *pkt);
  void		SendAck( ether_addr_t, ether_addr_t, ether_addr_t,
			 unsigned int, simtime_t);
  void		CheckBuffer( ether_addr_t, unsigned int);
  void		sendCollisionPacket( packet_t *pkt);
  void		receiveCollisionPacket( packet_t *pkt);
  void		receiveHelloPacket( packet_t *pkt);
  void		receiveBackupPacket( packet_t *pkt);
  void		receiveAckPacket( packet_t *pkt);
  void		receiveReqRepDataPacket( packet_t *pkt);
  bool		determineAckStatus( packet_t *pkt);
  void		performAck( packet_t *pkt);
  
  // these return pointers to packet_t, but I wasn't able to figure out
  // how to avoid compiler errors, therefore cast them to void *
  void		*createBasicPkt();
  void		*createDataPkt( payload_t &pld, ether_addr_t dst,
				unsigned int size);
  void		*createT3Pkt( packet_t *pkt);
  void		*createRequestPkt( ether_addr_t dst);
  void		*createReplyPkt( ether_addr_t src, unsigned int expectedHop);
  void		*createAckPkt( ether_addr_t src_addr, ether_addr_t cur_addr,
			       ether_addr_t pre_addr, unsigned int seq_number,
			       simtime_t delay);
  void		*createCollPkt( packet_t *pkt);
  void		*createBackupPkt( packet_t *pkt);
  void		*createHelloPkt();

  bool		m_mac_busy;		// if the mac layer is busy
  unsigned int	m_seq_number;		// current sequence number
  packet_queue_t	m_send_queue;	// list of packets to send
  packet_t	*m_active_packet;

  class seq_number_t
      {
       public:
	seq_number_t () : state( Initial), current(0), bits(0u) {}
	bool check(int n)
	    {
	      if( n + (int)sizeof(unsigned long) <= current )
		return true;
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
	unsigned int	hopCount() const;
	void		hopCount( unsigned int newHC, bool print);
       private:
	unsigned int	currentHC;
	unsigned int	pendingHC;
	enum { Initial, Steady, Changing}	state;
	int		updateCtr;
	int		current;
	uint32_t	bits;
	static int	maxCounter;
      };

/*******************************************************************
 * For each source, we must maintain a @seq_number_t@ object, so we better
 * keep them in a map.
 *******************************************************************/

  typedef std::map<ether_addr_t, seq_number_t, ether_addr_t::compare> cache_t;
  cache_t	m_seq_cache;
  packet_queue_t m_data_buffer;    

  void		recvPktAtDest( packet_t *pkt, cache_t::iterator iter);
};

template <class PLD> double	SSRv02<PLD>::slotWidth = 0.010;
template <class PLD> double	SSRv02<PLD>::transitionTime = 0.0001;
template <class PLD> int	SSRv02<PLD>::numT2Slots = DefaultNumT2Slots;
template <class PLD> int	SSRv02<PLD>::numT3Slots = DefaultNumT3Slots;
template <class PLD> const char	*SSRv02<PLD>::pktRcvdName[2] = {"REP", "DAT"};
template <class PLD> int	SSRv02<PLD>::seq_number_t::maxCounter = 2;

template <class PLD>
unsigned int SSRv02<PLD>::seq_number_t::hopCount() const
{
  switch( state)
  {
    case Initial:
      return 99;
    case Steady:
    case Changing:
      return currentHC;
    default:
      assert( 0);	// something really bad happened
      return 0;
  }
}

template <class PLD>
void SSRv02<PLD>::seq_number_t::hopCount(
  unsigned int	newHC,
  bool		print)
{
  switch( state)
  {
    case Initial:
      currentHC = newHC;
      updateCtr = maxCounter;
      state = Steady;
      break;
    case Steady:
      if( newHC != currentHC)
      {
	--updateCtr;
	pendingHC = newHC;
	state = Changing;
	if( print) printf( " start change\n");
      }
      break;
    case Changing:
      if( newHC == currentHC)
      {
	if( ++updateCtr == maxCounter)
	{
	  state = Steady;
	  if( print) printf(" back out of change\n");
	}
	else
	  if( print) printf(" reversing\n");
      }
      else if( newHC == pendingHC)
      {
	if( --updateCtr == 0)
	{
	  currentHC = newHC;
	  updateCtr = maxCounter;
	  state = Steady;
	  if( print) printf(" end change\n");
	}
	else
	  if( print) printf(" still changing\n");
      }
      else	// newHC is neither the old nor new HC
      {
	pendingHC = newHC;
	if( print) printf(" new HC during change\n");
      }
      break;
    default:
      assert( 0);	// something really bad happened
  }
  return;
}

template <class PLD>
void SSRv02<PLD>::setSlotWidth(
  double	sw)
{
  slotWidth = sw;
  return;
}

template <class PLD>
double SSRv02<PLD>::getSlotWidth()
{
  return slotWidth;
}

template <class PLD>
void SSRv02<PLD>::setTransitionTime(
  double	tt)
{
  transitionTime = tt;
  return;
}

template <class PLD>
double SSRv02<PLD>::getTransitionTime()
{
  return transitionTime;
}

template <class PLD>
void SSRv02<PLD>::setNumT2Slots(
  int		num)
{
  numT2Slots = num;
  return;
}

template <class PLD>
int SSRv02<PLD>::getNumT2Slots()
{
  return numT2Slots;
}

template <class PLD>
void SSRv02<PLD>::setNumT3Slots(
  int		num)
{
  numT3Slots = num;
  return;
}

template <class PLD>
int SSRv02<PLD>::getNumT3Slots()
{
  return numT3Slots;
}

template <class PLD>
void *SSRv02<PLD>::createBasicPkt()
{
  packet_t	*newPkt = packet_t::alloc();

  newPkt->hdr.send_time = SimTime();
  newPkt->hdr.actual_hop = 1;
  newPkt->hdr.resend = 0;
  newPkt->hdr.seq_number = 0;
  newPkt->hdr.numSlots = numT2Slots;
  newPkt->hdr.collDetected = false;
  newPkt->hdr.collPktSent = false;
  newPkt->hdr.slotNumber = 0;
  newPkt->hdr.delay = 0.0;
  newPkt->hdr.t1Index = 12345;		//mwl
  newPkt->hdr.t2Index = 12345;		//mwl
  newPkt->hdr.t3Flag = false;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createDataPkt(
  payload_t	&pld,
  ether_addr_t	dst,
  unsigned int	size)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = DATA;
  newPkt->hdr.size = size + DATA_SIZE;
  newPkt->hdr.src_addr = MyEtherAddr;
  newPkt->hdr.dst_addr = dst;
  newPkt->hdr.cur_addr = MyEtherAddr;
  newPkt->hdr.pre_addr = MyEtherAddr;
  newPkt->pld = pld;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createT3Pkt(
  packet_t	*pkt)
{
  packet_t	*newPkt = pkt->copy();

  assert( pkt->hdr.type == DATA || pkt->hdr.type == REPLY);
  newPkt->hdr.resend = pkt->hdr.resend + 1;
  newPkt->hdr.numSlots = numT2Slots;
  newPkt->hdr.collDetected = false;
  newPkt->hdr.collPktSent = false;
  newPkt->hdr.t3Flag = true;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createRequestPkt(
  ether_addr_t	dst)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = REQUEST;
  newPkt->hdr.size = REQUEST_SIZE;
  newPkt->hdr.expected_hop  =  1;
  newPkt->hdr.max_hop = TimeToLive;
  newPkt->hdr.src_addr = MyEtherAddr;
  newPkt->hdr.pre_addr = MyEtherAddr;
  newPkt->hdr.dst_addr = dst;
  newPkt->hdr.cur_addr = MyEtherAddr;
//mwl  newPkt->hdr.delay = AckWindow;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createReplyPkt(
  ether_addr_t	src,
  unsigned int	expectedHop)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = REPLY;
  newPkt->hdr.size = REPLY_SIZE;
  newPkt->hdr.expected_hop = expectedHop;
  newPkt->hdr.max_hop = expectedHop + AdditionalHop;
  newPkt->hdr.src_addr = MyEtherAddr;
  newPkt->hdr.dst_addr = src;
  newPkt->hdr.cur_addr = MyEtherAddr;
  newPkt->hdr.pre_addr = MyEtherAddr;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createAckPkt(
  ether_addr_t	src_addr,
  ether_addr_t	cur_addr,
  ether_addr_t	pre_addr,
  unsigned int	seq_number,
  simtime_t	delay)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = ACK;
  newPkt->hdr.size = ACK_SIZE;
  newPkt->hdr.src_addr = src_addr;
  newPkt->hdr.cur_addr = cur_addr;
  newPkt->hdr.pre_addr = pre_addr;
  newPkt->hdr.seq_number = seq_number;
  newPkt->hdr.delay = delay;
  newPkt->hdr.expected_hop = 0 mwl;	// should this be moved to createBasicPkt
  newPkt->hdr.max_hop = 0;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createCollPkt(
  packet_t	*pkt)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = COLL;
  newPkt->hdr.size = COLL_SIZE;
  newPkt->hdr.expected_hop = 1;
  newPkt->hdr.src_addr = pkt->hdr.src_addr;
  newPkt->hdr.dst_addr = pkt->hdr.dst_addr;
  newPkt->hdr.cur_addr = pkt->hdr.cur_addr;
  newPkt->hdr.pre_addr = pkt->hdr.pre_addr;
  newPkt->hdr.slotNumber = pkt->hdr.slotNumber;
  newPkt->hdr.seq_number = pkt->hdr.seq_number;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createBackupPkt(
  packet_t	*pkt)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = BACKUP;
  newPkt->hdr.size = BACKUP_SIZE;
  newPkt->hdr.expected_hop = 1;
  newPkt->hdr.src_addr = pkt->hdr.src_addr;
  newPkt->hdr.dst_addr = pkt->hdr.dst_addr;
  newPkt->hdr.cur_addr = pkt->hdr.cur_addr;
  newPkt->hdr.pre_addr = pkt->hdr.pre_addr;
  return newPkt;
}

template <class PLD>
void *SSRv02<PLD>::createHelloPkt()
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = HELLO;
  newPkt->hdr.size = HELLO_SIZE;
  newPkt->hdr.expected_hop = 1;
  newPkt->hdr.src_addr = MyEtherAddr;
  newPkt->hdr.dst_addr = 0;		//what's the dest addr of a hello pkt?
  newPkt->hdr.cur_addr = MyEtherAddr;
  newPkt->hdr.pre_addr = MyEtherAddr;
  return newPkt;
}

template <class PLD>
SSRv02<PLD>::SSRv02()
{
  connect transitionTimer.to_component, TransitionTimer;
  connect endT1Timer.to_component, EndT1Timer;
  connect endT2Timer.to_component, EndT2Timer;
//mwl  connect endT3Timer.to_component, EndT3Timer;
  connect endCollTimer.to_component, EndCollTimer;
  return;
}

template <class PLD>
SSRv02<PLD>::~SSRv02()
{
  return;
}

template <class PLD>
void SSRv02<PLD>::Start() 
{
  m_mac_busy = false;
//mwl  m_seq_number = 1u;
  m_seq_number = (int) MyEtherAddr << 16;
  m_seq_cache.insert( make_pair( MyEtherAddr, seq_number_t()));

  SentPackets = RecvPackets = RecvUniPackets = 0l;
  TotalDelay = 0.0;
  TotalHop = 0;
  TotalSamples = 0;
  RecvDataPackets = 0;
  for( int i = HCArraySize-1; i >= 0; i--)
    HopCounts[i] = 0;
  for( int i = MaxT2Slots-1; i >= 0; i--)
    T2PktsSent[i] = T2Collisions[i] = 0;
  for( int i = MaxT3Slots-1; i >= 0; i--)
    T3PktsSent[i] = T3Collisions[i] = 0;
  DropPkts = T1PktsSent = T3AbnormalPktsSent = 0;
  T1Collisions = IgnoreCollisions = 0;

  T1PktsRcvd[0] = 0;
  T1PktsRcvd[1] = 0;
  T4PktsRcvd[0] = 0;
  T4PktsRcvd[1] = 0;
//mwl  OtherPktsRcvd[0] = 0;
//mwl  OtherPktsRcvd[1] = 0;
  CanceledPktsRcvd[0] = 0;
  CanceledPktsRcvd[1] = 0;
  BadAddrPktsRcvd[0] = 0;
  BadAddrPktsRcvd[1] = 0;
  WrongHopPktsRcvd[0] = 0;
  WrongHopPktsRcvd[1] = 0;
  for( int i = MaxT2Slots-1; i>= 0; --i)
    T2PktsRcvd[0][i] = T2PktsRcvd[1][i] = 0;
  for( int i = MaxT3Slots-1; i>= 0; --i)
    T3PktsRcvd[0][i] = T3PktsRcvd[1][i] = 0;
  return;
}

template <class PLD>
void SSRv02<PLD>::Stop() 
{
  //printf("%s: sent %d, recv %d\n",GetName(),SentPackets,RecvPackets);
  //printf("%s: %f \n", GetName(), (double)RecvPackets/RecvUniPackets);
  if( m_collPktsSent_queue.size() != 0)
  {
    printf( "%3d: contents of m_collPktsSent_queue\n", (int) MyEtherAddr);
    packet_queue_t::iterator iter = m_collPktsSent_queue.begin();
    for( ; iter != m_collPktsSent_queue.end(); iter++)
      printf( "  %s\n", (*iter)->dump().c_str());	//mwl
  }
  return;
}

template <class PLD>
void SSRv02<PLD>::from_transport(
  payload_t	&pld,
  ether_addr_t	&dst,
  unsigned int	size)
{
  if( size > 1500)
    printf( "ft: ea%3d; Oversize packet\n", (int) MyEtherAddr);
  //create the data packet
  packet_t	*p = (packet_t *) createDataPkt( pld, dst, size);

  Printf((DumpPackets, "%d SSRv02: %d->%d *%s\n", (int) MyEtherAddr,
	  (int) MyEtherAddr, (int) dst, p->dump().c_str()));
  mwlPrint( ("ft: %3d->%3d *%s @ %f ea %3d\n", (int) MyEtherAddr, (int) dst,
	     p->dump().c_str(), SimTime(), (int) MyEtherAddr));

  cache_t::iterator iter = m_seq_cache.find( dst);
  if( iter == m_seq_cache.end())
  {
    // the destination is not in the active node table
    // must send a REQUEST packet first;
    packet_t	*rp = (packet_t *) createRequestPkt( dst);

    Printf((DumpPackets, "%d SSRv02: %d->%d %s\n", (int) MyEtherAddr,
	    (int) MyEtherAddr, (int) dst, rp->dump().c_str()));
    mwlPrint( ("ft: (REQ) %3d->%3d %s @ %f ea %3d\n", (int) MyEtherAddr,
	       (int) dst, rp->dump().c_str(), SimTime(), (int) MyEtherAddr));
    //printf("rr%d sends a REQUEST to rr%d %s\n",(int)MyEtherAddr,(int)dst,rp->dump().c_str());
    SendPacket( rp);
    m_data_buffer.push_back( p);	// add to end of queue
  }
  else
  {
    // The SSR spec calls for decrementing expected_hop before transmitting.
    // In SENSE, >1 nodes reference same packet in memory, so can't decrement
    p->hdr.expected_hop = (*iter).second.hopCount();
    p->hdr.max_hop = p->hdr.expected_hop + AdditionalHop;
    p->hdr.delay = AckWindow;
    SendPacket( p);
  }
}

template <class PLD>
void SSRv02<PLD>::SendPacket(
  packet_t	*pkt,
  bool		new_seq)
{
  mwlPrint( ("SP: ea %3d; sending %s @ %f\n", (int) MyEtherAddr,
	     pkt->dump().c_str(), SimTime()));
  pkt->hdr.canceled = false;
  if( new_seq && pkt->hdr.src_addr == MyEtherAddr)
  {
    m_seq_number++;
    m_seq_cache[MyEtherAddr].check(m_seq_number);
    pkt->hdr.seq_number = m_seq_number;
    //Printf((DumpPackets,"SSRv02%d assigns seq number to %s \n",(int)MyEtherAddr,pkt->dump().c_str()));
//    mwlPrint( ("SP: ea %3d; assigning sn%08X\n", (int) MyEtherAddr, m_seq_number));
    printf( "SP: ea %3d; assigning sn%08X\n", (int) MyEtherAddr, m_seq_number);
  }
  SentPackets++;

  if( m_mac_busy == false)
  {
    m_mac_busy = true;
    m_active_packet = pkt;
    m_active_packet->inc_ref();
    SendToMac( pkt);
  }
  else
  {
    mwlPrint( ("SP: ea %3d; mac busy\n", (int) MyEtherAddr));
    packet_queue_t::iterator iter = m_send_queue.begin();

    while( iter != m_send_queue.end() && (*iter)->hdr.delay <= pkt->hdr.delay)
      iter++;
    m_send_queue.insert( iter, pkt);
  }
  return;
}

/*
** SendToMac was added in February 2006.
**
** Its purpose is to simulate the small amount of time that elapses between
** when the hardware stops sensing the carrier and it is able to actually start
** transmission of the packet.
**
** For backwards compatibility, the transition time may be zero. In this case,
** forward the packet directly to the mac layer instead of starting a timer.
*/
template <class PLD>
void SSRv02<PLD>::SendToMac(
  packet_t	*pkt)
{
//  mwlPrint( ("STM: %3d->%3d; ea %3d @ %f\n", (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr, (int) MyEtherAddr, SimTime()));
  m_mac_busy = true;
  if( transitionTime != 0)
  {
    // start timer
    transitionTimer.Set( SimTime() + transitionTime);
    // save packet
    transitionPkt = pkt;
  }
  else
  {
    pkt->hdr.collDetected = false;
    pkt->hdr.xmit_start_time = SimTime();
    to_mac( pkt, pkt->hdr.size, pkt->hdr.delay);
  }
  return;
}

/*
** TransitionTimer was added in February 2006.
**
** Timer has expired indicating that the hardware has finished its transition
** from sensing the carrier and is finally starting to transmit the packet.
*/
int		mwl = 0;

template <class PLD>
void SSRv02<PLD>::TransitionTimer(
  trigger_t	&)
{
  if( (int) MyEtherAddr == 63)
    mwl++;
  // send saved packet to mac
  transitionPkt->hdr.collDetected = false;
  transitionPkt->hdr.xmit_start_time = SimTime();
  to_mac( transitionPkt, transitionPkt->hdr.size, transitionPkt->hdr.delay);
  transitionPkt = 0;
  return;
}

/*
** EndT1Timer was added in April 2006.
**
** Timer has expired indicating that the time is now just past the end of
** T1.
** If a collision has been detected:
** - schedule a COLL
** - cancel the T2 and T3 timers
** - If there's an associated packet on the collision queue, remove it
** - remove the packet from the ack queue
*/
template <class PLD>
void SSRv02<PLD>::EndT1Timer(
  packet_t	*pkt,
  unsigned int	index)
{
  mwlPrint( ("ET1T: ea %3d; 0x%08X %3d->%3d; sn%08X @ %f\n", (int) MyEtherAddr,
	     (int) pkt, (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr,
	     pkt->hdr.seq_number, SimTime()));
  if( pkt->hdr.collDetected == true)
    sendCollisionPacket( pkt);
  return;
}

/*
** EndT2Timer was added in April 2006.
**
** Timer has expired indicating that the time is now just past the end of
** T2.
** If a collision has been detected (it must have occurred during T2):
** - schedule a COLL
** - cancel the T2 and T3 timers
** - If there's an associated packet on the collision queue, remove it
** - remove the packet from the ack queue
*/
template <class PLD>
void SSRv02<PLD>::EndT2Timer(
  packet_t	*pkt,
  unsigned int	)
{
  mwlPrint( ("ET2T: ea %3d; 0x%08X %3d->%3d; sn%08X @ %f\n", (int) MyEtherAddr,
	     (int) pkt, (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr,
	     pkt->hdr.seq_number, SimTime()));
  if( pkt->hdr.collDetected == true)
    sendCollisionPacket( pkt);
  else if( pkt->hdr.resend < MaxResend)
  {
    packet_t	*newPkt = (packet_t *) createT3Pkt( pkt);
    newPkt->hdr.delay = 0.0;		// don't need AckWindow
    SendPacket( newPkt, false);		// same sequence #
    m_T3_queue.remove( pkt);		// remove old packet from t3 queue
    pkt->free();			// free old packet
    newPkt->inc_ref();			// save
    m_T3_queue.push_back( newPkt);	// add new packet to t3 queue
  }
  else
    mwlPrint( ("ET2T: ea %3d; 0x%08X %3d->%3d; sn%08X MaxResent\n",
	       (int) MyEtherAddr, (int) pkt, (int) pkt->hdr.src_addr,
	       (int) pkt->hdr.dst_addr, pkt->hdr.seq_number));
  return;
}

/*
** EndCollTimer was added in April 2006.
**
** The timer has expired indicating that the time is now just past the end
** of T3 after sending a COLL. This means that the node didn't overhear a
** retransmission of the packet. This in turn means that the other nodes in
** range either didn't hear the COLL packet or ignored it. The former could be due
** to a collision (nothing to be done about that). The latter happens when the
** receiver has already forwarded the packet. In this case, the COLL packet was
** a false alarm. So, remove the packet from m_collPktsSent_queue.
*/
template <class PLD>
void SSRv02<PLD>::EndCollTimer(
  packet_t	*pkt,
  unsigned int	index)
{
  packet_queue_t::iterator	iter;

  mwlPrint( ("ECT: 0x%08X %3d->%3d; ea %3d; sn%08X @ %f\n", (int) pkt,
	     (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr,
	     (int) MyEtherAddr, pkt->hdr.seq_number, SimTime()));
  for( iter = m_collPktsSent_queue.begin();
       iter != m_collPktsSent_queue.end(); iter++)
  {
    if( (*iter)->hdr.src_addr == pkt->hdr.src_addr &&
	(*iter)->hdr.dst_addr == pkt->hdr.dst_addr &&
	(*iter)->hdr.seq_number == pkt->hdr.seq_number)
      break;	// match found
  }
  m_collPktsSent_queue.erase( iter);
  (*iter)->free();
  pkt->free();
  return;
}

/*
** Find the packet's source address in this node's cache. If the entry is not
** found, create it. If the hop count in the cache is greater than the hop
** count of the packet, update the cache.
*/
template <class PLD>
void SSRv02<PLD>::updateHopCountInCache(
  ether_addr_t	dst,
  unsigned int	hopCount,
  bool		always,		// if this is false, only lower hop count
  const char	*str,
  packet_t	*mwl)
{
  cache_t::iterator	iter;

  iter = m_seq_cache.find( dst); 
  // If the destination address is not found, create a new record.
  if( iter == m_seq_cache.end())
  {
    mwlPrint( ("%s: uHCIC: ea %3d; dst %3d; hc %2d new\n", str,
	       (int) MyEtherAddr, (int) dst, hopCount));
    iter = (m_seq_cache.insert( make_pair( dst, seq_number_t()))).first;
    iter->second.hopCount( hopCount, false);
  }
  else if( always == true || iter->second.hopCount() > hopCount)
  {
    mwlPrint( ("%s: uHCIC: ea %3d; dst %3d; hc %2d->%2d update", str,
	       (int) MyEtherAddr, (int) dst, iter->second.hopCount(), hopCount));
    iter->second.hopCount( hopCount, mwlDoPrint);
  }
  return;
}

/*
** A collision has been detected, so send a COLL packet.
** - Cancel pending Tx timers
** - Cancel Ack timer
** - Remove from t3 and ack queues
** - Add to collision sent queue
** - Update packet header to indicate that a collision packet has been sent
** - Create and send COLL pkt
*/
template <class PLD>
void SSRv02<PLD>::sendCollisionPacket(
  packet_t	*pkt)
{
  mwlPrint( ("sCP: 0x%08X %3d->%3d; ea %3d @ %f\n", (int) pkt,
	     (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr,
	     (int) MyEtherAddr, SimTime()));
  if( endT2Timer.Active( pkt->hdr.t2Index) == true)
    endT2Timer.Cancel( pkt->hdr.t2Index);
//mwl  if( endT3Timer.Active( pkt->hdr.t3Index) == true)
//mwl    endT3Timer.Cancel( pkt->hdr.t3Index);
  m_T3_queue.remove( pkt);
  mwlPrint( ("sCP: Adding %s to m_collPktsSent_queue ea %3d\n",
	     pkt->dump().c_str(), (int) MyEtherAddr));
  m_collPktsSent_queue.push_back( pkt);
  pkt->hdr.collPktSent = true;
  
  // create and sent COLL
  packet_t	*collPkt = (packet_t *) createCollPkt( pkt);
  SendPacket( collPkt, false);		// don't change sequence number

/*
** start timer to remove packet from m_collPktsSent_queue
** Make sure the time is greater than endT3Timer.
*/
  double	start = (2 * transitionTime + (1 + numT2Slots + numT3Slots) *
			 slotWidth + AckWindow) * 2 + SimTime();
  pkt->hdr.collIndex = endCollTimer.Set( pkt, start);
  return;
}

template <class PLD>
void SSRv02<PLD>::receiveCollisionPacket(
  packet_t	*collPkt)
{
  packet_queue_t::iterator	iter;

  mwlPrint( ("rCP: ea %3d; %3d->%3d; %s @ %f\n", (int) MyEtherAddr,
	     (int) collPkt->hdr.src_addr, (int) collPkt->hdr.dst_addr,
	     collPkt->dump().c_str(), SimTime()));
/*
** Search m_send_queue, the list of packets that are waiting to be sent. If the
** packet is found, remove it from the queue. If the packet was on the queue,
** this means that the packet was scheduled to be forwarded in a later slot and
** the collision indicates that there are other nodes who have already tried to
** forward it.
*/
  CancelPacket( collPkt->hdr.src_addr, collPkt->hdr.seq_number);
#ifdef	MWL_20060430
  for( iter = m_send_queue.begin(); iter != m_send_queue.end(); iter++)
  {
    mwlPrint( ("rCP: ea %3d;   Found %s on m_send_queue\n", (int) MyEtherAddr,
	       (*iter)->dump().c_str()));
    if( (*iter)->hdr.src_addr == collPkt->hdr.src_addr &&
	(*iter)->hdr.dst_addr == collPkt->hdr.dst_addr &&
	(*iter)->hdr.seq_number == collPkt->hdr.seq_number)
      break;	// match found
  }
  if( iter != m_send_queue.end())
  {
    (*iter)->free();
    m_send_queue.erase( iter);
    return;		// the packet can't be on the other queue
  }
/*
** Should the collision packet cancel the currently active packet?
*/
  if( m_active_packet->hdr.src_addr == collPkt->hdr.src_addr &&
      m_active_packet->hdr.dst_addr == collPkt->hdr.dst_addr &&
      m_active_packet->hdr.seq_number == collPkt->hdr.seq_number)
  {
    CancelPacket( mwl);
  }
#endif	//MWL_20060430
/*
** Search m_T3_queue (the list of packets sent but not yet ACKed) for a packet
** with the matching source, destination and sequence number.
*/
  for( iter = m_T3_queue.begin(); iter != m_T3_queue.end(); iter++)
  {
    mwlPrint( ("rCP: ea %3d;   Found %s on m_T3_queue\n", (int) MyEtherAddr,
	       (*iter)->dump().c_str()));
    if( (*iter)->hdr.src_addr == collPkt->hdr.src_addr &&
	(*iter)->hdr.dst_addr == collPkt->hdr.dst_addr &&
	(*iter)->hdr.seq_number == collPkt->hdr.seq_number)
      break;	// match found
  }

/*
** If found, cancel timers and remove the original packet from queues.
** If this node had forwarded the packet during the slot with the collision,
** then attempt again to forward the original packet.
*/
  if( iter != m_T3_queue.end())
  {
    packet_t	*orig = *iter, *newPkt;

    if( endT1Timer.Active( orig->hdr.t1Index) == true)
      endT1Timer.Cancel( orig->hdr.t1Index);
    if( endT2Timer.Active( orig->hdr.t2Index) == true)
      endT2Timer.Cancel( orig->hdr.t2Index);
//mwl    if( endT3Timer.Active( orig->hdr.t3Index) == true)
//mwl      endT3Timer.Cancel( orig->hdr.t3Index);
    if( orig->hdr.slotNumber == collPkt->hdr.slotNumber)
    {
      newPkt = orig->copy();
      newPkt->hdr.expected_hop++;
      mwlPrint( ("rCP: Forwarding packet 0x%08X (%3d->%3d) from T3 queue\n",
		 (int) newPkt, (int) newPkt->hdr.src_addr,
		 (int) newPkt->hdr.dst_addr));
      ForwardPacket( newPkt);
    }
    mwlPrint( ("rCP: ea %3d; freeing 0x%08X\n", (int) MyEtherAddr,
	       (int) orig));
    m_T3_queue.remove( orig);
    orig->free();
  }
  else
    mwlPrint( ("rCP: ea %3d; not found on m_T3_queue\n", (int) MyEtherAddr));
  collPkt->free();	// done with COLL
  return;
}

template <class PLD>
void SSRv02<PLD>::receiveHelloPacket(
  packet_t	*pkt)
{
/*
** Need to do something with the hello packet.
** How does a node decide that it needs to send a hello packet?
** The spec says that when a node that caused a routing hole comes back on
** line, it should send a hello packet. How to determine this.
** When receiving this packet, the node will adjust its hop count to (hello
** pkt + 1) only if the current hop count >= (hello pkt + 2). If the HC is
** changed, this node sends a new hello packet with the adjusted HC.
*/
  mwlPrint( ("SSRV02::rHP: received HELLO packet @ %f (MWL)\n", SimTime()));
  pkt->free();		// done with HELLO packet
  return;
}

template <class PLD>
void SSRv02<PLD>::receiveBackupPacket(
  packet_t	*pkt)
{
  mwlPrint( ("SSRV02::rBP: BACKUP pkt received @ %f (MWL)\n", SimTime()));
  pkt->free();		// done with BACKUP packet
  return;
}

/*
** This node has just received an ACK packet.
** - If the ACK is for a packet that is scheduled to be transmitted, then
**   cancel the transmission. (Another node has already forwarded the packet.)
** - If the ACK was sent by the destination node, then cancel retransmission of
**   the packet. This is the equivalent of a node forwarding the packet. The
**   previous address field in the packet header works for identification.
*/
template <class PLD>
void SSRv02<PLD>::receiveAckPacket(
  packet_t	*pkt)
{
  mwlPrint( ("rA: ea %3d; 0x%08X [%3d, sn%08X] @ %f\n", (int) MyEtherAddr,
	     (int) pkt, (int) pkt->hdr.src_addr, pkt->hdr.seq_number,
	     SimTime()));

  // cancel pending send
  CancelPacket( pkt->hdr.src_addr, pkt->hdr.seq_number);

  // cancel retransmissions
  if( pkt->hdr.pre_addr == MyEtherAddr)
    performAck( pkt);
//    CancelRetransmission( pkt->hdr.src_addr, pkt->hdr.seq_number);
  return;
}

template <class PLD>
void SSRv02<PLD>::receiveReqRepDataPacket(
  packet_t	*pkt)
{
  cache_t::iterator iter;
  ether_addr_t	src = pkt->hdr.src_addr;

  updateHopCountInCache( src, pkt->hdr.actual_hop, false, "rRRDP", pkt);
  iter = m_seq_cache.find( src); 
/*
** The method check (defined at the top of this file) determines if this
** sequence number is greater than the sequence number in the cache. If the
** packet's sequence number is greater, then this packet is new. This
** comparison is not valid for resent packets or DATA or REPLY packets with
** the T3 flag set to true.
*/
  if( iter->second.check( pkt->hdr.seq_number) == false || pkt->hdr.resend != 0
      || pkt->hdr.t3Flag == true)
  {
    RecvUniPackets++;
    if( pkt->hdr.dst_addr == MyEtherAddr)
      recvPktAtDest( pkt, iter);
    else if( pkt->hdr.actual_hop < pkt->hdr.max_hop)
    {
      // create new packet to forward
      packet_t	*np = pkt->copy();

      np->hdr.cur_addr = MyEtherAddr;
      np->hdr.pre_addr = pkt->hdr.cur_addr;
      np->hdr.resend = 0;
      np->hdr.numSlots = numT2Slots;
      np->hdr.collDetected = false;
      np->hdr.collPktSent = false;
      // don't reset t3Flag since it's used by ForwardPacket
      //Printf((DumpPackets,"rr%d creates COPY %s\n",(int)MyEtherAddr,np->dump().c_str()));
      np->hdr.actual_hop++;
#ifdef VISUAL_ROUTE
      np->hdr.path.AddNode( MyEtherAddr);
#endif
      if( np->hdr.type == REQUEST)
      {
	np->hdr.t3Flag = false;
	//np->hdr.delay=ForwardDelay*(1-RXThresh/power);
	np->hdr.delay = Random( ForwardDelay);
	TotalDelay += np->hdr.delay;
	TotalSamples++;
	//Printf((DumpPackets,"rr%d forwards REQUEST %s \n",(int)MyEtherAddr, (int)pkt->dump().c_str()));
	mwlPrint( ("rRRDP: ea %3d forwards REQUEST %s @ %f\n",
		   (int) MyEtherAddr, pkt->dump().c_str(), SimTime()));
	SendPacket( np, false);		        
      }
/*
** If the destination is found in the cache and the hop count in the cache is
** less than the maximum allowed by the received packet, then forward the
** packet.
*/
      else
	ForwardPacket( np);
    }
  }
  else
  {
    // received packet is not new
//MWL	FLOODING
    if( pkt->hdr.type != REQUEST)
    {
      mwlPrint( ("rRRDP: ea %3d canceling packet @ %f\n", (int) MyEtherAddr,
		 SimTime()));
      CancelPacket( pkt->hdr.src_addr, pkt->hdr.seq_number);
      if( determineAckStatus( pkt) == true)
	performAck( pkt);
    }
  }
  pkt->free();		// done with received packet
  return;
}

/*******************************************************************
 * We are now receiving a packet from the mac layer.
 *******************************************************************/

template <class PLD>
void SSRv02<PLD>::from_mac_data(
  packet_t	*pkt,
  double	power)
{
//mwl  cache_t::iterator iter;
//mwl  ether_addr_t	src = pkt->hdr.src_addr;
  RecvPackets++;

  Printf((DumpPackets,"%d SSRv02: %d %d %s \n", (int) MyEtherAddr,
	  (int) pkt->hdr.cur_addr, (int) MyEtherAddr, pkt->dump().c_str()));
#ifdef	MWL_PRINT
  fflush( stdout);
  switch( pkt->hdr.type)
  {
    case REQUEST:
    case REPLY:
    case DATA:
    case ACK:
    case COLL:
    case HELLO:
    case BACKUP:
      mwlPrint( ("fmd: ea %3d<-%3d %s @ %f\n", (int) MyEtherAddr,
		 (int) pkt->hdr.cur_addr, pkt->dump().c_str(), SimTime()));
      break;
    default:
      mwlPrint( ("fmd: ea %3d; 0x%08X Bad packet type: 0x%08X\n",
		 (int) MyEtherAddr, (int) pkt, pkt->hdr.type));
      break;
  }
#endif	//MWL_PRINT
    
  switch( pkt->hdr.type)
  {
    case ACK:
      receiveAckPacket( pkt);
      break;
    case COLL:
      receiveCollisionPacket( pkt);
      break;
    case HELLO:
      receiveHelloPacket( pkt);
      break;
    case BACKUP:
      receiveBackupPacket( pkt);
      break;
    case REQUEST:
    case REPLY:
    case DATA:
      receiveReqRepDataPacket( pkt);
      break;
  }
  return;
}

template <class PLD>
void SSRv02<PLD>::recvPktAtDest(
  packet_t	*pkt,
  cache_t::iterator iter)
{
  if( pkt->hdr.resend != 0)
  {
    mwlPrint( ("rPAD: ea %3d; dropping resent packet 0x%08X %s @ %f\n",
	       (int) MyEtherAddr, (int) pkt, pkt->dump().c_str(), SimTime()));
    return;
  }

  switch( pkt->hdr.type)
  {
    // these cases can't happen, but they make the compiler happy
    case ACK:
    case HELLO:
    case COLL:
    case BACKUP:
      mwlPrint( ("SSRV02::rPAD: %s received @ %f\n", pkt->dump().c_str(),
		 SimTime()));
      break;
    case REQUEST:
    {
      packet_t	*rp;

      // send reply without any delay
      rp = (packet_t *) createReplyPkt( pkt->hdr.src_addr, iter->second.hopCount());
      Printf((DumpPackets, "%d SSRv02: %d->%d %s\n", (int) MyEtherAddr,
	      (int) MyEtherAddr, (int) pkt->hdr.src_addr,
	      rp->dump().c_str()));
      mwlPrint( ("rPAD: ea %d creates REPLY %s @ %f\n", (int) MyEtherAddr,
		 rp->dump().c_str(), SimTime()));

      // Send reply and check if any packets need to be sent to the src
      // of the request
      SendPacket( rp);
      CheckBuffer( pkt->hdr.src_addr, iter->second.hopCount());
    }
    break;
    case REPLY:
      Printf((DumpPackets,"%d SSRv02: %d<-%d %s\n", (int)MyEtherAddr,
	      (int)MyEtherAddr, (int)pkt->hdr.src_addr,
	      pkt->dump().c_str()));
      mwlPrint( ("rPAD: ea %3d; received REPLY from %4d @ %f\n",
		 (int) MyEtherAddr, (int) pkt->hdr.src_addr, SimTime()));
      // destination sends Ack after AckWindow
      SendAck( pkt->hdr.src_addr, MyEtherAddr, pkt->hdr.cur_addr,
	       pkt->hdr.seq_number, AckWindow);
      CheckBuffer( pkt->hdr.src_addr, iter->second.hopCount());
#ifdef VISUAL_ROUTE
      pkt->hdr.path.AddNode(MyEtherAddr);
      printf( "VR: %f: 0x%08X %3d->%3d sn%08X %s\n", SimTime(), (int) pkt, (int) pkt->hdr.src_addr,
	      (int) pkt->hdr.dst_addr, pkt->hdr.seq_number, pkt->hdr.path.ToString());
#endif //VISUAL_ROUTE
      break;
    case DATA:
      // the packet arrives at its destination
      // destination sends Ack after AckWindow
      SendAck( pkt->hdr.src_addr, MyEtherAddr, pkt->hdr.cur_addr,
	       pkt->hdr.seq_number, AckWindow);
      pkt->inc_pld_ref();
      mwlPrint( ("rPAD: ea %3d; data to transport @ %f\n", (int) MyEtherAddr,
		 SimTime()));
      to_transport( pkt->pld);
      {
	int	hc = pkt->hdr.actual_hop;
	TotalHop += hc;
	if( hc > HCArraySize-1)
	  hc = HCArraySize-1;
	HopCounts[hc]++;
      }
      RecvDataPackets++;
#ifdef VISUAL_ROUTE
      pkt->hdr.path.AddNode(MyEtherAddr);
//mwl      printf( "%f: %3d->%3d %s\n", SimTime(), (int) pkt->hdr.src_addr,
//mwl	      (int) pkt->hdr.dst_addr, pkt->hdr.path.ToString());
      printf( "VR: %f: 0x%08X %3d->%3d sn%08X %s\n", SimTime(), (int) pkt, (int) pkt->hdr.src_addr,
	      (int) pkt->hdr.dst_addr, pkt->hdr.seq_number, pkt->hdr.path.ToString());
#endif //VISUAL_ROUTE
      break;
  }
  return;
}

template <class PLD>
int SSRv02<PLD>::mwlGetHC(
  int		addr)
{
  cache_t::iterator	iter;
  ether_addr_t		ea = addr;

  iter = m_seq_cache.find( ea);
  return (int) iter->second.hopCount();
}

template <class PLD>
void SSRv02<PLD>::ForwardPacket(
  packet_t	*pkt)
{
  cache_t::iterator	iter;
  double	backOffTime;
  bool		forwardPkt;
  char		mwlFlag = pkt->hdr.t3Flag == true ? 'T' : 'F';
  int		slot, numSlots, h_table, h_pkt;

  iter = m_seq_cache.find( pkt->hdr.dst_addr);
  if( iter == m_seq_cache.end())
  {
    mwlPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; %c not in cache\n",
	       (int) MyEtherAddr, (int) pkt->hdr.pre_addr,
	       (int) pkt, pkt->hdr.seq_number, mwlFlag));
    pkt->free();	// dest not found, drop packet
    return;
  }

  h_table = iter->second.hopCount();
  h_pkt = pkt->hdr.expected_hop - 1;	// value in pkt not decremented when sent
  numSlots = pkt->hdr.numSlots;
  
  pkt->hdr.expected_hop = (*iter).second.hopCount();
//mwl These lines prevent any packet from every timing out
//mwl  if( pkt->hdr.max_hop < pkt->hdr.expected_hop + AdditionalHop)
//mwl    pkt->hdr.max_hop = pkt->hdr.expected_hop + AdditionalHop;
	    
  if( pkt->hdr.t3Flag == false)
  {
    // if h_table == h_pkt-1, then normal case and forward during T2
    // T2 is slots 1..numSlots
    if( h_table == h_pkt)
    {
      // When the protocol is modified so that the number of slots is dynamic,
      // use numSlots from original packet (Future implementation)
      slot = (int) Random( numSlots);
      forwardPkt = true;
      T2PktsSent[ slot]++;
      slot++;
    }
    // if h_table < h_pkt-1, then shorter route, send during T1
    // T1 is slot 0
    else if( h_table < h_pkt)
    {
      slot = 0;
      forwardPkt = true;
      T1PktsSent++;
    }
    else	// major topology change or isolated node; drop packet
    {
      DropPkts++;
      forwardPkt = false;
    }
  }
  else		// t3Flag == true
  {
    pkt->hdr.t3Flag = false;	// pkt no longer T3 enabling
    // if h_table <= h_pkt-1, then attempt to forward during first slot
    // This node should have responded T1 or T2 when the original DATA packet
    // was sent.
    if( h_table <= h_pkt)
    {
      slot = 0;
      forwardPkt = false;
      T3AbnormalPktsSent++;
      mwlPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; T abnormal packet; %2d %2d\n",
		 (int) MyEtherAddr, (int) pkt->hdr.pre_addr,
		 (int) pkt, pkt->hdr.seq_number, h_table,
		 h_pkt));
    }
    // else h_table > h_pkt-1, longer route, send during T3
    // T3 is slots 1..numT3Slots
    // slot within T3 determined by h_table - h_pkt
    else if( h_table <= h_pkt + numT3Slots)
    {
      slot = h_table - h_pkt;
      T3PktsSent[ slot]++;
      forwardPkt = true;
    }
    else	// major topology change or isolated node; drop packet
    {
      DropPkts++;
      forwardPkt = false;
    }
  }

  // Backoff has been calculated, forward packet if required
  if( forwardPkt == true)
  {
    mwlPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; %c pkt %2d; table %2d; slot %2d\n",
	       (int) MyEtherAddr, (int) pkt->hdr.pre_addr, (int) pkt,
	       pkt->hdr.seq_number, mwlFlag, h_pkt, h_table, slot));
    backOffTime = slot * slotWidth + Random( transitionTime);
    // insert an AckWindow before first slot
    pkt->hdr.delay = backOffTime + AckWindow;
    pkt->hdr.slotNumber = slot;
    SendPacket( pkt, false);
  }
  else
  {
    mwlPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; %c pkt %2d; table %2d; dropping\n",
	       (int) MyEtherAddr, (int) pkt->hdr.pre_addr, (int) pkt,
	       pkt->hdr.seq_number, mwlFlag, h_pkt, h_table));
    pkt->free();	// not forwarding, so free packet
  }
  return;
}

/*
** Determine if a packet should generate an ACK packet.
** If the packet that was just received is a REPLY or DATA packet, then check
** to see if the packet is a rebroadcast of a packet that was sent by this
** node. (Packet is on either the m_T3_queue or m_collPktsSent_queue.)
** In addition, also gather statistics on the classification of the packet that
** was received.
*/
template <class PLD>
bool SSRv02<PLD>::determineAckStatus(
  packet_t	*pkt)
{
  if( pkt->hdr.type == REQUEST)
    return false;

  packet_queue_t::iterator	iter;
  packet_t	*orig;
  bool		doAck;
  int		i = pkt->hdr.type == REPLY ? 0 : 1;	// for stats keeping

  mwlPrint( ("dAS: ea %3d; Looking %s on m_T3_queue\n", (int) MyEtherAddr,
	     pkt->dump().c_str()));
  // get original packet from queue of sent packets
  for( iter = m_T3_queue.begin(); iter != m_T3_queue.end(); iter++)
  {
    mwlPrint( ("dAS: ea %3d;   Found %s on m_T3_queue\n", (int) MyEtherAddr,
	       (*iter)->dump().c_str()));
    if( (*iter)->hdr.src_addr == pkt->hdr.src_addr &&
	(*iter)->hdr.dst_addr == pkt->hdr.dst_addr &&
	(*iter)->hdr.seq_number == pkt->hdr.seq_number &&
	(*iter)->hdr.type == pkt->hdr.type)
      break;		// match found
  }
  // if not found, check COLL packet queue
  if( iter == m_T3_queue.end())
  {
    for( iter = m_collPktsSent_queue.begin(); iter!=m_collPktsSent_queue.end();
	 iter++)
    {
      mwlPrint( ("dAS: ea %3d;   Found %s on m_collPktsSent_queue\n",
		 (int) MyEtherAddr, (*iter)->dump().c_str()));
      if( (*iter)->hdr.src_addr == pkt->hdr.src_addr &&
	  (*iter)->hdr.dst_addr == pkt->hdr.dst_addr &&
	  (*iter)->hdr.seq_number == pkt->hdr.seq_number &&
	  (*iter)->hdr.type == pkt->hdr.type)
	break;		// match found
    }
    if( iter == m_collPktsSent_queue.end())
      return false;
    mwlPrint( ("dAS: ea %3d;   match m_collPktsSent_queue\n", (int) MyEtherAddr));
  }
  else
    mwlPrint( ("dAS: ea %3d;   match m_T3_queue\n", (int) MyEtherAddr));
  orig = *iter;

  doAck = true;
  if( orig->hdr.canceled == true)
    CanceledPktsRcvd[i]++;
  else if( orig->hdr.cur_addr != pkt->hdr.pre_addr)
    BadAddrPktsRcvd[i]++;
  else if( orig->hdr.actual_hop+1 != pkt->hdr.actual_hop)
    WrongHopPktsRcvd[i]++;
  else
  {
    // account for the AckWindow added to forwarding delay
    double	delta = SimTime() - orig->hdr.xmit_end_time - AckWindow;
    int		slot = (int) (delta / slotWidth);

    if( slot < 1)
      T1PktsRcvd[i]++;
    else if( slot <= numT2Slots)
      T2PktsRcvd[i][ slot - 1]++;
    else if( slot <= numT2Slots + numT3Slots)
      T3PktsRcvd[i][ slot - numT2Slots - 1]++;
    else
    {
      // don't want to ack these packets
      // wait for timer to expire (started in from_mac_ack)
      doAck = false;
      T4PktsRcvd[i]++;
    }
    if( doAck == true && pkt->hdr.expected_hop != orig->hdr.expected_hop - 1)
      updateHopCountInCache( orig->hdr.dst_addr, pkt->hdr.expected_hop + 1,
			     true, "dAS", orig);
  }
//mwl  else
//mwl    OtherPktsRcvd[i]++;
  return doAck;
}

/*
** This node has detected the retransmission of a packet that it had sent.
** - Cancel retransmission of the packet
** - Send an ACK packet
*/
template <class PLD>
void SSRv02<PLD>::performAck(
  packet_t	*pkt)
{
  CancelRetransmission( pkt->hdr.src_addr, pkt->hdr.seq_number);
  if( pkt->hdr.pre_addr == MyEtherAddr)	//mwl this test shouldn't be needed
  {
    mwlPrint( ("pA: ea %3d; 0x%08X; sn%08X; sending ACK\n", (int) MyEtherAddr,
	       (int) pkt, pkt->hdr.seq_number));
    SendAck( pkt->hdr.src_addr, MyEtherAddr, MyEtherAddr, pkt->hdr.seq_number,
	     0.0);
  }
  else
  {
    mwlPrint( ("pA: ea %3d; 0x%08X; sn%08X; resend<-MaxResend\n",
	       (int) MyEtherAddr, (int) pkt, pkt->hdr.seq_number));
    //printf("rr%d receives again %s\n",(int)MyEtherAddr,(*iter)->dump().c_str());
//    (*iter)->hdr.resend = MaxResend;
  }
  return;
}

/*
** A receive-receive collision has been detected.
** This will affect every packet on the m_T3_queue since these are the packets
** that have yet to be rebroadcast by the next node.
*/
template <class PLD>
void SSRv02<PLD>::from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2)
{
  packet_queue_t::iterator	iter;

  // for each pkt on m_T3_queue
  for( iter = m_T3_queue.begin(); iter != m_T3_queue.end(); iter++)
  {
    packet_t	*pkt = *iter;
    double	delta;
    int		slot;
/*
** determine time slot of collision
** Remember, AckWindow is added to the packet's transmission time, so account
** for that value here. If a collision occurs before the end of AckWindow, it
** must be an external collision, so it is ignored.
*/
    delta = SimTime() - pkt->hdr.xmit_end_time - AckWindow;
    if( delta < 0.0)
      continue;
    slot = (int) (delta / slotWidth);
    // record statistics
    if( slot < 1)
      T1Collisions++;
    else if( slot <= numT2Slots)
      T2Collisions[ slot - 1]++;
    else if( slot <= numT2Slots + numT3Slots)
      T3Collisions[ slot - 1 - numT2Slots]++;
    else
      IgnoreCollisions++;

    // record in packet when collision was detected
    if( slot < 1 + numT2Slots + numT3Slots)
    {
      mwlPrint( ("fmrrc: ea %3d; pkt 0x%08X sn%08X st %f @ %f [%d] (%3d, sn%08X; %3d, sn%08X)\n",
		 (int) MyEtherAddr, (unsigned int) (*iter),
		 (*iter)->hdr.seq_number, (*iter)->hdr.xmit_start_time,
		 SimTime(), slot, (int) pkt1->hdr.cur_addr,
		 pkt1->hdr.seq_number, (int) pkt2->hdr.cur_addr,
		 pkt2->hdr.seq_number));
      pkt->hdr.collDetected = true;
      pkt->hdr.slotNumber = slot;
    }
  }
  return;
}

template <class PLD>
void SSRv02<PLD>::CancelPacket(
  ether_addr_t	&src,
  unsigned int	seq_number)
{
//  mwlPrint( ("CP: ea %3d; src %3d; sn%08X @ %f\n", (int) MyEtherAddr,
//	     (int) src, seq_number, SimTime()));
  /*
  ** If the incoming packet is the same as the outgoing packet, cancel the
  ** outgoing packet. But, if the outgoing packet is an ACK, don't cancel it.
  ** This prevents the case of a slightly delayed DATA (or REPLY) packet that
  ** doesn't cause a collision from cancelling an ACK.
  */
  if( m_mac_busy == true && m_active_packet->hdr.src_addr == src &&
      m_active_packet->hdr.seq_number == seq_number &&
      m_active_packet->hdr.type != ACK)
  {
    mwlPrint( ("CP: ea %3d; src %3d; sn%08X cancel active @ %f\n",
	       (int) MyEtherAddr, (int) src, seq_number, SimTime()));
    //Printf((DumpPackets,"rr%d cancels the current packet\n",(int)MyEtherAddr));
    m_active_packet->hdr.canceled = true;
    cancel();
    return;
  }

  // Now, look for the packet in m_send_queue.
  packet_queue_t::iterator iter;
  for( iter = m_send_queue.begin(); iter != m_send_queue.end(); iter++)
    if( (*iter)->hdr.src_addr == src && (*iter)->hdr.seq_number == seq_number)
      break;

  // If not found, then nothing to do.
  if( iter == m_send_queue.end())
    return;

  packet_t	*pktFromQ = *iter;
  // If the pending packet is an ACK, don't cancel it.
  if( pktFromQ->hdr.type != ACK)
  {
    CanceledPackets++;
//mwl    if( pktFromQ->hdr.suboptimal == true)
//mwl      CanceledSuboptimal++;
    pktFromQ->free();
    m_send_queue.erase( iter);
  }
  return;
}

/*
** Cancel future retransmissions of a packet.
** - Remove the packet from the m_T3_queue
** - Cancel the T1, T2 & T3 timers
** - Remove the packet from the m_collPktsSent_queue
*/
template <class PLD>
void SSRv02<PLD>::CancelRetransmission(
  ether_addr_t	&src,
  unsigned int	seq_number)
{
  packet_queue_t::iterator	iter;
  packet_t	*orig;

  mwlPrint( ("CR: ea %3d; [%3d, sn%08X]\n", (int) MyEtherAddr, (int) src,
	     seq_number));
/*
** Search m_T3_queue looking for a matching packet. If it is found, then send
** an ACK message.
*/
  for( iter = m_T3_queue.begin(); iter != m_T3_queue.end(); iter++)
    if( (*iter)->hdr.src_addr == src && (*iter)->hdr.seq_number == seq_number)
      break;		// match found

  // if found, cancel timers
  if( iter != m_T3_queue.end())
  {  
    orig = *iter;
    mwlPrint( ("CR: ea %3d; removing 0x%08X from m_T3_queue\n",
	       (int) MyEtherAddr, (int) orig));
    // cancel COLL packet timers
    if( endT1Timer.Active( orig->hdr.t1Index) == true)
      endT1Timer.Cancel( orig->hdr.t1Index);
    if( endT2Timer.Active( orig->hdr.t2Index) == true)
      endT2Timer.Cancel( orig->hdr.t2Index);
//mwl    if( endT3Timer.Active( orig->hdr.t3Index) == true)
//mwl      endT3Timer.Cancel( orig->hdr.t3Index);
    m_T3_queue.erase( iter);
    orig->free();
  }

  // if pkt on Coll Sent queue, remove it
  for( iter = m_collPktsSent_queue.begin(); iter != m_collPktsSent_queue.end();
       iter++)
    if( (*iter)->hdr.src_addr == src && (*iter)->hdr.seq_number == seq_number)
      break;		// match found
  if( iter != m_collPktsSent_queue.end())
  {
    orig = *iter;
    mwlPrint( ("CR: ea %3d; removing 0x%08X from m_collPktsSent_queue\n",
	       (int) MyEtherAddr, (int) orig));
    if( endCollTimer.Active( orig->hdr.collIndex) == true)
      endCollTimer.Cancel( orig->hdr.collIndex);
    m_collPktsSent_queue.erase( iter);
    orig->free();
  }
  return;
}

/*
** The routing layer has received an acknowledgement from the mac layer.
** False indicates that the transmission failed.
*/
template <class PLD>
void SSRv02<PLD>::from_mac_ack(
  bool		flag)
{
  bool		dataReply = m_active_packet->hdr.canceled == false &&
      (m_active_packet->hdr.type == REPLY ||
       m_active_packet->hdr.type == DATA);
  bool		testQueue = true;

  //Printf((DumpPackets,"rr%d receives an ack from mac\n",(int)MyEtherAddr));

  // a non-ack packet failed, so resend it
  /*
  ** Resending is handled differently with ASHR. The old way (SSR) was to
  ** always resechedule non-ACK packets. With ASHR, a DATA or REPLY packet that
  ** is being sent by an intermediate node is not rescheduled. ASHR will send a
  ** COLL packet at the end of T2. Note that if a collision occurs when the
  ** source sends a packet, no COLL packet will be sent. Therefore, in this
  ** case the packet must be rescheduled.
  */
  if( flag == false && m_active_packet->hdr.type != ACK)
  {
    if( MyEtherAddr == m_active_packet->hdr.src_addr ||
	(m_active_packet->hdr.type != DATA &&
	 m_active_packet->hdr.type != REPLY))
    {
      mwlPrint( ("fma: ea %3d; cur %3d; reschedule %s; @ %f\n",
		 (int) MyEtherAddr, (int) m_active_packet->hdr.cur_addr,
		 m_active_packet->dump().c_str(), SimTime()));
      //Printf((DumpPackets,"%d resends %s\n", (int)MyEtherAddr,m_active_packet->dump().c_str()));
      m_active_packet->inc_ref();
      SendToMac( m_active_packet);
      testQueue = false;	// just sent to mac, so don't test queue
    }
    else
    {
      mwlPrint( ("fma: ea %3d; cur %3d; dropping %s; @ %f\n",
		 (int) MyEtherAddr, (int) m_active_packet->hdr.cur_addr,
		 m_active_packet->dump().c_str(), SimTime()));
      // add to T3 queue
      m_active_packet->inc_ref();
      m_T3_queue.push_back( m_active_packet);
    }
  }
  else
  {
    m_active_packet->hdr.xmit_end_time = SimTime();
    if( dataReply == true)
    {
      double	start;

      mwlPrint( ("fma: ea %3d; Starting timers %3d->%3d; %c; %c; %s @ %f\n",
		 (int) MyEtherAddr, (int) m_active_packet->hdr.src_addr,
		 (int) m_active_packet->hdr.dst_addr,
		 m_active_packet->hdr.type == DATA ? 'D' : m_active_packet->hdr.type == REPLY ? 'R' : '?',
		 m_active_packet->hdr.t3Flag == true ? 'T' : 'F',
		 m_active_packet->dump().c_str(), SimTime()));
      // start T1, T2 & T3 timers & add pkt to end of queue

      if( endT1Timer.Active( m_active_packet->hdr.t1Index) == true)
	mwlPrint( ("fma: ea %3d; sn%08X; T1 (%d) active (%f)\n", (int) MyEtherAddr, m_active_packet->hdr.seq_number, m_active_packet->hdr.t1Index, endT1Timer.GetTime( m_active_packet->hdr.t1Index)));
      if( endT2Timer.Active( m_active_packet->hdr.t2Index) == true)
	mwlPrint( ("fma: ea %3d; sn%08X; T2 (%d) active (%f)\n", (int) MyEtherAddr, m_active_packet->hdr.seq_number, m_active_packet->hdr.t2Index, endT2Timer.GetTime( m_active_packet->hdr.t2Index)));

      start = SimTime() + slotWidth + 2 * transitionTime + AckWindow;
      m_active_packet->hdr.t1Index = endT1Timer.Set( m_active_packet, start);
      start += (numT2Slots + 1) * slotWidth;
      m_active_packet->hdr.t2Index = endT2Timer.Set( m_active_packet, start);
//mwl      start += numT3Slots * slotWidth;
//mwl      m_active_packet->hdr.t3Index = endT3Timer.Set( m_active_packet, start);

      // add to T3 queue
      m_active_packet->inc_ref();
      m_T3_queue.push_back( m_active_packet);
    }
    else
    {
      mwlPrint( ("fma: ea %3d; dropping !dataReply %3d->%3d; sn %08X @ %f\n",
		 (int) MyEtherAddr, (int) m_active_packet->hdr.src_addr,
		 (int) m_active_packet->hdr.dst_addr,
		 m_active_packet->hdr.seq_number, SimTime()));
      ;
    }
    m_active_packet->free();
  }
  if( testQueue == true)
    if( m_send_queue.size() != 0)
    {
      packet_t	*packet = m_send_queue.front();

      m_send_queue.pop_front();
      mwlPrint( ("fma: ea %3d; 0x%08X; sn %08X from send queue\n",
		 (int) MyEtherAddr, (int) packet, packet->hdr.seq_number));
      //Printf((DumpPackets,"rr%d forwards %s from queue\n",(int)MyEtherAddr,packet->dump().c_str()));
      m_active_packet = packet;
      m_active_packet->inc_ref();
      SendToMac( packet);
    }
    else
      m_mac_busy = false;
  return;
}

template <class PLD>
void SSRv02<PLD>::SendAck(
  ether_addr_t	src_addr,
  ether_addr_t	cur_addr,
  ether_addr_t	pre_addr,
  unsigned int	seq_number,
  simtime_t	delay)
{
  packet_t	*p = (packet_t *) createAckPkt( src_addr, cur_addr, pre_addr,
						seq_number, delay);

  Printf((DumpPackets,"%d SSRv02: %d->%d %s\n",(int)MyEtherAddr,(int)cur_addr,
	  (int)pre_addr,p->dump().c_str()));
  mwlPrint( ("SA: ack %3d->%3d %s @ %f\n", (int) cur_addr, (int) pre_addr,
	     p->dump().c_str(), SimTime()));
  SendPacket( p, false);
  return;
}

template <class PLD>
void SSRv02<PLD>::CheckBuffer(
  ether_addr_t	src_addr,
  unsigned int	expected_hop)
{
  packet_queue_t::iterator	db_iter;

  db_iter = m_data_buffer.begin();
  while( db_iter != m_data_buffer.end())
  {
    if( (*db_iter)->hdr.dst_addr == src_addr)
    {
      packet_queue_t::iterator	t = db_iter;
      db_iter++;
      (*t)->hdr.expected_hop = expected_hop;
      (*t)->hdr.max_hop = expected_hop + AdditionalHop;
      (*t)->hdr.delay = AckWindow;
      SendPacket( *t);
      m_data_buffer.erase( t);
    }
    else
      db_iter++;
  }
  return;
}

template <class PLD>
bool SSRv02_Struct<PLD>::hdr_struct::dump(
  std::string	&str) const 
{ 
  static char	pkt_type[][10]={ "REQ", "REPLY", "DATA", "ACK", "COLL",
				 "HELLO", "BACK"};
//mwl  char		buffer[30];
  char		buffer[50];

///mwl  sprintf( buffer, "0x%08X %s %d->%d %d %d", (int) this, pkt_type[type],
  sprintf( buffer, "0x%08X %s %c %d->%d sn%08X %d", (int) this, pkt_type[type],
	   t3Flag == true ? 'T' : 'F', (int) src_addr, (int) dst_addr,
	   seq_number, size); 
  str = buffer;
  return type == DATA;
}

#endif /*SSRv02_h*/
