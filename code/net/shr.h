/*************************************************************************
 *   @<title> Self Healing Routing </title>@
 *
 *   @<!-- Copyright 2006, 2003 Mark Lisee, Gilbert (Gang) Chen, Boleslaw
 *   K. Szymanski and Rensselaer Polytechnic Institute. All worldwide rights
 *   reserved.  A license to use, copy, modify and distribute this software
 *   for non-commercial research purposes only is hereby granted, provided
 *   that this copyright notice and accompanying disclaimer is not modified or
 *   removed from the software.
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

#ifndef _shr_h_
#define _shr_h_
#include <map>
#include <list>

#define	MWL_DOUBLE_DELAY
const int	MWLTimeoutMultiplier = 10;

#ifdef	SHR_PRINT
#define shrDoPrint	(true)
#define	shrPrint(x)	 printf x
#else	//SHR_PRINT
#define	shrPrint(x)
#define	shrDoPrint	(false)
#endif	//SHR_PRINT

template <class PLD>
struct SHR_Struct
{
  enum PktType { REQUEST, REPLY, DATA, ACK, HELLO };
  enum AckType { AT_Destination, AT_Intermediate };
  // RREQ: type + src_addr + dst_addr + seq_number + hop
  enum { REQUEST_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // RREP: type + src_addr + dst_addr + seq_number + 2 * hop
  enum { REPLY_SIZE = 4 * sizeof(int) + 4 * ether_addr_t::LENGTH };
  // DATA: type + src_addr + dst_addr + seq_number + 2* hop 
  enum { DATA_SIZE = 4 * sizeof(int) + 4 * ether_addr_t::LENGTH };
  // ACK: type + ACK Type + src_addr + dst_addr + seq_number
  enum { ACK_SIZE = 3 * sizeof(int) + 2 * ether_addr_t::LENGTH };
  // HELLO: type + src_addr + dest_addr + hopCount
  enum { HELLO_SIZE = 2 * sizeof( int) + 2 * ether_addr_t::LENGTH };
    
  struct hdr_struct
  {
    PktType	type;
    AckType	ackType;
    ether_addr_t src_addr;
    ether_addr_t dst_addr;
    ether_addr_t cur_addr;
    ether_addr_t pre_addr;
    unsigned int seq_number;
    unsigned int size;
    double	send_time;
    unsigned int actual_hop;
    unsigned int expected_hop;
    unsigned int max_hop;
    double	delay;
    int		ackIndex;
    int		pktDelayIndex;
    bool	canceled;
    int		resend;
    bool	suboptimal;
    bool	newSeqNumber;
    int		helloData;
    Path	path;

    bool dump(std::string& str) const;
  };
  typedef PLD payload_t;
  typedef smart_packet_t<hdr_struct,PLD> packet_t;
};

/*
** Array to keep track of how many packets required a certain number of hop
** counts to reach the destination.
*/
#define	HCArraySize	(20+1)

enum SHRBackOff
{
  SHRBackOff_SSR = 1,
  SHRBackOff_SHR = 2,
  SHRBackOff_Incorrect = 3
};

/*
** The AckWindow is a period of time during which only Acks from intermediate
** nodes are sent. This prevents the Acks from colliding with packets that are
** being forwarded. Note that the destination sends its Ack as a normal packet,
** i.e. after the AckWindow. This is because when node N forwards a packet, it
** causes node N-1 to send an Ack (during the AckWindow) and N+1 to forward the
** packet (after the AckWindow).
*/
template <class PLD>
component SHR : public TypeII, public SHR_Struct<PLD>
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
    
  // statistics
  int		SentPackets;
  int		SentSuboptimal;
  int		CanceledPackets;
  int		CanceledSuboptimal;
  int		RecvPackets;
  int		RecvUniPackets;
  int		RecvDataPackets;
  double	TotalDelay;
  int		TotalSamples;
  int		TotalHop;
  int		HopCounts[ HCArraySize];

  Timer <trigger_t> transitionTimer;	// time it takes to go from sense to send
  inport void	TransitionTimer( trigger_t &);
  packet_t	*transitionPkt;		// pkt to be sent when timer expires

  inport inline void	from_transport( payload_t& pld, ether_addr_t& dst,
					unsigned int size);
  inport inline void	from_mac_data( packet_t* pkt, double power);
  inport inline void	from_mac_ack( bool errflag);
  inport inline void	from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2);

  // This is called when the power manager wakes up the node.
  inport void	from_pm_node_up();

  outport void		cancel();
  outport void		to_transport( payload_t &pld);
  outport void		to_mac( packet_t *pkt, unsigned int size,
				double backoff);

  InfiTimer <SHR_Struct<PLD>::packet_t *> ackTimer;
  inport inline void	AckTimer( packet_t *pkt, unsigned int index);
/*
** These methods must be defined even though they aren't used with the radio
** model. This is due to a limitation of the cxx utility.
**
** This timer schedules the first HELLO packet that is generated when a node
** comes back. Events on this timer should be scheduled only as a result of
** parsing the configuration file.
*/
 private:
  Timer <trigger_t> helloTimer;
 public:
  inport inline void	HelloTimer( trigger_t &);
/*
** The public interface to schedule a HELLO packet.
*/
 public:
  void		scheduleHELLO( double time);

  void		Start();
  void		Stop();
  void		clearStats();
		SHR();
  virtual	~SHR();
  int		getHCFromCostTable( int addr);
  void		dumpStatic( FILE *fp, int indent, int offset) const;
  void		dumpCacheStatic( FILE *fp, int indent, int offset) const;
  void		dump( FILE *fp, int indent, int offset) const;
  void		dumpCostTable( FILE *fp, int indent, int offset) const;
  void		generateHELLO();
  static void		setSlotWidth( double);
  static double		getSlotWidth();
  static void		setTransitionTime( double);
  static double		getTransitionTime();
  static void		setBackOff( SHRBackOff);
  static SHRBackOff	getBackOff();
  static void		setContinuousBackOff( bool);
  static bool		getContinuousBackOff();
  static void		setRouteRepair( int);
  static void		setVisualizer( Visualizer *ptr);
	
 private:
  enum UpdateType {Lower, Always, Hello};
  bool		updateHopCountInCache( ether_addr_t src,
				       unsigned int hopCount, UpdateType,
				       const char *, packet_t *pkt);
 protected:
  static SHRBackOff	backOff;
  static bool		continuousBackOff;
  static double		slotWidth;
  static double		transitionTime;
  static bool		RouteRepairEnabled;
  static Visualizer	*visualizer;
  void		CancelPacket( ether_addr_t& src, unsigned int seq_number);
  void		SendPacket( packet_t *pkt);
  void		AssignSeqNumber( packet_t *pkt);
  void		SendToMac( packet_t *pkt);
  void		SendAck( ether_addr_t, ether_addr_t, ether_addr_t, unsigned int,
			 simtime_t, bool fromDest);
  void		CheckBuffer( ether_addr_t, unsigned int);
  void		AddToSendQueue( packet_t *pkt);
  void		forwardAsFlood( packet_t *pkt);
  
  // these return pointers to packet_t, but I wasn't able to figure out
  // how to avoid compiler errors, therefore cast them to void *
  void		*createBasicPkt();
  void		*createDataPkt( payload_t &pld, ether_addr_t dst,
				unsigned int size);
  void		*createRequestPkt( ether_addr_t dst);
  void		*createReplyPkt( ether_addr_t src, unsigned int expectedHop);
  void		*createAckPkt( ether_addr_t src_addr, ether_addr_t cur_addr,
			       ether_addr_t pre_addr, unsigned int seq_number,
			       simtime_t delay, bool fromDest);
  void		*createHelloPkt( ether_addr_t dest_addr, int hopCount);

  bool		m_mac_busy;                  // if the mac layer is busy
  unsigned int	m_seq_number;        // current sequence number
  typedef std::list<packet_t* > packet_queue_t;

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
    void		hopCount( unsigned int newHC, bool force, bool print);
    static void		setCounter( int);
    static int		getCounter();
    void		dumpStatic( FILE *fp, int indent, int offset) const;
    void		dump( FILE *fp, int dest, int indent, int offset) const;
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
 private:
  void		receiveAck( packet_t *ackPkt);
  void		receiveDataRep( packet_t *rcvdPkt);
  void		receiveHelloPacket( packet_t *pkt);
  void		*getFromAckList( packet_t *rcvdPkt);
  void		removeFromAckList( packet_t *pktToRemove, bool cancelTimer);
  void		doRouteRepair( packet_t *pktFromList, packet_t *rcvdPkt);
  static bool	doubleMsgPrinted;
  packet_queue_t m_send_queue;
  packet_queue_t m_ack_list;
  packet_t	*m_active_packet;

 public:
  InfiTimer <SHR_Struct<PLD>::packet_t *> pktDelayTimer;
  inport inline void	PktDelayTimer( packet_t *pkt, unsigned int index);
 private:
  packet_queue_t m_delay_list;
};

