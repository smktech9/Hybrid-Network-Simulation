#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TCPWirelessProgram");

int main(int argc, char* argv[])
{

	//declare number of wifi devices per base station
	uint32_t nWifi = 1;

	//declare variables for plotting throughput and fairness
	std::string graphicsFileName1, graphicsFileNameFairness;
	std::string plotFileName1, plotFileNameFairness;
	std::string plotTitle;
	std::string dataTitle;

	//create datasets for throughput and fairness graphs
	Gnuplot2dDataset dataset1, datasetFairness;
	dataTitle = "Throughput";
    
	// Instantiate the dataset, set its title, and make the points be
	// plotted along with connecting lines. 
	dataset1.SetTitle(dataTitle);
	dataset1.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	//create filenames for throughput and fairness graphs
	datasetFairness.SetTitle("Fairness");
	datasetFairness.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    
	//set extensions for filename
	std::string fileNameWithNoExtension1 = "Wireless-Throughput";
	std::string fileNameWithNoExtensionFairness = "Wireless-Fairness";
    
	graphicsFileName1 = fileNameWithNoExtension1 + ".png";
	plotFileName1 = fileNameWithNoExtension1 + ".plt";
	plotTitle = "Throughput Vs Packet Sizes";

	graphicsFileNameFairness = fileNameWithNoExtensionFairness + ".png";
	plotFileNameFairness = fileNameWithNoExtensionFairness + ".plt";
   
	//store packet sizes required in an array
	uint32_t packetSize[10] = {40, 44, 48, 52, 60, 552, 576, 628, 1420, 1500};
	std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;
	for(int j = 0 ; j < 10 ; j++)
	{
		//Configure TCP Segment Size
		Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize[j]));

		//create the 2 nodes with point-to-point connection
		NodeContainer p2pNodes;
		p2pNodes.Create (2);

		//declare helper for creating point-to-point link and set data rate and delay values given in assignment
		PointToPointHelper pointToPoint;
		pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
		pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ms"));

		//Install the devices as end points of the point-to-point channel
		NetDeviceContainer p2pDevices;
		p2pDevices = pointToPoint.Install (p2pNodes);

		//Create tcp source node
		NodeContainer wifiStaNode0;
		wifiStaNode0.Create (nWifi);

		//Create tcp sink node
		NodeContainer wifiStaNode1;
		wifiStaNode1.Create (nWifi);

		//Create the base station nodes using the point-to-point nodes
		NodeContainer wifiApNodeBS1 = p2pNodes.Get (0);
		NodeContainer wifiApNodeBS2 = p2pNodes.Get (1);

		//Create channel and phy layer helper for both base stations
		YansWifiChannelHelper channelBS1 = YansWifiChannelHelper::Default ();
		YansWifiPhyHelper phyBS1 = YansWifiPhyHelper();

		YansWifiChannelHelper channelBS2 = YansWifiChannelHelper::Default ();
		YansWifiPhyHelper phyBS2 = YansWifiPhyHelper();

		phyBS1.SetChannel (channelBS1.Create ());
		phyBS2.SetChannel (channelBS2.Create ());

		//create service set identifiers for both base stations
		WifiMacHelper macBS1;
		Ssid ssidBS1 = Ssid ("ns-3-ssid");

		WifiMacHelper macBS2;
		Ssid ssidBS2 = Ssid ("ns-3-ssid");

		//Create wifi helper and set wifi standard    
		WifiHelper wifiBS1;
		WifiHelper wifiBS2;

		wifiBS1.SetStandard (WIFI_STANDARD_80211n);
		wifiBS2.SetStandard (WIFI_STANDARD_80211n);

		//configure station manager
		wifiBS1.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));
		wifiBS2.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));

		//Configure mac layer values for wifi nodes
		macBS1.SetType ("ns3::StaWifiMac",
		"Ssid", SsidValue (ssidBS1),
		"ActiveProbing", BooleanValue (false));

		macBS2.SetType ("ns3::StaWifiMac",
		"Ssid", SsidValue (ssidBS2),
		"ActiveProbing", BooleanValue (false));

		// Set channel width
		//Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (2000));

		//install wifi models to both the wifi nodes
		NetDeviceContainer staDevice0;
		staDevice0 = wifiBS1.Install (phyBS1, macBS1, wifiStaNode0);

		NetDeviceContainer staDevice1;
		staDevice1 = wifiBS2.Install (phyBS2, macBS1, wifiStaNode1);

		//Configure mac layer values for base stations nodes, setting them as access point(AP) nodes
		macBS1.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidBS1));
		macBS2.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidBS2));

		//install wifi models to both the wifi base stations
		NetDeviceContainer apDevicesBS1;
		apDevicesBS1 = wifiBS1.Install (phyBS1, macBS2, wifiApNodeBS1);

		NetDeviceContainer apDevicesBS2;
		apDevicesBS2 = wifiBS2.Install (phyBS2, macBS2, wifiApNodeBS2);

		//Set the location of all nodes using MobilityHelper and keep them steady
		MobilityHelper mobility;

		mobility.SetPositionAllocator ("ns3::GridPositionAllocator", 
										"MinX", 
										DoubleValue (0.0), 
										"MinY", DoubleValue (0.0), 
										"DeltaX", DoubleValue (5.0),
										"DeltaY", DoubleValue (10.0), 
										"GridWidth", UintegerValue (4), 
										"LayoutType", 
										StringValue ("RowFirst"));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (wifiStaNode0);
		mobility.Install (wifiApNodeBS1);
		mobility.Install (wifiApNodeBS2);
		mobility.Install (wifiStaNode1);

		//install internet stack on all nodes
		InternetStackHelper stack;
		stack.Install (wifiApNodeBS1);
		stack.Install (wifiStaNode0);
		stack.Install (wifiApNodeBS2);
		stack.Install (wifiStaNode1);

		//assign ipv4 addresses to all devices
		Ipv4AddressHelper address;

		address.SetBase ("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer p2pInterfaces;
		p2pInterfaces = address.Assign (p2pDevices);

		address.SetBase ("10.1.2.0", "255.255.255.0");
		Ipv4InterfaceContainer wifiInterfaceContainer0 = address.Assign (staDevice0);
		address.Assign (apDevicesBS1);

		address.SetBase ("10.1.3.0", "255.255.255.0");
		Ipv4InterfaceContainer wifiInterfaceContainer1 = address.Assign (staDevice1);
		address.Assign (apDevicesBS2);

		//use global routing and create routing tables
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		//ps stores current packet size
		uint32_t ps = packetSize[j];

		//set tcp congestion control algorithm as Westwood for source node n0
		TypeId tid = TypeId::LookupByName("ns3::TcpWestwood");
		std::stringstream nodeId;
		nodeId << wifiStaNode0.Get(0)->GetId();
		std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
		Config::Set(specificNode, TypeIdValue(tid));
	    
		//use 8080 as sink port	    
		uint16_t sinkPort = 8080;
	
		//create socket and tcp sink application and install it on node n1	    
		Address sinkAddress (InetSocketAddress(wifiInterfaceContainer1.GetAddress (0), sinkPort));
		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps = packetSinkHelper.Install (wifiStaNode1.Get (0));
		sinkApps.Start (Seconds (0.));

		//create and install tcp source application with congestion control algorithm as Westwood on node n2 using our custom made application class "MyApp"
		OnOffHelper serverHelper1 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper1.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
		//serverHelper1.SetAttribute ("MaxBytes", UintegerValue(5*ps));

		ApplicationContainer serverApp1 = serverHelper1.Install (wifiStaNode0.Get(0));

		//set tcp congestion control algorithm as Veno for source node n0
		tid = TypeId::LookupByName("ns3::TcpVeno");
		Config::Set(specificNode, TypeIdValue(tid));

		//create and install tcp source application with congestion control algorithm as Veno on node n0 using our custom made application class "MyApp"
		OnOffHelper serverHelper2 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper2.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper2.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
		//serverHelper2.SetAttribute ("MaxBytes", UintegerValue(5*ps));

		ApplicationContainer serverApp2 = serverHelper2.Install (wifiStaNode0.Get(0));

		//set tcp congestion control algorithm as Vegas for source node n0
		tid = TypeId::LookupByName("ns3::TcpVegas");
		Config::Set(specificNode, TypeIdValue(tid));

		//create and install tcp source application with congestion control algorithm as Vegas on node n0 using our custom made application class "MyApp"
		OnOffHelper serverHelper3 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper3.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper3.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
		//serverHelper3.SetAttribute ("MaxBytes", UintegerValue(5*ps));

		ApplicationContainer serverApp3 = serverHelper3.Install (wifiStaNode0.Get(0));

		//Start all applications
		serverApp1.Start (Seconds (10.0));
		serverApp2.Start (Seconds (10.0));
		serverApp3.Start (Seconds (10.0));

		//create flow monitor
		Ptr<FlowMonitor> flowMonitor;
		FlowMonitorHelper flowHelper;
		flowMonitor = flowHelper.InstallAll();

		/*
		AnimationInterface anim("wireless.xml");
		anim.SetConstantPosition(wifiStaNode0.Get(0), 0.0, 0.0);
		anim.SetConstantPosition(wifiApNodeBS1.Get(0), 5.0, 0.0);
		anim.SetConstantPosition(wifiApNodeBS2.Get(0), 10.0, 0.0);
		anim.SetConstantPosition(wifiStaNode1.Get(0), 15.0, 0.0);
		*/

		Simulator::Stop(Seconds(11.0));
		Simulator::Run();
		    
		//obtain the flow statistics from flow monitor		    
		FlowMonitor :: FlowStatsContainer fsc = flowMonitor->GetFlowStats();
		if(fsc.empty()) std::cout << "No flow";
		FlowMonitor :: FlowStatsContainerI it;
		//uint32_t flowID;

		//throughput is calculated as (number of bits received by sink) / (time when last packet was received - time when first packet was transmitted)
		//jain's fairness index = (sum of throughputs)^2 / (number of flows * sum of squares of throughputs)
		uint32_t flowID;
		double avg_throughput = 0.0;
		double throughput_sum = 0.0, throughput_square_sum = 0.0;

		std::cout << "Packet-Size : " << ps << std::endl<< std::endl;
		for(it = fsc.begin() ; it != fsc.end(); it++)
		{
			FlowMonitor :: FlowStats fs = it->second;
			flowID = it->first;
			uint64_t recv_bytes = fs.rxBytes;
			Time t = fs.timeFirstTxPacket;
			double t1 = t.GetSeconds();
			t = fs.timeLastRxPacket;
			double t2 = t.GetSeconds();
			    
			double throughput = (recv_bytes * 8.0) / (t2-t1);
			throughput = throughput / 1024;
			throughput_sum += throughput;
			throughput_square_sum += throughput * throughput;
			    
			if(flowID == 1 || flowID == 2 || flowID == 3) 
			{	
				std::cout << "n0 ---> n1" << std::endl;
			}
			else 
			{
				std::cout << "n1 ---> n0" << std::endl;
			}
			if(flowID == 1 || flowID == 2) 
			{	
				std::cout << "TCP Flow Type : Westwood" << std::endl;
			}
			if(flowID == 3 || flowID == 4) 
			{	
				std::cout << "TCP Flow Type : Veno" << std::endl; 
			}
			if(flowID == 5 || flowID == 6) 
			{	
				std::cout << "TCP Flow Type : Vegas" << std::endl;  
			}
			std::cout << "Throughput : " << throughput << " Kbps"<< std::endl; 
			std::cout << "Recevied Bytes : " << recv_bytes << std::endl; 
			std::cout << "Time : " << t2-t1 << " s" << std::endl<< std::endl; 
			    
		}
		avg_throughput = throughput_sum / 6.0;
		dataset1.Add(packetSize[j], avg_throughput);
		    
		double fairness = (throughput_sum * throughput_sum) / (6.0 * throughput_square_sum);
		datasetFairness.Add(packetSize[j], fairness);

		std :: cout << "Average Throughput: " << avg_throughput << " Kbps" << std::endl;
		std :: cout << "FairnessIndex: " << fairness << std::endl;
    		    
		std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;
		//std::cout << "Time Up. Stopped Application\n" ;
		    	    
		Simulator::Destroy ();
	}
  
	// Instantiate the plot and set its title.
	Gnuplot plot1(graphicsFileName1);
	plot1.SetTitle(plotTitle);

	// Make the graphics file, which the plot file will create when it
	// is used with Gnuplot, be a PNG file.
	plot1.SetTerminal("png");

	// Set the labels for each axis.
	plot1.SetLegend("Packet Size(Bytes)", "Throughput(KBps)");

	// Set the range for the x axis.
	plot1.AppendExtra("set xrange [0:1500]");
	    
	// Add the dataset to the plot.
	plot1.AddDataset(dataset1);

	// Open the plot file.
	std::ofstream plotFile1(plotFileName1.c_str());

	// Write the plot file.
	plot1.GenerateOutput(plotFile1);

	// Close the plot file.
	plotFile1.close();
	    
	// Instantiate the plot and set its title.
	Gnuplot plot4(graphicsFileNameFairness);
	plot4.SetTitle("Fairness Vs Packet Size");

	// Make the graphics file, which the plot file will create when it
	// is used with Gnuplot, be a PNG file.
	plot4.SetTerminal("png");

	// Set the labels for each axis.
	plot4.SetLegend("Packet Size(Bytes)", "Fairness Index");

	// Set the range for the x axis.
	plot4.AppendExtra("set xrange [0:1500]");
	// Add the dataset to the plot.
	plot4.AddDataset(datasetFairness);

	// Open the plot file.
	std::ofstream plotFile4(plotFileNameFairness.c_str());

	// Write the plot file.
	plot4.GenerateOutput(plotFile4);

	// Close the plot file.
	plotFile4.close();
	return 0;
}
