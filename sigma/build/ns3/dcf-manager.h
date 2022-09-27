/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef DCF_MANAGER_H
#define DCF_MANAGER_H

#include <vector>
#include "ns3/event-id.h"
#include "ns3/nstime.h"

namespace ns3 {

class WifiPhy;
class PhyListener;
class DcaTxop;
class MacLow;

/**
 * \brief Manage a set of ns3::DcaTxop
 * \ingroup wifi
 *
 * Handle a set of independent ns3::DcaTxop, each of which represents
 * a single DCF within a MAC stack. Each ns3::DcaTxop has a priority
 * implicitely associated with it (the priority is determined when the
 * ns3::DcaTxop is added to the DcfManager: the first DcaTxop to be
 * added gets the highest priority, the second, the second highest
 * priority, and so on.) which is used to handle "internal" collisions.
 * i.e., when two local DcaTxop are expected to get access to the
 * medium at the same time, the highest priority local DcaTxop wins
 * access to the medium and the other DcaTxop suffers a "internal"
 * collision.
 */
class DcfManager : public Object
{
public:
  DcfManager ();
  virtual ~DcfManager ();

  /**
   * Set up listener for Phy events.
   *
   * \param phy
   */
  void SetupPhyListener (Ptr<WifiPhy> phy);
  /**
   * Remove current registered listener for Phy events.
   *
   * \param phy
   */
  void RemovePhyListener (Ptr<WifiPhy> phy);
  /**
   * Set up listener for MacLow events.
   *
   * \param low
   */
  void SetupLow (Ptr<MacLow> low);

  /**
   * \param slotTime the duration of a slot.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSlot (Time slotTime);
  /**
   * \param sifs the duration of a SIFS.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSifs (Time sifs);
  /**
   * \param eifsNoDifs the duration of a EIFS minus the duration of DIFS.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetEifsNoDifs (Time eifsNoDifs);

  /**
   * \return value set previously using SetEifsNoDifs.
   */
  Time GetEifsNoDifs () const;

  /**
   * \param dcf a new DcaTxop.
   *
   * The DcfManager does not take ownership of this pointer so, the callee
   * must make sure that the DcaTxop pointer will stay valid as long
   * as the DcfManager is valid. Note that the order in which DcaTxop
   * objects are added to a DcfManager matters: the first DcaTxop added
   * has the highest priority, the second DcaTxop added, has the second
   * highest priority, etc.
   */
  void Add (Ptr<DcaTxop> dcf);

  /**
   * \param state a DcaTxop
   *
   * Notify the DcfManager that a specific DcaTxop needs access to the
   * medium. The DcfManager is then responsible for starting an access
   * timer and, invoking DcaTxop::DoNotifyAccessGranted when the access
   * is granted if it ever gets granted.
   */
  void RequestAccess (Ptr<DcaTxop> state);