template <class PLD> SHRBackOff	SHR<PLD>::backOff;
template <class PLD> bool	SHR<PLD>::continuousBackOff;
template <class PLD> double	SHR<PLD>::slotWidth;
template <class PLD> double	SHR<PLD>::transitionTime;
template <class PLD> bool	SHR<PLD>::RouteRepairEnabled;
template <class PLD> int	SHR<PLD>::seq_number_t::maxCounter = 1;
template <class PLD> bool	SHR<PLD>::doubleMsgPrinted = false;
template <class PLD> Visualizer	*SHR<PLD>::visualizer = NULL;

template <class PLD>
void SHR<PLD>::setVisualizer(
  Visualizer	*ptr)
{
  visualizer = ptr;
  return;
}

template <class PLD>
int SHR<PLD>::seq_number_t::getCounter()
{
  return maxCounter;
}

template <class PLD>
void SHR<PLD>::seq_number_t::setCounter(
  int	newCounter)
{
  maxCounter = newCounter;
  return;
}

template <class PLD>
unsigned int SHR<PLD>::seq_number_t::hopCount() const
{
  switch( state)
  {
    case Initial:
      return 0;
    case Steady:
    case Changing:
      return currentHC;
    default:
      assert( 0);	// something really bad happened
      return 0;
  }
}

