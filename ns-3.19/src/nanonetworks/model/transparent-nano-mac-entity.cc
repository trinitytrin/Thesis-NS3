/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013,2014 TELEMATICS LAB, DEI - Politecnico di Bari
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
 * Author: Giuseppe Piro <peppe@giuseppepiro.com>, <g.piro@poliba.it>
 */


#include "transparent-nano-mac-entity.h"
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include "simple-nano-device.h"
#include "nano-mac-queue.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
 #include "ns3/packet.h"
#include "ns3/config.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <list>
#include <string.h>



NS_LOG_COMPONENT_DEFINE ("TransparentNanoMacEntity");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (TransparentNanoMacEntity);

TypeId TransparentNanoMacEntity::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TransparentNanoMacEntity")
    .SetParent<NanoMacEntity> ()
     .AddTraceSource ("PhyRxInfo",
                       "Trace source indicating a packet has been received successfully and its info retreiving ",
                        MakeTraceSourceAccessor (&TransparentNanoMacEntity::m_phyRxInfoTrace))
     .AddTraceSource ("PhyTxInfo",
                       "Trace source indicating a packet has been Transmitted successfully and its info retreiving ",
                        MakeTraceSourceAccessor (&TransparentNanoMacEntity::m_phyTxInfoTrace))
        ;
  return tid;
}


TransparentNanoMacEntity::TransparentNanoMacEntity ()
{
  SetMacQueue (CreateObject <NanoMacQueue> ());
  SetDevice (0);
  Simulator::Schedule (Seconds(0.001), &NanoMacEntity::CheckForNeighbors, this);
}


TransparentNanoMacEntity::~TransparentNanoMacEntity ()
{
}

void 
TransparentNanoMacEntity::DoDispose (void)
{
  NanoMacEntity::DoDispose ();
}

void
TransparentNanoMacEntity::DoSendPacket ()
{
  if (m_queue.size () > 0)
    {
	  NS_LOG_FUNCTION (this);
	  Ptr<NanoSpectrumPhy> phy = GetDevice ()->GetPhy ();

	  Ptr<Packet> p = (m_queue.front ())->Copy ();
	  m_queue.pop_front ();
	  phy->StartTx (p);

	  if (m_queue.size () > 0)
		{
		  double txtime = phy->GetTxDuration (p);
		  Simulator::Schedule (FemtoSeconds (txtime), &TransparentNanoMacEntity::DoSendPacket, this);
		}
    }
}


void
TransparentNanoMacEntity::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  NanoMacHeader header;
  uint32_t src = GetDevice ()->GetNode ()->GetId ();
  uint32_t dst = 0;
  header.SetSource (src);
  header.SetDestination (dst);

  NS_LOG_FUNCTION (this << "mac header" << header);
  p->AddHeader (header);

  m_queue.push_back (p);

  if (m_queue.size () == 1)
	  DoSendPacket ();
}

void
TransparentNanoMacEntity::BroadcastPacket (Ptr<Packet> pkt)
{
}

void
TransparentNanoMacEntity::Send (Ptr<Packet> p, uint32_t dst)
{
  NS_LOG_FUNCTION (this << p);

  

  NanoMacHeader header;
  uint32_t src = GetDevice ()->GetNode ()->GetId ();
  header.SetSource (src);
  header.SetDestination (dst);

  NS_LOG_FUNCTION (this << "mac header" << header);
  p->AddHeader (header);

  m_queue.push_back (p);
        this->m_phyTxInfoTrace(p);
       // NS_LOG_UNCOND ("\n Sending packet in mac layer from node "<< src <<" to "<< dst << "\n");
  DoSendPacket ();
}

void
TransparentNanoMacEntity::Receive (Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this);
        
        Ptr<Packet> p = pkt->Copy ();
        
        //NS_LOG_UNCOND ("\n packet received in mac layer by node "<< GetDevice ()->GetNode ()->GetId () << "\n");
        NanoMacHeader macHeader;
        p->RemoveHeader (macHeader);
        uint32_t dst = macHeader.GetDestination ();
        uint32_t src = macHeader.GetSource ();
        if(GetDevice ()->GetNode ()->GetId () == dst)
        {
                //NS_LOG_UNCOND ("\n I am node: " << dst << " and I got the packet successfully from node: "<<src<<" \n");
                //NS_LOG_UNCOND (" Received packet size (after removing header): " << p->GetSize () <<"\n");
                this->m_phyRxInfoTrace(pkt);
        }
}


} // namespace ns3
