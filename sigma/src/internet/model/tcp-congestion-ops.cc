/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
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
 */
#include "tcp-congestion-ops.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"

//madi include
#include "ns3/config-store.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpCongestionOps");

NS_OBJECT_ENSURE_REGISTERED (TcpCongestionOps);

TypeId
TcpCongestionOps::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCongestionOps")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

TcpCongestionOps::TcpCongestionOps () : Object ()
{
}

TcpCongestionOps::TcpCongestionOps (const TcpCongestionOps &other) : Object (other)
{
}

TcpCongestionOps::~TcpCongestionOps ()
{
}


// RENO

NS_OBJECT_ENSURE_REGISTERED (TcpNewReno);

TypeId
TcpNewReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewReno")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpNewReno> ()
  ;
  return tid;
}

TcpNewReno::TcpNewReno (void) : TcpCongestionOps ()
{
  NS_LOG_FUNCTION (this);
}

TcpNewReno::TcpNewReno (const TcpNewReno& sock)
  : TcpCongestionOps (sock)
{
  NS_LOG_FUNCTION (this);
}

TcpNewReno::~TcpNewReno (void)
{
}

/**
 * \brief Tcp NewReno slow start algorithm
 *
 * Defined in RFC 5681 as
 *
 * > During slow start, a TCP increments cwnd by at most SMSS bytes for
 * > each ACK received that cumulatively acknowledges new data.  Slow
 * > start ends when cwnd exceeds ssthresh (or, optionally, when it
 * > reaches it, as noted above) or when congestion is observed.  While
 * > traditionally TCP implementations have increased cwnd by precisely
 * > SMSS bytes upon receipt of an ACK covering new data, we RECOMMEND
 * > that TCP implementations increase cwnd, per:
 * >
 * >    cwnd += min (N, SMSS)                      (2)
 * >
 * > where N is the number of previously unacknowledged bytes acknowledged
 * > in the incoming ACK.
 *
 * The ns-3 implementation respect the RFC definition. Linux does something
 * different:
 * \verbatim
u32 tcp_slow_start(struct tcp_sock *tp, u32 acked)
  {
    u32 cwnd = tp->snd_cwnd + acked;

    if (cwnd > tp->snd_ssthresh)
      cwnd = tp->snd_ssthresh + 1;
    acked -= cwnd - tp->snd_cwnd;
    tp->snd_cwnd = min(cwnd, tp->snd_cwnd_clamp);

    return acked;
  }
  \endverbatim
 *
 * As stated, we want to avoid the case when a cumulative ACK increases cWnd more
 * than a segment size, but we keep count of how many segments we have ignored,
 * and return them.
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 * \return the number of segments not considered for increasing the cWnd
 */
uint32_t
TcpNewReno::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);
  std::cout<<"RAN Utilization TCP-SS []"<<std::endl;
  std::cout<<"IM IN NEWRENO"<<std::endl;
  if (segmentsAcked >= 1)
    {
      tcb->m_cWnd += tcb->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - 1;
    }

   std::cout<<"NewReno TCP-SS- []"<<std::endl;

  return 0;
}

/**
 * \brief NewReno congestion avoidance
 *
 * During congestion avoidance, cwnd is incremented by roughly 1 full-sized
 * segment per round-trip time (RTT).
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 */
void
TcpNewReno::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked > 0)
    {
      double adder = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ();
      adder = std::max (1.0, adder);
      tcb->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }

  //madi
  //tcb->m_cWnd = tcb->m_initialCWnd*tcb->m_segmentSize*50;//tcb->m_ranBdp; //tcb->m_initialCWnd*tcb->m_segmentSize*50; // tcb->m_ranBdp;
}

/**
 * \brief Try to increase the cWnd following the NewReno specification
 *
 * \see SlowStart
 * \see CongestionAvoidance
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 */
void
TcpNewReno::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      CongestionAvoidance (tcb, segmentsAcked);
    }

  /* At this point, we could have segmentsAcked != 0. This because RFC says
   * that in slow start, we should increase cWnd by min (N, SMSS); if in
   * slow start we receive a cumulative ACK, it counts only for 1 SMSS of
   * increase, wasting the others.
   *
   * // Uncorrect assert, I am sorry
   * NS_ASSERT (segmentsAcked == 0);
   */
}

std::string
TcpNewReno::GetName () const
{
  return "TcpNewReno";
}

uint32_t
TcpNewReno::GetSsThresh (Ptr<const TcpSocketState> state,
                         uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);

  return std::max (2 * state->m_segmentSize, bytesInFlight / 2);
}

Ptr<TcpCongestionOps>
TcpNewReno::Fork ()
{
  return CopyObject<TcpNewReno> (this);
}


/****************** TCP EDGE CC CONTROL *****************************/

NS_OBJECT_ENSURE_REGISTERED (TcpEdgeCC);

TypeId
TcpEdgeCC::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpEdgeCC")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpEdgeCC> ()
  ;
  return tid;
}