template <class PLD>
void SHR<PLD>::seq_number_t::hopCount(
  unsigned int	newHC,
  bool		force,	// if true, then don't go through count
  bool		print)
{
  switch( state)
  {
    case Initial:
      currentHC = newHC;
      updateCtr = maxCounter;
      state = Steady;
      if( print) printf( "\n");
      break;
    case Steady:
      if( newHC != currentHC)
      {
	if( force == true || maxCounter < 2)		// single step, so don't use state Changing
	  currentHC = newHC;
	else
	{
	  --updateCtr;
	  pendingHC = newHC;
	  state = Changing;
	  if( print) printf( " start change");
	}
      }
      if( print) printf( "\n");
      break;
    case Changing:			// only happens if maxCounter >= 2
      if( force == true)
      {
	state = Steady;
	currentHC = newHC;
	updateCtr = maxCounter;
	if( print) printf( " forced\n");
      }
      else if( newHC == currentHC)
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
void *SHR<PLD>::createBasicPkt()
{
  packet_t	*newPkt = packet_t::alloc();

  // type, ack_type, addresses set by caller
  newPkt->hdr.seq_number = 0xFFFFFFFF;
  // size set by caller
  newPkt->hdr.send_time = SimTime();
  newPkt->hdr.actual_hop = 1;
  newPkt->hdr.expected_hop = 0;
  newPkt->hdr.max_hop = 1;
  newPkt->hdr.delay = 0.0;
  newPkt->hdr.ackIndex = 0;
  newPkt->hdr.pktDelayIndex = 0;
  newPkt->hdr.canceled = false;
  newPkt->hdr.resend = 0;
  newPkt->hdr.suboptimal = false;
  newPkt->hdr.newSeqNumber = false;
  return newPkt;
}

/*
** There is no delay when the source sends an packet.
** This method isn't called when a packet is forwarded. Instead, the incoming
** packet is copied to make the outgoing packet.
*/
template <class PLD>
void *SHR<PLD>::createDataPkt(
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
  newPkt->hdr.newSeqNumber = true;
  newPkt->pld = pld;
  return newPkt;
}

template <class PLD>
void *SHR<PLD>::createRequestPkt(
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
  newPkt->hdr.newSeqNumber = true;
  return newPkt;
}

template <class PLD>
void *SHR<PLD>::createReplyPkt(
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
  newPkt->hdr.newSeqNumber = true;
  return newPkt;
}

template <class PLD>
void *SHR<PLD>::createAckPkt(
  ether_addr_t	src_addr,
  ether_addr_t	cur_addr,
  ether_addr_t	pre_addr,
  unsigned int	seq_number,
  simtime_t	delay,
  bool		fromDest)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = ACK;
  newPkt->hdr.ackType = fromDest == true ? AT_Destination : AT_Intermediate;
  newPkt->hdr.size = ACK_SIZE;
  newPkt->hdr.src_addr = src_addr;
  newPkt->hdr.cur_addr = cur_addr;
  newPkt->hdr.pre_addr = pre_addr;
  newPkt->hdr.seq_number = seq_number;
  newPkt->hdr.delay = delay;
  newPkt->hdr.max_hop = 0;
  return newPkt;
}

//mwl: transfer changes here to SSRv02
template <class PLD>
void *SHR<PLD>::createHelloPkt(
  ether_addr_t	destAddr,
  int		hopCount)
{
  packet_t	*newPkt = (packet_t *) createBasicPkt();

  newPkt->hdr.type = HELLO;
  newPkt->hdr.size = HELLO_SIZE;
  newPkt->hdr.expected_hop = 1;
  newPkt->hdr.src_addr = MyEtherAddr;
  newPkt->hdr.dst_addr = destAddr;
  newPkt->hdr.cur_addr = MyEtherAddr;
  newPkt->hdr.pre_addr = MyEtherAddr;
  newPkt->hdr.helloData = hopCount;
  newPkt->hdr.delay = Random( ForwardDelay);
  newPkt->hdr.newSeqNumber = true;
  return newPkt;
}

#define	makeiS	char iS[81]; for( int i = indent-1; i >= 0; i--) iS[i] = ' '; iS[indent] = 0
#define	pIndent	fputs( iS, fp)

template <class PLD>
void SHR<PLD>::seq_number_t::dumpStatic(
  FILE		*fp,
  int		indent,
  int		offset) const
{
  indent += offset;
  makeiS;
  pIndent;
  fprintf( fp, "maxCounter %d\n", maxCounter);
  return;
}

template <class PLD>
void SHR<PLD>::seq_number_t::dump(
  FILE		*fp,
  int		dest,
  int		indent,
  int		offset) const
{
  makeiS;
  pIndent;
  fprintf( fp, "%3d { ", dest);
  switch( state)
  {
    case Initial:
      fprintf( fp, "currentHC 0, state Initial");
      break;
    case Steady:
      fprintf( fp, "currentHC %d, state Steady", currentHC);
      break;
    case Changing:
      fprintf( fp, "currentHC %d, state Changing, pendingHC %d, updateCtr %d",
	       currentHC, pendingHC, updateCtr);
      break;
  }
  fprintf( fp, " }\n");
  return;
}

template <class PLD>
void SHR<PLD>::dumpStatic(
  FILE		*fp,
  int		indent,
  int		offset) const
{
  indent += offset;
  makeiS;
  pIndent;
  fprintf( fp, "backOff %s, continuous %s, routeRepair %s,\n",
	   backOff == SHRBackOff_SSR ? "SSR" :
	   backOff == SHRBackOff_Incorrect ? "Incorrect" : "SHR",
	   continuousBackOff == true ? "true" : "false",
	   RouteRepairEnabled == true ? "true" : "false");
  if( continuousBackOff == false)
  {
    pIndent;
    fprintf( fp, "slotWidth %f, transitionTime %f,\n", slotWidth, transitionTime);
  }
  pIndent;
  fprintf( fp, "hopCounts %d\n", HCArraySize);
  return;
}

template <class PLD>
void SHR<PLD>::dumpCacheStatic(
  FILE		*fp,
  int		indent,
  int		offset) const
{
  m_seq_cache.begin()->second.dumpStatic( fp, indent, offset);
  return;
}

template <class PLD>
void SHR<PLD>::dump(
  FILE		*fp,
  int		indent,
  int		offset) const
{
//  cache_t::const_iterator	iter;

  indent += offset;
  makeiS;
  pIndent;
  fprintf( fp, "addr %d, seqNumber %d, ForwardDelay %f,\n",
	   (int) MyEtherAddr, m_seq_number, ForwardDelay);
  pIndent;
  fprintf( fp, "RXThresh %f, AckWindow %f, MaxResend %d, TimeToLive %d,\n",
	   RXThresh, AckWindow, MaxResend, TimeToLive);
  pIndent;
  fprintf( fp, "AdditionalHop %d, TotalDelay %f, TotalSamples %d, "
	   "TotalHop %d,\n", AdditionalHop, TotalDelay, TotalSamples,TotalHop);
  pIndent;
  fprintf( fp, "SentPackets %d, RecvPackets %d, RecvUniPackets %d, "
	   "RecvDataPackets %d,\n", SentPackets, RecvPackets, RecvUniPackets,
	   RecvDataPackets);
  pIndent;
  fprintf( fp, "SentSuboptimal %d, CanceledPackets %d, CanceledSuboptimal %d\n",
	   SentSuboptimal, CanceledPackets, CanceledSuboptimal);
  return;
}

template <class PLD>
void SHR<PLD>::dumpCostTable(
  FILE		*fp,
  int		indent,
  int		offset) const
{
  cache_t::const_iterator	iter;

  indent += offset;
  makeiS;
//  pIndent;

  for( iter = m_seq_cache.begin(); iter != m_seq_cache.end(); iter++)
    iter->second.dump( fp, (int) iter->first, indent+offset, offset);
  return;
}

template <class PLD>
void SHR<PLD>::setSlotWidth(
  double	sw)
{
  slotWidth = sw;
  return;
}

template <class PLD>
double SHR<PLD>::getSlotWidth()
{
  return slotWidth;
}

template <class PLD>
void SHR<PLD>::setTransitionTime(
  double	tt)
{
  transitionTime = tt;
  return;
}

template <class PLD>
double SHR<PLD>::getTransitionTime()
{
  return transitionTime;
}

template <class PLD>
void SHR<PLD>::setBackOff(
  SHRBackOff	bot)
{
  backOff = bot;
  return;
}

template <class PLD>
SHRBackOff SHR<PLD>::getBackOff()
{
  return backOff;
}

template <class PLD>
void SHR<PLD>::setContinuousBackOff(
  bool		flag)
{
  continuousBackOff = flag;
  return;
}

template <class PLD>
bool SHR<PLD>::getContinuousBackOff()
{
  return continuousBackOff;
}

template <class PLD>
void SHR<PLD>::setRouteRepair(
  int	numSteps)
{
  if( numSteps == 0)
    RouteRepairEnabled = false;
  else
  {
    RouteRepairEnabled = true;
    seq_number_t::setCounter( numSteps);
  }
  return;
}

template <class PLD>
SHR<PLD>::SHR()
{
  connect ackTimer.to_component, AckTimer;
  connect pktDelayTimer.to_component, PktDelayTimer;
  connect transitionTimer.to_component, TransitionTimer;
  connect helloTimer.to_component, HelloTimer;
  return;
}

template <class PLD>
SHR<PLD>::~SHR()
{
  return;
}

template <class PLD>
void SHR<PLD>::Start() 
{
  if( doubleMsgPrinted == false)
  {
    printf( "Timeout multiplier: %d\n", MWLTimeoutMultiplier);
#ifdef	MWL_DOUBLE_DELAY
    printf( "Doubling Lambda on retries\n");
#else	// MWL_DOUBLE_DELAY
    printf( "Lambda constant on retries\n");
#endif	// MWL_DOUBLE_DELAY
    doubleMsgPrinted = true;
  }
  m_mac_busy=false;
  m_seq_number = (int) MyEtherAddr << 16;
  m_seq_cache.insert(make_pair(MyEtherAddr,seq_number_t()));

  SentPackets=RecvPackets=RecvUniPackets=0l;
  SentSuboptimal = 0;
  CanceledPackets = 0;
  CanceledSuboptimal = 0;
  TotalDelay=0.0;
  TotalHop=0;
  TotalSamples=0;
  RecvDataPackets=0;
  for( int i = HCArraySize-1; i >= 0; i--)
    HopCounts[i] = 0;
}

template <class PLD>
void SHR<PLD>::clearStats()
{
  SentPackets = 0;
  SentSuboptimal = 0;
  CanceledPackets = 0;
  CanceledSuboptimal = 0;
  RecvPackets = 0;
  RecvUniPackets = 0;
  RecvDataPackets = 0;
  TotalDelay = 0;
  TotalSamples = 0;
  TotalHop = 0;
  for( int i = HCArraySize-1; i >= 0; i--)
    HopCounts[ i] = 0;
  return;
}

template <class PLD>
void SHR<PLD>::Stop() 
{
  //printf("%s: sent %d, recv %d\n",GetName(),SentPackets,RecvPackets);
  //printf("%s: %f \n", GetName(), (double)RecvPackets/RecvUniPackets);
  return;
}

template <class PLD>
void SHR<PLD>::from_transport(
  payload_t	&pld,
  ether_addr_t	&dst,
  unsigned int	size)
{
  if( size > 1510)
    printf( "ft: ea%3d; Oversize packet (%d) to %3d\n", (int) MyEtherAddr,
	    size, (int) dst);
  //create the data packet
  packet_t	*pkt = (packet_t *) createDataPkt( pld, dst, size);

  Printf((DumpPackets, "%d SHR: %d->%d *%s\n", (int) MyEtherAddr,
	  (int) MyEtherAddr, (int) dst, pkt->dump().c_str()));
  shrPrint( ("ft: %3d->%3d *%s @ %f ea %3d\n", (int) MyEtherAddr, (int) dst,
	     pkt->dump().c_str(), SimTime(), (int) MyEtherAddr));

  // look up destination in the cost table
  cache_t::iterator iter = m_seq_cache.find(dst);

  // If destination wasn't found, send a DREQ packet and put the data packet
  // on the queue of packets to be sent.
  if( iter == m_seq_cache.end())
  {
    // the destination is not in the active node table
    // must send a REQUEST packet first;
    packet_t	*rp = (packet_t *) createRequestPkt( dst);

    Printf( (DumpPackets, "%d SHR: %d->%d %s\n", (int) MyEtherAddr,
	    (int) MyEtherAddr, (int) dst, rp->dump().c_str()));
    SendPacket( rp);
    m_data_buffer.push_back( pkt);
  }
  // otherwise, set the expected and maximum hops from the cost table
  else
  {
    // The SSR/SHR specs call for decrementing expected_hop before transmitting.
    // In SENSE, >1 nodes reference same packet in memory, so can't decrement
    pkt->hdr.expected_hop = (*iter).second.hopCount();
    pkt->hdr.max_hop = pkt->hdr.expected_hop + AdditionalHop;
    pkt->hdr.delay = 0;		// send without delay
    SendPacket( pkt);
  }
}

template <class PLD>
void SHR<PLD>::AddToSendQueue(
  packet_t	*pkt)
{
  packet_queue_t::iterator	iter = m_send_queue.begin();

  while( iter != m_send_queue.end() && (*iter)->hdr.delay <= pkt->hdr.delay)
    iter++;
  m_send_queue.insert( iter, pkt);
  return;
}

/*
** Sending a packet from the network (SHR) layer to the MAC layer consists of
** calling four methods in order:
** 1. SendPacket: Packets without a delay are forwarded to AssignSeqNumber.
**    Packets with a delay are set aside until the delay has elapsed. The timer
**    routine, PktDelayTimer, forwards the packet to AssignSeqNumber. Because
**    this method may reorder the packets, sequence numbers should not be
**    assigned yet.
** 2. AssignSeqNumber: The packets can no longer be reordered. If applicable,
**    assign a sequence number to the packet. Either forward the packet to
**    SendToMac (MAC is idle) or put it on the send queue. from_mac_ack will
**    remove packets from the send queue and forward them to SendToMac.
** 3. SendToMac: If the simulation does not include a transition time, forward
**    the packet to to_mac. Otherwise, start the transition timer and let its
**    routine, TransitionTimer, forward the packet to to_mac
** 4. to_mac: The outport that connects the network and MAC layers. This
**    function should always be called with a delay of 0.
*/

/*
** Schedule a packet for transmission.
** If the packet doesn't have a delay, send the packet to the MAC layer.
** Otherwise, add the packet to the list of delayed packets and schedule an
** event for the end of the packet's delay.
*/
template <class PLD>
void SHR<PLD>::SendPacket(
  packet_t	*pkt)
{
  if( pkt->hdr.delay == 0.0)
    AssignSeqNumber( pkt);
  else
  {
    shrPrint( ( "SPT: ea %3d; 0x%08X; sn%08X; %f + %f seconds\n",
	    (int) MyEtherAddr, (int) pkt, pkt->hdr.seq_number, SimTime(),
	    pkt->hdr.delay));
    // add packet to delay list
    m_delay_list.push_back( pkt);
    // schedule event
    pkt->hdr.pktDelayIndex = pktDelayTimer.Set( pkt, SimTime() + pkt->hdr.delay);
  }
  return;
}

/*
** This method is called when the delay for a packet has expired.
** Remove the packet from the list and send it to the MAC layer.
*/
template <class PLD>
void SHR<PLD>::PktDelayTimer(
  packet_t	*pkt,
  unsigned int)
{
  shrPrint( ( "PDT: ea %3d; 0x%08X; sn%08X; %f\n", (int) MyEtherAddr,
	      (unsigned int) pkt, pkt->hdr.seq_number, SimTime()));
  m_delay_list.remove( pkt);
  AssignSeqNumber( pkt);
  return;
}

/*
** High level method to send a packet to the MAC layer.
** Assign a new sequence number if one is needed.
** If the MAC is idle, send the packet. Otherwise, put the packet on the send
** queue.
*/
template <class PLD>
void SHR<PLD>::AssignSeqNumber(
  packet_t	*pkt)
{
  pkt->hdr.canceled = false;
  if( pkt->hdr.newSeqNumber == true && pkt->hdr.src_addr == MyEtherAddr)
  {
    m_seq_number++;
    m_seq_cache[ MyEtherAddr].check( m_seq_number);
    pkt->hdr.seq_number = m_seq_number;
#ifdef VISUAL_ROUTE
    // this printf is used by the compareHC and checkHC utilities
    printf( "SP: sn%08X; max hop %d; %f\n", pkt->hdr.seq_number,
	    pkt->hdr.max_hop, SimTime());
#endif // VISUAL_ROUTE
  }
  pkt->hdr.newSeqNumber = false;
  shrPrint( ("SP: ea %3d; sn%08X; actHC %2d; pathHC %2d; sending %s @ %f\n",
	     (int) MyEtherAddr, pkt->hdr.seq_number, pkt->hdr.actual_hop,
	     pkt->hdr.path.getLength(), pkt->dump().c_str(), SimTime()));
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
    shrPrint( ("SP: ea %3d; sn%08X; mac busy, delaying\n", (int) MyEtherAddr, pkt->hdr.seq_number));
    AddToSendQueue( pkt);
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
void SHR<PLD>::SendToMac(
  packet_t	*pkt)
{
  m_mac_busy = true;
  if( transitionTime != 0)
  {
    // start timer
    transitionTimer.Set( SimTime() + transitionTime);
    // save packet
    transitionPkt = pkt;
  }
  else
    to_mac( pkt, pkt->hdr.size, 0);
  return;
}

/*
** TransitionTimer was added in February 2006.
**
** Timer has expired indicating that the hardware has finished its transition
** from sensing the carrier and is finally starting to transmit the packet.
*/
template <class PLD>
void SHR<PLD>::TransitionTimer(
  trigger_t	&)
{
  // send saved packet to mac
  to_mac( transitionPkt, transitionPkt->hdr.size, 0);
  transitionPkt = 0;
  return;
}

/*
** Find the packet's source address in this node's cache. If the entry is not
** found, create it. If the hop count in the cache is greater than the hop
** count of the packet, update the cache.
**
** Return true if the entry was created or updated, false otherwise.
*/
template <class PLD>
bool SHR<PLD>::updateHopCountInCache(
  ether_addr_t	dst,
  unsigned int	hopCount,
  UpdateType	type,
  const char	*str,
  packet_t	*mwl)
{
  if( dst == MyEtherAddr)
    return false;
  cache_t::iterator	iter;

  iter = m_seq_cache.find( dst); 
  // If the destination address is not found, create a new record.
  if( iter == m_seq_cache.end())
  {
    shrPrint( ("%10s: uHCIC: ea %3d; sn%08X; dst %3d; hc %2d new @ %f\n", str,
	       (int) MyEtherAddr, mwl->hdr.seq_number, (int) dst, hopCount, SimTime()));
    iter = (m_seq_cache.insert( make_pair( dst, seq_number_t()))).first;
    iter->second.hopCount( hopCount, true, false);	// true/false doesn't matter since the entry is new
    return true;
  }

  // if there's no change, return
  if( iter->second.hopCount() == hopCount)
    return false;

  // if the new packet won't update the cost table entry, return false
  if( type != Always && iter->second.hopCount() <= hopCount)
    return false;

  // update the cost table entry
  shrPrint( ("%10s: uHCIC: ea %3d; sn%08X; dst %3d; hc %2d->%2d update @ %f",
	     str, (int) MyEtherAddr, mwl->hdr.seq_number, (int) dst,
	     iter->second.hopCount(), hopCount, SimTime()));
  //mwl remember print in method hopCount
  iter->second.hopCount( hopCount, type == Hello ? true : false, shrDoPrint);
  return true;
}

/*
** Interface that is used by the adjacency matrix channel model. Do not use
** them with the radio model.
*/
template <class PLD>
void SHR<PLD>::HelloTimer(
  trigger_t	&)
{
#ifdef	USE_CONFIG_FILE
//  shrPrint( ("HT: ea %3d; HELLO timer @ %f\n", (int) MyEtherAddr, SimTime()));
  generateHELLO();
#else	// USE_CONFIG_FILE
  printf( "SHR::HelloTimer may only be used with a configuration file\n");
  assert( 0);
#endif	// USE_CONFIG_FILE
  return;
}

template <class PLD>
void SHR<PLD>::scheduleHELLO(
  double	time)
{
#ifdef	USE_CONFIG_FILE
  printf( "schedHELLO: ea %3d @ %f ", (int) MyEtherAddr, time);
  if( backOff == SHRBackOff_SHR && RouteRepairEnabled == true)
  {
    printf( "enabled\n");
    helloTimer.Set( time);
  }
  else
    printf( "ignored\n");
#else	// USE_CONFIG_FILE
  printf( "SHR::scheduleHELLO may only be used with a configuration file\n");
  assert( 0);
#endif	// USE_CONFIG_FILE
  return;
}

/*
** The inport that is called when the power manager wakes the node up.
** If the configuration file is in use, this must be defined, but it must not
** do anything
*/
template <class PLD>
void SHR<PLD>::from_pm_node_up()
{
#ifndef	USE_CONFIG_FILE
  generateHELLO();
#endif	// USE_CONFIG_FILE
  return;
}

/*
** The node has just come back up. Send out a HELLO packet for each entry in
** the Cost Table.
*/
template <class PLD>
void SHR<PLD>::generateHELLO()
{
  cache_t::iterator iter;
  ether_addr_t	dest;

  // convert entire cost table into hop count list
  for( iter = m_seq_cache.begin(); iter != m_seq_cache.end(); iter++)
  {
    packet_t	*pkt;
    dest = iter->first;
    if( dest == MyEtherAddr)
      continue;
    pkt = (packet_t *) createHelloPkt( dest, iter->second.hopCount());
    SendPacket( pkt);	// send with new sequence number
  }
  return;
}

/*
** The purpose of the HELLO packet is to inform the receiving node that sending
** node has come back to life. The HELLO packet contains a list of <addr, dist>
** pairs. For each pair, the node determines if the entry in its cost table
** needs to be updated. If so, update the cost table and add the updated entry
** to a list that will be used to generate a new HELLO packet. If no changes
** are made to the cost table, then no HELLO packet is sent.
*/
template <class PLD>
void SHR<PLD>::receiveHelloPacket(
  packet_t	*pkt)
{
  cache_t::iterator	iter;
  ether_addr_t	destAddr = pkt->hdr.dst_addr;
  int		hopCount = pkt->hdr.helloData;

  shrPrint( ("rHP: ea %3d; from %3d; for %3d; hc %d @ %f\n", (int) MyEtherAddr,
	     (int) pkt->hdr.cur_addr, (int) destAddr, hopCount, SimTime()));

/*
** Assume that the path from this node to the destination must go through the
** node that sent the HELLO packet. Therefore, increment the hop count.
** If this assumption is wrong, then the hop count in the HELLO packet is
** greater than the Cost Table entry.
*/
  hopCount++;

  // if the Cost Table entry is updated, forward the HELLO packet.
  if( updateHopCountInCache( destAddr, hopCount, Hello, "HELLO", pkt) == true)
    SendPacket( (packet_t *) createHelloPkt( destAddr, hopCount));
  return;
}

/*******************************************************************
 * We are now receiving a packet from the mac layer.
 *******************************************************************/

template <class PLD>
void SHR<PLD>::from_mac_data(
  packet_t	*pkt,
  double	power)
{
  ether_addr_t src = pkt->hdr.src_addr;
  bool		costTableUpdated = false;

  RecvPackets++;
  addToNeighborMatrix( (int) pkt->hdr.cur_addr, (int) MyEtherAddr);
  Printf((DumpPackets,"%d SHR: %d %d %s \n", (int) MyEtherAddr,
	  (int) pkt->hdr.cur_addr, (int) MyEtherAddr, pkt->dump().c_str()));
  shrPrint( ("fmd: ea %3d<-%3d sn%08X %s @ %f\n", (int) MyEtherAddr,
	     (int) pkt->hdr.cur_addr, pkt->hdr.seq_number, pkt->dump().c_str(),
	     SimTime()));
    
  if( pkt->hdr.type == HELLO)
  {
    receiveHelloPacket( pkt);
    pkt->free();
    return;
  }

  cache_t::iterator iter;
    
/*
** If the packet is not an ACK, then update the cost table.
**
** The result of updating the cost table is used if the packet is a RREP.
*/
  if( pkt->hdr.type != ACK)
  {
    costTableUpdated = updateHopCountInCache( src, pkt->hdr.actual_hop, Lower,
					      "rRRDP", pkt);
    iter = m_seq_cache.find( src); 
  }

/*
** The method check (defined at the top of this file) determines if this
** sequence number is greater than the sequence number in the cache. If the
** packet's sequence number is greater, then this packet is new. This
** comparison is not valid for resent packets.
*/
  if( pkt->hdr.type != ACK && (!iter->second.check( pkt->hdr.seq_number) ||
			       pkt->hdr.resend))
  {
    // the received packet is new
    RecvUniPackets++;
    if( pkt->hdr.dst_addr == MyEtherAddr)
    {
      if( pkt->hdr.resend == 0)
      {
	switch( pkt->hdr.type)
	{
	  // this case can't happen, but it makes the compiler happy
	  case HELLO:
	  case ACK:
	    break;
	  case REQUEST:
	  {
	    packet_t	*rp = (packet_t *) createReplyPkt( pkt->hdr.src_addr,
							   iter->second.hopCount());

	    Printf( (DumpPackets, "%d SHR: %d->%d %s\n", (int) MyEtherAddr,
		     (int) MyEtherAddr, (int) pkt->hdr.src_addr, rp->dump().c_str()));
	    shrPrint( ("rPAD: ea %d creates REPLY %s @ %f\n", (int) MyEtherAddr,
		       rp->dump().c_str(), SimTime()));

	    // Send reply and check if any packets need to be sent to the src
	    // of the request
	    SendPacket( rp);
	    CheckBuffer( pkt->hdr.src_addr, iter->second.hopCount());
	  }
	  break;
	  case REPLY:
	    Printf((DumpPackets,"%d SHR: %d<-%d %s\n", (int)MyEtherAddr,
		    (int)MyEtherAddr, (int)pkt->hdr.src_addr,
		    pkt->dump().c_str()));
	    // Packet arrives at destination, Ack after AckWindow
	    SendAck( pkt->hdr.src_addr, MyEtherAddr, pkt->hdr.cur_addr,
		     pkt->hdr.seq_number, AckWindow, true);
	    CheckBuffer( pkt->hdr.src_addr, iter->second.hopCount());
	    break;
	  case DATA:
	    // the packets arrives at its destination
	    // Packet arrives at destination, Ack after AckWindow
	    SendAck( pkt->hdr.src_addr, MyEtherAddr, pkt->hdr.cur_addr,
		     pkt->hdr.seq_number, AckWindow, true);
	    pkt->inc_pld_ref();
	    to_transport(pkt->pld);
	    {
	      int	hc = pkt->hdr.actual_hop;
	      TotalHop += hc;
	      if( hc > HCArraySize-1)
		hc = HCArraySize-1;
	      HopCounts[hc]++;
	    }
	    RecvDataPackets++;

	    pkt->hdr.path.AddNode(MyEtherAddr);
#ifdef VISUAL_ROUTE
    // this printf is used by the compareHC and checkHC utilities
	    printf( "VR: %f; %3d->%3d; sn%08X; %f; %d; %s\n", SimTime(),
		    (int) pkt->hdr.src_addr, (int) pkt->hdr.dst_addr,
		    pkt->hdr.seq_number, (SimTime() - pkt->hdr.send_time),
		    pkt->hdr.actual_hop, pkt->hdr.path.ToString());
#endif //VISUAL_ROUTE
	    visualizer->writePath( (int) pkt->hdr.src_addr,
				   (int) pkt->hdr.dst_addr,pkt->hdr.seq_number,
				   pkt->hdr.path, true);
	    break;
	}
      }
    }
    else  
    {
/*
** If the hop count in the cost table is less than the maximum allowed by the
** received packet, then forward the packet.
*/
      if( pkt->hdr.actual_hop < pkt->hdr.max_hop)
      {
	packet_t* np = pkt->copy();		// create copy of packet

	np->hdr.cur_addr = MyEtherAddr;
	np->hdr.pre_addr = pkt->hdr.cur_addr;
	np->hdr.resend = 0;
	np->hdr.actual_hop++;
	np->hdr.suboptimal = false;
	np->hdr.path.AddNode( MyEtherAddr);

/*
** How the packet is forwarded depends on the packet type.
**
** Data packet require an election. If the destination isn't found in the cost
** table, don't participate in the election. Otherwise, the delay is dependent
** on the value of the cost table entry relative to the expected hop of the
** received packet.
*/
	if( np->hdr.type == DATA)
	{
	  iter = m_seq_cache.find( np->hdr.dst_addr);
	  if( iter != m_seq_cache.end())
	  {
	    // If the packet is on the ACK list, remove it.
	    // This happens when the retransmission is lost and the upstream
	    // node resends the packet.
	    {
	      packet_t	*pktFromList = (packet_t *) getFromAckList( pkt);
	      if( pktFromList != NULL)
		removeFromAckList( pktFromList, true);
	    }
	    np->hdr.expected_hop = (*iter).second.hopCount();
	    double	hop, backOffTime;

	    switch( backOff)
	    {
/*
** Calculate the amount of time to wait before responding.
** The form of this equation is different than that of the paper. In the paper
** the sender decrements the expected hop count before putting the value into
** the packet. In the simulation, the hop count is not decremented.
*/
	      case SHRBackOff_SSR:
	      case SHRBackOff_Incorrect:
		shrPrint( ("FP: ea %3d; cost table %2d; pkt %2d\n", (int) MyEtherAddr, iter->second.hopCount(), pkt->hdr.expected_hop));
		if( iter->second.hopCount() < pkt->hdr.expected_hop)
		  hop = Random( 1.0/(pkt->hdr.expected_hop - iter->second.hopCount()));
		else
		{
		  hop = Random( iter->second.hopCount() - pkt->hdr.expected_hop +1.0);
		  if( backOff == SHRBackOff_SSR)
		    hop += 1.0;
		  SentSuboptimal++;
		  np->hdr.suboptimal = true;
		}
		break;

/*
** Implement the fix that Boleslaw proposed on 3/21/2006.
** If h_table <= h_expected, then
**   backoff = lambda * U(0,1)/(h_e - h_t + 1)
** else
**   backoff = lambda * (h_t - h_e + U(0,1))
*/
	      case SHRBackOff_SHR:
		if( iter->second.hopCount() < pkt->hdr.expected_hop)
		  hop = Random(1.0)/(pkt->hdr.expected_hop - iter->second.hopCount());
		else
		{
		  hop = iter->second.hopCount() - pkt->hdr.expected_hop + 1.0 +
		      Random( 1.0);
		  SentSuboptimal++;
		  np->hdr.suboptimal = true;
		}
		break;
	      default:
		hop = 1;	// shouldn't happen, but quiets gcc
		assert( 0);
		break;
	    }
	    backOffTime = hop * ForwardDelay
#ifdef	MWL_DOUBLE_DELAY
		* (1 << pkt->hdr.resend)
#endif	// MWL_DOUBLE_DELAY
		;

	    if( continuousBackOff == false)	// convert lambda to slot
	    {
	      int	slot;
		
	      slot = (int) (backOffTime / slotWidth);
	      backOffTime = slot * slotWidth + Random( transitionTime);
	    }
	    // Don't forward packets until after the AckWindow
	    np->hdr.delay = backOffTime + AckWindow;
	    shrPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; pkt %2d; table %2d; backoff %f\n",
		       (int) MyEtherAddr, (int) pkt->hdr.cur_addr, (int) pkt,
		       pkt->hdr.seq_number, pkt->hdr.expected_hop-1, np->hdr.expected_hop,
		       backOffTime));
	    SendPacket( np);
	  }
	  else
	  {
	    shrPrint( ("FP: ea %3d; from %3d; 0x%08X; sn%08X; not in cache\n",
		       (int) MyEtherAddr, (int) pkt->hdr.cur_addr,
		       (int) pkt, pkt->hdr.seq_number));
	    np->free();
	  }
	}
/*
** Request packets are flooded: No election, forward after a random delay
*/
	else if( np->hdr.type == REQUEST)
	  forwardAsFlood( np);
/*
** Reply packets are conditionally flooded.
**
** RREP (Z -> Y) from X, where Y generated the RREQ for Z and X has forwarded
**      the packet to this node.
** If (Y is not in the cost table)
**   then the packet is dropped.
** else		(Y is in the cost table)
**   If (this packet updated Z's entry in the cost table)
**     forward packet w/o election
**   else
**     the packet is dropped.
*/
	else
	{
	  iter = m_seq_cache.find( np->hdr.dst_addr);
	  if( iter == m_seq_cache.end())
	    np->free();
	  else
	  {
	    if( costTableUpdated == true)
	      forwardAsFlood( np);
	    else
	      np->free();
	  }
	}
      }
      else
      {
/*
** TTL exceeded, dropping packet;
** Create a copy of the packet solely for the purpose of recording the full
** path. This is because pkt is used by other nodes.
*/
	packet_t	*np = pkt->copy();

	np->hdr.path.AddNode( MyEtherAddr);
	visualizer->writePath( (int) np->hdr.src_addr, (int) np->hdr.dst_addr,
			       np->hdr.seq_number, np->hdr.path, false);
	np->free();
      }
    }
  }
  else	// packet should be on ack queue
  {
    if( pkt->hdr.type == ACK)
      receiveAck( pkt);
    else
      receiveDataRep( pkt);
  }
  pkt->free();
  return;
}

