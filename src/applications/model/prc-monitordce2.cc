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
#include "prc-monitordce2.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "ns3/sta-wifi-mac.h" //agregadas
#include "ns3/wifi-net-device.h"//agregadas


NS_LOG_COMPONENT_DEFINE ("PrcMonitordce2");





namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PrcMonitordce2);

TypeId
PrcMonitordce2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PrcMonitordce2")
    .SetParent<Application> ()
    .AddConstructor<PrcMonitordce2> ()
    .AddAttribute ("Upstream",
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue ("127.0.0.1"),
                   MakeIpv4AddressAccessor (&PrcMonitordce2::m_rnrAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RnrPort", "The destination port of the outbound packets",
                   UintegerValue (8182),
                   MakeUintegerAccessor (&PrcMonitordce2::m_rnrPort),
                   MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("MyHost",
				  "The source Ipv4Address of the outbound packets",
				  Ipv4AddressValue ("127.0.0.1"),
				  MakeIpv4AddressAccessor (&PrcMonitordce2::m_myAddress),
				  MakeIpv4AddressChecker ())
   .AddAttribute ("MyPort", "The source port of the outbound packets",
				  UintegerValue (9591),
				  MakeUintegerAccessor (&PrcMonitordce2::m_myPort),
				  MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("UpdateTimer", "Time for updating stats in seconds",
				  UintegerValue (2),
				  MakeUintegerAccessor (&PrcMonitordce2::m_updateTimer),
				  MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

PrcMonitordce2::PrcMonitordce2 ()
  : m_socket (0),
	m_sendEvent (EventId ()),
	m_connected (false)
{
  NS_LOG_FUNCTION_NOARGS ();
}

PrcMonitordce2::~PrcMonitordce2 ()
{
  NS_LOG_FUNCTION_NOARGS ();
}


void
PrcMonitordce2::SetUpstream (Ipv4Address ip, uint16_t port)
{
  m_rnrAddress = ip;
  m_rnrPort = port;
}

void
PrcMonitordce2::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
PrcMonitordce2::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO("HAPPY");
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (m_myAddress, m_myPort);
      m_socket->Bind (local);
      InetSocketAddress rnr = InetSocketAddress (m_rnrAddress, m_rnrPort);
      m_socket->Connect(rnr);
      m_socket->SetConnectCallback (
        MakeCallback (&PrcMonitordce2::ConnectionSucceeded, this),
        MakeCallback (&PrcMonitordce2::ConnectionFailed, this));
      m_sendEvent = Simulator::Schedule (Seconds (m_updateTimer), &PrcMonitordce2::UpdateStats, this);
    }
}

void
PrcMonitordce2::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

void
PrcMonitordce2::SendSub ()
{
  //Create and send a subscription for all notifications to this host
  std::ostringstream filter;
  filter << "target_service=NS3-RMOON\n";
  Ptr<Packet> sub = CreateSubscriptionPacket(filter.str());
  int error = m_socket->Send(sub);
  if (error >= 0)
    NS_LOG_INFO("Subscription sent, " << error << " bytes");
}


void
PrcMonitordce2::UpdateStats ()
{
  NS_LOG_FUNCTION_NOARGS ();
 //int numsta;
  if (m_connected && m_sendEvent.IsExpired ())
    {
      Ptr<Node> node = GetNode();
      Ptr<NetDevice> device = node->GetDevice(1); // It has to be the wifi device
      Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
      Ptr<WifiRemoteStationManager> manager = wifiDevice->GetRemoteStationManager();
      Ptr<RuleBasedWifiManager> wifiManager = DynamicCast<RuleBasedWifiManager> (manager);

//      Ptr<WifiPhy> phy = wifiDevice->GetPhy();   //Para obtener la potencia del AP y bajarlo cuando queramos
//      Ptr<YansWifiPhy> yansPhy = DynamicCast<YansWifiPhy> (phy);


     // double powerStart1 = yansPhy->GetTxPowerStart();  //get power
   //  double powerEnd1 = yansPhy->GetTxPowerEnd();
//
//      NS_LOG_INFO ("PotenciaInicial:  "<<powerStart1);
//
//      powerStart1 = (powerStart1 -1);
//      powerEnd1 = (powerEnd1-1);
//      yansPhy->SetTxPowerStart(powerStart1);
//      yansPhy->SetTxPowerEnd(powerEnd1);
//
//      NS_LOG_INFO ("PotenciaFinal:  "<<powerStart1);




      std::vector <Mac48Address> stations = manager->GetAssociatedStations();

      uint32_t ap = node->GetId(); // Aqui obtenemos el nodo donde esta el AP
      //NS_LOG_INFO ("Node  "<< ap);

      std::stringstream ss; // Convertir a String la información del nodo
      ss << ap;
      std::string str2;
      ss >> str2;

     // NS_LOG_INFO ("Node  "<< str2);

      Ptr<WifiMac> mac = wifiDevice->GetMac();
      Ssid ssid = mac->GetSsid();
      //std::cout << "Ssid: " << ssid << std::endl;
      char *ssidap = ssid.PeekString();
      std:: string str(ssidap);
     // NS_LOG_INFO("Ssid: " << str);
      std:: string str3= str+str2; //Sumar string para indicar a que AP esta conectado
    //  NS_LOG_INFO ("AP: "<< str3);

      std:: string str1( "target_service="+str3+"/lupa/pdp\n");
      NS_LOG_INFO ("path: "<< str1);


      for (std::vector<Mac48Address>::const_iterator i = stations.begin (); i != stations.end (); i++)
        {
          Ptr<Packet> p;

          std::vector <Mac48Address> stations = manager->GetAssociatedStations();

          /* Send number of Associated Stations*/
          int NClientsConnected = stations.size();

       	  NS_LOG_INFO ("Number of Associated stations:  "<<NClientsConnected);
       	NS_LOG_UNCOND ((Simulator::Now ()).GetSeconds () << " " << str3 << " " << NClientsConnected);

  	  	// p=CreateTrapPacket ("NClientsConnected",NClientsConnected);
  	  	p=CreateTrapPacket ("NClientsConnected",NClientsConnected, *i);
  	  	 if ((m_socket->Send(p)) >= 0)
           {
             NS_LOG_INFO ("NClientsConnected event sent to RNR" << " Time: " << (Simulator::Now ()).GetSeconds () << " Stations: " << (int)NClientsConnected << " station:" << *i);
           }
         else
           {
             NS_LOG_INFO ("Error while sending event to the RNR " << m_socket->GetErrno());
           }


  	  	}
    }
  else
    NS_LOG_INFO ("**Updatedfff stats error**");


  m_sendEvent = Simulator::Schedule (Seconds (m_updateTimer), &PrcMonitordce2::UpdateStats, this);
}

void
PrcMonitordce2::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("PrcMonitor Connection succeeded");
  m_connected = true;
  SendSub();
}

void
PrcMonitordce2::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("PrcMonitor, Connection Failed");
}

