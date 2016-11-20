/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA    02111-1307    USA
 */

#include <list>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"

#define MAX_NODES 40
#define NODES_PER_UPDATE 5
#define CLIENT_START_TIME 2
#define CLIENT_UPDATE_TIME 5

// Application setings
#define F_CBR 1
#define F_BURST 2
#define SERVER_PORT 9
#define CBR_PACKET_SIZE 512
#define BURST_PACKET_SIZE 1500

/*
    Alexandre R Gomes RA: 191071
    Jaime Fabian
*/
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("mo655");

void ShowNodeInformation(NodeContainer in_c, size_t in_numOfNode)
{
  for(size_t i=0; i<in_numOfNode; ++i){
    Ptr<MobilityModel> mobility = in_c.Get(i)->GetObject<MobilityModel> ();
    Vector nodePos = mobility->GetPosition ();
    // Get Ipv4 instance of the node
    Ptr<Ipv4> ipv4 = in_c.Get(i)->GetObject<Ipv4> (); 
    // Get Ipv4 instance of the node
    Ptr<MacLow> mac48 = in_c.Get(i)->GetObject<MacLow> (); 
    // Get Ipv4InterfaceAddress of xth interface.
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();     
    //Mac48Address macAddr = mac48->GetAddress();
    std::cout << in_c.Get(i)->GetId() << " " << addr << " (" << nodePos.x << ", " <<     nodePos.y << ")" << std::endl;
  }
}

void throughput(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier) {
    // Do the calculations
    monitor->CheckForLostPackets ();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    
    double nearNodeTP = 0, nearNodeDelay = 0, nearNodeLost =0;
    double farNodeTP = 0, farNodeDelay = 0, farNodeLost = 0;
    double accumulatedThroughput = 0, execution_time = 0;
    double totalTx = 0, totalLost = 0, totalDelay = 0;
    int countNodes = 0;
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin(); 
        i!=stats.end();
        ++i) {
        
        //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        //std::cout << "\n Flow " << i->first<< " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        //std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        //std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        //std::cout << "  Time online: " << nodeTimeOnline << "\n";
        //std::cout << "  Tx Packets: " << i->second.txPackets << "\n";   // Transmitted bytes
        //std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";   // Received bytes
        //std::cout << "  Lost Packets: " <<  i->second.txPackets -  i->second.rxPackets << "\n"; // ...
        //std::cout << "  Pkt Lost Ratio: " << ((double)i->second.rxPackets-(double)i->second.rxPackets)/(double)i->second.txPackets << "\n";
        //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (nodeTimeOnline * 1e3)  << " Kbps\n";
        //std::cout << "  Delay: " <<  i->second.delaySum << "ms\n";

        double nodeTimeOnline = i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
        double lost = (double)i->second.txPackets - (double)i->second.rxPackets;
        double delay = i->second.SumDelay.GetMilliSeconds();
        //double delay2 = i->second.lastDelay.GetMilliSeconds();
        
        if (i->first == 1) {
            nearNodeTP += i->second.rxBytes * 8.0;
            nearNodeDelay = delay;
            nearNodeLost = lost;
            
            execution_time = nodeTimeOnline;
        } else if (i->first == 2) {
            farNodeTP += i->second.rxBytes * 8.0;
            farNodeDelay = delay;
            farNodeLost = lost;
        }
        accumulatedThroughput += i->second.rxBytes * 8.0;
        totalTx += (double)i->second.txPackets;
        totalLost += lost;
        totalDelay += delay;
        countNodes++;
    }
    nearNodeTP = nearNodeTP / (execution_time * 1e3 );
    farNodeTP = farNodeTP / (execution_time * 1e3 );
    
    accumulatedThroughput = accumulatedThroughput / (execution_time * 1e3 );
    totalDelay = totalDelay / countNodes;
    double lostRatio = totalLost / totalTx;
    
    std::cout << execution_time << ",";
    std::cout << nearNodeTP << "," << nearNodeDelay << "," << nearNodeLost << ","; 
    std::cout << farNodeTP << "," << farNodeDelay << "," << farNodeLost << ",";
    std::cout << accumulatedThroughput << "," << totalDelay << "," << lostRatio << "\n" << std::flush;
    //std::cout << "===========================\n" << std::flush;
    //std::cout << "throughput=" << accumulatedThroughput << "Kbps\n" << std::flush;
    //std::cout << "near=" << nearNodeTP << "Kbps\n" << std::flush;
    //std::cout << "far=" << farNodeTP << "Kbps\n" << std::flush;
    //std::cout << "===========================\n" << std::flush;

    // Reschedule the method after 1sec
    Simulator::Schedule(Seconds(1.0), &throughput, monitor, classifier);
}