template <class PLD>
void SHR<PLD>::forwardAsFlood(
  packet_t	*pkt)
{
  // AckWindow doesn't apply when flooding a packet.
  pkt->hdr.delay = Random( ForwardDelay);
  TotalDelay += pkt->hdr.delay;
  TotalSamples++;
  SendPacket( pkt);
  return;
}

/*
** The m_ack_list contains a list of packets that have already been sent and
** are waiting for acknowledgement.
**
** The processing for reception of an ACK is:
** - if the ACK was initiated by a packet sent by another node (pre_addr != self)
**   - cancel PKT
**   - remove PKT from ack queue
** - else (this node generated packet that initiated ACK)
**   - if the ACK is from the destination (cur_addr == dest_addr)
**     - get packet from ack queue
**     - Send an ACK. This is the same processing as when node detects that
**       a packet has been forwarded.
**     - remove PKT from ack queue
**   - else (ACK is from an intermediate node)
**     - ignore the ACK
*/
template <class PLD>
void SHR<PLD>::receiveAck(
  packet_t	*ackPkt)
{
  shrPrint( ("rA: ea %3d; 0x%08X; sn%08X; src %3d; cur %3d; pre %3d @ %f\n",
	     (int) MyEtherAddr, (int) ackPkt, ackPkt->hdr.seq_number,
	     (int) ackPkt->hdr.src_addr, (int) ackPkt->hdr.cur_addr,
	     (int) ackPkt->hdr.pre_addr, SimTime()));
#ifndef	SHR_ACK
  CancelPacket( ackPkt->hdr.src_addr, ackPkt->hdr.seq_number);
#else	// SHR_ACK
  packet_t	*pktFromList;

  // If the ack is in response to a packet generated by another node
  if( ackPkt->hdr.pre_addr != MyEtherAddr)
  {
    shrPrint( ("rA: ea %3d; sn%08X; Canceling packet\n", (int) MyEtherAddr, ackPkt->hdr.seq_number));
    CancelPacket( ackPkt->hdr.src_addr, ackPkt->hdr.seq_number);
    pktFromList = (packet_t *) getFromAckList( ackPkt);
    if( pktFromList != NULL)
      removeFromAckList( pktFromList, true);
  }
  else		// Ack is in response to packet generated by this node.
  {
    pktFromList = (packet_t *) getFromAckList( ackPkt);
    if( pktFromList != NULL)
    {
      // Respond to ack, but only if it's from the destination
      if( ackPkt->hdr.ackType == AT_Destination)
      {
	shrPrint( ("rA: ea %3d; sn%08X; Sending ack\n", (int) MyEtherAddr, ackPkt->hdr.seq_number));
	doRouteRepair( pktFromList, ackPkt);
	SendAck( ackPkt->hdr.src_addr, MyEtherAddr, ackPkt->hdr.cur_addr,
		 ackPkt->hdr.seq_number, 0.0, false);
	removeFromAckList( pktFromList, true);
      }
    }
  }
#endif	// SHR_ACK
  return;
}

