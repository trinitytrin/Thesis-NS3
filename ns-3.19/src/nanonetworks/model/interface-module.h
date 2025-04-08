/*Author: Tanzila Choudhury
 Added for interface module activities during dynamic tdma protocol */

#ifndef INTERFACE_MODULE_H
#define INTERFACE_MODULE_H



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


namespace ns3 {




class InterfaceModule: public SimpleRefCount<InterfaceModule>
{
private: 
	int id;
	int max_nodes; //total number of reachable nodes for this interface 
	int protocol_type;  // 1 for simpleTDMA, 2 for Dynamic TDMA, 3 for Dr. Qians AIMD one
	int num_emergency_nodes; //array of node numbers having emergency data 
	int* pull_slot_order; //array of order of the emergency nodes 
	int max_em_nodes_val;



public: 
	InterfaceModule(void);
	InterfaceModule(int id, int num_node, int protocol_t);
	~InterfaceModule(void);
	void ResetPullSlots(void);
	void AddToEmergencySlots(int nodeid, bool curemvalue);
	void CreatePullSlots(int num_node);
	void GetSlotOrder(void);
	int GetEmergencyNodeCount(void);
	Ptr<Packet> PullInfoPacket(void);
	void DumpPullSlots(void);
	void SetMaxEmNodeVal(int num);

};



} // namespace ns3


#endif // INTERFACE_MODULE_H
