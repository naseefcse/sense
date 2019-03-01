/*******************************************************************
 * @<title> Wireless Sensor Network Simulation </title>@
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
 * @<h1> Wireless Sensor Network Simulation </h1>@
 *
 * Building a wireless sensor network simulation in SENSE consists of
 * the following steps:
 * @<UL>@
 * @<LI>@ Designing a sensor node component
 * @<LI>@ Constructing a sensor network derived from @CostSimEng@
 * @<LI>@ Configuring the system and running the simulation
 * @</UL>@
 * 
 * Here, we assume that all components needed by a sensor node component
 * are available from the component repository. If this is not the case, the user
 * must develop new components, as described by other @<a href=tutorial.html>tutorials</a>@. We should
 * also mention that the first step of designing a sensor node component
 * is not always necessary, if a standard sensor node is to be used.
 *
 * This first line of this source file demands that @HeapQueue@ must be
 * used as the priority queue for event management.  For wireless network
 * simulation, because of the inherent drawback of @CalendarQueue@, and also
 * because of the particular channel component being used, @HeapQueue@
 * is often faster. 
 *******************************************************************/

/*******************************************************************
 * For layer XXX, XXX_Struct is the accompanying class that defines
 * data structures and types used in that layer.  The reason we need a separate
 * class for this purpose is that each XXX is a component, and that due to
 * the particular way in which the CompC++ compiler was implemented
 * data structures and types defined inside any component is not accessible from
 * outside.  Therefore, for each layer XXX, we must define all those
 * data structures and types in XXX_Struct, and then derive component XXX 
 * from XXX_Struct.
 *
 * The following three lines state:
 * @<UL>@
 * @<LI>@ The type of packets in the application layer is @CBR_Struct::packet_t@
 * @<LI>@ The network layer passes application layer packets by reference (which may
 * be faster than by pointer, for @CBR_Struct::packet_t@ is small, so 
 *  @app_packet_t@ becomes the template parameter of @net_struct@; the type of packets
 * in the network layer is then @net_packet_t@.
 * @<LI>@ Now that @net_packet_t@ is more than a dozen bytes long, so it is better
 * to pass it by pointer, so @net_packet_t*@ instead of @net_packet_t@ becomes the
 * template parameter of the @MAC80211_Struct@; the type of packets in the mac layer
 * is then @mac_packet_t@.  Physical layers also use @mac_packet_t@, so there is no
 * need to define more packet types.
 * @</UL>@
 *
 *******************************************************************/

typedef CBR_Struct::packet_t app_packet_t;
typedef SHR_Struct<app_packet_t>::packet_t net_packet_t;
typedef BcastMAC_Struct<net_packet_t*>::packet_t mac_packet_t;

const float	FinishTimeRatio = 0.95;

/*******************************************************************
 * Now we can begin to define the sensor node component. First we
 * instantiate every subcomponent used by the node component. We need to
 * determine the template parameter type for each subcomponent, usually starting from 
 * the application layer. Normally the application layer component does not have any
 * template parameter. 
 *
 * This picture shows the internal structure of a sensor node.
 *
 * @<center><img src=sim_flooding_node.gif></center>@
 *******************************************************************/


component SensorNode : public TypeII
{
 public:
  CBR		app;
  SHR <app_packet_t>		net;
  BcastMAC <net_packet_t*>	mac;
  // A transceiver that can transmit and receive at the same time (of course
  // a collision would occur in such cases)
  TransceiverType < mac_packet_t >	phy;
  // Linear battery
  SimpleBattery	battery;
  // PowerManagers manage the battery
  PowerManager	pm;
  // sensor nodes are immobile
  ImmobileType	mob;
    
  double	MaxX, MaxY;  // coordinate boundaries
  ether_addr_t	MyEtherAddr; // the ethernet address of this node
  int		ID; // the identifier
  bool		SrcOrDst;

  virtual ~SensorNode();
  void		Start();
  void		Stop();
  void		Setup( double txPower, double lambda);

/*******************************************************************
 * The following lines define one inport and two outports to be connected
 * to the channel components.
 *******************************************************************/

  outport void to_channel_packet(mac_packet_t* packet, double power, int id);
  inport void from_channel (mac_packet_t* packet, double power);
  outport void to_channel_pos(coordinate_t& pos, int id);
};

SensorNode::~SensorNode()
{
}

void SensorNode::Start()
{
}

void SensorNode::Stop()
{
}

/*******************************************************************
 * This function must be called before running the simulation.
 *******************************************************************/
