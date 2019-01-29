/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013
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
 * Author: Ghada Badawy <gbadawy@gmail.com>
 */

#ifndef MPDU_AGGREGATOR_H
#define MPDU_AGGREGATOR_H

#include "ns3/object.h"
#include "wifi-mode.h"
#include "qos-txop.h"
#include "ns3/nstime.h"
#include <vector>

namespace ns3 {

class AmpduSubframeHeader;
class WifiTxVector;
class Packet;
class WifiMacQueueItem;

/**
 * \brief Aggregator used to construct A-MPDUs
 * \ingroup wifi
 */
class MpduAggregator : public Object
{
public:
  /**
   * A list of deaggregated packets and their A-MPDU subframe headers.
   */
  typedef std::list<std::pair<Ptr<Packet>, AmpduSubframeHeader> > DeaggregatedMpdus;
  /**
   * A constant iterator for a list of deaggregated packets and their A-MPDU subframe headers.
   */
  typedef std::list<std::pair<Ptr<Packet>, AmpduSubframeHeader> >::const_iterator DeaggregatedMpdusCI;
  /**
   * EDCA queues typedef
   */
  typedef std::map<AcIndex, Ptr<QosTxop> > EdcaQueues;


  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  MpduAggregator ();
  virtual ~MpduAggregator ();

  /**
   * Aggregate an MPDU to an A-MPDU.
   *
   * \param mpdu the MPDU.
   * \param ampdu the A-MPDU.
   * \param isSingle whether it is a single MPDU.
   */
  static void Aggregate (Ptr<const WifiMacQueueItem> mpdu, Ptr<Packet> ampdu, bool isSingle);

  /**
   * \param packet Packet we have to insert into <i>aggregatedPacket</i>.
   * \param aggregatedPacket Packet that will contain <i>packet</i>, if aggregation is possible.
   * \param maxAmpduSize the maximum A-MPDU size.
   *
   * \return true if <i>packet</i> can be aggregated to <i>aggregatedPacket</i>, false otherwise.
   *
   * Adds <i>packet</i> to <i>aggregatedPacket</i>. In concrete aggregator's implementation is
   * specified how and if <i>packet</i> can be added to <i>aggregatedPacket</i>.
   *
   * \todo TO BE REMOVED
   */
  bool Aggregate (Ptr<const Packet> packet, Ptr<Packet> aggregatedPacket, uint32_t maxAmpduSize) const;
  /**
   * \param packet the packet we want to insert into <i>aggregatedPacket</i>.
   * \param aggregatedPacket packet that will contain the packet of size <i>packetSize</i>, if aggregation is possible.
   *
   * This method performs a VHT/HE single MPDU aggregation.
   *
   * \todo TO BE REMOVED
   */
  void AggregateSingleMpdu (Ptr<const Packet> packet, Ptr<Packet> aggregatedPacket) const;
  /**
   * \param mpdu the MPDU we want to insert into an A-MPDU subframe.
   * \param last true if it is the last MPDU.
   * \param isSingleMpdu true if it is a single MPDU
   *
   * Adds A-MPDU subframe header and padding to each MPDU that is part of an A-MPDU before it is sent.
   */
  void AddHeaderAndPad (Ptr<Packet> mpdu, bool last, bool isSingleMpdu) const;
  /**
   * \param packetSize size of the packet we want to insert into <i>aggregatedPacket</i>.
   * \param aggregatedPacket packet that will contain the packet of size <i>packetSize</i>, if aggregation is possible.
   * \param blockAckSize size of the piggybacked block ack request
   * \param maxAmpduSize the maximum A-MPDU size.
   *
   * \return true if the packet of size <i>packetSize</i> can be aggregated to <i>aggregatedPacket</i>, false otherwise.
   *
   * This method is used to determine if a packet could be aggregated to an A-MPDU without exceeding the maximum packet size.
   *
   * \todo TO BE REMOVED
   */
  bool CanBeAggregated (uint32_t packetSize, Ptr<Packet> aggregatedPacket, uint8_t blockAckSize, uint32_t maxAmpduSize) const;

  /**
   * Compute the size of the A-MPDU resulting from the aggregation of an MPDU of
   * size <i>mpduSize</i> and an A-MPDU of size <i>ampduSize</i>.
   *
   * \param mpduSize the MPDU size.
   * \param ampduSize the A-MPDU size.
   * \return the size of the resulting A-MPDU.
   */
  static uint32_t GetSizeIfAggregated (uint32_t mpduSize, uint32_t ampduSize);

  /**
   * Determine the maximum size for an A-MPDU of the given TID that can be sent
   * to the given receiver when using the given modulation class.
   *
   * \param recipient the receiver station address.
   * \param tid the TID.
   * \param modulation the modulation class.
   * \return the maximum A-MPDU size.
   */
  uint32_t GetMaxAmpduSize (Mac48Address recipient, uint8_t tid,
                            WifiModulationClass modulation) const;

  /**
   * Attempt to aggregate other MPDUs to the given MPDU, while meeting the
   * following constraints:
   *
   * - the size of the resulting A-MPDU does not exceed the maximum A-MPDU size
   * as determined for the modulation class indicated by the given TxVector
   *
   * - the time to transmit the resulting PPDU, according to the given TxVector,
   * does not exceed both the maximum PPDU duration allowed by the corresponding
   * modulation class (if any) and the given PPDU duration limit (if non null)
   *
   * For now, only non-broadcast QoS Data frames can be aggregated (do not pass
   * other types of frames to this method). MPDUs to aggregate are looked for
   * among those with the same TID and receiver as the given MPDU.
   *
   * The resulting A-MPDU is returned as a vector of the constituent MPDUs
   * (including the given MPDU), which are not actually aggregated (call the
   * Aggregate method afterwards to get the actual A-MPDU). If aggregation was
   * not possible (aggregation is disabled, there is no Block Ack agreement
   * established with the receiver, or another MPDU to aggregate was not found),
   * the returned vector is empty.
   *
   * \param mpdu the given MPDU.
   * \param txVector the TxVector used to transmit the frame
   * \param ppduDurationLimit the limit on the PPDU duration
   * \return the resulting A-MPDU, if aggregation is possible.
   */
  std::vector<Ptr<WifiMacQueueItem>> GetNextAmpdu (Ptr<const WifiMacQueueItem> mpdu,
                                                   WifiTxVector txVector,
                                                   Time ppduDurationLimit = Seconds (0)) const;

  /**
   * Deaggregates an A-MPDU by removing the A-MPDU subframe header and padding.
   *
   * \param aggregatedPacket the aggregated packet
   * \return list of deaggragted packets and their A-MPDU subframe headers
   */
  static DeaggregatedMpdus Deaggregate (Ptr<Packet> aggregatedPacket);

  /**
   * Set the map of EDCA queues.
   *
   * \param edcaQueues the map of EDCA queues.
   */
  void SetEdcaQueues (EdcaQueues edcaQueues);

private:
  /**
   * \param ampduSize the size of the A-MPDU that needs to be padded
   * \return the size of the padding that must be added to the end of an A-MPDU
   *
   * Calculates how much padding must be added to the end of an A-MPDU of the given size
   * (once another MPDU is aggregated).
   * Each A-MPDU subframe is padded so that its length is multiple of 4 octets.
   */
  static uint8_t CalculatePadding (uint32_t ampduSize);

  EdcaQueues m_edca;   //!< the map of EDCA queues
};

}  //namespace ns3

#endif /* MPDU_AGGREGATOR_H */
