 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
 *  
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *  
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *  
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *  
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *        	 	  Sourjya Dutta <sdutta@nyu.edu>
 *        	 	  Russell Ford <russell.ford@nyu.edu>
 *        		  Menglei Zhang <menglei@nyu.edu>
 */



#include <ns3/llc-snap-header.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include "mmwave-net-device.h"
#include <ns3/packet-burst.h>
#include <ns3/uinteger.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>
#include "mmwave-enb-net-device.h"
#include "mmwave-ue-net-device.h"
#include <ns3/ipv4-header.h>
#include <ns3/ipv4.h>
#include "mmwave-ue-phy.h"
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>

// madi include
#include "ns3/config-store.h"
#include "ns3/core-module.h"
//#include "ns3/tcp-socket-base.h"
#include <ns3/tcp-l4-protocol.h>
#include "ns3/tcp-header.h"
#include "ns3/internet-module.h"
namespace ns3{

NS_LOG_COMPONENT_DEFINE ("MmWaveUeNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MmWaveUeNetDevice);

static ns3::GlobalValue globalLinkUtilz ("LinkUsage",
                                   "percentage of utilization",
                                   ns3::DoubleValue (0),
                                   ns3::MakeDoubleChecker<double> ());
  
TypeId
MmWaveUeNetDevice::GetTypeId (void)
{
	static TypeId
	    tid =
	    TypeId ("ns3::MmWaveUeNetDevice")
	    .SetParent<MmWaveNetDevice> ()
	    .AddConstructor<MmWaveUeNetDevice> ()
		.AddAttribute ("EpcUeNas",
		                   "The NAS associated to this UeNetDevice",
		                   PointerValue (),
		                   MakePointerAccessor (&MmWaveUeNetDevice::m_nas),
		                   MakePointerChecker <EpcUeNas> ())
		.AddAttribute ("mmWaveUeRrc",
		                   "The RRC associated to this UeNetDevice",
		                   PointerValue (),
		                   MakePointerAccessor (&MmWaveUeNetDevice::m_rrc),
		                   MakePointerChecker <LteUeRrc> ())
		.AddAttribute ("MmWaveUePhy",
						"The PHY associated to this UeNetDevice",
						PointerValue (),
						MakePointerAccessor (&MmWaveUeNetDevice::m_phy),
						MakePointerChecker <MmWaveUePhy> ())
		.AddAttribute ("MmWaveUeMac",
						"The MAC associated to this UeNetDevice",
						PointerValue (),
						MakePointerAccessor (&MmWaveUeNetDevice::m_mac),
						MakePointerChecker <MmWaveUeMac> ())
		.AddAttribute ("Imsi",
			 "International Mobile Subscriber Identity assigned to this UE",
			 UintegerValue (0),
			 MakeUintegerAccessor (&MmWaveUeNetDevice::m_imsi),
			 MakeUintegerChecker<uint64_t> ())
		.AddAttribute ("AntennaNum",
					   "Antenna number of the device",
					   UintegerValue (16),
					   MakeUintegerAccessor (&MmWaveUeNetDevice::SetAntennaNum,
											 &MmWaveUeNetDevice::GetAntennaNum),
					   MakeUintegerChecker<uint8_t> ())
		.AddAttribute ("LteUeRrc",
						"The RRC layer associated with the ENB",
						PointerValue (),
						MakePointerAccessor (&MmWaveUeNetDevice::m_rrc),
						MakePointerChecker <LteUeRrc> ())
	;
	return tid;
}

MmWaveUeNetDevice::MmWaveUeNetDevice (void)
	: m_isConstructed (false)

{        // madi                                                                                                                                               
        m_NbrTcp = 0;
	NS_LOG_FUNCTION (this);
}

MmWaveUeNetDevice::~MmWaveUeNetDevice (void)
{

}

void
MmWaveUeNetDevice::DoInitialize (void)
{
	m_isConstructed = true;
	UpdateConfig ();
	m_phy->DoInitialize ();
	m_rrc->Initialize ();
        // madi
        m_NbrTcp = 0;

}
void
MmWaveUeNetDevice::DoDispose ()
{
	m_rrc->Dispose ();
}

uint32_t
MmWaveUeNetDevice::GetCsgId () const
{
  NS_LOG_FUNCTION (this);
  return m_csgId;
}

void
MmWaveUeNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change down to NAS and RRC
}

void
MmWaveUeNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      NS_LOG_LOGIC (this << " Updating configuration: IMSI " << m_imsi
                         << " CSG ID " << m_csgId);
      m_nas->SetImsi (m_imsi);
      m_rrc->SetImsi (m_imsi);
      m_nas->SetCsgId (m_csgId); // this also handles propagation to RRC
    }
  else
    {
      /*
       * NAS and RRC instances are not be ready yet, so do nothing now and
       * expect ``DoInitialize`` to re-invoke this function.
       */
    }
}