void SensorNode::Setup(
  double	txPower,
  double	lambda)
{
/*******************************************************************
 * At the beginning the amount of energy in each battery is 1,000,000 Joules.
 *******************************************************************/

  battery.InitialEnergy=1e6;
    
/*******************************************************************
 * Each subcomponent must als know the ethernet address of the 
 * sensor node it resides.
 * Remember the application layer is a CBR component, which would stop
 * at FinishTime to give the whole network an opportunity to clean up
 * any packets in transit.
 * Assiging @false@ to @app.DumpPackets@ means that if @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@
 * is defined, @app@ still won't print out anything.
 *******************************************************************/
    
  app.MyEtherAddr=MyEtherAddr;
  app.FinishTime=StopTime() * FinishTimeRatio;
  app.DumpPackets=false;
    
/*******************************************************************
 * Set the coordinate of the sensor node.  Must also give @ID@ to @mob@
 * since @ID@ was used to identify the index of the sensor node when
 * the position info is sent to the channel component.
 *******************************************************************/

  mob.setX( Random( MaxX));
  mob.setY( Random( MaxY));
  mob.setID( ID);

/*******************************************************************
 * When a net component is about to retransmit a packet that it received,
 * it cannot do so because otherwise all nodes that received the packet
 * may attempt to retransmit the packet immediately, inevitably resulting
 * in a collision.  @ForwardDelay@ gives the maximum delay time a 
 * needed-to-be-retransmit packet may incur.  The actual delay is randomly
 * chosen between [0,@ForwardDelay@].
 *******************************************************************/

  net.MyEtherAddr = MyEtherAddr;
  net.ForwardDelay = lambda;
  net.AckWindow = 0.0015;
  net.DumpPackets = true;
  net.MaxResend = 3;
  net.TimeToLive = 15;
  net.AdditionalHop = 4;
    
/*******************************************************************
 * If @Promiscuity@ is ture, then the mac component will forward every packet
 * even if it not destined to this sensor node, to the network layer.
 * And we want to debug the mac layer, so we set @mac.DumpPackets@ to true.
 *******************************************************************/
 
  mac.MyEtherAddr=MyEtherAddr;
  mac.Promiscuity=true;
  mac.DumpPackets=false;
  mac.IFS=0.01;
   
/*******************************************************************
 *  The PowerManager takes care of power consumption at different states.
 *  The following lines state the power consumption is 1.6W at transmission
 *  state, 1.2 at receive state, and 1.115 at idle state.
 *******************************************************************/

  pm.TXPower=1.6;
  pm.RXPower=1.2;
  pm.IdlePower=1.15;

/*******************************************************************
 *  @phy.TxPower@ is the transmission power of the antenna.
 *  @phy.RXThresh@ is the lower bound on the receive power of any packet
 *  that can be successfuly received.
 *  @phy.CSThresh@ is the lower bound on tye receive power of any packet
 *  that can be detected.
 *  @phy@ also needs to know the id because it needs to communicate with
 *  the channel component.
 *******************************************************************/

  phy.setTXPower( txPower);
  phy.setTXGain( 1.0);
  phy.setRXGain( 1.0);
  phy.setFrequency( 9.14e8);
  phy.setRXThresh( 3.652e-10);
  phy.setCSThresh( 1.559e-11);
  phy.setID( ID);

  net.RXThresh = phy.getRXThresh();
/*******************************************************************
 *  Now we can establish the connections between components.  The connections
 *  will become much clearer if we look at the diagram.
 *******************************************************************/


  connect app.to_transport, net.from_transport;
  connect net.to_transport, app.from_transport;
    
  connect mac.to_network_data, net.from_mac_data;
  connect mac.to_network_ack, net.from_mac_ack;
  connect net.to_mac, mac.from_network;
  connect net.cancel, mac.cancel;
  connect mac.to_network_recv_recv_coll, net.from_mac_recv_recv_coll;
    
  connect mac.to_phy, phy.from_mac;
  connect phy.to_mac, mac.from_phy;

  connect phy.to_power_switch, pm.switch_state;
  connect pm.to_battery_query, battery.query_in;
  connect pm.to_battery_power, battery.power_in;

  connect pm.from_pm_node_up, net.from_pm_node_up;

/*******************************************************************
 *  These three connect statements are different.  All above ones
 *  are between an outport of a subcomponent and an outport of another
 *  subcomponent, while these three are between a port of the sensor
 *  node and a port of a subcomponent.  We can view these connections
 *  as mapping from the ports of subcomponents to its own ports, i.e.,
 *  to expose the ports of internal components.
 *  Also remember the connect statement is so designed that it can take
 *  only two ports, and that packets always flow through from the first port
 *  to the second port, so when connection two inports, the inport of
 *  the subcomponent must be placed in the second place.
 *******************************************************************/

  connect phy.to_channel, to_channel_packet;
  connect mob.pos_out, to_channel_pos;
  connect from_channel, phy.from_channel;

  SrcOrDst=false;
  return;
}

