#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TCPWiredProgram");

//Class extending the Application class
class MyApp : public Application
{
	public:
		//constructor
		MyApp ();
		
		//destructor
		virtual ~MyApp();
    
		//function to initialize variables
		void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, 
		uint32_t nPackets, DataRate dataRate);
  
	private:
		//functions to execute when application is started
		virtual void StartApplication (void);

		//functions to execute when application is stopped
		virtual void StopApplication (void);
    
		//function to schedule packet transmission
		void ScheduleTx (void);

		//function to send packet
		void SendPacket (void);
  
		//variable declarations
		Ptr<Socket>     m_socket;
		Address         m_peer;
		uint32_t        m_packetSize;
		uint32_t        m_nPackets;
		DataRate        m_dataRate;
		EventId         m_sendEvent;
		bool            m_running;
		uint32_t        m_packetsSent;    
};

//initializing variables with default values in constructor  
MyApp::MyApp ()
: m_socket (0),
m_peer (),
m_packetSize (0),
m_nPackets (0),
m_dataRate (0),
m_sendEvent (),
m_running (false),
m_packetsSent (0)
{
}
  
//destructor definition
MyApp::~MyApp()
{
	m_socket = 0;
	//std::cout << "Destroyed";
}
  
//initializing variables with argument values
void MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
	m_nPackets = nPackets;
	m_dataRate = dataRate;
	//std::cout << "Setup Done\n" ;
}


void MyApp::StartApplication (void)
{
	//application running status is set to true
	m_running = true;
    
	//Number of packets initialized to 0
	m_packetsSent = 0;
    
	//std::cout << "Application Started\n" ;
    
	//Bind address to socket
	m_socket->Bind ();
	//std::cout << "Bind Successful\n" ;
    
	//connect with sink node
	m_socket->Connect (m_peer);
	//std::cout << "Connection Establishhed\n" ;
    
	//send packet from source to sink
	SendPacket ();
}
  
void MyApp::StopApplication (void)
{
	//set application running status to false
	m_running = false;
	//std::cout << "Stopping Application\n" ;
  
	//If some event is still running , cancel it
	if (m_sendEvent.IsRunning ())
	{
		Simulator::Cancel (m_sendEvent);
	}
    
	//close the socket
	if (m_socket)
	{
		m_socket->Close ();
	}
}
  
//function to create and send packet to sink
void MyApp::SendPacket (void)
{
	//create packet of size m_packetSize using dummy data
	Ptr<Packet> packet = Create<Packet> (m_packetSize);
    
	//send the packet
	m_socket->Send (packet);
  	
	//std::cout << "Packet Sent of size " << m_packetSize << "\n";	
    
	//if all packets are not sent, schedule the next packet
	if (++m_packetsSent < m_nPackets)
	{
		ScheduleTx ();
	}
}
  
//function to schedule packet transmission
void MyApp::ScheduleTx (void)
{
	//check if application is still running
	if (m_running)
	{
		//schedule next packet at time after previous transmission is completed
		Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
		m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
	}
}