bool
MmWaveUeNetDevice::DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION (this << dest << protocolNumber);
    if (protocolNumber != Ipv4L3Protocol::PROT_NUMBER)
	{
	  NS_LOG_INFO("unsupported protocol " << protocolNumber << ", only IPv4 is supported");
	  return false;
	}

    /*********** MADI Dyn RW Modification *************/
    Ptr<MmWavePhyMacCommon> mymac = this->GetMac()->GetConfigurationParameters();
    uint32_t maxSym = mymac->GetSymbPerSlot()-1; // |DL CRTL|..DATA....|UL CTRL| : so 2 symb for control                                                        
    Ptr<MmWaveAmc> myamc = CreateObject <MmWaveAmc> (mymac);                                                                                
    //uint32_t tbsmax = (myamc->GetTbSizeFromMcsSymbols(dciInfoElem.m_mcs,maxSym) /8);                                                        
    UePrb myUE = mymac->GetUeFromRnti(this->GetMac()->GetUeRnti());//UePrb((unsigned)dciInfoElem.m_numSym);
    //mymac->AddUeInPrbUlTab(dciInfoElem.m_rnti,myUE);
    uint8_t allocated = mymac->GetPrbTabSum();
    if (myUE.GetState() /*!= NULL*/){
      uint32_t SymExp = (((unsigned)myUE.GetUlSymb()))+(maxSym - std::min((uint32_t)allocated,maxSym));
      uint32_t tbsExp = (myamc->GetTbSizeFromMcsSymbols(myUE.GetUlMcs(),SymExp) /8);
      uint32_t TbsCurr = (myamc->GetTbSizeFromMcsSymbols(myUE.GetUlMcs(),(unsigned)myUE.GetUlSymb()) /8);
      double    utlz = ((double)TbsCurr)/((double)tbsExp);
      utlz = ((double)((unsigned)myUE.GetUlSymb()))/((double)SymExp);
      //std::cout<<"Utilization ["<<utlz<<"]"<<std::endl;
      globalLinkUtilz.SetValue(DoubleValue(utlz));
      std::cout<<"MaxSymb = "<<maxSym<<" RNTI = "<<this->GetMac()->GetUeRnti()<<" UlSym = "<<(unsigned)myUE.GetUlSymb()<<" UlTBS = "<<(myamc->GetTbSizeFromMcsSymbols(myUE.GetUlMcs(),myUE.GetUlSymb()) /8)<<" Allocated ="<<(unsigned)allocated<<" SymExp ="<<SymExp<<" ExpTBS = "<<(myamc->GetTbSizeFromMcsSymbols(myUE.GetUlMcs(),SymExp) /8)<<" DLSym = "<<(unsigned)myUE.GetDlSymb()<<std::endl;

      /********** GET RTT from TCP TS option ********/
      Ipv4Header tempIpv4Header;
      TcpHeader tempTcpHeader;
      double mtime=0;
      int oldnum = 0;
      Ptr<Packet> L3P = packet->Copy();
      L3P->RemoveHeader(tempIpv4Header);
      L3P->RemoveHeader(tempTcpHeader);
      DoubleValue doubleValue;
      GlobalValue::GetValueByName ("RttMs", doubleValue);
      double RTTst = doubleValue.Get ();
      std::cout<<"configured RTTmin = "<<RTTst<<std::endl;
      uint16_t srcPort = tempTcpHeader.GetSourcePort();
      oldnum = srcPort - 49153;
      const std::string& delim="|";
      std::string myflag = TcpHeader::FlagsToString (tempTcpHeader.GetFlags(),delim);
      std::cout<<"Flags-to String []: "<<myflag<<std::endl;
      if((myflag.compare("FIN|ACK")==0)/*||(tempTcpHeader.GetFlags()==TcpHeader::FIN|TcpHeader::ACK)*/){
        m_NbrTcp-=1;
        std::cout<<"FOUND at last TCP FIN"<<std::endl;
      }
      if((tempTcpHeader.GetFlags()==TcpHeader::SYN)/*||(tempTcpHeader.GetFlags()==TcpHeader::ACK)*/){
        std::cout<<"SENDING SYN-ACK"<<std::endl;
         // madi
        //oldnum = m_NbrTcp;
        m_NbrTcp+=1;
        Ptr<const TcpOptionTS> ts;
        Time oneway;
        //double mtime;
        ts = DynamicCast<const TcpOptionTS> (tempTcpHeader.GetOption (TcpOption::TS));
        oneway = TcpOptionTS::ElapsedTimeFromTsValue (ts->GetEcho()/*GetTimestamp()*/);
        //oneway = TcpOptionTS::ElapsedTimeFromTsValue (oneway.GetMilliSeconds());
        std::cout<<"TCP FLAGs :"<<tempTcpHeader.GetFlags()<<"ONE WAY = "<<  /*ts->GetEcho()*/oneway.GetMilliSeconds()<<" stamp ="<<TcpOptionTS::ElapsedTimeFromTsValue (ts->GetTimestamp())<<"Nbr TCP :["<<m_NbrTcp<<std::endl;
        mtime = oneway.GetMilliSeconds()<100 ? oneway.GetSeconds()*2 : RTTst/1000;
          
      }

      //std::string specificTest = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketList/*/RanBdp"

      /********** Set UE Node RW *******/
      Ptr<Node> UeNode = this->GetNode();
      std::stringstream nodeId;
      nodeId <<UeNode->GetId ();
      std::string specificTest = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketList/"+std::to_string(oldnum)+"/RanBdp";
      Config::Set (specificTest, UintegerValue ((tbsExp/0.0001)/*/m_NbrTcp*//*25*/));
      std::cout<<" SrcPort = "<<srcPort<<std::endl;
      std::string specificNode = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketList/"+std::to_string(oldnum)+"/InitialCwnd";//"/$ns3::TcpSocket::InitialCwnd";
      std::string specificNodeWmax = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketList/*/MaxWindowSize";
      if (mtime!=0){
        double bdp = (((tbsExp/0.0001/*25*/)*mtime)/1400)/*/m_NbrTcp*/;
        //bdp = bdp*1.11;
        Config::Set (specificNode, UintegerValue (bdp/*/m_NbrTcp*/));
        //Config::Set (specificNodeWmax, UintegerValue (bdp*1400));
        std::cout<<"BDP(MSS) = "<<bdp/m_NbrTcp<<"nbrTCP = "<<m_NbrTcp<<" SrcPort = "<<srcPort<<std::endl;
        
      //Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10/*10*/));
      }
    }
    /*********** End MADI Dyn RW **********************/

    return m_nas->Send (packet);
}

