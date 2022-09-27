/*s program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Michele Polese <michele.polese@gmail.com>
 */

#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
//#include "ns3/gtk-config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include <ns3/random-variable-stream.h>
#include <ns3/lte-ue-net-device.h>
#include "ns3/rng-seed-manager.h"
#include "ns3/random-variable-stream.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/internet-apps-module.h"
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <list>
#include <boost/algorithm/string.hpp>

using namespace ns3;
#define TCP_PROTOCOL     "ns3::TcpBbr"
using namespace std;
/**
 * Sample simulation script for testing different CCAs in standard 5G mmwave in LOS and NLOS.
 * 
 */
NS_LOG_COMPONENT_DEFINE ("5GmmWave");
static ns3::GlobalValue globalRTT ("RttMs",
                                   "E2E RTT (ms)",
                                   ns3::DoubleValue (1),
                                   ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue globalConn ("NbrConn",
                                   "integer",
                                   ns3::DoubleValue (1),
                                   ns3::MakeDoubleChecker<double> ());

/*static ns3::GlobalValue globalLinkUtilz ("LinkUsage",
                                   "percentage of utilization",
                                   ns3::DoubleValue (1),
                                   ns3::MakeDoubleChecker<double> ());*/

int main (int argc, char *argv[])
{
        RngSeedManager::SetSeed (3);
	std::cout << "LEGACY 5G mmWave" << std::endl;
	std::cout << "Mamoutou Diarra Sophia-Antipolis, France " <<std::endl;
	std::cout << "mamoutou.diarra@inria.fr" << std::endl;

        //////////// Command Variables ///////////////////////
        double simTime = 1; // seconds
        double AppStartTime = 0.1; //seconds
        uint16_t stream = 1;
	uint16_t nodeNum = 1;
        uint16_t ueNumPergNb = 1;
	bool harqEnabled = true;
	bool rlcAmEnabled = true;
	bool fixedTti = true;//false;//true;
	unsigned symPerSf = 30;
	double sfPeriod = 100.0;
	bool tcp = true, dl= true, ul=false;
	double  mmeLatency= 15.0;
	uint16_t typeOfSplitting = 1; // 3 : p-split
	uint16_t Velocity =0;
	std::string scheduler ="MmWaveFlexTtiPfMacScheduler"; //"MmWaveFlexTtiMacScheduler";
	std::string pathLossModel = "BuildingsObstaclePropagationLossModel";
        std::string X2dataRate = "100Gb/s";
        uint32_t data = 0;
	bool bbr = false;
	bool randomStart= false;
        bool nice = false;
        bool web = false;
	bool dynIW = false;
        int scen = 0;

	double x2Latency= 1;	
        uint32_t serverDelay = 2;
	bool channelVariant = false;
        std::string transport_prot = "TcpBbr";
        uint32_t run = 1; 
        //The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'
	std::string scenario = "UMi-StreetCanyon";
	std::string condition = "l";
        double hBS = 10;
	double hUT = 1.5;
        double txPower = 35.0; // Transmitted power for both eNB and UE [dBm]
        double noiseFigure = 9.0; // Noise figure for both eNB and UE [dB]
        int buff = 10;
	std::string pacingRate = "800Mbps";

	// Command line arguments
	CommandLine cmd;
       
	cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
	cmd.AddValue ("velocity" , "UE's velocity", Velocity);
	cmd.AddValue("x2LinkDataRate", "X2 link data rate " , X2dataRate);
	cmd.AddValue("pathLossModel", "path loss modles", pathLossModel);
	cmd.AddValue ("scheduler", "lte scheduler", scheduler);
	cmd.AddValue("rlcAmEnabled", "lte rlc avilability",rlcAmEnabled);
	cmd.AddValue("harqEnabled", "harq enable or not", harqEnabled);
	cmd.AddValue("serverDelay","Delay from server to proxy", serverDelay);
        cmd.AddValue("nice","simulate Nice Jean Medecin Avenue", nice);
        cmd.AddValue ("scen", "test scenario",scen);
        cmd.AddValue ("web", "simulate 16Mbps BBR web browsing while Cubic is downloading",web);	

	cmd.AddValue("X2LinkDelay" , "X2 link delay", x2Latency);
	cmd.AddValue("channelVariant", "channel state", channelVariant);
        cmd.AddValue ("stream", "number of TCP flows",stream);
        cmd.AddValue ("bbr", "enable bbr else cubic by default",bbr);
        cmd.AddValue ("data", "file size in Mbytes",data);
        cmd.AddValue ("buff", "TCP and RLC BUFF sizes",buff);
        cmd.AddValue ("run", "run number",run);
        cmd.AddValue("dynIW","Set RW value to BDP", dynIW);
	cmd.AddValue("numUE","Set Number of UEs in each Cell", nodeNum);
	cmd.AddValue("randomStart","Set start time of the flows",randomStart);
	cmd.Parse(argc, argv);
        RngSeedManager::SetRun (run);
	Config::SetDefault("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue(2));
	Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
	Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower",DoubleValue(txPower));
	Config::SetDefault ("ns3::MmWaveUePhy::TxPower",DoubleValue(txPower));
        Config::SetDefault ("ns3::MmWaveEnbPhy::NoiseFigure", DoubleValue (noiseFigure));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::FixedTti", BooleanValue(fixedTti));
	Config::SetDefault ("ns3::MmWaveFlexTtiPfMacScheduler::FixedTti", BooleanValue(fixedTti));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::FixedTti", BooleanValue(fixedTti));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::SymPerSlot", UintegerValue(24));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolPerSlot", UintegerValue(24));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue(14));
	Config::SetDefault ("ns3::MmWaveFlexTtiPfMacScheduler::SymPerSlot", UintegerValue(14));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue(14*12)); //14
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue(132)); //132 //138 //32=50MHZ 66=100MHZ 132=200MHZ
	//Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolPerSlot", UintegerValue(14/**12*/)); //14
	Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue((uint32_t)10));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkWidth",DoubleValue(132*15000*2^(3)));//200MHz bandwidth
	//Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkWidth",DoubleValue(208000000));
        Config::SetDefault ("ns3::MmWavePhyMacCommon::SubcarriersPerChunk", UintegerValue(132)); //132
        Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(1)); //1
        Config::SetDefault ("ns3::MmWavePhyMacCommon::SlotsPerSubframe", UintegerValue(8)); //8
        Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePerFrame", UintegerValue(10));  
	Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDataRate", DataRateValue(DataRate ("10Gb/s")));
        Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDataRate", DataRateValue(DataRate ("10Gb/s")));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MicroSeconds(0)));
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (buff*1024*1024));
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (buff*1024*1024));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1400));
	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (buff*1024*1024));
	Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (buff*1024*1024));
	Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue(MilliSeconds(1.0)));
	Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (buff*1024*1024));
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpEdgeCC::GetTypeId ()));
	Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::InCar",BooleanValue(false));
	Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition",StringValue(condition));
        Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue(scenario));
        Config::SetDefault("ns3::TcpSocketBase::WindowScaling",BooleanValue(true));
        Config::SetDefault("ns3::TcpSocketBase::Timestamp",BooleanValue(true));
	/******** Begin TCP pacing ***************/
        //Config::SetDefault ("ns3::TcpSocketState::EnablePacing", BooleanValue (true));
	//Config::SetDefault ("ns3::TcpSocketState::MaxPacingRate", StringValue (pacingRate));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (3));
	/******** End TCP pacing ***************/

	/********** Change Buffer StatusReport Timer for fast Uplink allocation ***********/

	Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
        Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
        Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
        Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));

	/***************** End BufferStatus Timer ***********************************/

	/****** Configure Dyn-BDB IW **********/
	 globalRTT.SetValue (DoubleValue (serverDelay));
	 globalConn.SetValue(DoubleValue(stream));
	 //globalLinkUtilz.SetValue(DoubleValue(0));
	Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10/*10*/));
	Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (true));
	if (dynIW){
	  double bdp = ((11352/0.0001)*serverDelay)/1000;
	  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (bdp/1400));
	  //Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10/*10*/));
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (bdp));
        Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (bdp));
	}
	/******** END CONFIG IW *****************/
	Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
	mmwaveHelper->SetSchedulerType ("ns3::"+scheduler);
        //mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiPfMacScheduler");//("ns3::MmWaveFlexTtiMacScheduler");
	mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	mmwaveHelper->SetEpcHelper (epcHelper);
	mmwaveHelper->SetHarqEnabled (harqEnabled);
	
        Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::ThreeGppUmiStreetCanyonPropagationLossModel"));
        if(nice) // Building (NLOS) simulation
	  mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppBuildingsPropagationLossModel"));
	
	mmwaveHelper->Initialize();
	cmd.Parse(argc, argv);
	
	std::cout<<"Download Size: "<<data*1024*1024<<std::endl;
	std::cout<<"Server to Proxy RTT: "<<serverDelay<<std::endl;
	std::cout<<"Channel variance: "<<channelVariant<<std::endl;
	std::cout<<"RLC Buffer size: "<<buff<<std::endl;
	//uint16_t nodeNum = 2;
        if ((scen == 1) || (scen == 2) || (scen == 3))
           stream = 3;
        else if (scen == 4)
           stream = 6;
        else if (scen == 11)
	  web = true;
        
        if (web)
           stream = 2;

	Ptr<Node> pgw = epcHelper->GetPgwNode ();
	NodeContainer remoteHostContainer;
        NodeContainer InternetGw;
        InternetGw.Create(1);
	remoteHostContainer.Create (stream);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);
        internet.Install(InternetGw);
	Ipv4Address remoteHostAddr;
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Node> remoteHost ;
        Ptr<Node> rightRouter = InternetGw.Get(0);
	if(nice){
          // Avenue Jean MEdecin

	  //first building at y=3m,ymax=58m, 10m between buildings
          Ptr < Building > building1;
          building1 = Create<Building> ();
          building1->SetBoundaries (Box (20,63,
                                       3, 58,
                                       0.0, 28));
	  building1->SetBuildingType (Building::Residential);
	  building1->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building1->SetNFloors (1);
	  building1->SetNRoomsX (1);
	  building1->SetNRoomsY (1);

          Ptr < Building > building2;
          building2 = Create<Building> ();
          building2->SetBoundaries (Box (20,67,
                                       68, 114,
                                       0.0, 26));
	  building2->SetBuildingType (Building::Residential);
	  building2->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building2->SetNFloors (1);
	  building2->SetNRoomsX (1);
	  building2->SetNRoomsY (1);

          Ptr < Building > building3;
          building3 = Create<Building> ();
          building3->SetBoundaries (Box (20,122,
                                       124,198,
                                       0.0,31));
	  building3->SetBuildingType (Building::Residential);
	  building3->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building3->SetNFloors (1);
	  building3->SetNRoomsX (1);
	  building3->SetNRoomsY (1);

          Ptr < Building > building4;
          building4 = Create<Building> ();
          building4->SetBoundaries (Box (20,72,
                                       215,237,
                                       0.0,31));
	  building4->SetBuildingType (Building::Residential);
	  building4->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building4->SetNFloors (1);
	  building4->SetNRoomsX (1);
	  building4->SetNRoomsY (1);

	  // Second building row

          Ptr < Building > building5;
          building5 = Create<Building> ();
          building5->SetBoundaries (Box (78,160,
                                       3,66,
                                       0.0,30));
	  building5->SetBuildingType (Building::Residential);
	  building5->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building5->SetNFloors (1);
	  building5->SetNRoomsX (1);
	  building5->SetNRoomsY (1);

          Ptr < Building > building6;
          building6 = Create<Building> ();
          building6->SetBoundaries (Box (82,143,
                                       75,140,
                                       0.0,24));
	  building6->SetBuildingType (Building::Residential);
	  building6->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building6->SetNFloors (1);
	  building6->SetNRoomsX (1);
	  building6->SetNRoomsY (1);

          Ptr < Building > building7;
          building7 = Create<Building> ();
          building7->SetBoundaries (Box (137,175,
                                       149,221,
                                       0.0,28));
	  building7->SetBuildingType (Building::Residential);
	  building7->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building7->SetNFloors (1);
	  building7->SetNRoomsX (1);
	  building7->SetNRoomsY (1);

          Ptr < Building > building8;
          building8 = Create<Building> ();
          building8->SetBoundaries (Box (87,130,
                                       232,268,
                                       0.0,27));
	  building8->SetBuildingType (Building::Residential);
	  building8->SetExtWallsType (Building::ConcreteWithoutWindows);
	  building8->SetNFloors (1);
	  building8->SetNRoomsX (1);
	  building8->SetNRoomsY (1);
	}


	remoteHost = remoteHostContainer.Get (0);
	PointToPointHelper p2ph;      
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (serverDelay/2)));
        NetDeviceContainer internetDevices = p2ph.Install (pgw, rightRouter);
        CsmaHelper csma;
        csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
        csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));
        csma.SetDeviceAttribute ("Mtu", UintegerValue (2500));
        csma.SetChannelAttribute ("FullDuplex", BooleanValue (true));
        NodeContainer sgiDevices;
        sgiDevices.Add(rightRouter);
        sgiDevices.Add(remoteHostContainer);
        NetDeviceContainer internetNet = csma.Install (sgiDevices);      
        Ipv4AddressHelper ipv4h2;
        ipv4h2.SetBase ("2.1.0.0", "255.255.0.0");
        Ipv4InterfaceContainer internetIpIfaces2 = ipv4h2.Assign (internetNet);
        Ipv4AddressHelper ipv4h;
	std::ostringstream subnet;
	subnet<<1<<".1.0.0";
	ipv4h.SetBase (subnet.str ().c_str (), "255.255.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
        Ptr<Ipv4> ipv4 = rightRouter->GetObject<Ipv4> ();
        Ptr<Ipv4> ipv42 = pgw->GetObject<Ipv4> ();
        std::cout<<"router if2 = "<<ipv4->GetAddress (2, 0).GetLocal ()<<std::endl;
        std::cout<<"router if1 = "<<ipv4->GetAddress (1, 0).GetLocal ()<<std::endl;
        std::cout<<"pgw if1 = "<<ipv42->GetAddress (1, 0).GetLocal ()<<std::endl;
        std::cout<<"pgw if2 = "<<ipv42->GetAddress (2, 0).GetLocal ()<<std::endl;
        Ipv4Address nhop = ipv4->GetAddress (1, 0).GetLocal ();

        for (int i=0; i<remoteHostContainer.GetN();i++){
          remoteHost = remoteHostContainer.Get (i);
          Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
          remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.255.0.0"),nhop,1);
        }
                
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (rightRouter->GetObject<Ipv4> ());
        remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.255.0.0"),2);
        Ipv4Address nhop2 = ipv4->GetAddress (2, 0).GetLocal ();
        Ptr<Ipv4StaticRouting> pgwStaticRouting = ipv4RoutingHelper.GetStaticRouting (pgw->GetObject<Ipv4> ());
        pgwStaticRouting->AddNetworkRouteTo (Ipv4Address ("2.1.0.0"), Ipv4Mask ("255.255.0.0"),nhop2,2);

	// create LTE, mmWave eNB nodes and UE node
	NodeContainer ueNodes;
	NodeContainer mmWaveEnbNodes_28G;
	NodeContainer allEnbNodes;
	mmWaveEnbNodes_28G.Create(1);
	ueNodes.Create(nodeNum);
	allEnbNodes.Add(mmWaveEnbNodes_28G);

	Vector mmw1Position = Vector(0.0,0.0, hBS);
	// Install Mobility Model
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (0.0, 0.0, hBS));
	enbPositionAlloc->Add (mmw1Position);
	MobilityHelper enbmobility;
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (allEnbNodes);
	MobilityHelper uemobility;
	Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();

        if (nice)
	  uePositionAlloc->Add(Vector(100,3, hUT));
        else
	  uePositionAlloc->Add(Vector(10,0, hUT));

	uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
	uemobility.SetPositionAllocator(uePositionAlloc);
	uemobility.Install (ueNodes);
	uemobility.AssignStreams(ueNodes,0);

	BuildingsHelper::Install(mmWaveEnbNodes_28G);
	BuildingsHelper::Install (ueNodes);
	BuildingsHelper::MakeMobilityModelConsistent();

	NetDeviceContainer mmWaveEnbDevs_28GHZ = mmwaveHelper->InstallEnbDevice(mmWaveEnbNodes_28G);
	NetDeviceContainer device = mmwaveHelper->InstallUeDevice(ueNodes);
	if (nice) {
          
          ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 1, 0));
        }

	internet.Install (ueNodes);
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (device));

	// Assign IP address to UEs, and install applications
	for (uint32_t u = 0; u < ueNodes.GetN ();u++) {
	
	  Ptr<Node> ueNode = ueNodes.Get (u);
	  // Set the default gateway for the UE
	  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
	  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
	}

	mmwaveHelper->AttachToClosestEnb(device, mmWaveEnbDevs_28GHZ);
	uint16_t dlPort = 2000;
	uint16_t ulPort = 1234; //2000;
	ApplicationContainer clientApps;
	ApplicationContainer serverApps;