  /**
   * \param duration expected duration of reception
   *
   * Notify the DCF that a packet reception started
   * for the expected duration.
   */
  void NotifyRxStartNow (Time duration);
  /**
   * Notify the DCF that a packet reception was just
   * completed successfully.
   */
  void NotifyRxEndOkNow (void);
  /**
   * Notify the DCF that a packet reception was just
   * completed unsuccessfully.
   */
  void NotifyRxEndErrorNow (void);
  /**
   * \param duration expected duration of transmission
   *
   * Notify the DCF that a packet transmission was
   * just started and is expected to last for the specified
   * duration.
   */
  void NotifyTxStartNow (Time duration);
  /**
   * \param duration expected duration of cca busy period
   *
   * Notify the DCF that a CCA busy period has just started.
   */
  void NotifyMaybeCcaBusyStartNow (Time duration);
  /**
   * \param duration expected duration of channel switching period
   *
   * Notify the DCF that a channel switching period has just started.
   * During switching state, new packets can be enqueued in DcaTxop/EdcaTxop
   * but they won't access to the medium until the end of the channel switching.
   */
  void NotifySwitchingStartNow (Time duration);
  /**
   * Notify the DCF that the device has been put in sleep mode.
   */
  void NotifySleepNow (void);
  /**
   * Notify the DCF that the device has been put in off mode.
   */
  void NotifyOffNow (void);
  /**
   * Notify the DCF that the device has been resumed from sleep mode.
   */
  void NotifyWakeupNow (void);
  /**
   * Notify the DCF that the device has been resumed from off mode.
   */
  void NotifyOnNow (void);
  /**
   * \param duration the value of the received NAV.
   *
   * Called at end of rx
   */
  void NotifyNavResetNow (Time duration);
  /**
   * \param duration the value of the received NAV.
   *
   * Called at end of rx
   */
  void NotifyNavStartNow (Time duration);
  /**
   * Notify that ACK timer has started for the given duration.
   *
   * \param duration
   */
  void NotifyAckTimeoutStartNow (Time duration);
  /**
   * Notify that ACK timer has resetted.
   */
  void NotifyAckTimeoutResetNow ();
  /**
   * Notify that CTS timer has started for the given duration.
   *
   * \param duration
   */
  void NotifyCtsTimeoutStartNow (Time duration);
  /**
   * Notify that CTS timer has resetted.
   */
  void NotifyCtsTimeoutResetNow ();

protected:
  // Inherited from ns3::Object
  void DoDispose (void);

private:
  /**
   * Update backoff slots for all DcaTxops.
   */
  void UpdateBackoff (void);
  /**
   * Return the most recent time.
   *
   * \param a
   * \param b
   *
   * \return the most recent time
   */
  Time MostRecent (Time a, Time b) const;
  /**
   * Return the most recent time.
   *
   * \param a
   * \param b
   * \param c
   * \param d
   * \param e
   * \param f
   * \param g
   *
   * \return the most recent time
   */
  Time MostRecent (Time a, Time b, Time c, Time d, Time e, Time f, Time g) const;
  /**
   * Access will never be granted to the medium _before_
   * the time returned by this method.
   *
   * \returns the absolute time at which access could start to be granted
   */
  Time GetAccessGrantStart (void) const;
  /**
   * Return the time when the backoff procedure
   * started for the given DcaTxop.
   *
   * \param state
   *
   * \return the time when the backoff procedure started
   */
  Time GetBackoffStartFor (Ptr<DcaTxop> state);
  /**
   * Return the time when the backoff procedure
   * ended (or will ended) for the given DcaTxop.
   *
   * \param state
   *
   * \return the time when the backoff procedure ended (or will ended)
   */
  Time GetBackoffEndFor (Ptr<DcaTxop> state);

  void DoRestartAccessTimeoutIfNeeded (void);

  /**
   * Called when access timeout should occur
   * (e.g. backoff procedure expired).
   */
  void AccessTimeout (void);
  /**
   * Grant access to DCF
   */
  void DoGrantAccess (void);
  /**
   * Check if the device is busy sending or receiving,
   * or NAV or CCA busy.
   *
   * \return true if the device is busy,
   *         false otherwise
   */
  bool IsBusy (void) const;
  /**
   * Check if the device is between frames (in DIFS or AIFS interval)
   *
   * \param state the state to check
   * \return true if the device is within AIFS,
   *         false otherwise
   */
  bool IsWithinAifs (Ptr<DcaTxop> state) const;

  /**
   * typedef for a vector of DcaTxops
   */
  typedef std::vector<Ptr<DcaTxop> > States;

  States m_states;              //!< the DCF states
  Time m_lastAckTimeoutEnd;     //!< the last ACK timeout end time
  Time m_lastCtsTimeoutEnd;     //!< the last CTS timeout end time
  Time m_lastNavStart;          //!< the last NAV start time
  Time m_lastNavDuration;       //!< the last NAV duration time
  Time m_lastRxStart;           //!< the last receive start time
  Time m_lastRxDuration;        //!< the last receive duration time
  bool m_lastRxReceivedOk;      //!< the last receive OK
  Time m_lastRxEnd;             //!< the last receive end time
  Time m_lastTxStart;           //!< the last transmit start time
  Time m_lastTxDuration;        //!< the last transmit duration time
  Time m_lastBusyStart;         //!< the last busy start time
  Time m_lastBusyDuration;      //!< the last busy duration time
  Time m_lastSwitchingStart;    //!< the last switching start time
  Time m_lastSwitchingDuration; //!< the last switching duration time
  bool m_rxing;                 //!< flag whether it is in receiving state
  bool m_sleeping;              //!< flag whether it is in sleeping state
  bool m_off;                   //!< flag whether it is in off state
  Time m_eifsNoDifs;            //!< EIFS no DIFS time
  EventId m_accessTimeout;      //!< the access timeout ID
  uint32_t m_slotTimeUs;        //!< the slot time in microseconds
  Time m_sifs;                  //!< the SIFS time
  PhyListener* m_phyListener;   //!< the phy listener
};

} //namespace ns3

#endif /* DCF_MANAGER_H */