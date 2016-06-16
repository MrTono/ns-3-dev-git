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

#ifndef PRC_MONITORDCE2_H
#define PRC_MONITORDCE2_H

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

class PrcMonitordce2 : public Application
{
public:
  static TypeId
  GetTypeId (void);

  PrcMonitordce2 ();

  virtual ~PrcMonitordce2 ();

  /**
   * \brief set the rnr address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetUpstream (Ipv4Address ip, uint16_t port);
  void SetNodeContainer (NodeContainer apNodes);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void SendSub (void);
 // double* GetApsPower(NodeContainer apNodes); // agregado
  Ssid GetSsid(int nNode, NodeContainer apNodes);
  void Associate(Ssid ssid);
  void UpdateStats (void);
 // double RxPower (Ptr<Node> ap, Ptr<Node> sta); // agregado
 Ptr<Packet> CreateTrapPacket(std::string mib, double value, Mac48Address address);
 // Ptr<Packet> CreateTrapPacket(std::string mib, double value);
  /**
   * \brief create a Packet with a suscription
   * \param filter string of filters with the form "id1=value\n"id2=value\n"...
   */
  Ptr<Packet> CreateSubscriptionPacket(std::string filter);

  Ptr<Socket> m_socket;
  EventId m_sendEvent;
  bool m_connected;
  Ipv4Address m_rnrAddress;
  uint16_t m_rnrPort;
  Ipv4Address m_myAddress;
  uint16_t m_myPort;
  uint32_t m_updateTimer;

};

} // namespace ns3

#endif // PRC_MONITORDCE2_H
