/*Author: Tanzila Choudhury
 Added for interface module activities during dynamic tdma protocol */


#include "interface-module.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/trace-helper.h>
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include <ns3/ptr.h>

NS_LOG_COMPONENT_DEFINE("InterfaceModule");

namespace ns3{

InterfaceModule::InterfaceModule(void)
{
	id=0;
	max_nodes=0;
	protocol_type=1;
	num_emergency_nodes=0;
	pull_slot_order=NULL;
	max_em_nodes_val = 0;


}

InterfaceModule::InterfaceModule(int iid, int num_node, int protocol_t)
{
	id=iid;
	max_nodes=num_node;
	protocol_type=protocol_t;
	num_emergency_nodes=0;

	pull_slot_order=new int[num_node];

	for(int i=0; i<num_node; i++) //initial slot order with -1 means no node has currently emergency data 
	{
		pull_slot_order[i]=-1;
	}
	max_em_nodes_val = 0;
		
}


InterfaceModule::~InterfaceModule(void)
{
	if(pull_slot_order!=NULL)
		delete[] pull_slot_order;
	
}


void InterfaceModule::SetMaxEmNodeVal(int num)
{
	max_em_nodes_val = num;	
}



/*to reset the pull slot ordering*/
void InterfaceModule::ResetPullSlots(void)
{
	for(int i=0; i<max_nodes; i++) 
	{
		pull_slot_order[i]=-1;
	}
	
	num_emergency_nodes=0;
}

/* when interface gets an emergency data, it will add an emergency slot for pull cycle for that node and increase the total number of emergency nodes*/
void InterfaceModule::AddToEmergencySlots(int nodeid, bool curemvalue)
{
	//check if already reached max possible node limit, then return, dont add more nodes
	if(num_emergency_nodes==max_em_nodes_val)
		return;

	
	if(curemvalue==true)	
	{
		pull_slot_order[num_emergency_nodes]=nodeid;
		num_emergency_nodes++;
	}
	
}




/*For creating slots for pull order in case it was not created during constructor call or the default constructor was called */
void InterfaceModule:: CreatePullSlots(int num_node)
{
	pull_slot_order=new int[num_node];
}

/* For getting total number of nodes that are sending emergency data to interface*/
int InterfaceModule::GetEmergencyNodeCount(void)
{
	return num_emergency_nodes;
}



void InterfaceModule:: GetSlotOrder(void)
{

}


/*For creating a packet that will be broadcast in the beginning of the pull slot informing all the nodes that how many extra slots will be given and who can send at those slots*/
Ptr<Packet> InterfaceModule:: PullInfoPacket(void)
{
	std::cout<<"\n inside PullInfoPacket function. \n";	
	uint32_t final_packet=0x02; // first 4 bits reserved for information about packet data. 2 for broadcast from interface, second 4 bits for #of emergency nodes.

	uint8_t buffer[8];
	std::cout<<"\n Before Packing, Final packet value: "<<std::hex<<final_packet <<std::dec<<std::endl;

	if(num_emergency_nodes>0 && num_emergency_nodes<max_em_nodes_val)                
	{	
		final_packet = (((uint32_t) num_emergency_nodes<<4) | final_packet) ;
		uint32_t temp_pack = 0;
		//memcpy(buffer, &final_packet, sizeof(uint64_t));
		
		for(int j=num_emergency_nodes-1; j>=0; j--)
		{
			temp_pack = ((uint32_t) (pull_slot_order[j] << (8+ (4*j))) | temp_pack);
			std::cout<<"\n debugging: j="<< j << " em_node#="<<pull_slot_order[j]<<" shifted:"<< 4*(j+1)<< " bits and packetvalue= "<<std::hex <<temp_pack<<std::dec << std::endl;
		}
	
		final_packet = temp_pack | final_packet; // 1st 4 bit number of nodes and rest are their corresponding number in order

	}
	
	std::cout<<"\n After Packing, Final packet value: "<<std::hex<<final_packet <<std::dec<<std::endl;	
	memcpy(buffer, &final_packet, sizeof(uint32_t));
	
	Ptr<Packet> p = Create<Packet> (buffer,sizeof(uint32_t));	
	
	//this->ResetPullSlots(); // after creating packet reset pull slot info for next cycle again	
	return p;
	
}





/*for dubugging purpose*/
void InterfaceModule:: DumpPullSlots(void)
{
	std::cout<<"\n\nDumping pull slots array.."<<std::endl;
	for(int i=0; i<sizeof(pull_slot_order)+1; i++) 
	{
		std::cout<<pull_slot_order[i]<<std::endl;
	}
}

} //namespace ns3
