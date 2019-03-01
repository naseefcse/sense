#define queue_t HeapQueue

#include "../../common/sense.h"
#include "../../libraries/visualizer/inc/Visualizer.h"
#include "../../app/random_neighbor.h"
#include "../../mob/immobile.h"
#include "../../mac/mac_80211.h"
#include "../../mac/null_mac.h"
#include "../../phy/transceiver.h"
#include "../../phy/simple_channel.h"
#include "../../energy/battery.h"
#include "../../energy/power.h"

#cxxdef mac_component MAC80211
#cxxdef mac_struct MAC80211_Struct

typedef mac_struct <RandomNeighbor_Struct::packet_t> :: packet_t mac_packet_t;

component SensorNode : public TypeII
{
public:

    RandomNeighbor app;
    Immobile mob;
    mac_component <RandomNeighbor_Struct::packet_t> mac;
    DuplexTransceiver < mac_packet_t > phy;
    SimpleBattery battery;
    PowerManager pm;

    double MaxX, MaxY;
    ether_addr_t MyEtherAddr;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int ID;

    int SentPackets, RecvPackets;

    SensorNode();
    virtual ~SensorNode();
    void Start();
    void Stop();
    void Setup();

    outport void to_channel_packet(mac_packet_t* packet, double power, int id);
    inport void from_channel (mac_packet_t* packet, double power);
    outport void to_channel_pos(coordinate_t& pos, int id);
    // This is called when the power manager wakes up the node.
    inport void	from_pm_node_up();
};

SensorNode::SensorNode()
{
}
SensorNode::~SensorNode()
{
}

void SensorNode::Start()
{

}

void SensorNode::Stop()
{
    SentPackets=app.SentPackets;
    RecvPackets=app.RecvPackets;
}

void SensorNode::Setup()
{

    battery.InitialEnergy=1e6;
    
    app.BroadcastInterval=BroadcastInterval;
    app.UnicastInterval=UnicastInterval;
    app.MyEtherAddr=MyEtherAddr;
    app.MaxPacketSize=2*mac.RTSThreshold;
    app.DumpPackets=true;

    mob.setX( Random( MaxX));
    mob.setY( Random( MaxY));
    mob.setID( ID);

    mac.MyEtherAddr=MyEtherAddr;
    mac.DumpPackets=true;
    mac.Promiscuity=false;

   	pm.TXPower=1.6;
   	pm.RXPower=1.2;
   	pm.IdlePower=1.15;

    phy.setTXPower( 0.02283);
    phy.setTXGain( 1.0);
    phy.setRXGain( 1.0);
    phy.setFrequency( 9.14e8);
    phy.setRXThresh( 3.652e-10);
    phy.setCSThresh( 1.559e-11);
    phy.setID( ID);

    connect app.to_transport, mac.from_network;
    connect mac.to_network_data, app.from_transport_data;
    connect mac.to_network_ack, app.from_transport_ack;

    connect mac.to_phy, phy.from_mac;
    connect phy.to_mac, mac.from_phy;

    connect phy.to_channel, to_channel_packet;
    connect from_channel, phy.from_channel;
    connect phy.to_power_switch, pm.switch_state;
    
    connect pm.to_battery_query, battery.query_in;
    connect pm.to_battery_power, battery.power_in;

    connect pm.from_pm_node_up, from_pm_node_up;

    connect mob.pos_out, to_channel_pos;
    return;
}

/*
** The inport that is called when the power manager wakes the node up.
** This must be defined even if the protocol doesn't use it.
*/
void SensorNode::from_pm_node_up()
{
  return;
}

component Sim_80211 : public CostSimEng
{
public:

    void Stop();

    double MaxX, MaxY;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int NumNodes;

    SensorNode [] nodes;
    SimpleChannel < mac_packet_t > channel;
    
    void Setup();
};

void Sim_80211 :: Stop()
{
    int i,sent=0,recv=0;
    for(i=0;i<NumNodes;i++)
    {
	    sent+=nodes[i].SentPackets;
	    recv+=nodes[i].RecvPackets;
    }

    printf("total packets sent: %d, received: %d\n",sent,recv);
}
void Sim_80211 :: Setup()
{
    int i;
    nodes.SetSize(NumNodes);
    for(i=0;i<NumNodes;i++)
    {
      nodes[i].MaxX=MaxX;
      nodes[i].MaxY=MaxY;
      nodes[i].MyEtherAddr=i;
      nodes[i].BroadcastInterval=BroadcastInterval;
      nodes[i].UnicastInterval=UnicastInterval;
      nodes[i].ID=i;
      nodes[i].Setup();
    }
      
/*********************************************************************
 * Pass the pointer to the Visualizer to the protocol (for packet routes) and
 * mobility (for node location) components. If the Visualizer is to create
 * the output files, it must have been instantiated before these calls. See
 * the 'v' argument in sim_shr.h.
 *********************************************************************/
    nodes[0].mob.setVisualizer( Visualizer::instantiate());

    channel.setNumNodes( NumNodes);
    channel.setDumpPackets( false);
    channel.setCSThresh( nodes[0].phy.getCSThresh());
    channel.setRXThresh( nodes[0].phy.getRXThresh());
    channel.useFreeSpace();

    channel.setWaveLength( speed_of_light/nodes[0].phy.getFrequency());
    channel.setX( MaxX);
    channel.setY( MaxY);
    channel.setGridEnabled( false);
    channel.setMaxTXPower( nodes[0].phy.getTXPower());
    
    channel.Setup();

    for(i=0;i<NumNodes;i++)
    {
      connect nodes[i].to_channel_packet,channel.from_phy;
      connect nodes[i].to_channel_pos,channel.pos_in;
      connect channel.to_phy[i],nodes[i].from_channel ;
    }

/*********************************************************************
 * Initialize the power manager for each node.
 *********************************************************************/
    for(i=0;i<NumNodes;i++)
    {
      nodes[i].pm.BeginTime=0.0;
      nodes[i].pm.FinishTime=StopTime();
      nodes[i].pm.failureStats( 0.0);
    }
    return;
}

int main(int argc, char* argv[])
{
    Sim_80211 sim;

    sim.StopTime( 1000);
    sim.Seed=1234;

    sim.MaxX=5000;
    sim.MaxY=5000;
    sim.BroadcastInterval=5;
    sim.UnicastInterval=10;
    sim.NumNodes=100;

    if(argc>=2)sim.StopTime( atof(argv[1]));
    if(argc>=3)sim.NumNodes=atoi(argv[2]);
    if(argc>=4)sim.MaxX=sim.MaxY=atof(argv[3]);

    printf("StopTime: %.1f, NumNodes: %d, Terrain: %.0f by %.0f\n", 
	   sim.StopTime(), sim.NumNodes, sim.MaxX, sim.MaxY);

    sim.Setup();
    sim.Run();

    return 0;
}
