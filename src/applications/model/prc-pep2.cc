
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

#include "ns3/wifi-remote-station-manager.h"
#include "ns3/rule-based-wifi-manager.h"
#include "ns3/wifi-net-device.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-phy.h"
#include "prc-pep2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("PrcPep2");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PrcPep2);

TypeId
PrcPep2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PrcPep2")
    .SetParent<Application> ()
    .AddConstructor<PrcPep2> ()
    .AddAttribute ("Upstream",
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue ("127.0.0.1"),
                   MakeIpv4AddressAccessor (&PrcPep2::m_rnrAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RnrPort", "The destination port of the outbound packets",
                   UintegerValue (8182),
                   MakeUintegerAccessor (&PrcPep2::m_rnrPort),
                   MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("MyHost",
				  "The source Ipv4Address of the outbound packets",
				  Ipv4AddressValue ("127.0.0.1"),
				  MakeIpv4AddressAccessor (&PrcPep2::m_myAddress),
				  MakeIpv4AddressChecker ())
   .AddAttribute ("MyPort", "The source port of the outbound packets",
				  UintegerValue (9592),
				  MakeUintegerAccessor (&PrcPep2::m_myPort),
				  MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("UpdateTimer", "Time for updating stats in seconds",
				  UintegerValue (10),
				  MakeUintegerAccessor (&PrcPep2::m_updateTimer),
				  MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

PrcPep2::PrcPep2 ()
  : m_socket (0),
    m_connected (false),
	m_sendEvent (EventId ())
{
  NS_LOG_FUNCTION_NOARGS ();
}

PrcPep2::~PrcPep2 ()
{
  NS_LOG_FUNCTION_NOARGS ();
}


void
PrcPep2::SetRemote (Ipv4Address ip, uint16_t port)
{
  m_rnrAddress = ip;
  m_rnrPort = port;
}

void
PrcPep2::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}



void
PrcPep2::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (m_myAddress, m_myPort);
      m_socket->Bind (local);
      InetSocketAddress rnr = InetSocketAddress (m_rnrAddress, m_rnrPort);
      m_socket->Connect(rnr);
      m_socket->SetConnectCallback (
        MakeCallback (&PrcPep2::ConnectionSucceeded, this),
        MakeCallback (&PrcPep2::ConnectionFailed, this));
    }
  m_socket->SetRecvCallback (MakeCallback (&PrcPep2::HandleRead, this));
}

void
PrcPep2::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

void
PrcPep2::SendSub ()
{
  //Create and send a subscription for all notifications to this host
  std::ostringstream filter;
  filter << "target_service=NS3-PEP\n";
  Ptr<Packet> sub = CreateSubscriptionPacket(filter.str());
  int error = m_socket->Send(sub);
  if (error >= 0)
    NS_LOG_INFO("Subscription sent, " << error << " bytes");
}

void
PrcPep2::HandleRead (Ptr<Socket> socket)
{
  if (m_connected)
    {
      Ptr<Packet> packet;
      Ptr<Node> node = GetNode();
      Ptr<NetDevice> device = node->GetDevice(1);
      Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
      Ptr<WifiPhy> phy = wifiDevice->GetPhy();   //Para obtener la potencia del AP y bajarlo cuando queramos
      Ptr<YansWifiPhy> yansPhy = DynamicCast<YansWifiPhy> (phy);
     // Ptr<WifiRemoteStationManager> manager = wifiDevice->GetRemoteStationManager();
     // Ptr<RuleBasedWifiManager> wifiManager = DynamicCast<RuleBasedWifiManager> (manager);

      while (packet = socket->Recv())
        {
  	   	   NS_LOG_APPEND_CONTEXT;
          NS_LOG_INFO ("PrcPep2: Packet received, " << packet->GetSize() << " bytes");
          if (packet->GetSize () > 0)
            {
              Command2 cmd = ParseActionPacket(packet);

              if (cmd.action == "decrease_power")
                {
                  NS_LOG_INFO ("Decrease power in " << cmd.level << " for station " << cmd.station);

//                  double powerStart1 = yansPhy->GetTxPowerStart();  //get power
//                  double powerEnd1 = yansPhy->GetTxPowerEnd();
//                  NS_LOG_INFO ("PotenciaInicial:  "<<powerStart1);
//                  powerStart1 = (powerStart1 -1);
//                  powerEnd1 = (powerEnd1-1);
//
//                  yansPhy->SetTxPowerStart(powerStart1);
//                  yansPhy->SetTxPowerEnd(powerEnd1);
//                  NS_LOG_INFO ("PotenciaFinal:  "<<powerStart1);

                }
              else if (cmd.action == "increase_power")
                {
                  NS_LOG_INFO ("Increase power in " << cmd.level << " for station " << cmd.station);
//
//                  double powerStart1 = yansPhy->GetTxPowerStart();  //get power
//                  double powerEnd1 = yansPhy->GetTxPowerEnd();
//
//                  powerStart1 = (powerStart1 +1);
//                  powerEnd1 = (powerEnd1+1);
//
//                  yansPhy->SetTxPowerStart(powerStart1);
//                  yansPhy->SetTxPowerEnd(powerEnd1);
//                  NS_LOG_INFO ("PotenciaFinal:  "<<powerStart1);

                 // Mac48Address address = Mac48Address(cmd.station.c_str());

                 // Simulator::Schedule (Seconds(0), &RuleBasedWifiManager::IncreasePower, wifiManager, address, cmd.level);
                }
            }
        }
    }
  else
    {
      NS_LOG_INFO ("PrcPep: Error in HandleRead, not connected");
    }
  m_socket->SetRecvCallback (MakeCallback (&PrcPep2::HandleRead, this));
}

void
PrcPep2::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("PrcPep Connection succeeded");
  m_connected = true;
  SendSub();
}