template <class PLD>
void SHR<PLD>::receiveDataRep(
  packet_t	*rcvdPkt)
{
  packet_queue_t::iterator iter;
  packet_t	*pktFromList;

  shrPrint( ("rDR: ea %3d; 0x%08X; sn%08X; src %3d; cur %3d; pre %3d @ %f\n",
	     (int) MyEtherAddr, (int) rcvdPkt, rcvdPkt->hdr.seq_number,
	     (int) rcvdPkt->hdr.src_addr, (int) rcvdPkt->hdr.cur_addr,
	     (int) rcvdPkt->hdr.pre_addr, SimTime()));

  CancelPacket( rcvdPkt->hdr.src_addr, rcvdPkt->hdr.seq_number);
  // m_ack_list is for storing packets that need to be ACKed
  // In theory, an ACK with a previous address of this node should be on the
  // ack list, but it's possible for things to get out of sync.
  pktFromList = (packet_t *) getFromAckList( rcvdPkt);

  // If packet wasn't found, there's nothing to do.
  if( pktFromList == NULL)
    return;

  if( rcvdPkt->hdr.pre_addr == MyEtherAddr)
  {
    doRouteRepair( pktFromList, rcvdPkt);
    SendAck( rcvdPkt->hdr.src_addr, MyEtherAddr, rcvdPkt->hdr.cur_addr,
	     rcvdPkt->hdr.seq_number, 0.0, false);
    removeFromAckList( pktFromList, true);
  }
  else		// forwarding of packet from another node
  {
    shrPrint( ("rDR: ea %3d; 0x%08X; sn%08X; resend <- %d; iter %3d<-%3d; "
	       "pkt %3d<-%3d\n", (int) MyEtherAddr, (unsigned int) pktFromList,
	       pktFromList->hdr.seq_number, MaxResend,
	       (int) pktFromList->hdr.cur_addr, (int)pktFromList->hdr.pre_addr,
	       (int) rcvdPkt->hdr.cur_addr, (int) rcvdPkt->hdr.pre_addr));
    pktFromList->hdr.resend = MaxResend;
  }
  return;
}

