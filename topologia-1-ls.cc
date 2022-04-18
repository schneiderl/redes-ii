#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-list-routing-helper.h"


#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Topologia1-link-state");
Address serverAddress;

//Metodo para derrubar links
void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

int main (int argc, char **argv)
{
  bool verbose = false;
  bool printRoutingTables = false;
  bool showPings = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.AddValue ("printRoutingTables", "Print routing tables at 30, 60 and 90 seconds", printRoutingTables);
  cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnableAll (LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE));
      LogComponentEnable ("Topologia1-ls", LOG_LEVEL_INFO);
      LogComponentEnable ("Ipv4Interface", LOG_LEVEL_ALL);
      LogComponentEnable ("Icmpv4L4Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("ArpCache", LOG_LEVEL_ALL);
      LogComponentEnable ("V4Ping", LOG_LEVEL_ALL);
    }
	
  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> pcT = CreateObject<Node> ();
  Names::Add ("TNode", pcT);
  Ptr<Node> pcR = CreateObject<Node> ();
  Names::Add ("RNode", pcR);
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);
  Ptr<Node> c = CreateObject<Node> ();
  Names::Add ("RouterC", c);

  NodeContainer net1 (pcT, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (b, c);
  NodeContainer net4 (c, pcR);
  NodeContainer routers (a, b, c);
  NodeContainer nodes (pcT, pcR);

  // Enable OLSR
  NS_LOG_INFO ("Enabling OLSR Routing.");
  OlsrHelper olsr;

  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false);
  internet.SetRoutingHelper (list); 
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false);
  internetNodes.Install (nodes);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer ndc1 = p2p.Install (net1);
  
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer ndc2 = p2p.Install (net2);
  
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("50ms"));
  NetDeviceContainer ndc3 = p2p.Install (net3);
  
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  NetDeviceContainer ndc4 = p2p.Install (net4);
  
  NS_LOG_INFO ("Assign IPv4 Addresses.");
  Ipv4AddressHelper ipv4;
  
  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.3.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);
  serverAddress = Address(iic4.GetAddress (1));
  
  Ptr<Ipv4StaticRouting> staticRouting2;
  staticRouting2 = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (pcT->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting2->SetDefaultRoute ("10.0.0.2", 1 );
  staticRouting2 = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (pcR->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting2->SetDefaultRoute ("10.0.3.1", 1 );
	
  NS_LOG_INFO ("Create Applications.");
/*   uint32_t packetSize = 1024;
   Time interPacketInterval = Seconds (1.0);
   V4PingHelper ping ("10.0.3.2");	
  
   ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
   ping.SetAttribute ("Size", UintegerValue (packetSize));
   if (showPings)
     {
       ping.SetAttribute ("Verbose", BooleanValue (true));
     }
   ApplicationContainer apps = ping.Install (pcT);
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (110.0));*/


// Enable UDP nodeT -> nodeR
// Create a UdpEchoServer application on node T	
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (pcR);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));

// Create a UdpEchoClient application to send UDP datagrams from node T to node R
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds (1.0);
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (pcT);

// Gravando o ping de T
  V4PingHelper ping ("10.0.3.2");	
  ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping.SetAttribute ("Size", UintegerValue (packetSize));
  if (showPings)
    {
      ping.SetAttribute ("Verbose", BooleanValue (true));
    }
  apps = ping.Install (pcT);

  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (110.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("Topologia1-ls.tr"));
  p2p.EnablePcapAll ("Topologia1-link-state", true);
	
  /* Derrubando a conexao entre os links T e A */
  Simulator::Schedule (Seconds (40), &TearDownLink, pcT, a, 1, 0);	
  
  /* Now, do the actual simulation. */
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (131.0));
  

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);


  AnimationInterface anim ("animation_top1-ls.xml");

  Ptr<ConstantPositionMobilityModel> s1 = pcT->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s2 = pcR->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s3 = a->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s4 = b->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s5 = c->GetObject<ConstantPositionMobilityModel> ();

  s1->SetPosition (Vector ( 10.0,50,0.0  ));
  s3->SetPosition (Vector ( 30.0,50.0,0.0  ));
  s4->SetPosition (Vector ( 50.0,50.0,0.0  ));
  s5->SetPosition (Vector ( 70.0,50.0,0.0  ));
  s2->SetPosition (Vector ( 90.0,50.0,0.0  ));

  anim.UpdateNodeDescription(pcT, "T");
  anim.UpdateNodeDescription(pcR, "R");
  anim.UpdateNodeDescription(a, "Router A");
  anim.UpdateNodeDescription(b, "Router B");
  anim.UpdateNodeDescription(c, "Router C");
	
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
