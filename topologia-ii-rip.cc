#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Topologia1");
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
  std::string SplitHorizon ("PoisonReverse");

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.AddValue ("printRoutingTables", "Print routing tables at 30, 60 and 90 seconds", printRoutingTables);
  cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
  cmd.AddValue ("splitHorizonStrategy", "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)", SplitHorizon);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnableAll (LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE));
      LogComponentEnable ("Topologia2", LOG_LEVEL_INFO);
      LogComponentEnable ("Rip", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv4Interface", LOG_LEVEL_ALL);
      LogComponentEnable ("Icmpv4L4Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("ArpCache", LOG_LEVEL_ALL);
      LogComponentEnable ("V4Ping", LOG_LEVEL_ALL);
    }

  if (SplitHorizon == "NoSplitHorizon")
    {
      Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::NO_SPLIT_HORIZON));
    }
  else if (SplitHorizon == "SplitHorizon")
    {
      Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::SPLIT_HORIZON));
    }
  else
    {
      Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::POISON_REVERSE));
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
  Ptr<Node> d = CreateObject<Node> ();
  Names::Add ("RouterD", d);

/*    T->A,B
   A->C,D
   B->C,D
   C->R
   D->R */

  NodeContainer net1 (pcT, a);
  NodeContainer net2 (pcT, b);
  NodeContainer net3 (a, c);
  NodeContainer net4 (a, d);
  NodeContainer net5 (a, b);
  NodeContainer net6 (b, c);
  NodeContainer net7 (b, d);
  NodeContainer net8 (c, pcR);
  NodeContainer net9 (d, pcR);


  NodeContainer routers (a, b, c, d);
  NodeContainer nodes (pcT, pcR);
  
  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);
  NetDeviceContainer ndc4 = csma.Install (net4);
  NetDeviceContainer ndc5 = csma.Install (net5);
  NetDeviceContainer ndc6 = csma.Install (net6);
  NetDeviceContainer ndc7 = csma.Install (net7);
  NetDeviceContainer ndc8 = csma.Install (net8);
  NetDeviceContainer ndc9 = csma.Install (net9);


  NS_LOG_INFO ("Create IPv4 and routing");
  RipHelper ripRouting;

  // Rule of thumb:
  // Interfaces are added sequentially, starting from 0
  // However, interface 0 is always the loopback...
  ripRouting.ExcludeInterface (a, 1);
  ripRouting.ExcludeInterface (d, 4);

  Ipv4ListRoutingHelper listRH;
  listRH.Add (ripRouting, 0);
  
  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false);
  internet.SetRoutingHelper (listRH);
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false);
  internetNodes.Install (nodes);
  
  NS_LOG_INFO ("Assign IPv4 Addresses.");
  Ipv4AddressHelper ipv4;
  
  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);
  serverAddress = Address(iic1.GetAddress (1));

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.3.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);

  ipv4.SetBase (Ipv4Address ("10.0.4.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic5 = ipv4.Assign (ndc5);

  ipv4.SetBase (Ipv4Address ("10.0.5.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic6 = ipv4.Assign (ndc6);

  ipv4.SetBase (Ipv4Address ("10.0.6.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic7 = ipv4.Assign (ndc7);

  ipv4.SetBase (Ipv4Address ("10.0.7.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic8 = ipv4.Assign (ndc8);

  ipv4.SetBase (Ipv4Address ("10.0.8.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic9 = ipv4.Assign (ndc9);
  serverAddress = Address(iic9.GetAddress (1));

  Ptr<Ipv4StaticRouting> staticRouting;
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (pcT->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.4.1", 1 );
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (pcR->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.8.1", 1 );
  
  if (printRoutingTables)
    {
      RipHelper routingHelper;

      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

      routingHelper.PrintRoutingTableAt (Seconds (30.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (30.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (30.0), c, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), d, routingStream);

      routingHelper.PrintRoutingTableAt (Seconds (60.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (60.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (60.0), c, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (60.0), d, routingStream);

      routingHelper.PrintRoutingTableAt (Seconds (90.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), c, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), d, routingStream);
    }
	
  NS_LOG_INFO ("Create Applications.");
//   uint32_t packetSize = 1024;
//   Time interPacketInterval = Seconds (1.0);
//   V4PingHelper ping ("10.0.2.2");	
  
//   ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
//   ping.SetAttribute ("Size", UintegerValue (packetSize));
//   if (showPings)
//     {
//       ping.SetAttribute ("Verbose", BooleanValue (true));
//     }
//   ApplicationContainer apps = ping.Install (pcT);
//   apps.Start (Seconds (1.0));
//   apps.Stop (Seconds (110.0));


// Enable UDP nodeT -> nodeR
// Create a UdpEchoServer application on node T	
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (pcR);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));

// Create a UdpEchoClient application to send UDP datagrams from node T to node R
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1000;
  Time interPacketInterval = Seconds (1.0);
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (pcT);

  // /* Gravando o ping de T*/
  // V4PingHelper ping ("10.0.0.2");	
  // ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  // ping.SetAttribute ("Size", UintegerValue (packetSize));
  // if (showPings)
  //   {
  //     ping.SetAttribute ("Verbose", BooleanValue (true));
  //   }
  // apps = ping.Install (pcT);

  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("Topologia2-rip.tr"));
  csma.EnablePcapAll ("Topologia2-rip", true);
	
  /* Derrubando a conexao entre os links T e A */
  Simulator::Schedule (Seconds (40), &TearDownLink, b, d, 1, 0);	
  Simulator::Schedule (Seconds (40), &TearDownLink, a, c, 1, 0);

  /* Now, do the actual simulation. */
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (131.0));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);
  
  AnimationInterface anim ("animation_top2.xml");

  anim.UpdateNodeDescription(pcT, "T");
  anim.UpdateNodeDescription(pcR, "R");
  anim.UpdateNodeDescription(a, "Router A");
  anim.UpdateNodeDescription(b, "Router B");
  anim.UpdateNodeDescription(c, "Router C");
  anim.UpdateNodeDescription(d, "Router D");

  Ptr<ConstantPositionMobilityModel> s1 = pcT->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s2 = pcR->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s3 = a->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s4 = b->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s5 = c->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s6 = d->GetObject<ConstantPositionMobilityModel> ();


  s1->SetPosition (Vector ( 0.0,47.5,0.0  ));
  s2->SetPosition (Vector ( 60.0,47.5,0.0  ));
  s3->SetPosition (Vector ( 20.0,22.0,0.0  ));
  s4->SetPosition (Vector ( 20.0,66.0,0.0  ));
  s5->SetPosition (Vector ( 40.0,22.0,0.0  ));
  s6->SetPosition (Vector ( 40.0,66.5,0.0  ));
	
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