Ptr<MmWaveUePhy>
MmWaveUeNetDevice::GetPhy (void) const
{
	return m_phy; //Inherited from mmwaveNetDevice
}

Ptr<MmWaveUeMac>
MmWaveUeNetDevice::GetMac (void) const
{
	return m_mac;
}

Ptr<EpcUeNas>
MmWaveUeNetDevice::GetNas (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas;
}


Ptr<LteUeRrc>
MmWaveUeNetDevice::GetRrc (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rrc;
}

uint64_t
MmWaveUeNetDevice::GetImsi () const
{
	return m_imsi;
}

uint16_t
MmWaveUeNetDevice::GetEarfcn () const
{
	return m_earfcn;
}

void
MmWaveUeNetDevice::SetEarfcn (uint16_t earfcn)
{
	m_earfcn = earfcn;
}

void
MmWaveUeNetDevice::SetTargetEnb (Ptr<MmWaveEnbNetDevice> enb)
{
	m_targetEnb = enb;
}

Ptr<MmWaveEnbNetDevice>
MmWaveUeNetDevice::GetTargetEnb (void)
{
	return m_targetEnb;
}

uint8_t
MmWaveUeNetDevice::GetAntennaNum () const
{
	return m_antennaNum;
}

void
MmWaveUeNetDevice::SetAntennaNum (uint8_t antennaNum)
{
	m_antennaNum = antennaNum;
}

}