TcpEdgeCC::TcpEdgeCC (void) : TcpCongestionOps ()
{
  NS_LOG_FUNCTION (this);
}

TcpEdgeCC::TcpEdgeCC (const TcpEdgeCC& sock)
  : TcpCongestionOps (sock)
{
  NS_LOG_FUNCTION (this);
}

TcpEdgeCC::~TcpEdgeCC (void)
{
}

/**
 * \brief Tcp NewReno slow start algorithm
 *
 * Defined in RFC 5681 as
 *
 * > During slow start, a TCP increments cwnd by at most SMSS bytes for
 * > each ACK received that cumulatively acknowledges new data.  Slow
 * > start ends when cwnd exceeds ssthresh (or, optionally, when it
 * > reaches it, as noted above) or when congestion is observed.  While
 * > traditionally TCP implementations have increased cwnd by precisely
 * > SMSS bytes upon receipt of an ACK covering new data, we RECOMMEND
 * > that TCP implementations increase cwnd, per:
 * >
 * >    cwnd += min (N, SMSS)                      (2)
 * >
 * > where N is the number of previously unacknowledged bytes acknowledged
 * > in the incoming ACK.
 *
 * The ns-3 implementation respect the RFC definition. Linux does something
 * different:
 * \verbatim
u32 tcp_slow_start(struct tcp_sock *tp, u32 acked)
  {
    u32 cwnd = tp->snd_cwnd + acked;

    if (cwnd > tp->snd_ssthresh)
      cwnd = tp->snd_ssthresh + 1;
    acked -= cwnd - tp->snd_cwnd;
    tp->snd_cwnd = min(cwnd, tp->snd_cwnd_clamp);

    return acked;
  }
  \endverbatim
 *
 * As stated, we want to avoid the case when a cumulative ACK increases cWnd more
 * than a segment size, but we keep count of how many segments we have ignored,
 * and return them.
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 * \return the number of segments not considered for increasing the cWnd
 */