int main(int argc, char* argv[])
{   

	//set time unit as nanoseconds
	Time::SetResolution(Time::NS);
    
	//enable logs for the given classes
	LogComponentEnable("TcpSocket", LOG_LEVEL_INFO);
	LogComponentEnable("PointToPointHelper", LOG_LEVEL_INFO);
    
	//declare variables for plotting throughput and fairness
	std::string graphicsFileName1, graphicsFileNameFairness;
	std::string plotFileName1, plotFileNameFairness;
	std::string plotTitle;
	std::string dataTitle;
    
	//create datasets for throughput and fairness graphs
	Gnuplot2dDataset dataset1, datasetFairness;
	dataTitle = "Throughput";
    
	// Instantiate the datasets, set their titles, and make the points be
	// plotted along with connecting lines. 
	dataset1.SetTitle(dataTitle);
	dataset1.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	datasetFairness.SetTitle("Fairness");
	datasetFairness.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    
	//create filenames for throughput and fairness graphs
	std::string fileNameWithNoExtension1 = "Wired-Throughput";
	std::string fileNameWithNoExtensionFairness = "Wired-Fairness";
    
	//set extensions for filename
	graphicsFileName1 = fileNameWithNoExtension1 + ".png";
	plotFileName1 = fileNameWithNoExtension1 + ".plt";
	plotTitle = "Throughput Vs Packet Sizes";
		    
	graphicsFileNameFairness        = fileNameWithNoExtensionFairness + ".png";
	plotFileNameFairness            = fileNameWithNoExtensionFairness + ".plt";
   
	//store packet sizes to be used in an array
	uint32_t packetSize[10] = {40, 44, 48, 52, 60, 552, 576, 628, 1420, 1500};
	std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;
	
	//for each packet size
	for(int j = 0 ; j < 10 ; j++)
	{		     
		//create the 4 nodes needed along with containers
		NodeContainer n2;
		n2.Create(1);
    
		NodeContainer n3;
		n3.Create(1);
    
		NodeContainer r1;
		r1.Create(1);
    
		NodeContainer r2;
		r2.Create(1);
    
		//add r1 node to n2 container
		n2.Add(r1);
    
		//declare helper for creating point-to-point link and set data rate and delay values given in assignment
		PointToPointHelper pointToPoint1;
		pointToPoint1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		pointToPoint1.SetChannelAttribute("Delay", StringValue("20ms"));
 
		//install the devices n2 and r1 as end points of the point-to-point channel
		NetDeviceContainer n2r1;
		n2r1 = pointToPoint1.Install(n2);
    
		//add r2 node to r1 container
		r1.Add(r2);
    
		//declare helper for creating point-to-point link and set data rate and delay values given in assignment
		PointToPointHelper pointToPoint2;
		pointToPoint2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		pointToPoint2.SetChannelAttribute("Delay", StringValue("50ms"));
 
		//set droptail queue with size as bandwidth-delay product
		pointToPoint2.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::BYTES, 6250)));
    
		//install the devices r1 and r2 as end points of the point-to-point channel
		NetDeviceContainer r1r2;
		r1r2 = pointToPoint2.Install(r1);
    
		//add n3 node to r2 container
		r2.Add(n3);
    
		//declare helper for creating point-to-point link and set data rate and delay values given in assignment
		PointToPointHelper pointToPoint3;
		pointToPoint3.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		pointToPoint3.SetChannelAttribute("Delay", StringValue("20ms"));
    
		//install the devices r2 and n3 as end points of the point-to-point channel
		NetDeviceContainer r2n3;
		r2n3 = pointToPoint3.Install(r2);
    
		//add r2 node to n2 container
		//this fills n2 container with all 4 nodes n2,r1,r2,n3
		n2.Add(r2);
    
		//install internet stack on all nodes
		InternetStackHelper stack;
		stack.Install(n2);
   
		//assign ipv4 addresses to all devices
		Ipv4AddressHelper address;

		address.SetBase("10.1.1.0", "255.255.255.0");

		Ipv4InterfaceContainer n2r1Interfaces = address.Assign(n2r1);
    
		address.SetBase("10.1.2.0", "255.255.255.0");

		Ipv4InterfaceContainer r1r2Interfaces = address.Assign(r1r2);
    
		address.SetBase("10.1.3.0", "255.255.255.0");

		Ipv4InterfaceContainer r2n3Interfaces = address.Assign(r2n3);

		//use global routing and create routing tables
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    
		//ps stores current packet size
		uint32_t ps = packetSize[j];
		    	
		//set tcp congestion control algorithm as Westwood for source node n2	    	
		TypeId tid = TypeId::LookupByName("ns3::TcpWestwood");
		std::stringstream nodeId;
		nodeId << n2.Get(0)->GetId();
		std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
		Config::Set(specificNode, TypeIdValue(tid));

		//use sink port as 8080	    
		uint16_t sinkPort = 8080;
		    
		//create socket and tcp sink application and install it on node n3
		Address sinkAddress (InetSocketAddress(r2n3Interfaces.GetAddress (1), sinkPort));
		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps = packetSinkHelper.Install (n3.Get (0));
		sinkApps.Start (Seconds (0.));
		sinkApps.Stop (Seconds (10.));
		    
		  
		//create socket for tcp source node n2 with congestion control algorithm as Westwood
		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ());
		    
		//create and install tcp source application with congestion control algorithm as Westwood on node n2 using our custom made application class "MyApp"
		Ptr<MyApp> app = CreateObject<MyApp> ();
		app->Setup (ns3TcpSocket, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app);
		app->SetStartTime(Seconds(1.0));
		app->SetStopTime(Seconds(10.0));
		    
		//set tcp congestion control algorithm as Veno for source node n2
		tid = TypeId::LookupByName("ns3::TcpVeno");
		Config::Set(specificNode, TypeIdValue(tid));
		    
		   
		//create socket for tcp source node n2 with congestion control algorithm as Veno
		Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ());
		    
		//create and install tcp source application with congestion control algorithm as Veno on node n2 using our custom made application class "MyApp"
		Ptr<MyApp> app2 = CreateObject<MyApp> ();
		app2->Setup (ns3TcpSocket2, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app2);
		app2->SetStartTime(Seconds(1.0));
		app2->SetStopTime(Seconds(10.0));
		    
		//set tcp congestion control algorithm as Vegas for source node n2
		tid = TypeId::LookupByName("ns3::TcpVegas");
		Config::Set(specificNode, TypeIdValue(tid));
		    
		//create socket for tcp source node n2 with congestion control algorithm as Vegas
		Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ());
		    
		//create and install tcp source application with congestion control algorithm as Vegas on node n2 using our custom made application class "MyApp" 
		Ptr<MyApp> app3 = CreateObject<MyApp> ();
		app3->Setup (ns3TcpSocket3, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app3);
		app3->SetStartTime(Seconds(1.0));
		app3->SetStopTime(Seconds(10.0));
		       
		//create flow monitor
		Ptr<FlowMonitor> flowMonitor;
		FlowMonitorHelper flowHelper;
		flowMonitor = flowHelper.InstallAll();
		    
		/*
		Ptr< Node > n2ptr = n2.Get(0);
		Ptr< Node > n3ptr = n3.Get(0);
		Ptr< Node > r1ptr = r1.Get(0);
		Ptr< Node > r2ptr = r2.Get(0);
		AnimationInterface anim("wired.xml");
		anim.SetConstantPosition(n2ptr, 0.0, 0.0);
		anim.SetConstantPosition(r1ptr, 2.0, 0.0);
		anim.SetConstantPosition(r2ptr, 4.0, 0.0);
		anim.SetConstantPosition(n3ptr, 6.0, 0.0);
		*/
		Simulator::Stop(Seconds(15.0));
		    
    	Simulator::Run();
    		    
    	//obtain the flow statistics from flow monitor
		FlowMonitor :: FlowStatsContainer fsc = flowMonitor->GetFlowStats();
		if(fsc.empty()) std::cout << "No flow";
		FlowMonitor :: FlowStatsContainerI it;
		    
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
			if(flowID == 1 || flowID == 2 || flowID == 3) std::cout << "n0 ---> n1" << std::endl;
			else std::cout << "n1 ---> n0" << std::endl;
			if(flowID == 1 || flowID == 2) std::cout << "TCP Flow Type : Westwood" << std::endl;
			if(flowID == 3 || flowID == 4) std::cout << "TCP Flow Type : Veno" << std::endl; 
			if(flowID == 5 || flowID == 6) std::cout << "TCP Flow Type : Vegas" << std::endl;  
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
		    
		Simulator::Destroy();
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