#if 0
        TypeId tid;
	if (scen == 6)
	  {
	    tid = TypeId::LookupByName ("ns3::TcpCubic");
	    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
	  }
	else if (scen == 7)
	  {
	    tid = TypeId::LookupByName ("ns3::TcpNewReno");
	    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
	  }
	else if (scen == 9)
	  {
	    tid = TypeId::LookupByName("ns3::TcpEdgeCC");
	    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
	    //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBbr::GetTypeId ()));
	  }
	else
	  {
	    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBbr::GetTypeId ()));
	  }
#endif
	
        for (int i=0; i<ueNodes.GetN();i++)
	  {
	    //remoteHost = remoteHostContainer.Get(i);
	    for (uint32_t j = 0; j < remoteHostContainer.GetN (); ++j)
	     {
	       remoteHost = remoteHostContainer.Get(j);
               PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (),(stream*i)+ulPort+j));
               serverApps.Add (dlPacketSinkHelper.Install (remoteHost));
             }
	    
	  }

std::cout<<"before server"<<std::endl;	
  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Node> ueMobile = ueNodes.Get (i);
      for (int j=0; j<remoteHostContainer.GetN();j++)
	{
	  remoteHost = remoteHostContainer.Get (j);
	  Ptr<Ipv4> ServerAddr = remoteHost->GetObject<Ipv4> ();
	  Ipv4Address ServerIpv4 = ServerAddr->GetAddress (1, 0).GetLocal ();
	  BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (/*Ipv4AddressValue (*/ServerIpv4/*)*//*ueIpIface.GetAddress (i)*/,(stream*i)+ulPort+j));
          source.SetAttribute ("MaxBytes", UintegerValue (data*1024*1024));
	  if (i == 1)
	    source.SetAttribute ("MaxBytes", UintegerValue (5*1024*1024));
          clientApps.Add (source.Install (ueMobile));
	  
	  NS_LOG_INFO ("Create V4Ping Appliation");
	  Ptr<V4Ping> app = CreateObject<V4Ping> ();
	  app->SetAttribute ("Remote", Ipv4AddressValue (nhop));
          app->SetAttribute ("Interval",TimeValue(Seconds (0.05)));
          app->SetAttribute ("Verbose",BooleanValue(true));
          app->SetAttribute ("Size",UintegerValue(300));
          remoteHost->AddApplication (app);
          app->SetStartTime (Seconds (0.0));
          app->SetStopTime (Seconds (0.1));

	}

      /*Ptr<V4Ping> app = CreateObject<V4Ping> ();
      remoteHost = remoteHostContainer.Get (0);
      Ptr<Ipv4> ServerAddr = pgw->GetObject<Ipv4>(); //remoteHost->GetObject<Ipv4> ();
      Ipv4Address ServerIpv4 = ServerAddr->GetAddress (1, 0).GetLocal ();
      app->SetAttribute ("Remote", Ipv4AddressValue (ServerIpv4));
          app->SetAttribute ("Interval",TimeValue(Seconds (0.0014)));
          app->SetAttribute ("Verbose",BooleanValue(true));
          app->SetAttribute ("Size",UintegerValue(1400));
          ueMobile->AddApplication (app);
          app->SetStartTime (Seconds (0.0));
          app->SetStopTime (Seconds (1.5));*/

      
    }
  
  
