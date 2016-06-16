/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Universidad de la Republica - Uruguay
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
 * Author: Matías Richart <mrichart@fing.edu.uy>
 */

#ifndef PRC_MONITOR_EXAMPLE_H
#define PRC_MONITOR_EXAMPLE_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/ssid.h"
#include "ns3/node-container.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup Station2server
 * \class Station2
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class PrcMonitorExample : public Application
{
public:
  static TypeId
  GetTypeId (void);

  PrcMonitorExample ();

  virtual ~PrcMonitorExample ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);

  void SetNodeContainer (NodeContainer apNodes);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  double RxPower (Ptr<Node> ap, Ptr<Node> sta);
  int GetBetter(NodeContainer apNodes);
  Ssid GetSsid(int nNode, NodeContainer apNodes);
  void Associate(Ssid ssid);
  void SetIpv4Address(Ptr<Node> node, Ipv4Address addr, Ipv4Mask mask);
  double* GetApsPower(NodeContainer apNodes);
  void List (NodeContainer apNodes);
  void HandleRead (Ptr<Socket> socket);

  void ScheduleTransmit (Time dt);
  void Send (void);

  void SendList (void);

  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;
  NodeContainer m_apNodes;
  uint8_t* m_data;





};

} // namespace ns3

#endif // PRC_MONITOR_EXAMPLE_H