uint32_t
TcpEdgeCC::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);
  //std::cout<<"RAN Utilization TCP-SS []"<<std::endl;
  if (segmentsAcked >= 1)
   {
      //tcb->m_cWnd += tcb->m_segmentSize;
     uint32_t old_cWnd = tcb->m_cWnd;
     std::cout<<"FIRST CWND = ["<<tcb->m_cWnd<<"]"<<std::endl;
      DoubleValue doubleValue;
      DoubleValue ranUsageRaw;
      double maxUsage =1;//1;//0.96;//0.95;//1;//2;//0.96;//2;//0.96;//0.954545;//0.96;//0.95;
      std::cout<<"Before Global[]"<<std::endl;
      GlobalValue::GetValueByName("LinkUsage",ranUsageRaw);
      std::cout<<"After Global[]"<<std::endl;
      double ranUsage = ranUsageRaw.Get ();
      std::cout<<"RAN Utilization TCP-SS ["<<ranUsage<<"]"<<std::endl;
      double currentUsage;
      double currTcpSpeed; 
      GlobalValue::GetValueByName ("RttMs", doubleValue);
      double RTTst = doubleValue.Get ();
      double myRTT = ((Time)(tcb->m_minRtt)).GetSeconds () > 1000 ? (RTTst/1000)+0.0001 : ((Time)(tcb->m_minRtt)).GetSeconds ()+0.0001;
      //myRTT = ((Time)(tcb->m_minRtt)).GetSeconds ();
      double lastRtt = ((Time)(tcb->m_lastRtt)).GetSeconds ();
      currTcpSpeed = ((double)((uint32_t)(tcb->m_cWnd/*tcb->m_bytesInFlight*/)))/myRTT;
      
      if (((uint32_t)tcb->m_bytesInFlight + (tcb->m_segmentSize*segmentsAcked))>= (tcb->m_initialCWnd*tcb->m_segmentSize)){
      tcb->m_pacing = false;
      tcb->m_isRanFull = true;
      std::cout<<"[EdgeCC] : PACING IS DISABLED"<<std::endl;
      }
      //tcb->m_maxPacingRate = DataRate(std::to_string((currTcpSpeed*8)/1000)+"Kb/s");
      //tcb->m_currentPacingRate = DataRate(std::to_string((currTcpSpeed*8)/1000)+"Kb/s");


      
      currentUsage = currTcpSpeed/tcb->m_ranBitRate;
      std::cout<<" [EdgeCC] CURRENT TCP USAGE = {"<<currentUsage<<"} ; RAN USAGE = {"<<ranUsage<<"}"<<std::endl;
      if (lastRtt >=2*myRTT){
          tcb->m_isRanFull = true;
          tcb->m_ranBdp =(tcb->m_ranBitRate*myRTT)*(myRTT/lastRtt); // decrease Bytes inflight
          tcb->m_ranBdp = std::min(tcb->m_ranBitRate*myRTT,(double)tcb->m_ranBdp); // so that RAN capacity is not exceeded when RTTmin gets > lastRTT
          // This BDP value is maintained until lastRTT is less than 1.5*minRTT
          tcb->m_cWnd = static_cast<uint32_t>(tcb->m_ranBdp);
          std::cout<<"EdgeCC State = [Decrease] : oldCWND = ["<<old_cWnd<<"]; NewCWND = ["<<tcb->m_cWnd<<"]"<<std::endl;
          //tcb->m_cWnd = static_cast<uint32_t>(tcb->m_ranBdp);
          return segmentsAcked - 1;
        }
      else {
        //if ((ranUsage!=0)&&(ranUsage<maxUsage)){
           uint32_t oldBdp = tcb->m_ranBdp > 14600 ? tcb->m_ranBdp : tcb->m_ranBitRate*myRTT;
           tcb->m_ranBdp = tcb->m_ranBdp > 14600 ? tcb->m_ranBdp : tcb->m_ranBitRate*myRTT;
           //newBdp += (tcb->m_segmentSize*segmentsAcked)//((1.2-ranUsage))*((double)tcb->m_ranBdp);
           //tcb->m_ranBdp = std::min((double)newBdp,(double)(tcb->m_ranBitRate*myRTT)*1.2);//+= ((1-ranUsage))*((double)m_tcb->m_ranBdp);
           double adder = /*static_cast<double>*/(double)((tcb->m_segmentSize * ((maxUsage-ranUsage/*currentUsage*/)*(tcb->m_ranBdp))) / tcb->m_ranBdp);//tcb->m_cWnd.Get ();
           tcb->m_ranBdp += (/*static_cast<uint32_t>*/(double) (adder))*segmentsAcked;
           tcb->m_cWnd = static_cast<uint32_t>(tcb->m_ranBdp);
           std::cout<<" EdgeCC [INCREASE] oldCWND = ["<<old_cWnd<<"]; IncrBytes = ["<<adder<<"] NewCWND = ["<<tcb->m_cWnd<<"]"<<std::endl;
           //return segmentsAcked - 1;
           /* }
        else {
          tcb->m_cWnd = old_cWnd;
          std::cout<<" EdgeCC [STABLE] oldCWND = ["<<old_cWnd<<"];  NewCWND = ["<<tcb->m_cWnd<<"]"<<std::endl;
          //return 0;
          }*/
           //return segmentsAcked - 1;
      }

      //tcb->m_cWnd = static_cast<uint32_t>(tcb->m_ranBdp);
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      //tcb->m_cWnd = static_cast<uint32_t> (tcb->m_ranBdp);
      //std::cout<<"CURRENT CWND = ["<<tcb->m_cWnd<<"]"<<std::endl;
      std::cout<<" TCP RTTmin = ["<<myRTT<<"]; lastRTT = ["<<lastRtt<<"]"<<std::endl;
      std::cout<<" TCP RanBitRate = ["<<tcb->m_ranBitRate<<"]; RanBDP = ["<<(tcb->m_ranBitRate*myRTT)*(myRTT/lastRtt)<<"]"<<std::endl;
      return 0; //segmentsAcked - 1;//segmentsAcked;//- 1;
      }

  return 0;
}

/**
 * \brief NewReno congestion avoidance
 *
 * During congestion avoidance, cwnd is incremented by roughly 1 full-sized
 * segment per round-trip time (RTT).
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 */
void
TcpEdgeCC::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked > 0)
    {
      double adder = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ();
      adder = std::max (1.0, adder);
      tcb->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }
  std::cout<<"RAN Utilization TCP-SS-CA []"<<std::endl;

  //madi
  //tcb->m_cWnd = tcb->m_initialCWnd*tcb->m_segmentSize*50;//tcb->m_ranBdp; //tcb->m_initialCWnd*tcb->m_segmentSize*50; // tcb->m_ranBdp;
}

/**
 * \brief Try to increase the cWnd following the NewReno specification
 *
 * \see SlowStart
 * \see CongestionAvoidance
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 */
void
TcpEdgeCC::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      //CongestionAvoidance (tcb, segmentsAcked);
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  /* At this point, we could have segmentsAcked != 0. This because RFC says
   * that in slow start, we should increase cWnd by min (N, SMSS); if in
   * slow start we receive a cumulative ACK, it counts only for 1 SMSS of
   * increase, wasting the others.
   *
   * // Uncorrect assert, I am sorry
   * NS_ASSERT (segmentsAcked == 0);
   */
  // std::cout<<"RAN Utilization TCP-INCREASE-CA []"<<std::endl;
}

std::string
TcpEdgeCC::GetName () const
{
  return "TcpEdgeCC";
}

uint32_t
TcpEdgeCC::GetSsThresh (Ptr<const TcpSocketState> state,
                         uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);

  return std::max (2 * state->m_segmentSize, bytesInFlight / 2);
}

Ptr<TcpCongestionOps>
TcpEdgeCC::Fork ()
{
  return CopyObject<TcpEdgeCC> (this);
}


} // namespace ns3

