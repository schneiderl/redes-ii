#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RipSimpleRouting");
Address serverAddress;
//Metodo para derrubar links
void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

void UpLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetUp (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetUp (interfaceB);
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
      LogComponentEnable ("topologia-i-rip", LOG_LEVEL_INFO);
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
  Ptr<Node> src = CreateObject<Node> ();
  Names::Add ("SrcNode", src);
  Ptr<Node> dst = CreateObject<Node> ();
  Names::Add ("DstNode", dst);
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);
  Ptr<Node> c = CreateObject<Node> ();
  Names::Add ("RouterC", c);
  NodeContainer net1 (src, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (b, c);
  NodeContainer net4 (b, dst);
  NodeContainer routers (a, b, c);
  NodeContainer nodes (src, dst);
  
  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);
  NetDeviceContainer ndc4 = csma.Install (net4);

  NS_LOG_INFO ("Create IPv4 and routing");
  RipHelper ripRouting;

  // Rule of thumb:
  // Interfaces are added sequentially, starting from 0
  // However, interface 0 is always the loopback...
  ripRouting.ExcludeInterface (a, 1);
  ripRouting.ExcludeInterface (c, 2);
  
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
  

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.3.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);
  serverAddress = Address(iic4.GetAddress (1));
  
  Ptr<Ipv4StaticRouting> staticRouting;
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (src->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.0.2", 1 );
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (dst->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.3.1", 1 );
  
      RipHelper routingHelper;

      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

      routingHelper.PrintRoutingTableAt (Seconds (30.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (30.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (30.0), c, routingStream);

      routingHelper.PrintRoutingTableAt (Seconds (60.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (60.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (60.0), c, routingStream);

      routingHelper.PrintRoutingTableAt (Seconds (90.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (90.0), c, routingStream);
	
  NS_LOG_INFO ("Create Applications.");
  // uint32_t packetSize = 1024;
  // Time interPacketInterval = Seconds (1.0);
  // V4PingHelper ping ("10.0.2.2");	
  
  // ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  // ping.SetAttribute ("Size", UintegerValue (packetSize));
  // if (showPings)
  //   {
  //     ping.SetAttribute ("Verbose", BooleanValue (true));
  //   }
  // ApplicationContainer apps = ping.Install (src);
  // apps.Start (Seconds (1.0));

// Enable UDP nodeT -> nodeR
// Create a UdpEchoServer application on node T	
  NS_LOG_INFO ("Create UdpServer application on node R.");
  uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps = server.Install (dst);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));
 
  NS_LOG_INFO ("Create UdpClient application on node 0 to send to node T.");
  uint32_t MaxPacketSize = 1024;
  Time interPacketInterval = Seconds (1);
  uint32_t maxPacketCount = 1000;
  UdpClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (src);

// Gravando o ping de T
  V4PingHelper ping ("10.0.3.2");	
  ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping.SetAttribute ("Size", UintegerValue (MaxPacketSize));
  if (showPings)
    {
      ping.SetAttribute ("Verbose", BooleanValue (true));
    }
  apps = ping.Install (src);


  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("topologia-i-rip.tr"));
  csma.EnablePcapAll ("topologia-i-rip", true);

  Simulator::Schedule (Seconds (40), &TearDownLink, src, a, 1, 0);	


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);


  AnimationInterface anim ("animation_top1.xml");

  Ptr<ConstantPositionMobilityModel> s1 = src->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s2 = dst->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s3 = a->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s4 = b->GetObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> s5 = c->GetObject<ConstantPositionMobilityModel> ();


  s1->SetPosition (Vector ( 10.0,50,0.0  ));
  s3->SetPosition (Vector ( 30.0,50.0,0.0  ));
  s4->SetPosition (Vector ( 50.0,50.0,0.0  ));
  s5->SetPosition (Vector ( 70.0,50.0,0.0  ));
  s2->SetPosition (Vector ( 90.0,50.0,0.0  ));

  anim.UpdateNodeDescription(src, "T");
  anim.UpdateNodeDescription(dst, "R");
  anim.UpdateNodeDescription(a, "Router A");
  anim.UpdateNodeDescription(b, "Router B");
  anim.UpdateNodeDescription(c, "Router C");

  /* Now, do the actual simulation. */
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (131.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