/*******************************************************************
 *  Once we have the sensor node component ready, we can start to 
 *  build the entire simulation, which is named @RoutingSim@.  It must
 *  be derived from the simulation engine class @CostSimEng@.
 *  This is the structure of the network.
 *
 * @<center><img src=sim_flooding_net.gif></center>@
 *******************************************************************/

component RoutingSim : public CostSimEng
{
 public:
  void		Start();
  void		Stop();
  void		ClearStats();
		RoutingSim();
  void		GenerateHELLO( int id);

/*******************************************************************
 *  These are simulation parameters.  We don't want configurators of
 *  the simulation to access the parameters of those inter-components.
 *******************************************************************/

  double	MaxX, MaxY;
  int		NumNodes;
  int		NumSourceNodes;
  int		NumConnections;
  int		PacketSize;
  int		RouteRepair;	// # steps in changing hop count, see seq_number_t
  double	Interval;
  double	ActiveCycle;
  double	ActivePercent;
  SHRBackOff	BackOff;
  bool		ContinuousBackOff;
  double	ForwardDelay;	// aka lambda
  double	SlotWidth;
  double	TransitionTime;
  double	TXPower;
  bool		OneSink;
  bool		UnidirectionalTraffic;

/*******************************************************************
 *  Here we declare sense nodes as an array of @SensorNode@, and a
 *  channel component.
 *******************************************************************/

  SensorNode[]	nodes;
  ChannelType < mac_packet_t > channel;

  void		Setup();
 private:
  void		packetTimes( int source, int dest);
};

/*
** It would be nice to make these part of RoutingSim, but that doesn't seem
** possible. Compcxx seems to require that components be declared in either of
** two ways:
** 1) As a member of another component.
** 2) On the stack.
** The inability to have a pointer to a component is a major annoyance. It is
** also not possible to create a wrapper class since a class isn't a component.
** 
** The neighbor matrix needs to be accessed by code that doesn't have
** an instance of RoutingSim. But, RoutingSim is a component and the CXX
** preprocessor doesn't allow global pointers of type RoutingSim. Neither does
** it allow doing something like "RoutingSim::method(...)". Therefore, I am
** making these global variables.
*/
bool		dumpStatus;
const char	*dumpFile;
#ifdef	USE_CONFIG_FILE
bool		configStatus;
const char	*configFile;
#endif	// USE_CONFIG_FILE
bool		*neighborMatrix;
int		numNodes;

RoutingSim::RoutingSim()
{
  NumConnections = 1;
  PacketSize = 1000;
  StopTime( 1000);
  numNodes = NumNodes = 110;
  MaxX = MaxY = 2000;
  NumSourceNodes = 0;
  ActivePercent = 1.0;		// 1.0 == 100%
  ActiveCycle = 100.0;		// in seconds
  Interval = 20.0;
  BackOff = SHRBackOff_SSR;
  ContinuousBackOff = true;
  OneSink = false;
  UnidirectionalTraffic = false;
  SlotWidth = 0.001;
  TransitionTime = 0.0;
  TXPower = 0.0280;
  RouteRepair = 0;
  dumpStatus = false;
#ifdef	USE_CONFIG_FILE
  configStatus = false;
#endif	// USE_CONFIG_FILE
  neighborMatrix = NULL;
  ForwardDelay = 0.1;
  return;
}

void RoutingSim::Start()
{
  return;
}

void RoutingSim::ClearStats()
{
  int		i;

  for( i = 0; i < NumNodes; i++)
  {
    nodes[ i].app.clearStats();
    nodes[ i].net.clearStats();
    nodes[ i].mac.clearStats();
    nodes[ i].battery.clearStats();
  }
  return;
}

void RoutingSim::GenerateHELLO(
  int		id)
{
  nodes[ id].net.generateHELLO();
  return;
}

/*******************************************************************
 *  After the simulation is stopped, we will collect some statistics.
 *******************************************************************/