int 
main (int argc, char *argv[])
{
    int nClients = MAX_NODES;
    int traffic = F_CBR;
    bool bMobility = false;
    bool verbose = true;
    bool tracing = false;
    
    CommandLine cmd;
    cmd.AddValue ("nClients", "Number of wifi STA devices", nClients);
    //cmd.AddValue ("nContainers", "Number of ApplicationsContainers", nContainers);
    cmd.AddValue ("bMobility", "Enable/Disable mobility [True/False]", bMobility);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
    cmd.AddValue ("traffic", "Traffic type: 1 - CBR; 2 - Burst.", traffic);

    cmd.Parse (argc,argv);
       int nContainers = nClients / NODES_PER_UPDATE;
       int execution_time = 6 + nContainers * 5;
    // Check for valid number of csma or wifi nodes
    // 250 should be enough, otherwise IP addresses 
    // soon become an issue
    if (nClients > 40 ){
        std::cout << "Too many wifi or csma nodes, no more than 40 each." << std::endl;
        return 1;
    }
    
    if (nClients % 5 != 0){
        std::cout << "nClients is not divisible by 5." << std::endl;
        return 1;
    }
        
    if (verbose){
        //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
    }
    
    // Creating NodeContainers: Hosts, Server and AP.
    NodeContainer wifiClientNodes;
    wifiClientNodes.Create (nClients);
    
    NodeContainer wifiServerNode;
    wifiServerNode.Create (1);

    NodeContainer wifiApNode;
    wifiApNode.Create (1);

    // Create a channel helper in a default working state:
    // By default, we create a channel model with a propagation delay
    // equal to a constant, the speed of light,
    // and a propagation loss based on a log distance model with a 
    // reference loss of 46.6777 dB at reference distance of 1m.
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    // Make it easy to create and manage PHY objects for the yans model.
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    // A Yans wifi channelThis wifi channel implements the propagation model
    // described in "Yet Another Network Simulator"
    phy.SetChannel (channel.Create () );

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("MO655");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

    // Installing wifi network devices
    NetDeviceContainer clientDevices;
    clientDevices = wifi.Install (phy, mac, wifiClientNodes);

    // Chang to Point to Point
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

    NetDeviceContainer p2pdevices;
    p2pdevices = pointToPoint.Install (wifiServerNode.Get(0), wifiApNode.Get(0));
    
    //NetDeviceContainer serverDevice;
    //serverDevice = wifi.Install (phy, mac, wifiServerNode);
    //serverDevice = p2pdevices.Get(0);

    // Set to AP type and install
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);

    
    // 2) Mobilidade
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> initialAlloc = 
    CreateObject<ListPositionAllocator> ();
    initialAlloc->Add (Vector (25., 25., 0.) );
    initialAlloc->Add (Vector (26., 25., 0.) );
    initialAlloc->Add (Vector (26., 26., 0.) );
    initialAlloc->Add (Vector (0., 0., 0.) );
    mobility.SetPositionAllocator(initialAlloc);

    //srand(time(NULL));
    //mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
    //                             "X", StringValue ("ns3::UniformRandomVariable[Min=20.0|Max=30.0]"),
    //                             "Y", StringValue ("ns3::UniformRandomVariable[Min=20.0|Max=30.0]"));
    // Set mobility to Constant position
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);
    mobility.Install (wifiServerNode);
    mobility.Install (wifiClientNodes.Get(0) );
    mobility.Install (wifiClientNodes.Get(1) );

    mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=49.0]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=49.0]"));
    // If mobility is [On]
    if (bMobility) {
        mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
            "Bounds", RectangleValue (Rectangle (1, 49, 1, 49)),
            "Speed", StringValue("ns3::UniformRandomVariable[Min=0.6|Max=1]") ); // 0.6m/s=2.16km/h, 1m/s=3.6km/h
    }