template <class PLD>
void *SHR<PLD>::getFromAckList(
  packet_t	*pkt)
{
  packet_queue_t::iterator iter;

  // m_ack_list is for storing packets that need to be ACKed
  for( iter = m_ack_list.begin(); iter != m_ack_list.end(); iter++)
    if( (*iter)->hdr.src_addr == pkt->hdr.src_addr &&
	(*iter)->hdr.seq_number == pkt->hdr.seq_number)
      break;

  if( iter != m_ack_list.end())
    return *iter;
  return NULL;
}

template <class PLD>
void SHR<PLD>::removeFromAckList(
  packet_t	*pkt,
  bool		cancelTimer)
{
  shrPrint( ("rFAL: ea %3d; sn%08X\n", (int) MyEtherAddr, pkt->hdr.seq_number));
  if( cancelTimer == true)
  {
    ackTimer.Cancel( pkt->hdr.ackIndex);
    pkt->free();
  }
  m_ack_list.remove( pkt);
  return;
}

template <class PLD>
void SHR<PLD>::doRouteRepair(
  packet_t	*pktFromList,
  packet_t	*rcvdPkt)
{
  if( RouteRepairEnabled == false ||
      rcvdPkt->hdr.expected_hop == pktFromList->hdr.expected_hop - 1)
    return;
  
  const char	*str;
  switch( rcvdPkt->hdr.type)
  {
    case ACK:
      str = "dAS-ack";
      break;
    case DATA:
      str = "dAS-data";
      break;
    case REPLY:
      str = "dAS-reply";
      break;
    default:
      str = "dAS-other";
      break;
  }
  updateHopCountInCache( pktFromList->hdr.dst_addr, rcvdPkt->hdr.expected_hop
			 + 1, Always, str, rcvdPkt);
  return;
}

