/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Duy Nguyen
 * Copyright (c) 2015 Ghada Badawy
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
 *Author: Duy Nguyen <duy@soe.ucsc.edu>
 *        Ghada Badawy <gbadawy@gmail.com>
 *        Matias Richart <mrichart@fing.edu.uy>
 */



#ifndef MINSTREL_HT_WIFI_MANAGER_H
#define MINSTREL_HT_WIFI_MANAGER_H

#include "wifi-remote-station-manager.h"
#include "wifi-mode.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include <vector>
#include <map>
#include <deque>


namespace ns3 {

struct MinstrelHtWifiRemoteStation;

/**
 * A struct to contain all statistics information related to a data rate.
 */
struct HtRateInfo
{
  /**
   * Perfect transmission time calculation, or frame calculation.
   * Given a bit rate and a packet length n bytes.
   */
  Time perfectTxTime;


  uint32_t retryCount;          //!< Retry limit.
  uint32_t adjustedRetryCount;  //!< Adjust the retry limit for this rate.
  uint32_t numRateAttempt;      //!< Number of transmission attempts so far.
  uint32_t numRateSuccess;      //!< Number of successful frames transmitted so far.
  uint32_t prob;                //!< (# frame success )/(# total frames)

  /**
   * EWMA calculation
   * ewma_prob =[prob *(100 - ewma_level) + (ewma_prob_old * ewma_level)]/100
   */
  uint32_t ewmaProb;

  uint32_t prevNumRateAttempt;  //!< Number of transmission attempts with previous rate.
  uint32_t prevNumRateSuccess;  //!< Number of successful frames transmitted with previous rate.
  uint32_t numSamplesSkipped;   //!< Number of times this rate statistics were not updated because no attempts have been made.
  uint64_t successHist;         //!< Aggregate of all transmission successes.
  uint64_t attemptHist;         //!< Aggregate of all transmission attempts.
  uint32_t throughput;          //!< Throughput of this rate.
};

/**
 * Data structure for a Minstrel Rate table.
 * A vector of a struct RateInfo.
 */
typedef std::vector<struct HtRateInfo> HtMinstrelRate;

/**
 * A struct to contain information of a group.
 */
struct GroupInfo
{
  /**
   * MCS rates are divided into groups based on the number of streams and flags that they use.
   */
  uint8_t m_col;                  //!< Sample table column.
  uint8_t m_index;                //!< Sample table index.

  bool m_supported;               //!< If the rates of this group are supported by the station.

  uint32_t m_maxTpRate;           //!< The max throughput rate of this group.
  uint32_t m_maxTpRate2;          //!< The second max throughput rate of this group.
  uint32_t m_maxProbRate;         //!< The highest success probability rate of this group.

  HtMinstrelRate m_minstrelTable; //!< Information about rates of this group.
};

/**
 * Data structure for a table of groups. Each group is of type GroupInfo.
 * A vector of a GroupInfo.
 */
typedef std::vector<struct GroupInfo> McsGroupData;

/**
 * Data structure for a Sample Rate table.
 * A vector of a vector uint32_t.
 */
typedef std::vector<std::vector<uint32_t> > HtSampleRate;

/**
 * Data structure to save transmission times calculations per rate.
 * A vector fo Time, WifiMode pairs.
 */
typedef std::vector<std::pair<Time,WifiMode> > TxTime;

/**
 * Data structure to contain the information that defines a group.
 * It also contains the transmission times for all the MCS in the group.
 * A group is a collection of MCS defined by the number of spatial streams,
 * if it uses or not Short Guard Interval and the channel width used.
 */
struct McsGroup
{
  uint8_t streams;
  uint8_t sgi;
  uint32_t chWidth;
  TxTime calcTxTime;
};

/**
 * Data structure for a table of group definitions.
 * A vector of a McsGroup.
 */
typedef std::vector<struct McsGroup> MinstrelMcsGroups;

/**
 * Constants for maximum values.
 */

uint8_t MAX_SUPPORTED_STREAMS = 2;  //!< Maximal number of streams supported by the phy layer.
uint8_t MAX_STREAM_GROUPS = 4;      //!< Maximal number of groups per stream (2 possible channel widths and 2 possible SGI configurations).
uint8_t MAX_GROUP_RATES = 8;        //!< Number of rates (or MCS) per group.
uint8_t N_GROUPS = MAX_SUPPORTED_STREAMS * MAX_STREAM_GROUPS; //!< Number of groups Minstrel should consider.

/**
 * \author Ghada Badawy
 * \brief Implementation of Minstrel HT Rate Control Algorithm
 * \ingroup wifi
 *
 * http://lwn.net/Articles/376765/
 */
class MinstrelHtWifiManager : public WifiRemoteStationManager
{

public:
  static TypeId GetTypeId (void);
  MinstrelHtWifiManager ();
  virtual ~MinstrelHtWifiManager ();

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

  virtual void SetupPhy (Ptr<WifiPhy> phy);