Ptr<Packet>
PrcMonitordce2::CreateTrapPacket(std::string mib, double value, Mac48Address address)
//PrcMonitordce2::CreateTrapPacket(std::string mib, double value)
{
	 Ptr<Node> node = GetNode();
	 Ptr<NetDevice> device = node->GetDevice(1); // It has to be the wifi device
	 Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
	 Ptr<WifiMac> mac = wifiDevice->GetMac();
	 Ssid ssid = mac->GetSsid();
	 char *ssidap = ssid.PeekString();
	 std:: string str(ssidap);
	 uint32_t ap = node->GetId(); // Aqui obtenemos el nodo donde esta el AP
     std::stringstream ss; // Convertir a String la información del nodo
     ss << ap;
     std::string str2;
     ss >> str2;
     std:: string str3= str+str2; //Sumar string para indicar a que AP esta conectado
     std:: string str1( "target_service="+str3+"/lupa/pdp\n");





	Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();

	uint32_t nid = urv->GetInteger(0, 999999);
    std::ostringstream msg;
    msg 	<< "NOTIFICATION\n"
    		<< "notification_id=" << nid << "\n"
    		<< "message_type=trap\n"
    		<< "host=127.0.0.1\n"
    		<< "service=NS3-RMOON\n"
    		<< str1
    		<< "target_host=127.0.0.1\n"
    		<< "timestamp=" << Simulator::Now () << "\n"
    		<< "mib=" << mib << "\n"
    		<< "value=" << value << "\n"
 //   		<< "station=" << address << "\n"
    		<<"END\n";

    size_t size = strlen(msg.str().c_str());
    uint8_t* data = new uint8_t [size];
    memcpy (data, msg.str().c_str(), size);
    Ptr<Packet> p = Create<Packet> (data, size);
    return p;
}


Ptr<Packet>
PrcMonitordce2::CreateSubscriptionPacket(std::string filter)
{
	Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
	uint32_t sid = urv->GetInteger(0, 999999);
	int ttl = 20; //XXX
    std::ostringstream msg;
    msg 	<< "SUBSCRIBE\n"
    		<< "subscription_id=" << sid << "\n"
    		<< "host=127.0.0.1\n"
    		<< "service=NS3-RMOON\n"
    		<< "timestamp=" << Simulator::Now () << "\n"
    		<< "ttl=" << ttl << "\n"
    		<< "FILTER\n"
    		<< filter
    		<< "END\n";

    size_t size = strlen(msg.str().c_str());
    uint8_t* data = new uint8_t [size];
    memcpy (data, msg.str().c_str(), size);
    Ptr<Packet> p = Create<Packet> (data, size);
    return p;
}

} // Namespace ns3
