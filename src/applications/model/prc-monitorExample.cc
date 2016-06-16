///* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
///*
// * Copyright (c) 2013 Universidad de la Republica - Uruguay
// *
// * This program is free software; you can redistribute it and/or modify
// * it under the terms of the GNU General Public License version 2 as
// * published by the Free Software Foundation;
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program; if not, write to the Free Software
// * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// *
// * Author: Matías Richart <mrichart@fing.edu.uy>
// */
//
//#include "ns3/wifi-remote-station-manager.h"
//#include "ns3/rule-based-wifi-manager.h"
//#include "ns3/wifi-net-device.h"
//#include "ns3/log.h"
//#include "ns3/ipv4-address.h"
//#include "ns3/nstime.h"
//#include "ns3/inet-socket-address.h"
//#include "ns3/socket.h"
//#include "ns3/simulator.h"
//#include "ns3/socket-factory.h"
//#include "ns3/packet.h"
//#include "ns3/uinteger.h"
//#include "ns3/yans-wifi-phy.h"
//#include "prc-monitorExample.h"
//#include <stdlib.h>
//#include <stdio.h>
//#include <errno.h>
//#include <string.h>
//#include <iostream>
//#include <fstream>
//#include "ns3/sta-wifi-mac.h" //agregadas
//#include "ns3/wifi-net-device.h"//agregadas
//#include "ns3/propagation-loss-model.h"
//#include "ns3/mobility-model.h"
//#include "ns3/ipv4-interface-address.h"
//#include "ns3/ipv4-address-generator.h"
//#include "seq-ts-header.h"
//
//NS_LOG_COMPONENT_DEFINE ("PrcMonitorExample");
//
//namespace ns3 {
//
//NS_OBJECT_ENSURE_REGISTERED (PrcMonitorExample);
//
//TypeId
//PrcMonitorExample::GetTypeId (void)
//{
//	  static TypeId tid = TypeId ("ns3::PrcMonitorExample")
//	    .SetParent<Application> ()
//	    .AddConstructor<PrcMonitorExample> ()
//	    .AddAttribute ("Upstream",
//	                   "The destination Ipv4Address of the outbound packets",
//	                   Ipv4AddressValue ("127.0.0.1"),
//	                   MakeIpv4AddressAccessor (&PrcMonitorExample::m_rnrAddress),
//	                   MakeIpv4AddressChecker ())
//	    .AddAttribute ("RnrPort", "The destination port of the outbound packets",
//	                   UintegerValue (8182),
//	                   MakeUintegerAccessor (&PrcMonitorExample::m_rnrPort),
//	                   MakeUintegerChecker<uint16_t> ())
//	   .AddAttribute ("MyHost",
//					  "The source Ipv4Address of the outbound packets",
//					  Ipv4AddressValue ("127.0.0.1"),
//					  MakeIpv4AddressAccessor (&PrcMonitorExample::m_myAddress),
//					  MakeIpv4AddressChecker ())
//	   .AddAttribute ("MyPort", "The source port of the outbound packets",
//					  UintegerValue (9591),
//					  MakeUintegerAccessor (&PrcMonitorExample::m_myPort),
//					  MakeUintegerChecker<uint16_t> ())
//	   .AddAttribute ("UpdateTimer", "Time for updating stats in seconds",
//					  UintegerValue (1),
//					  MakeUintegerAccessor (&PrcMonitorExample::m_updateTimer),
//					  MakeUintegerChecker<uint32_t> ())
//	    ;
//	  return tid;
//}
//
//PrcMonitorExample::PrcMonitorExample ()
//  : m_socket (0),
//	m_sendEvent (EventId ()),
//	m_connected (false)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//}
//
//
//PrcMonitorExample::~PrcMonitorExample ()
//{
//  NS_LOG_FUNCTION_NOARGS ();
//}
//
///*
//*returns de reception power in function of the transmission
//*power and the nodes movility model
//*/
//
//double
//PrcMonitorExample::RxPower (Ptr<Node> ap, Ptr<Node> sta)
//{
//  Ptr<LogDistancePropagationLossModel> o = CreateObject<LogDistancePropagationLossModel>();
//  double power = o->CalcRxPower (16.0206, ap->GetObject<MobilityModel>(),sta->GetObject<MobilityModel>()); //tx power in dbm, 16 is the default one, src, dest.
//  return power;
//}
//
///*
//* Returns the better ap as the one witch I can get best reception power
//*/
//int
//PrcMonitorExample::GetBetter(NodeContainer apNodes)
//{
//  Ptr<Node> sta = GetNode();
//  double bestPower = -200;
//  int bestNode = 0;
//  int nNodes = apNodes.GetN();
//  for (int i=0; i<nNodes; i++)
//  {
//    double power = RxPower(apNodes.Get(i), sta);
//    std::cout << "Power: " << power << std::endl;
//    if (power > bestPower)
//    {
//      bestPower = power;
//      bestNode = i;
//    }
//  }
//  return bestNode;
//}
//
///*
//* Returns a list with the power of all APs
//*/
//double*
//PrcMonitorExample::GetApsPower(NodeContainer apNodes)
//{
//  Ptr<Node> sta = GetNode();
//  int nNodes = apNodes.GetN();
//  double* result = new double[nNodes];
//  for (int i=0; i<nNodes; i++)
//  {
//    double power = RxPower(apNodes.Get(i), sta);
//    result[i] = power;
//    std::cout << "Power: " << power << std::endl;
//  }
//  return result;
//}
//
///*
//* Returns the SSID of a node
//*/
//Ssid
//PrcMonitorExample::GetSsid(int nNode, NodeContainer apNodes)
//{
//  Ptr<Node> node = apNodes.Get(nNode);
//  Ptr<NetDevice> device = node->GetDevice(1);
//  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
//  Ptr<WifiMac> mac = wifiDevice->GetMac();
//  Ssid ssid = mac->GetSsid();
//  std::cout << "Ssid: " << ssid << std::endl;
//  return ssid;
//}
//
///*
//* Associate the local node to the node with ssid passed
//*/
//void
//PrcMonitorExample::Associate(Ssid ssid)
//{
//  Ptr<Node> node = GetNode();
//  Ptr<NetDevice> device = node->GetDevice(0);
//  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
//  Ptr<WifiMac> mac = wifiDevice->GetMac();
//  Ptr<StaWifiMac> nqMac = DynamicCast<StaWifiMac> (mac);
//  mac->SetSsid(ssid);
//  nqMac->ReAssociated(); //new function added to nqMac to deassociate and reassociate
//}
//
///*
//* Set an IPv4 address to the node passed
//*/
//
//void
//PrcMonitorExample::SetIpv4Address(Ptr<Node> node, Ipv4Address addr, Ipv4Mask mask)
//{
//  Ptr<NetDevice> device = node->GetDevice(0);
//  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
//  NS_ASSERT_MSG (ipv4, "Ipv4AddressHelper::Allocate(): Bad ipv4");
//
//  int32_t interface = ipv4->GetInterfaceForDevice (device);
//  if (interface == -1)
//  {
//    interface = ipv4->AddInterface (device);
//  }
//  NS_ASSERT_MSG (interface >= 0, "Ipv4AddressHelper::Allocate(): "
//     "Interface index not found");
//
//  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (addr, mask);
//  ipv4->RemoveAddress (interface, 0);
//  ipv4->AddAddress (interface, ipv4Addr);
//  ipv4->SetMetric (interface, 1);
//  ipv4->SetUp (interface);
//}
//
///*
//* Set the global variable that contains a container for the APs of the simulation
//*/
//void
//PrcMonitorExample::SetNodeContainer (NodeContainer apNodes)
//{
//  m_apNodes = apNodes;
//}
//
//void
//PrcMonitorExample::SetRemote (Ipv4Address ip, uint16_t port)
//{
//  m_peerAddress = ip;
//  m_peerPort = port;
//}
//
//void
//PrcMonitorExample::DoDispose (void)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//  Application::DoDispose ();
//}
//
///*
//* Send the list of visible aps to the node
//*
//*/
//
//void
//PrcMonitorExample::List (NodeContainer apNodes)
//{
//  double* power = GetApsPower(apNodes);
//  int cantAps = apNodes.GetN();
//  double* visibleAps = new double[cantAps];
//  int posVisibleAps = 0;
//  for(int i=0; i<cantAps; i++)
//  {
//    if (power[i]>-100)
//    {
//      visibleAps[posVisibleAps] = i;
//      posVisibleAps++;
//    }
//  }
//  int size = 0;
//  char* result = NULL;
//  char* aux2 = NULL;
//  char* string = NULL;
//  char* aux = NULL;
//  for(int i=0; i<posVisibleAps; i++)
//  {
//    Ssid ssid = GetSsid(visibleAps[i], apNodes);
//    string = ssid.PeekString();
//    size += strlen(string) + 1;
//    aux2 = new char[strlen(string) + 2];
//    strcpy(aux2,string);
//    strcat(aux2,";");
//    if (result != NULL)
//    {
//      aux = new char[size + 1];
//      strcpy(aux, result);
//      delete result;
//      result = strcat(aux, aux2);
//      delete aux2;
//    }else
//      result = aux2;
//  }
//
//  //if there is no ap on range do not send the list
//  if (result != NULL){
//    printf("%s\n", result);
//
//    m_size = size;
//    m_data = new uint8_t [m_size];
//    memcpy (m_data, result, m_size);
//    delete result;
//
//    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
//    m_sendEvent = Simulator::Schedule (Seconds (0.0), &PrcMonitorExample::SendList, this);
//  }
//}
//
//void
//PrcMonitorExample::StartApplication (void)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//
//  if (m_socket == 0)
//    {
//      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
//      m_socket = Socket::CreateSocket (GetNode (), tid);
//      uint16_t port = 4000;
//      InetSocketAddress local = InetSocketAddress (Ipv4Address ("127.0.0.1"),
//                                                   port);
//      m_socket->Bind (local);
//    }
//
//  m_socket->SetRecvCallback (MakeCallback (&PrcMonitorExample::HandleRead, this));
//
//}
//
//void
//PrcMonitorExample::StopApplication ()
//{
//  NS_LOG_FUNCTION_NOARGS ();
//  Simulator::Cancel (m_sendEvent);
//}
//
//void
//PrcMonitorExample::SendList (void)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//  NS_ASSERT (m_sendEvent.IsExpired ());
//  Ptr<Packet> p = Create<Packet> (m_data, m_size);
//
//  InetSocketAddress to = InetSocketAddress (m_peerAddress, m_peerPort);
//
//  if ((m_socket->SendTo (p, 0, to)) >= 0)
//    {
//      ++m_sent;
//     NS_LOG_INFO (m_size << " bytes to " << m_peerAddress << " Uid: " << p->GetUid ()
//                  << " Time: " << (Simulator::Now ()).GetSeconds ());
//    }
//  else
//    {
//      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
//                   << m_peerAddress);
//    }
//}
//
//void
//PrcMonitorExample::Send (void)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//  NS_ASSERT (m_sendEvent.IsExpired ());
//  SeqTsHeader seqTs;
//  seqTs.SetSeq (m_sent);
//  Ptr<Packet> p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
//  p->AddHeader (seqTs);
//
//  if ((m_socket->Send (p)) >= 0)
//    {
//
//      ++m_sent;
//     NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//                  << m_peerAddress << " Uid: " << p->GetUid ()
//                  << " Time: " << (Simulator::Now ()).GetSeconds ());
//
//    }
//  else
//    {
//      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
//                   << m_peerAddress);
//    }
//
//  if (m_sent < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &PrcMonitorExample::Send, this);
//    }
//}