std::cout<<"after tcp"<<std::endl;

   mmwaveHelper -> EnableTraces();
   // pcap
  // csma.EnablePcapAll("csma");
   // DIFFERENT START TIME FOR CLIENTS APPS
  double min = 0.0;
  double max = 1.0;
  Ptr<UniformRandomVariable> fuzz = CreateObject<UniformRandomVariable>();
  fuzz->SetAttribute("Min", DoubleValue(min));
  fuzz->SetAttribute("Max", DoubleValue(max));
   
  serverApps.Start(Seconds(AppStartTime));
  //clientApps.StartWithJitter(Seconds(AppStartTime),fuzz);
  serverApps.Stop(Seconds(simTime));
  // clientApps.Stop(Seconds(simTime));
  if(randomStart){
    uint32_t nApplications = clientApps.GetN ();
    double step = 0;
    for (uint32_t i=0; i < nApplications; ++i)
  {
    Ptr<Application> p = clientApps.Get (i);
    p->SetStartTime(Seconds(AppStartTime+step));  // some Application method
    p->SetStopTime(Seconds(simTime+step));
    step+=0.1;
  }
 }
  else {
    clientApps.Start(Seconds(AppStartTime));
    clientApps.Stop(Seconds(simTime)); 
  }
  AsciiTraceHelper asciiTraceHelper;

	Simulator::Stop(Seconds(simTime));
	Simulator::Run();