void RoutingSim::Stop()
{
  int		i, j, sent, recv, samples, recv_data, hopCounts[ HCArraySize];
  int		sentSuboptimal, canceledSuboptimal, canceledPkts;
  double	hop;
  double	app_recv;
  double	delay;
  int		numCollisions;

  if( dumpStatus == true)
  {
    FILE	*fp;
    int		len = strlen( dumpFile);
    char	*str = new char[ len + 5 + 1];
    
    strcpy( str, dumpFile);
    strcpy( str + len, ".stat");
    fp = fopen( str, "w");
    if( fp == 0)
      printf( "***** Unable to open %s for writing\n", str);
    else
    {
      char		comma = ' ';

      fprintf( fp, "common {\n  NumNodes %d,\n", NumNodes);
      nodes[0].net.dumpStatic( fp, 0, 2);
      fprintf( fp, "}\ncostTable {\n");
      nodes[0].net.dumpCacheStatic( fp, 0, 2);
      fprintf( fp, "}\nsenders {");
      for( i = 0; i < NumNodes; i++)
	if( nodes[i].SrcOrDst == true)
	{
	  fprintf( fp, "%c %d", comma, nodes[i].ID);
	  comma = ',';
	}
      fprintf( fp, "}\n");
      for( i = 0; i < NumNodes; i++)
      {
	char	comma;

	fprintf( fp, "node {\n");
	fprintf( fp, "  location { %f, %f }\n", nodes[i].mob.getX(),
		 nodes[i].mob.getY());
	fprintf( fp, "  net {\n");
	nodes[i].net.dump( fp, 2, 2);
	fprintf( fp, "  }\n  costTable {\n");
	nodes[i].net.dumpCostTable( fp, 2, 2);
	fprintf( fp, "  }\n  neighbors {");
	comma = ' ';
	for( j = 0; j < NumNodes; j++)
	{
	  if( j == i)
	    continue;
	  if( neighborMatrix[ i * NumNodes + j] == true)
	  {
	    fprintf( fp, "%c %3d", comma, j);
	    comma = ',';
	  }
	}
	fprintf( fp, "}\n}\n");
      }
    }
    delete [] neighborMatrix;
    delete [] str;
  }

  // Application layer stats
  for( sent = recv = i = 0, delay = 0.0; i < NumNodes; i++)
  {
    sent+=nodes[i].app.SentPackets;
    recv+=nodes[i].app.RecvPackets;
    delay+=nodes[i].app.TotalDelay;
  }
  printf( "APP -- pkts sent: %10d, rcvd: %10d, success rate: %.3f, "
	  "end-to-end delay: %.3f\n", sent, recv, (double) recv/sent,
	  delay/recv);

  // Network (SSR) layer stats
  app_recv=recv;
  delay=0.0;
  samples=0;
  hop=0.0;
  recv_data=0;
  sentSuboptimal = 0;
  canceledPkts = 0;
  canceledSuboptimal = 0;
  for( j = HCArraySize-1; j >= 0; j--)
    hopCounts[j] = 0;
  for( sent = recv = i = 0; i < NumNodes; i++)
  {
    sent+=nodes[i].net.SentPackets;
    sentSuboptimal += nodes[i].net.SentSuboptimal;
    canceledPkts += nodes[i].net.CanceledPackets;
    canceledSuboptimal += nodes[i].net.CanceledSuboptimal;
    recv+=nodes[i].net.RecvPackets;
    hop+=nodes[i].net.TotalHop;
    delay+=nodes[i].net.TotalDelay;
    samples+=nodes[i].net.TotalSamples;
    recv_data+=nodes[i].net.RecvDataPackets;
    for( j = HCArraySize-1; j >= 0; j--)
      hopCounts[j] += nodes[i].net.HopCounts[j];
  }
  printf( "NET -- pkts sent: %10d, rcvd: %10d, avg hop: %.3f, "
	  "backoff delay: %.3f, suboptimal: %10d, canceled pkts: %10d, "
	  "canceled suboptimal: %10d\n", sent, recv, hop/recv_data,
	  delay/samples, sentSuboptimal, canceledPkts, canceledSuboptimal);
  printf( "NET -- hop counts [");
  for( i = 0; i < HCArraySize; i++)
    printf( " %4d", hopCounts[ i]);
  printf( "]\n");
  numCollisions = 0;		//mwl

  // MAC layer stats
  for( sent = recv = i = 0; i < NumNodes; i++)
  {
    sent+=nodes[i].mac.SentPackets;
    recv+=nodes[i].mac.RecvPackets;
    numCollisions += nodes[i].mac.rrCollisions;
  }
  printf( "MAC -- pkts sent: %10d, rcvd: %10d, # collisions %10d\n", sent,
	  recv, numCollisions);

  // Battery stats
  {
    double	ie = 0, re = 0;
    for( i=0; i < NumNodes; i++)
    {
      ie += nodes[i].battery.InitialEnergy;
      re += nodes[i].battery.RemainingEnergy;
    }
    printf( "BAT -- Initial: %f, Remaining: %f\n", ie, re);
  }
  return;
}