//void
//PrcMonitorExample::HandleRead (Ptr<Socket> socket)
//{
//  Ptr<Packet> packet;
//  Address from;
//  while (packet == socket->RecvFrom (from))
//    {
//      if (packet->GetSize () > 0)
//        {
//          uint8_t* buffer = new uint8_t[100];
//          int size = packet->CopyData(buffer, 100);
//          char* aux = new char[100];
//          memcpy(aux, buffer, 100);
//          if (strncmp (aux, "list", size) == 0)
//          {
//            List(m_apNodes);
//          }
//          else if (strncmp (aux, "associate", 9) == 0)
//          {
//            std::cout << "Associate" << std::endl;
//            std::cout << aux+10 << std::endl;
//            Associate(Ssid(aux+10, size-10));
//            if (strncmp (aux+10, "ap-0", 4) == 0){
//              SetIpv4Address(GetNode (), Ipv4Address("10.1.0.2"), Ipv4Mask("255.255.255.0"));
//            } else  if (strncmp (aux+10, "ap-1", 4) == 0){
//              SetIpv4Address(GetNode (), Ipv4Address("10.1.1.2"), Ipv4Mask("255.255.255.0"));
//            } else if (strncmp (aux+10, "ap-2", 4) == 0){
//              SetIpv4Address(GetNode (), Ipv4Address("10.1.2.2"), Ipv4Mask("255.255.255.0"));
//            } else if (strncmp (aux+10, "ap-3", 4) == 0){
//              SetIpv4Address(GetNode (), Ipv4Address("10.1.3.2"), Ipv4Mask("255.255.255.0"));
//            }
//          }
//        }
//    }
//  m_socket->SetRecvCallback (MakeCallback (&PrcMonitorExample::HandleRead, this));
//}
//
//} // Namespace ns3