  /**
   * TracedCallback signature for rate change events.
   *
   * \param [in] rate The new rate.
   * \param [in] address The remote station MAC address.
   */
  typedef void (*RateChangeTracedCallback)(const uint64_t rate, const Mac48Address remoteAddress);

private:
  // Overriden from base class.
  virtual WifiRemoteStation * DoCreateStation (void) const;
  virtual void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
  virtual void DoReportRtsFailed (WifiRemoteStation *station);
  virtual void DoReportDataFailed (WifiRemoteStation *station);
  virtual void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void DoReportFinalRtsFailed (WifiRemoteStation *station);
  virtual void DoReportFinalDataFailed (WifiRemoteStation *station);
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station, uint32_t size);
  virtual WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
  virtual bool IsLowLatency (void) const;
  virtual bool DoNeedDataRetransmission (WifiRemoteStation *st, Ptr<const Packet> packet, bool normally);
  virtual void DoDisposeStation (WifiRemoteStation *station);

  /// Estimates the TxTime of a frame with a given mode and group (stream, guard interval and channel width).
  Time CalculateTxDuration (Ptr<WifiPhy> phy, uint8_t streams, uint8_t sgi, uint32_t chWidth, WifiMode mode);

  /// Obtain the TXtime saved in the group information.
  Time GetCalcTxTime (uint32_t groupId, WifiMode mode) const;

  /// Save a TxTime to the vector of groups.
  void AddCalcTxTime (uint32_t groupId, WifiMode mode, Time t);

  /// Update the number of retries and reset accordingly.
  void UpdateRetry (MinstrelHtWifiRemoteStation *station);

  /// Getting the next sample from Sample Table.
  uint32_t GetNextSample (MinstrelHtWifiRemoteStation *station);

  /// Set the next sample from Sample Table.
  void SetNextSample (MinstrelHtWifiRemoteStation *station);

  /// Find a rate to use from Minstrel Table.
  uint32_t FindRate (MinstrelHtWifiRemoteStation *station);

  /// Updating the Minstrel Table every 1/10 seconds.
  void UpdateStats (MinstrelHtWifiRemoteStation *station);

  /// Initialize Minstrel Table.
  void RateInit (MinstrelHtWifiRemoteStation *station);

  /**
   * Estimate the time to transmit the given packet with the given number of retries.
   * This function is "roughly" the function "calc_usecs_unicast_packet" in minstrel.c
   * in the madwifi implementation.
   *
   * The basic idea is that, we try to estimate the "average" time used to transmit the
   * packet for the given number of retries while also accounting for the 802.11 congestion
   * window change. The original code in the madwifi seems to estimate the number of backoff
   * slots as the half of the current CW size.
   *
   * There are four main parts:
   *  - wait for DIFS (sense idle channel)
   *  - ACK timeouts
   *  - DATA transmission
   *  - backoffs according to CW
   */
  Time CalculateTimeUnicastPacket (Time dataTransmissionTime, uint32_t shortRetries, uint32_t longRetries);

  /// Initialize Sample Table.
  void InitSampleTable (MinstrelHtWifiRemoteStation *station);

  /// Printing Sample Table.
  void PrintSampleTable (MinstrelHtWifiRemoteStation *station, std::ostream &os);

  /// Printing Minstrel Table.
  void PrintTable (MinstrelHtWifiRemoteStation *station, std::ostream &os);

  /// Check for initializations.
  void CheckInit (MinstrelHtWifiRemoteStation *station);

  /**
   * For managing rates from different groups, a global index for
   * all rates in all groups is used.
   * The group order is fixed by BW -> SGI -> #streams.
   * Following functions convert from groupId and rateId to
   * global index and vice versa.
   */

  /// Return the rate index inside a group.
  uint32_t  GetRateId (uint32_t index);

  /// Return the group id from global index.
  uint32_t GetGroupId (uint32_t index);

  /// Returns the global index corresponding to the MCS inside a group.
  uint32_t GetIndex (uint32_t groupid, uint32_t mcsIndex);

  /// Calculates the group id from the number of streams, if using sgi and the channel width used.
  uint32_t GetGroupId (uint8_t txstreams, uint8_t sgi, uint8_t ht40);

  Time m_updateStats;         //!< How frequent do we calculate the stats (1/10 seconds).
  double m_lookAroundRate;    //!< The % to try other rates than our current rate.
  double m_ewmaLevel;         //!< Exponential weighted moving average level (or coefficient).

  uint32_t m_nSampleCol;      //!< Number of sample columns.
  uint32_t m_frameLength;     //!< Frame length used for calculate modes TxTime.

  MinstrelMcsGroups m_minstrelGroups; //!< Global array for groups information.


  Ptr<UniformRandomVariable> m_uniformRandomVariable; //!< Provides uniform random variables.

  /**
   * The trace source fired when the transmission rate change.
   */
  TracedCallback<uint64_t, Mac48Address> m_rateChange;
};

} // namespace ns3

#endif /* MINSTREL_HT_WIFI_MANAGER_H */
