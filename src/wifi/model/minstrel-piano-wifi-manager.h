/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Duy Nguyen
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
 * Author: Duy Nguyen <duy@soe.ucsc.edu>
 */



#ifndef MINSTREL_PIANO_WIFI_MANAGER_H
#define MINSTREL_PIANO_WIFI_MANAGER_H

#include "wifi-remote-station-manager.h"
#include "wifi-mode.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

struct MinstrelPianoWifiRemoteStation;

/**
 * A struct to contain all information related to a data rate
 */
struct RatePowerInfo
{
  /**
   * Perfect transmission time calculation, or frame calculation
   * Given a bit rate and a packet length n bytes
   */
  Time perfectTxTime;

  uint32_t retryCount;  ///< retry limit
  uint32_t adjustedRetryCount;  ///< adjust the retry limit for this rate
  uint32_t numRateAttempt;  ///< how many number of attempts so far
  uint32_t numRateSuccess;    ///< number of successful pkts
  uint32_t prob;  ///< (# pkts success )/(# total pkts)
  uint32_t longRetry;
  uint32_t shortRetry;
  uint32_t retry;

  /**
   * EWMA calculation
   * ewma_prob =[prob *(100 - ewma_level) + (ewma_prob_old * ewma_level)]/100
   */
  uint32_t ewmaProb;

  uint32_t prevNumRateAttempt;  ///< from last rate
  uint32_t prevNumRateSuccess;  ///< from last rate
  uint64_t successHist;  ///< aggregate of all successes
  uint64_t attemptHist;  ///< aggregate of all attempts
  uint32_t throughput;  ///< throughput of a rate

  /*
  * Information for reference packets
  */
  uint32_t numRefAttempt;  ///< how many number of attempts so far
  uint32_t numRefSuccess;    ///< number of successful pkts
  uint32_t refProb;  ///< (# pkts success )/(# total pkts)

  uint32_t ewmaRefProb;

  uint32_t prevNumRefAttempt;  ///< from last rate
  uint32_t prevNumRefSuccess;  ///< from last rate
  uint64_t successRefHist;  ///< aggregate of all successes
  uint64_t attemptRefHist;  ///< aggregate of all attempts

  /*
  * Information for data packets
  */
  uint32_t numDataAttempt;  ///< how many number of attempts so far
  uint32_t numDataSuccess;    ///< number of successful pkts
  uint32_t dataProb;  ///< (# pkts success )/(# total pkts)

  uint32_t ewmaDataProb;

  uint32_t prevNumDataAttempt;  ///< from last rate
  uint32_t prevNumDataSuccess;  ///< from last rate
  uint64_t successDataHist;  ///< aggregate of all successes
  uint64_t attemptDataHist;  ///< aggregate of all attempts

  /*
  * Information for sample packets
  */
  uint32_t numSampleAttempt;  ///< how many number of attempts so far
  uint32_t numSampleSuccess;    ///< number of successful pkts
  uint32_t sampleProb;  ///< (# pkts success )/(# total pkts)

  uint32_t ewmaSampleProb;

  uint32_t prevNumSampleAttempt;  ///< from last rate
  uint32_t prevNumSampleSuccess;  ///< from last rate
  uint64_t successSampleHist;  ///< aggregate of all successes
  uint64_t attemptSampleHist;  ///< aggregate of all attempts

  /*
  * Powers for this rate
  */

  uint8_t refPower;
  uint8_t dataPower;
  uint8_t samplePower;
};

/**
 * Data structure for a Minstrel Rate table
 * A vector of a struct RateInfo
 */
typedef std::vector<struct RatePowerInfo> MinstrelPianoRate;

/**
 * Data structure for a Sample Rate table
 * A vector of a vector uint32_t
 */
typedef std::vector<std::vector<uint32_t> > SampleRate;