//    mobility.Install (wifiClientNodes);
    
    // For the othes clients (except nearest and farest
    for (int idx=2; idx < nClients; idx++) {
        mobility.Install (wifiClientNodes.Get(idx) );
    }

    // Aggregate IP/TCP/UDP functionality to existing Nodes.
    InternetStackHelper stack;
    stack.Install (wifiApNode);
    stack.Install (wifiClientNodes);
    stack.Install (wifiServerNode);

    // IPv4
    Ipv4AddressHelper address;

    // Wifi network
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer clientInterfaces;
    clientInterfaces = address.Assign (clientDevices);
    address.Assign (apDevices);
    
    // P2P 
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer serverInterface;
    serverInterface = address.Assign (p2pdevices);

    // Application {
    // Setting up traffic application by type
    if (traffic == F_CBR) {
        std::cout << "[!] CBR Traffic\n";
        //Trafego CBR - UDP
        PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", Address ());;
        OnOffHelper appClientOnOff ("ns3::UdpSocketFactory", Address());
        
        //packetSinkHelper.SetAttribute("Protocol",StringValue("ns3::UdpSocketFactory"));
        packetSinkHelper.SetAttribute ("Local", AddressValue (InetSocketAddress (serverInterface.GetAddress (1), SERVER_PORT)));
        ApplicationContainer servidorUdp;
        servidorUdp = packetSinkHelper.Install(wifiServerNode);
        
        //appClientOnOff.SetAttribute("Protocol", StringValue("ns3::UdpSocketFactory"));
        appClientOnOff.SetAttribute("DataRate", DataRateValue(DataRate("256Kbps")));
        appClientOnOff.SetAttribute("PacketSize", UintegerValue(CBR_PACKET_SIZE));
        appClientOnOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=10]"));
        appClientOnOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        appClientOnOff.SetAttribute("Remote", AddressValue(InetSocketAddress(serverInterface.GetAddress (1), SERVER_PORT)));
        
        for (int i=0; i < nContainers; i++) {
     
            for (int j=0; j < NODES_PER_UPDATE; j++){
                ApplicationContainer tmpClientApps = 
                    appClientOnOff.Install (wifiClientNodes.Get(i * 5 + j));

                int l_time = CLIENT_START_TIME + i * CLIENT_UPDATE_TIME;
                
                tmpClientApps.Start (Seconds (l_time));
                tmpClientApps.Stop (Seconds (execution_time));
            }
        }
    }
    else if (traffic == F_BURST) {
        std::cout << "[!] Burst Traffic \n";
        //Trafego Rajadas - TCP
        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", Address ());;
        OnOffHelper appClientOnOff ("ns3::TcpSocketFactory", Address());
         
        packetSinkHelper.SetAttribute ("Local", AddressValue (InetSocketAddress (serverInterface.GetAddress (0), SERVER_PORT)));
        ApplicationContainer servidorTcp;    
        servidorTcp = packetSinkHelper.Install(wifiServerNode);
     
        appClientOnOff.SetAttribute("DataRate", DataRateValue(DataRate("512kbps")));
        appClientOnOff.SetAttribute("PacketSize", UintegerValue(BURST_PACKET_SIZE));
        appClientOnOff.SetAttribute("OnTime", StringValue("ns3::NormalRandomVariable[Mean=5.|Variance=2.|Bound=5.]"));
        appClientOnOff.SetAttribute("OffTime", StringValue("ns3::NormalRandomVariable[Mean=5.|Variance=2.|Bound=5.]"));
        appClientOnOff.SetAttribute("Remote", AddressValue(InetSocketAddress(serverInterface.GetAddress (0), SERVER_PORT)));
        
        for (int i=0; i < nContainers; i++) { 
            for (int j=0; j < NODES_PER_UPDATE; j++){
                ApplicationContainer tmpClientApps = 
                    appClientOnOff.Install (wifiClientNodes.Get(i * 5 + j));

                int l_time = CLIENT_START_TIME + i * CLIENT_UPDATE_TIME;
                
                tmpClientApps.Start (Seconds (l_time));
                tmpClientApps.Stop (Seconds (execution_time));
            }
        }
    } else{
        std::cout << "Invalid traffic type.\n Use 1 for CBR or 2 for Burst.\n";
    }
    // }
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (execution_time));

    if (tracing == true){
        //pointToPoint.EnablePcapAll ("third");
        phy.EnablePcap ("third", apDevices.Get (0));
        //csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

    // FLow
    ShowNodeInformation(wifiServerNode, 1);
    ShowNodeInformation(wifiClientNodes, nClients);
    
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    
    Simulator::Schedule(Seconds(1.0), &throughput, monitor, DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier()) );
    
    Simulator::Run ();
    
    Simulator::Destroy ();
    return 0;
}