/************************************************************************
 * The simulation has a @Setup()@ function which must be called before
 * the simulation can be run.  The reason we don't do this in the constructor
 * is that we must assign values to
 * its parameters after the simulation component has been instantiated. 
 * The @Setup()@ function, which you can rename to anything you like, 
 * first maps component
 * parameters to corresponding simulation parameters (for instance, assign
 * the value of the simulation parameter @interval@ to the component parameter
 * @source.interval@).  It then connects pairs of inport and outports. 
 ************************************************************************/

void RoutingSim::packetTimes(
  int		source,
  int		dest)
{
  int		xmitSize = Random( PacketSize) + PacketSize / 2;
  double	xmitTime = Random( Interval) + Interval/2;

  nodes[ source].app.Connections.push_back(
    make_triple( ether_addr_t( dest), xmitSize, xmitTime));
  printf( "node %3d->%3d transmissions: %4d @ %f\n", source, dest, xmitSize,
	  xmitTime);
  return;
}

void RoutingSim::Setup()
{
  int i;
    
/************************************************************************
 * The size of the sensor node array must be set using @SetSize()@ before
 * the array can ever be used.
 ************************************************************************/
    
  nodes.SetSize(NumNodes);
  for(i=0;i<NumNodes;i++)
  {
    nodes[i].MaxX=MaxX;
    nodes[i].MaxY=MaxY;
    nodes[i].MyEtherAddr=i;
    nodes[i].ID=i;
    nodes[i].Setup( TXPower, ForwardDelay);
    nodes[i].mob.Setup();
  }    

  if( dumpStatus == true)
  {
    neighborMatrix = new bool[ NumNodes * NumNodes];
    for( i = NumNodes * NumNodes-1; i >= 0; i--)
      neighborMatrix[ i] = false;
  }

/************************************************************************
 * Set the slot width, transition time and backoff type (lambda or slot).
 * This is done only once since they are static (class wide) for the SHR class.
 * But, this must be done after @nodes@ has been initialized. The [] operator
 * is overloaded and throws an exception if nodes hasn't been initialized.
 ************************************************************************/

  nodes[0].net.setSlotWidth( SlotWidth);
  nodes[0].net.setTransitionTime( TransitionTime);
  nodes[0].net.setBackOff( BackOff);
  nodes[0].net.setContinuousBackOff( ContinuousBackOff);
  nodes[0].net.setRouteRepair( RouteRepair);
      
/*********************************************************************
 * Pass the pointer to the Visualizer to the protocol (for packet routes) and
 * mobility (for node location) components. If the Visualizer is to create
 * the output files, it must have been instantiated before these calls. See
 * the 'v' argument in main.
 *********************************************************************/
  nodes[0].net.setVisualizer( Visualizer::instantiate());
  nodes[0].mob.setVisualizer( Visualizer::instantiate());

/************************************************************************
 * The channel component needs to know the total number of sensor nodes.
 * It also needs to know the value of @CSThresh@ since it won't sent packets
 * to nodes that can't detect them.  @RXThresh@ is also needed to produce
 * the same receive power in those nodes that can just correctly receive packets
 * when using different propagation models.
 * 
 * In this example @FreeSpace@ is used.
 ************************************************************************/

  channel.setNumNodes( NumNodes);
  channel.setDumpPackets( false);
  channel.setCSThresh( nodes[0].phy.getCSThresh());
  channel.setRXThresh( nodes[0].phy.getRXThresh());
  channel.useFreeSpace();
  channel.setWaveLength( speed_of_light / nodes[0].phy.getFrequency());
  channel.setX( MaxX);
  channel.setY( MaxY);
  channel.setGridEnabled( false);
  channel.setMaxTXPower( nodes[0].phy.getTXPower());
    
/************************************************************************
 * The channel component also has a @Setup()@ function which is to 
 * set the size of its outport array.
 ************************************************************************/
  channel.Setup();

  for(i=0;i<NumNodes;i++)
  {
    connect nodes[i].to_channel_packet,channel.from_phy;
    connect nodes[i].to_channel_pos,channel.pos_in;
    connect channel.to_phy[i],nodes[i].from_channel ;
  }
/************************************************************************
 * This is to create communication pairs.
 ************************************************************************/

  bool		randomNodes;
  randomNodes = channel.initSources( NumSourceNodes, OneSink);

  int		src, dest = 0, count = (int) (1.5 * NumNodes);

  if( randomNodes == true && OneSink == true)
  {
    dest = Random( NumNodes);
    nodes[ dest].SrcOrDst = true;
  }

  for( i = 0; i < NumSourceNodes; i++)
  {
    if( randomNodes == true)
    {
      double	dx, dy;
      do
      {
	do
	  src = Random( NumNodes);
	while( nodes[ src].SrcOrDst == true);
	if( OneSink == false)
	{
	  do
	    dest = Random( NumNodes);
	  while( nodes[ dest].SrcOrDst == true);
	}
	dx = nodes[ src].mob.getX() - nodes[ dest].mob.getX();
	dy = nodes[ src].mob.getY() - nodes[ dest].mob.getY();
	--count;
      }
      while( src == dest || (dx*dx + dy*dy) < MaxX * MaxY / 4 && count > 0);
      if( count <= 0)
      {
	printf( "***** Unable to find enough source nodes.\n");
	assert( 0);
      }
    }
    else
      channel.getSourcePair( src, dest);
    nodes[ src].SrcOrDst = true;
    if( OneSink == false)
      nodes[ dest].SrcOrDst = true;
    packetTimes( src, dest);
    if( UnidirectionalTraffic == false)
      packetTimes( dest, src);
  }
  
  if( ActiveCycle == 0.0)	// failures are permanent
  {
    double	startupTime = 5 * Interval;
    double	steadyStateTime = FinishTimeRatio * StopTime() - startupTime;

    for( i = 0; i < NumNodes; i++)
    {
      nodes[i].pm.BeginTime = 0.0;
      nodes[i].pm.FinishTime = StopTime();
      if( nodes[i].SrcOrDst == false && Random() > ActivePercent)
	nodes[i].pm.failureStats( startupTime + Random() * steadyStateTime);
      else
	nodes[i].pm.failureStats( 0.0);		// source & dest don't go down
    }
  }
  else			// failures are transient
  {
    for( i = 0; i < NumNodes; i++)
    {
      nodes[i].pm.BeginTime = 0.0;
      nodes[i].pm.FinishTime = StopTime();
      if( nodes[i].SrcOrDst == false)
	nodes[i].pm.failureStats( 5 * Interval + (double) i/NumNodes * ActiveCycle,
				  ActiveCycle, ActivePercent);
      else
	nodes[i].pm.failureStats( 0.0);		// source & dest don't go down
    }
  }

#ifdef	USE_CONFIG_FILE
/*
** Check the execution time of all of the events in the configuration file.
** If an event is scheduled after the end of traffic generation or
** after the end of the simulation (StopTime), inform the user
*/
  bool		status;
  int		node;
  double	time, finishTime = StopTime() * FinishTimeRatio;
  SimEvent	event;

  for( status = firstEvent( time, node, event); status == true;
       status = nextEvent( time, node, event))
  {
    if( time > StopTime())
    {
      printf( "FYI: Event '%s' for node %d is after end of simulation "
	      "(%.0f > %.0f)\n", eventToString( event), node, time,StopTime());
      continue;
    }
    if( time > finishTime)
    {
      printf( "FYI: Event '%s' for node %d is after last packet generation "
	      "(%.0f > %.0f)\n", eventToString( event), node, time,finishTime);
      continue;
    }

/*
** This is a major kludge, but it's unavoidable with the current state of
** compcxx. See my comments after the definition of RoutingSim.
** Since it is not possible to pass pointers (or references) to a component,
** this method must reexamine the event list looking for when nodes come back
** up. When an event is found, schedule that event on the component's timer.
*/
    if( event == NodeUp)
      nodes[ node].net.scheduleHELLO( time);
  }
#endif	// USE_CONFIG_FILE

  return;
}