int k=0;
//int j=0;

  for (uint16_t i=0;i<nodeNum;i++)
    { std::cout <<"************** UE ["<<i<<"] ********************* "<<std::endl;
     
      for ( int j=0; j<stream; j++)
        {
   std::cout <<"Stream ["<<j<<"] : ---"<<std::endl;     
  Ptr<PacketSink> p_sink = DynamicCast<PacketSink> (serverApps.Get(k));                                                                                        
                                                                                                         
   std::cout << " Total Bytes received (Bytes):" << (p_sink->GetTotalRx())<< std::endl;
   std::cout << " Total MBytes received (Mbytes):" << (p_sink->GetTotalRx())/(1024*1024) << std::endl;
  std::cout << " Total TCP throughput (Mbps):" <<
      (p_sink->GetTotalRx()*8) / ((1000000)*(simTime-AppStartTime)) << std::endl;
  k++;
        }
    }

#if 0  
  for (uint16_t i=0;i<nodeNum*stream;i++)
    { std::cout <<"************** UE ["<<i<<"] ********************* "<<std::endl;

      //for ( int j=0; j<stream; j++)
      //{
   std::cout <<"Stream ["<<i<<"] : ---"<<std::endl;
  Ptr<PacketSink> p_sink = DynamicCast<PacketSink> (serverApps.Get(i));

   std::cout << " Total Bytes received (Bytes):" << (p_sink->GetTotalRx())<< std::endl;
   std::cout << " Total MBytes received (Mbytes):" << (p_sink->GetTotalRx())/(1024*1024) << std::endl;
  std::cout << " Total TCP throughput (Mbps):" <<
      (p_sink->GetTotalRx()*8) / ((1000000)*(simTime-AppStartTime)) << std::endl;
  //k++;
  //}
    }
#endif
/************************/
	Simulator::Destroy();
	
	return 0;
	
}