template <class PLD>
int SHR<PLD>::getHCFromCostTable(
  int		addr)
{
  cache_t::iterator	iter;
  ether_addr_t		ea = addr;

  iter = m_seq_cache.find( ea);
  return (int) iter->second.hopCount();
}

/*
** Receive notification of receive-receive collisions. This must be defined
** since the cxx compiler doesn't support #ifdef.
*/
template <class PLD>
void SHR<PLD>::from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2)
{
//  addToNeighborMatrix( (int) pkt1->hdr.pre_addr, (int) MyEtherAddr);
//  addToNeighborMatrix( (int) pkt2->hdr.pre_addr, (int) MyEtherAddr);
  shrPrint( ("fmrrc: ea %3d; sn%08X (%3d) & sn%08X (%3d) @ %f\n",
	     (int) MyEtherAddr, pkt1->hdr.seq_number, (int) pkt1->hdr.cur_addr,
	     pkt2->hdr.seq_number, (int) pkt2->hdr.cur_addr, SimTime()));
  return;
}

template <class PLD>
void SHR<PLD>::CancelPacket(
  ether_addr_t	&src,
  unsigned int	seq_number)
{
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
//    shrPrint( ("CP: ea %3d; in progress 0x%08X; sn%08X\n", (int) MyEtherAddr, (unsigned int) m_active_packet, m_active_packet->hdr.seq_number));
    //Printf((DumpPackets,"rr%d cancels the current packet\n",(int)MyEtherAddr));
    CanceledPackets++;
    if( m_active_packet->hdr.suboptimal == true)
      CanceledSuboptimal++;
    m_active_packet->hdr.canceled = true;
    cancel();
    return;
  }

  packet_queue_t::iterator iter;
  packet_t	*pktFromQ;

  // Now, look for the packet in the backoff list
  // Packets sourced from this node don't have a sequence number, but those
  // packets won't be cancelled.
  for( iter = m_delay_list.begin(); iter != m_delay_list.end(); iter++)
    if( (*iter)->hdr.src_addr == src && (*iter)->hdr.seq_number == seq_number)
      break;

  if( iter != m_delay_list.end())
  {
    shrPrint( ("CP: ea %3d; 0x%08X; sn%08X; %f\n", (int) MyEtherAddr,
	    (unsigned int) *iter, (*iter)->hdr.seq_number, SimTime()));
    pktDelayTimer.Cancel( (*iter)->hdr.pktDelayIndex);
    (*iter)->free();
    m_delay_list.erase( iter);
    return;
  }

  // Now, look for the packet in m_send_queue.
  for( iter = m_send_queue.begin(); iter != m_send_queue.end(); iter++)
    if( (*iter)->hdr.src_addr == src && (*iter)->hdr.seq_number == seq_number)
      break;

  // If not found, then nothing to do.
  if( iter == m_send_queue.end())
    return;

  pktFromQ = *iter;
  // If the pending packet is an ACK, don't cancel it.
  if( pktFromQ->hdr.type != ACK)
  {
    CanceledPackets++;
    if( pktFromQ->hdr.suboptimal == true)
      CanceledSuboptimal++;
    pktFromQ->free();
    m_send_queue.erase( iter);
  }
  return;
}

