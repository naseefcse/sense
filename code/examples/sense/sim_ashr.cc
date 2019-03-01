/*******************************************************************
 * @<title> Autonomic Self-Healing Routing Simulation </title>@
 *
 * @<!-- Copyright 2006 Mark Lisee, Joel Branch, Gilbert (Gang) Chen,
 * Boleslaw K. Szymanski and Rensselaer Polytechnic Institute.
 *
 * Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
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

#define queue_t HeapQueue

/*******************************************************************
 * This header file is absolutely required
 *******************************************************************/

#include <ctime>
#include "../../common/sense.h"

/*******************************************************************
 * The following header files are necessary only if the corresponding
 * components are needed by the sensor node component
 *******************************************************************/

#include "../../app/cbr.h"
#include "../../mob/immobile.h"
#include "../../net/ssab.h"
#include "../../net/ssrv02.h"
#include "../../mac/bcast_mac.h"
#include "../../phy/transceiver.h"
#include "../../phy/simple_channel.h"
#include "../../energy/battery.h"
#include "../../energy/power.h"

/*******************************************************************
 * #cxxdef is similar to #define, except it is only recognized by the
 * CompC++ compiler.  The following two lines state that the flooding
 * component will be used for the network layer. These two macros
 * can also be overridden by command line macros definitions (whose
 * format is '-D<macro>=<something>'.
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
typedef SSRv02_Struct<app_packet_t>::packet_t net_packet_t;
typedef BcastMAC_Struct<net_packet_t*>::packet_t mac_packet_t;

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
  SSRv02 <app_packet_t> net;
  BcastMAC <net_packet_t*> mac;
  // A transceiver that can transmit and receive at the same time (of course
  // a collision would occur in such cases)
  DuplexTransceiver < mac_packet_t > phy;
  // Linear battery
  SimpleBattery battery;
  // PowerManagers manage the battery
  PowerManager	pm;
  // sensor nodes are immobile
  Immobile	mob;
    
  double	MaxX, MaxY;  // coordinate boundaries
  ether_addr_t	MyEtherAddr; // the ethernet address of this node
  int		ID; // the identifier
  bool		SrcOrDst;

  virtual ~SensorNode();
  void		Start();
  void		Stop();
  void		Setup( double txPower);

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
  double	txPower)
{
/*******************************************************************
 * At the beginning the amount of energy in each battery is 1,000,000 Joules.
 *******************************************************************/

  battery.InitialEnergy = 1e6;
    
/*******************************************************************
 * Each subcomponent must also know the ethernet address of the 
 * sensor node it resides on.
 * Remember the application layer is a CBR component, which would stop
 * at FinishTime to give the whole network an opportunity to clean up
 * any packets in transit.
 * Assiging @false@ to @app.DumpPackets@ means that if @<a href=manual.html#COST_DEBUG>COST_DEBUG</a>@
 * is defined, @app@ still won't print out anything.
 *******************************************************************/
    
  app.MyEtherAddr = MyEtherAddr;
  app.FinishTime = StopTime()*0.9;
  app.DumpPackets = false;
    
/*******************************************************************
 * Set the coordinate of the sensor node.  Must also give @ID@ to @mob@
 * since @ID@ was used to identify the index of the sensor node when
 * the position info is sent to the channel component.
 *******************************************************************/

  mob.InitX = Random(MaxX);
  mob.InitY = Random(MaxY);
  mob.ID = ID;

/*******************************************************************
 * When a net component is about to retransmit a packet that it received,
 * it cannot do so because otherwise all nodes that received the packet
 * may attempt to retransmit the packet immediately, inevitably resulting
 * in a collision.  @ForwardDelay@ gives the maximum delay time a 
 * needed-to-be-retransmit packet may incur.  The actual delay is randomly
 * chosen between [0,@ForwardDelay@].
 *******************************************************************/

  net.MyEtherAddr = MyEtherAddr;
  net.ForwardDelay = 0.1;
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
 
  mac.MyEtherAddr = MyEtherAddr;
  mac.Promiscuity = true;
  mac.DumpPackets = false;
  mac.IFS = 0.01;
   
/*******************************************************************
 *  The PowerManager takes care of power consumption at different states.
 *  The following lines state the power consumption is 1.6W at transmission
 *  state, 1.2 at receive state, and 1.115 at idle state.
 *******************************************************************/

  pm.TXPower = 1.6;
  pm.RXPower = 1.2;
  pm.IdlePower = 1.15;