void
PrcPep2::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("PrcPep, Connection Failed");
}

Command2
PrcPep2::ParseActionPacket(Ptr<Packet> p)
{
    std::ostringstream msg;
    msg << p->PeekData() << "\n";
    //NS_LOG_INFO ("***PrcPep, parsing message***");
    //NS_LOG_INFO (msg.str());
    size_t ini = msg.str().find("Command=", 0);
    size_t end = msg.str().find("\n", ini);
    std::string action = "NULL";
    if (ini != msg.str().npos)
    	action = msg.str().substr(ini+8, end-ini-8);
    NS_LOG_INFO ("=action: " << action);
    ini = msg.str().find("level=", 0);
    end = msg.str().find("\n", ini);
    std::string level = "NULL";
    if (ini != msg.str().npos)
    	level = msg.str().substr(ini+6, end-ini-6);
    NS_LOG_INFO ("=level: " << level);
    ini = msg.str().find("station=", 0);
    end = msg.str().find("\n", ini);
    std::string station = "NULL";
    if (ini != msg.str().npos)
    	station = msg.str().substr(ini+8, end-ini-8);
    NS_LOG_INFO ("=sta: " << station);
    Command2 a;
    a.action = action;
    a.level = atoi(level.c_str());
    a.station = station;
    return a;
}

Ptr<Packet>
PrcPep2::CreateSubscriptionPacket(std::string filter)
{
	Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
	uint32_t sid = urv->GetInteger(0, 999999);
	int ttl = 20; //XXX
    std::ostringstream msg;
    msg 	<< "SUBSCRIBE\n"
    		<< "subscription_id=" << sid << "\n"
    		<< "host=" << GetNode()->GetId() << "\n"
    		<< "service=NS3-PEP\n"
    		<< "timestamp=" << Simulator::Now () << "\n"
    		<< "ttl=" << ttl << "\n"
    		<< "FILTER\n"
    		<< filter
    		<<"END\n";

    size_t size = strlen(msg.str().c_str());
    uint8_t* data = new uint8_t [size];
    memcpy (data, msg.str().c_str(), size);
    Ptr<Packet> p = Create<Packet> (data, size);
    return p;
}

} // Namespace ns3