/************************************************************************
 * @<h2>Running the Simulation</h2>@
 * To run the simulation, first we need to create a
 * simulation object from the simulation component class. 
 * Several default simulation parameters must be determined. 
 * @StopTime@ denotes the ending time of the simulation.
 * @Seed@ is the initial seed of the random number generator used
 * by the simulation.
 *
 * Use the make file to compile and link the program.
 * 
 * To run the simulation, simply type in:
 *
 * sim_shr <flags>	(This is not complete.)
 * mwl
 ************************************************************************/

int main(
  int		argc,
  char		*argv[])
{
  RoutingSim	sim;
  int		seed;
  bool		doDecrement, proceed = true;
  const char	*UnsupportedOption = "***** Unsupported option %s *****\n";

  seed = (int) time( 0);

  for( argv++; argc > 1; argc--, argv++)
  {
    if( *argv[0] != '-')
    {
      printf( UnsupportedOption, *argv);
      proceed = false;
      continue;
    }
    doDecrement = true;
    switch( (*argv)[1])
    {
      case 'A':		// The sum of a node's mean up time and down time
	sim.ActiveCycle = atof( argv[1]);
	if( sim.ActiveCycle < 0.0)
	{
	  printf( "***** Warning: Nodes have ActiveCycle < 0.0 *****\n");
	  proceed = false;
	}
	break;
      case 'a':		// percent active, cycles node between idle and off, see power.h
	sim.ActivePercent = atof( argv[1]);
	if( sim.ActivePercent > 1.0)
	{
	  printf( "***** Warning: Nodes active more than 100%% of the time *****\n");
	  proceed = false;
	}
	if( sim.ActivePercent <= 0.0)
	{
	  printf( "***** Warning: Nodes never active *****\n");
	  proceed = false;
	}
	break;
      case 'b':		// backoff type
      {
	char	*str = new char[ strlen(argv[1])+1], *p1, *p2;
	for( p1 = argv[1], p2 = str; *p1 != 0; p1++, p2++)
	  *p2 = tolower( *p1);
	*p2 = 0;
	if( strcmp( "ssr", str) == 0)
	  sim.BackOff = SHRBackOff_SSR;
	else if( strcmp( "shr", str) == 0)
	  sim.BackOff = SHRBackOff_SHR;
	else if( strcmp( "incorrect", str) == 0)
	  sim.BackOff = SHRBackOff_Incorrect;
	else
	{
	  printf( UnsupportedOption, argv[1]);
	  proceed = false;
	}
	delete [] str;
	break;
      }
#ifdef	USE_CONFIG_FILE
      case 'c':		// enable reading of configuration file (if that makes sense)
	configStatus = true;
	configFile = argv[1];
	doDecrement = true;
	break;
#endif	// USE_CONFIG_FILE
      case 'C':		// continuous vs. slotting
	switch( *argv[1])
	{
	  case 't':
	  case 'T':
	    sim.ContinuousBackOff = true;
	    break;
	  case 'f':
	  case 'F':
	    sim.ContinuousBackOff = false;
	    break;
	  default:
	    printf( UnsupportedOption, argv[1]);
	    proceed = false;
	    break;
	}
	break;
      case 'd':		// at end of simulation, dump status of nodes
	dumpStatus = true;
	dumpFile = argv[1];
	doDecrement = true;
	break;
      case 'e':		// simulation end time
	sim.StopTime( atof( argv[1]));
	break;
      case 'i':		// interval
	sim.Interval = atof( argv[1]);
	break;
      case 'l':		// lambda (aka ForwardDelay)
	sim.ForwardDelay = atof( argv[1]);
	break;
#ifndef	USE_CONFIG_FILE
      case 'n':		// number of nodes
	numNodes = sim.NumNodes = atoi( argv[1]);
	break;
#endif	// USE_CONFIG_FILE
      case 'p':		// antenna transmission power in watts
	sim.TXPower = atof( argv[1]);
	break;
      case 'r':		// random number seed
	seed = atoi( argv[1]);
	break;
      case 'R':		// # packets in route repair
	sim.RouteRepair = atoi( argv[1]);
	if( sim.RouteRepair < 0)
	{
	  printf( "***** Route repair must be >= 0\n");
	  proceed = false;
	}
	break;
      case 's':		// number of sources
	sim.NumSourceNodes = atoi( argv[1]);
	break;
      case 'S':		// traffic pattern: 
	switch( *argv[1])
	{
	  case 't':	// many sources to single destination
	  case 'T':
	    sim.OneSink = true;
	    break;
	  case 'f':	// many (source, destination) pairs (parallel streams)
	  case 'F':
	    sim.OneSink = false;
	    break;
	  default:
	    printf( UnsupportedOption, argv[1]);
	    proceed = false;
	    break;
	}
	break;
      case 't':		// transition timer
	sim.TransitionTime = atof( argv[1]);
	break;
      case 'u':		// unidirectional traffic
	switch( *argv[1])
	{
	  case 't':	// all traffic from sources to destination(s)
	  case 'T':
	    sim.UnidirectionalTraffic = true;
	    break;
	  case 'f':	// traffic in both directions
	  case 'F':
	    sim.UnidirectionalTraffic = false;
	    break;
	  default:
	    printf( UnsupportedOption, argv[1]);
	    proceed = false;
	    break;
	}
	break;
      case 'v':		// enable visualizer
	Visualizer::instantiate( argv[1]);
	break;
      case 'w':		// slot width
	sim.SlotWidth = atof( argv[1]);
	break;
      case 'x':		// x (and y) dimension
	sim.MaxX = sim.MaxY = atof( argv[1]);
	break;
      case 'z':		// clear stats counters
	sim.ClearStatsTime( atof( argv[ 1]));
	break;
      default:
	doDecrement = false;
	proceed = false;
	printf( UnsupportedOption, *argv);
	break;
    }
    if( doDecrement == true)
    {
      argv++;
      argc--;
    }
  }

  if( proceed == false)
    return 1;

#ifdef	USE_CONFIG_FILE
  if( configFile == NULL)
  {
    printf( "***** A configuration file is required\n");
    assert( 0);
  }
  parseConfigInfo( configFile);
  numNodes = sim.NumNodes = getNumNodes();
#endif	// USE_CONFIG_FILE

  if( sim.NumSourceNodes == 0)
    sim.NumSourceNodes = sim.NumNodes / 10;
  if( sim.NumSourceNodes <= 0)
  {
    printf( "***** Warning: no source nodes\n");
    return 1;
  }
  if( sim.NumSourceNodes * (sim.UnidirectionalTraffic == true ? 1 : 2) >= sim.NumNodes)
  {
    printf( "***** Warning: more source nodes than actual nodes\n");
    return 1;
  }
  sim.Seed = seed;

/*
** Display parameters that define the simulation.
*/
  printf( "SSR/SHR:\t\t");
#ifdef	SHR_ACK
  printf( "SHR_ACK defined\n");
#else	//SHR_ACK
  printf( "SHR_ACK not defined\n");
#endif	//SHR_ACK
  printf( "Channel model:\t\t%s\n", ChannelModel);
#ifdef	USE_CONFIG_FILE
  if( configStatus == true)
    printf( "Configuration file:\t%s\n", configFile);
  else
    printf( "Configuration file not used\n");
#endif	// USE_CONFIG_FILE
  printf( "Random number seed:\t%d\nStopTime:\t\t%.0f\n"
	  "Clear Stats Time:\t%.0f\nNumber of Nodes:\t%d\n"
	  "Active Cycle:\t\t%5.2f\n"
	  "Percent Active:\t\t%5.2f\nTerrain:\t\t%.0f by %.0f\n"
	  "Number of Sources:\t%d\nPacket Size:\t\t%d\n"
	  "Interval:\t\t%f\nAntenna Power:\t\t%f\nSingle sink:\t\t%s\n"
	  "Unidirectional Traffic:\t%s\nLambda:\t\t\t%f\nRoute Repair:\t\t",
	  seed,
	  sim.StopTime(), sim.ClearStatsTime(), sim.NumNodes, sim.ActiveCycle,
	  sim.ActivePercent * 100.0, sim.MaxX, sim.MaxY, sim.NumSourceNodes,
	  sim.PacketSize, sim.Interval, sim.TXPower,
	  sim.OneSink == true ? "True" : "False",
	  sim.UnidirectionalTraffic == true ? "True" : "False", sim.ForwardDelay);
  if( sim.RouteRepair == 0)
    printf( "none");
  else
    printf( "%d packet%c", sim.RouteRepair, sim.RouteRepair == 1 ? ' ' : 's');
  printf( "\nBackoff Type:\t\t");
  switch( sim.BackOff)
  {
    case SHRBackOff_SSR:
      printf( "SSR formula\n");
      break;
    case SHRBackOff_SHR:
      printf( "SHR formula\n");
      break;
    case SHRBackOff_Incorrect:
      printf( "formula as published (incorrect)\n");
      break;
    default:
      printf( "unknown\n");
      break;
  }

  printf( "Continuous:\t\t");
  if( sim.ContinuousBackOff == false)
    printf( "false\nSlot Width:\t\t%f\nTransition Time:\t%f\n",
	    sim.SlotWidth, sim.TransitionTime);
  else
    printf( "true\n");

  sim.Setup();
#ifdef	USE_CONFIG_FILE
  dumpConnections( sim.MaxX, sim.MaxY);
#endif	// USE_CONFIG_FILE
  sim.Run();
  return 0;
}

void addToNeighborMatrix(
  int		src,
  int		dest)
{
  if( dumpStatus == true)
    neighborMatrix[ src * numNodes + dest] = true;
  return;
}