/**
 * \author Matias Richart
 * \brief Implementation of Minstrel Piano Rate and Power Control Algorithm
 * \based on implementation of Duy Nguyen of Minstrel Rate Control Algorithm
 * \ingroup wifi
 *
 * Implementation of Huehn, Thomas, and Cigdem Sengul. "Practical Power and Rate Control for WiFi."
 * Computer Communications and Networks (ICCCN), 2012 21st International Conference on. IEEE, 2012.
 */
class MinstrelPianoWifiManager : public WifiRemoteStationManager
{

public:
  static TypeId GetTypeId (void);
  MinstrelPianoWifiManager ();
  virtual ~MinstrelPianoWifiManager ();

  virtual void SetupPhy (Ptr<WifiPhy> phy);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

private:
  // overriden from base class
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

  /// for estimating the TxTime of a packet with a given mode
  Time GetCalcTxTime (WifiMode mode) const;
  void AddCalcTxTime (WifiMode mode, Time t);

  /// update the number of retries and reset accordingly
  void UpdateRetry (MinstrelPianoWifiRemoteStation *station);

  void UpdateAttempts (MinstrelPianoWifiRemoteStation *station);

  /// getting the next sample from Sample Table
  uint32_t GetNextSample (MinstrelPianoWifiRemoteStation *station);

  /// find a rate to use from Minstrel Piano Table
  uint32_t FindRate (MinstrelPianoWifiRemoteStation *station);

  /// find a power to use from Minstrel Piano Table
  uint8_t FindPower (MinstrelPianoWifiRemoteStation *station);

  void SetRatePower (MinstrelPianoWifiRemoteStation *station);

  /// updating the Minstrel Table every 1/10 seconds
  void UpdateStats (MinstrelPianoWifiRemoteStation *station);

  /// updating the Minstrel Piano Table every 10 packets
  void UpdatePowerStats (MinstrelPianoWifiRemoteStation *station);

  /// initialize Minstrel Table
  void RateInit (MinstrelPianoWifiRemoteStation *station);

  /// initialize Sample Table
  void InitSampleTable (MinstrelPianoWifiRemoteStation *station);

  /// printing Sample Table
  void PrintSampleTable (MinstrelPianoWifiRemoteStation *station);

  /// printing Minstrel Table
  void PrintTable (MinstrelPianoWifiRemoteStation *station);

  void CheckInit (MinstrelPianoWifiRemoteStation *station);  ///< check for initializations


  typedef std::vector<std::pair<Time,WifiMode> > TxTime;

  TxTime m_calcTxTime;  ///< to hold all the calculated TxTime for all modes
  Time m_updateStats;  ///< how frequent do we calculate the stats(1/10 seconds)
  double m_lookAroundRate;  ///< the % to try other rates than our current rate
  double m_ewmaLevel;  ///< exponential weighted moving average
  uint32_t m_segmentSize;  ///< largest allowable segment size
  uint32_t m_sampleCol;  ///< number of sample columns
  uint32_t m_pktLen;  ///< packet length used  for calculate mode TxTime
  uint32_t m_nsupported;  ///< modes supported

  double m_lookAroundSamplePower;  ///< the % to try other powers than our current power
  double m_lookAroundRefPower;  ///< the % to try other reference powers
  uint32_t m_pianoUpdateStats; ///< minimal number of packets needed for update piano stats
  uint8_t m_deltaInc; ///< how much to increase power
  uint8_t m_deltaDec; ///< how much to decrease power
  uint8_t m_delta; ///< power separation between data and sample packets
  uint32_t m_nPower;  ///< power levels supported
  double m_thInc; ///< threshold for increasing power
  double m_thDec; ///< threshold for decreasing power


  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   * The trace source fired when a txVector is asked
   */
  TracedCallback<WifiTxVector> m_getDataTxVector;

  /**
   * The trace source fired when the transmission power change
   */
  TracedCallback<uint8_t> m_powerChange;
  /**
   * The trace source fired when the transmission rate change
   */
  TracedCallback<uint32_t> m_rateChange;
};

} // namespace ns3

#endif /* MINSTREL_PIANO_WIFI_MANAGER_H */