/*******************************************************************
 *  @phy.TxPower@ is the transmission power of the antenna.
 *  @phy.RXThresh@ is the lower bound on the receive power of any packet
 *  that can be successfuly received.
 *  @phy.CSThresh@ is the lower bound on tye receive power of any packet
 *  that can be detected.
 *  @phy@ also needs to know the id because it needs to communicate with
 *  the channel component.
 *******************************************************************/

  phy.TXPower = txPower;
  phy.TXGain = 1.0;
  phy.RXGain = 1.0; 
  phy.Frequency = 9.14e8;
  phy.RXThresh = 3.652e-10;
  phy.CSThresh = 1.559e-11;
  phy.ID = ID;

  net.RXThresh = phy.RXThresh;
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

  SrcOrDst = false;
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
		RoutingSim();

/*******************************************************************
 *  These are simulation parameters.  We don't want configurators of
 *  the simulation to access the parameters of those inter-components.
 *******************************************************************/

  double	MaxX, MaxY;
  int		NumNodes;
  int		NumSourceNodes;
  int		NumConnections;
  int		PacketSize;
  double	Interval;
  double	ActivePercent;
  double	SlotWidth;
  double	TransitionTime;
  int		NumT2Slots;
  int		NumT3Slots;
  double	TXPower;

/*******************************************************************
 *  Here we declare sense nodes as an array of @SensorNode@, and a
 *  channel component.
 *******************************************************************/

  SensorNode[]	nodes;
  SimpleChannel < mac_packet_t > channel;

  void		Setup();
};

RoutingSim::RoutingSim()
{
  NumConnections = 1;
  PacketSize = 1000;
  StopTime = 1000;
  NumNodes = 110;
  MaxX = MaxY = 2000;
  NumSourceNodes = 0;
  ActivePercent = 1.0;
  Interval = 20.0;
  SlotWidth = 0.001;
  TransitionTime = 0.0;
  NumT2Slots = DefaultNumT2Slots;
  NumT3Slots = DefaultNumT3Slots;
  TXPower=0.0280;
  return;
}

void RoutingSim :: Start()
{
}

/*******************************************************************
 *  After the simulation is stopped, we will collect some statistics.
 *******************************************************************/