/*
** The routing layer has received an acknowledgement from the mac layer.
** False indicates that the transmission failed.
*/
template <class PLD>
void SHR<PLD>::from_mac_ack(
  bool		flag)
{
  bool		testQueue = true;
  bool		slotted = continuousBackOff == false &&
      (MyEtherAddr != m_active_packet->hdr.src_addr) ? true : false;

  //Printf((DumpPackets,"rr%d receives an ack from mac\n",(int)MyEtherAddr));

  // a non-ack packet failed, so resend it
  /*
  ** Continuous and slotted SSR handle resending differently.
  ** 
  ** Continuous: Just send the packet back to the MAC layer.
  ** 
  ** Slotted: Intermediate nodes delay the packet until the start of the next
  ** slot. Source nodes behave as continuous SSR.
  */
  if( flag == false && m_active_packet->hdr.type != ACK)
  {
    //Printf((DumpPackets,"%d resends %s\n", (int)MyEtherAddr,m_active_packet->dump().c_str()));
    if( slotted == true)
    {
      m_active_packet->hdr.delay = slotWidth;
      SendPacket( m_active_packet);
    }
    else
    {
//      printf( "fma: rescheduling\n");
      m_active_packet->hdr.delay = 0;
      m_active_packet->inc_ref();
      SendToMac( m_active_packet);
      testQueue = false;	// just sent to mac, so don't test queue
    }
  }
  else
  {
#ifdef SHR_ACK
// Because RREP packets are now flooded, they no longer put onto the ack queue.
    if( m_active_packet->hdr.canceled == false &&
	m_active_packet->hdr.type == DATA &&
	m_active_packet->hdr.actual_hop < m_active_packet->hdr.max_hop-1)
    {
      m_active_packet->inc_ref();
      m_active_packet->hdr.ackIndex =
	  ackTimer.Set( m_active_packet,
			 SimTime() + MWLTimeoutMultiplier *
#ifdef	MWL_DOUBLE_DELAY
			 (1<<m_active_packet->hdr.resend) *
#endif	// MWL_DOUBLE_DELAY
			 ForwardDelay);
      m_ack_list.push_back( m_active_packet);
    }
#endif //SHR_ACK
    m_active_packet->free();
  }
  if( testQueue == true)
    if( m_send_queue.size())
    {
      packet_t	*packet = m_send_queue.front();
      m_send_queue.pop_front();
      shrPrint( ("fma: ea %3d; sn%08X; sending from queue\n", (int)MyEtherAddr,
		 packet->hdr.seq_number));
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
void SHR<PLD>::AckTimer(
  packet_t	*pkt,
  unsigned int)
{
  shrPrint( ("AT: ea %3d; 0x%08X; sn%08X; resend %d\n", (int) MyEtherAddr,
	     (unsigned int) pkt, pkt->hdr.seq_number, pkt->hdr.resend));
  removeFromAckList( pkt, false);
  // Need to resend the packet, but do it without any delay
  pkt->hdr.delay = 0;
  if( pkt->hdr.resend < MaxResend)
  {
    //Printf((DumpPackets,"rr%d resends %s\n",(int)MyEtherAddr,pkt->dump().c_str()));
    pkt->hdr.resend++;
    SendPacket( pkt);
  }
}

template <class PLD>
void SHR<PLD>::SendAck(
  ether_addr_t	src_addr,
  ether_addr_t	cur_addr,
  ether_addr_t	pre_addr,
  unsigned int	seq_number,
  simtime_t	delay,
  bool		fromDest)
{
  packet_t	*pkt = (packet_t *) createAckPkt( src_addr, cur_addr, pre_addr,
						  seq_number, delay, fromDest);

  Printf( (DumpPackets, "%d SHR: %d->%d %s\n", (int) MyEtherAddr, (int)cur_addr,
	  (int) pre_addr, pkt->dump().c_str()));
  shrPrint( ("SA: sn%08X; %3d->%3d %s @ %f\n", seq_number, (int) cur_addr, (int) pre_addr,
	     pkt->dump().c_str(), SimTime()));
  SendPacket( pkt);
  return;
}

template <class PLD>
void SHR<PLD>::CheckBuffer(
  ether_addr_t	src_addr,
  unsigned int	expected_hop)
{
  packet_queue_t::iterator db_iter;
  db_iter = m_data_buffer.begin();
  while(db_iter != m_data_buffer.end())
  {
    if( (*db_iter)->hdr.dst_addr == src_addr)
    {
      packet_queue_t::iterator t=db_iter;
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
}

template <class PLD>
bool SHR_Struct<PLD>::hdr_struct::dump(
  std::string	&str) const 
{ 
  static char	pkt_type[5][10] = { "REQ", "REPLY", "DATA", "ACK", "HELLO"};
  char		buffer[30];

  sprintf( buffer, "%s %d->%d %d %d", pkt_type[type], (int) src_addr,
	   (int) dst_addr, seq_number, size); 
  str = buffer;
  return type == DATA;
}

#endif /*_shr_h_*/