void RoutingSim :: Stop()
{
  int		i, j, sent, recv, samples, recv_data, hopCounts[ HCArraySize];
  double	hop;
  double	app_recv;
  double	delay;
  int		T2PktsSent[ MaxT2Slots], T1PktsSent, T3PktsSent[ MaxT3Slots];
  int		dropPkts, t3AbnormalPktsSent = 0;
  int		T2Colls[ MaxT2Slots], T1Colls, T3Colls[ MaxT3Slots], IgnColls;
  int		numCollisions;
  int		T1PktsRcvd[2];
  int		T2PktsRcvd[2][ MaxT2Slots];
  int		T3PktsRcvd[2][ MaxT3Slots];
  int		T4PktsRcvd[2];
  int		CanceledPktsRcvd[2];
  int		BadAddrPktsRcvd[2];
  int		WrongHopPktsRcvd[2];

  for(sent=recv=i=0,delay=0.0;i<NumNodes;i++)
  {
    sent+=nodes[i].app.SentPackets;
    recv+=nodes[i].app.RecvPackets;
    delay+=nodes[i].app.TotalDelay;
  }
  printf( "APP -- pkts sent: %10d, rcvd: %10d, success rate: %.3f, "
	  "end-to-end delay: %.3f\n", sent, recv, (double) recv/sent,
	  delay/recv);
  app_recv=recv;
  delay=0.0;
  samples=0;
  hop=0.0;
  recv_data=0;
  for( j = HCArraySize-1; j >= 0; j--)
    hopCounts[j] = 0;
  for( int i = NumT2Slots-1; i >= 0; i--)
  {
    T2Colls[i] = T2PktsSent[i] = 0;
    T2PktsRcvd[0][i] = T2PktsRcvd[1][i] = 0;
  }
  for( int i = NumT3Slots-1; i >= 0; i--)
  {
    T3Colls[i] = T3PktsSent[i] = 0;
    T3PktsRcvd[0][i] = T3PktsRcvd[1][i] = 0;
  }
  dropPkts = T1PktsSent = 0;
  T1Colls = IgnColls = 0;
  T1PktsRcvd[0] = T1PktsRcvd[1] = 0;
  T4PktsRcvd[0] = T4PktsRcvd[1] = 0;
  CanceledPktsRcvd[0] = CanceledPktsRcvd[1] = 0;
  BadAddrPktsRcvd[0] = BadAddrPktsRcvd[1] = 0;
  WrongHopPktsRcvd[0] = WrongHopPktsRcvd[1] = 0;

  for( sent = recv = i = 0; i < NumNodes; i++)
  {
    sent += nodes[i].net.SentPackets;
    recv += nodes[i].net.RecvPackets;
    hop += nodes[i].net.TotalHop;
    delay += nodes[i].net.TotalDelay;
    samples += nodes[i].net.TotalSamples;
    recv_data += nodes[i].net.RecvDataPackets;
    dropPkts += nodes[i].net.DropPkts;
    T1PktsSent += nodes[i].net.T1PktsSent;
    T1Colls += nodes[i].net.T1Collisions;
    IgnColls += nodes[i].net.IgnoreCollisions;
    t3AbnormalPktsSent += nodes[i].net.T3AbnormalPktsSent;
    T1PktsRcvd[0] += nodes[i].net.T1PktsRcvd[0];
    T1PktsRcvd[1] += nodes[i].net.T1PktsRcvd[1];
    T4PktsRcvd[0] += nodes[i].net.T4PktsRcvd[0];
    T4PktsRcvd[1] += nodes[i].net.T4PktsRcvd[1];
    CanceledPktsRcvd[0] += nodes[i].net.CanceledPktsRcvd[0];
    CanceledPktsRcvd[1] += nodes[i].net.CanceledPktsRcvd[1];
    BadAddrPktsRcvd[0] += nodes[i].net.BadAddrPktsRcvd[0];
    BadAddrPktsRcvd[1] += nodes[i].net.BadAddrPktsRcvd[1];
    WrongHopPktsRcvd[0] += nodes[i].net.WrongHopPktsRcvd[0];
    WrongHopPktsRcvd[1] += nodes[i].net.WrongHopPktsRcvd[1];
    for( j = HCArraySize-1; j >= 0; j--)
      hopCounts[j] += nodes[i].net.HopCounts[j];
    for( int j = NumT2Slots-1; j >= 0; j--)
    {
      T2PktsSent[j] += nodes[i].net.T2PktsSent[j];
      T2Colls[j] += nodes[i].net.T2Collisions[j];
      T2PktsRcvd[0][j] += nodes[i].net.T2PktsRcvd[0][j];
      T2PktsRcvd[1][j] += nodes[i].net.T2PktsRcvd[1][j];
    }
    for( int j = NumT3Slots-1; j >= 0; j--)
    {
      T3PktsSent[j] += nodes[i].net.T3PktsSent[j];
      T3Colls[j] += nodes[i].net.T3Collisions[j];
      T3PktsRcvd[0][j] += nodes[i].net.T3PktsRcvd[0][j];
      T3PktsRcvd[1][j] += nodes[i].net.T3PktsRcvd[1][j];
    }
  }
  printf( "NET -- pkts sent: %10d, rcvd: %10d, avg hop: %.3f, "
	  "backoff delay: %.3f\n", sent, recv, hop/recv_data, delay/samples);
  printf( "NET -- hop counts [");
  for( i = 0; i < HCArraySize; i++)
    printf( " %4d", hopCounts[ i]);
  printf( "]\nNET -- T2 Sent [");
  for( i = 0; i < NumT2Slots; i++)
    printf( " %4d", T2PktsSent[ i]);
  printf( "]\nNET -- T3 Sent [");
  for( i = 0; i < NumT3Slots; i++)
    printf( " %4d", T3PktsSent[ i]);
  printf( "]\nNET -- T1 Sent %4d; Dropped Packets %4d; T3 Abnormal %4d\n"
	  "NET -- T2 Collisions [", T1PktsSent, dropPkts, t3AbnormalPktsSent);
  for( i = 0; i < NumT2Slots; i++)
    printf( " %4d", T2Colls[ i]);
  printf( "]\nNET -- T3 Collisions [");
  for( i = 0; i < NumT3Slots; i++)
    printf( " %4d", T3Colls[ i]);
  printf( "]\nNET -- T1 Collisions %4d; Ignored Collisions %4d\n", T1Colls,
	  IgnColls);
  numCollisions = 0;		//mwl
  for( sent = recv = i = 0; i < NumNodes; i++)
  {
    sent += nodes[i].mac.SentPackets;
    recv += nodes[i].mac.RecvPackets;
    numCollisions += nodes[i].mac.rrCollisions;
  }

  printf( "NET -- T1 Rcvd [ %4d %4d]\n", T1PktsRcvd[0], T1PktsRcvd[1]);
  for( j = 0; j < 2; j++)
  {
    printf( "NET -- T2 Rcvd[%s] [", nodes[0].net.pktRcvdName[j]);
    for( i = 0; i < NumT2Slots; i++)
      printf( " %4d", T2PktsRcvd[j][i]);
    printf( "]\n");
  }
  for( j = 0; j < 2; j++)
  {
    printf( "NET -- T3 Rcvd[%s] [", nodes[0].net.pktRcvdName[j]);
    for( i = 0; i < NumT3Slots; i++)
      printf( " %4d", T3PktsRcvd[j][i]);
    printf( "]\n");
  }
  printf( "NET -- T4 Rcvd [ %4d %4d]\n", T4PktsRcvd[0], T4PktsRcvd[1]);
  printf( "NET -- Cancel[ %4d %4d]; Bad[ %4d %4d]; WrongHop[ %4d %4d]\n",
	  CanceledPktsRcvd[0], CanceledPktsRcvd[1], BadAddrPktsRcvd[0],
	  BadAddrPktsRcvd[1], WrongHopPktsRcvd[0], WrongHopPktsRcvd[1]);
  printf( "MAC -- pkts sent: %10d, rcvd: %10d, # collisions %10d\n", sent,
	  recv, numCollisions);
  {
    double	ie = 0, re = 0;
    for( i=0; i < NumNodes; i++)
    {
      ie += nodes[i].battery.InitialEnergy;
      re += nodes[i].battery.RemainingEnergy;
    }
    printf( "BAT -- Initial: %f, Remaining: %f\n", ie, re);
  }

//mwl  for( i = 0; i < NumNodes; i++)
//mwl    printf( "ea %3d: 4 hc %2d; 69 hc %2d\n", i, nodes[i].net.mwlGetHC( 4), nodes[i].net.mwlGetHC( 69));
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

void RoutingSim :: Setup()
{
  int i,j;
    
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
    nodes[i].Setup( TXPower); // don't forget to call this function for each sensor node
  }    

/************************************************************************
 * Set the slot width, transition time.
 * This is done only once since they are static (class wide) for the SSRv02
 * class. But, this must be done after @nodes@ has been initialized. The []
 * operator is overloaded and throws an exception if nodes hasn't been
 * initialized.
 ************************************************************************/

  nodes[0].net.setSlotWidth( SlotWidth);
  nodes[0].net.setTransitionTime( TransitionTime);
  nodes[0].net.setNumT2Slots( NumT2Slots);
  nodes[0].net.setNumT3Slots( NumT3Slots);
      
/************************************************************************
 * The channel component needs to know the total number of sensor nodes.
 * It also needs to know the value of @CSThresh@ since it won't sent packets
 * to nodes that can't detect them.  @RXThresh@ is also needed to produce
 * the same receive power in those nodes that can just correctly receive packets
 * when using different propagation models.
 * 
 * In this example @FreeSpace@ is used.
 ************************************************************************/

  channel.NumNodes=NumNodes;
  channel.DumpPackets=false;
  channel.CSThresh=nodes[0].phy.CSThresh;
  channel.RXThresh=nodes[0].phy.RXThresh;
  channel.PropagationModel=channel.FreeSpace;

  channel.WaveLength=speed_of_light/nodes[0].phy.Frequency;
  channel.X=MaxX;
  channel.Y=MaxY;
  channel.GridEnabled=false;
  channel.MaxTXPower=nodes[0].phy.TXPower;
    
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

  int src,dst;
  for(i=0;i<NumSourceNodes;i++)
  {
    for(j=0;j<NumConnections;j++)
    {
      double dx,dy;
      do
      {
	src=Random(NumNodes);
	dst=Random(NumNodes);
	dx=nodes[src].mob.InitX-nodes[dst].mob.InitX;
	dy=nodes[src].mob.InitY-nodes[dst].mob.InitY;
      }
      while( src == dst || (dx*dx+dy*dy) < MaxX * MaxY / 4); 
      nodes[src].SrcOrDst=true;
      nodes[dst].SrcOrDst=true;
      nodes[src].app.Connections.push_back(
	make_triple(ether_addr_t(dst),Random(PacketSize)+PacketSize/2,
		    Random(Interval)+Interval/2));
      nodes[dst].app.Connections.push_back(
	make_triple(ether_addr_t(src),Random(PacketSize)+PacketSize/2,
		    Random(Interval)+Interval/2)); 
    }
  }

  for(i=0;i<NumNodes;i++)
  {
    nodes[i].pm.BeginTime=0.0;
    nodes[i].pm.FinishTime=StopTime;
    nodes[i].pm.ActiveCycle=100.0;
    nodes[i].pm.ActivePercent=ActivePercent;
    if(nodes[i].SrcOrDst)
    {
      nodes[i].pm.ActivePercent=1.0;
    }
    //nodes[i].mob.InitX=(i/20)*MaxX/20;
    //nodes[i].mob.InitY=(i%20)*MaxY/20;
  }
  /*nodes[260].app.Connections.push_back(make_triple(ether_addr_t(273),1000,5.34));
    nodes[29].app.Connections.push_back(make_triple(ether_addr_t(235),1000,1.0203));
  */
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
 * To compile the program, enter:
 *
 * ../../bin/cxx sim_rr.cc
 * g++ -Wall -o sim_rr sim_rr.cxx
 * 
 * To run the simulation, simply type in:
 *
 * sim_rr <flags>
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
      case '2':		// number of slots in T2
	sim.NumT2Slots = atoi( argv[1]);
	if( sim.NumT2Slots > MaxT2Slots)
	{
	  printf( "Simulation supports only %d slots in T2.\n", MaxT2Slots);
	  proceed = false;
	}
	break;
      case '3':		// number of slots in T3
	sim.NumT3Slots = atoi( argv[1]);
	if( sim.NumT3Slots > MaxT3Slots)
	{
	  printf( "Simulation supports only %d slots in T3.\n", MaxT3Slots);
	  proceed = false;
	}
	break;
      case 'a':		// percent active
	sim.ActivePercent = atof( argv[1]);
	if( sim.ActivePercent > 1.0)
	{
	  printf( "***** Warning: Nodes active more than 100%% of the time *****\n");
	  proceed = false;
	}
	break;
      case 'e':		// simulation end time
	sim.StopTime = atof( argv[1]);
	break;
      case 'i':		// interval
	sim.Interval = atof( argv[1]);
	break;
      case 'n':		// number of nodes
	sim.NumNodes = atoi( argv[1]);
	break;
      case 'p':		// antenna transmission power in watts
	sim.TXPower = atof( argv[1]);
	break;
      case 'r':		// random number seed
	seed = atoi( argv[1]);
	break;
      case 's':		// number of sources
	sim.NumSourceNodes = atoi( argv[1]);
	break;
      case 't':		// transition timer
	sim.TransitionTime = atof( argv[1]);
	break;
      case 'w':		// slot width
	sim.SlotWidth = atof( argv[1]);
	break;
      case 'x':		// x (and y) dimension
	sim.MaxX = sim.MaxY = atof( argv[1]);
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

  if( sim.NumSourceNodes == 0)
    sim.NumSourceNodes = sim.NumNodes / 10;
  sim.Seed = seed;

/*
** Display parameters that define the simulation.
*/
  printf( "Fault Tolerant SSR:\t");
#ifdef	RR_ACK
  printf( "RR_ACK defined\n");
#else	//RR_ACK
  printf( "RR_ACK not defined\n");
#endif	//RR_ACK
  printf( "Random number seed:\t%d\nStopTime:\t\t%.0f\n"
	  "Number of Nodes:\t%d\nPercent Active:\t\t%5.2f\n"
	  "Terrain:\t\t%.0f by %.0f\nNumber of Sources:\t%d\n"
	  "Packet Size:\t\t%d\nInterval:\t\t%f\nAntenna Power:\t\t%f\n"
	  "Slot Width:\t\t%f\nTransition Time:\t%f\n# slots in T2\t\t%d\n"
	  "# slots in T3\t\t%d\n", seed, sim.StopTime, sim.NumNodes,
	  sim.ActivePercent * 100.0, sim.MaxX, sim.MaxY, sim.NumSourceNodes,
	  sim.PacketSize, sim.Interval, sim.TXPower, sim.SlotWidth,
	  sim.TransitionTime, sim.NumT2Slots, sim.NumT3Slots);

  sim.Setup();
  sim.Run();
  return 0;
}
