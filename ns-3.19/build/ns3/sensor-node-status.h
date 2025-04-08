/*Author: Tanzila Choudhury
 Added for sensor node activities - sensing, harvesting, reading, making packet to transmit */

#ifndef SENSOR_NODE_STATUS_H
#define SENSOR_NODE_STATUS_H



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


class SensorNodeStatus: public SimpleRefCount<SensorNodeStatus>
{
private: 
	int id;
	int max_energy_unit;
	int tx_energy;
	int rx_energy;
	int sense_energy;
	int current_energy_unit;
	bool is_Dynamic;
	int slot_num;
	int emergency_slot;
	int data_memsize;
	int* data_memory;
	int sensing_index;
	Time harvest_time;
	Time dataread_time;
	Time tx_time;
	Time rx_time;
	Time node_cycle_time;
	bool lastDataSent;
	bool dataReady;
	int data_interval;
	Time sensing_start_time;
	Time last_read_time;
	Time last_sleep_time;

	int last_heard_time;
	int max_em_nodes;
	
	int num_em_slots;

	bool emslotmissedflag;

	int myslotmissed;
	int mytotalslots;
	int slotsmissedforharvesting;

	int gamma;
	int gamma_max;
	int cur_gamma;
	int em_bit;
	int next_tx_time;
	
	int recharged_times;

public: 
	SensorNodeStatus(void);
	SensorNodeStatus(int slotno, int maxEnergy, bool isDynamic, int memsize, int htime, int readtime, int txtime);
	~SensorNodeStatus(void);
	void ResetSensor(void);
 	int GetCurrentEnergy(void);
	int GetId(void);
	void SetDataInterval(int interval);
	void SetSensingStartTime(int sstime);
	int GetDataInterval(void);
	int GetSensingIndex(int currenttime);	
	void SetConsumeEnergy(int tx, int rx, int sense);
	void ReadData(int data, int currenttime);
	Ptr<Packet> TxData(void);
	Ptr<Packet> TxDynamicData(bool risky);
	Ptr<Packet> TxAdaptiveData(bool risky);

	void HarvestEnergy(int currenttime);
	int GetRechargedTimes(void);
	bool ReadyToTxSimpleTDMA(int starttime, int currenttime, int slotlen, int NUM_NODE);
	bool ReadyToTxDynamicTDMA(int starttime, int currenttime, int slotlen, int NUM_NODE);
	bool ReadyToTxAdaptiveTDMA(int starttime, int currenttime, int slotlen, int NUM_NODE);
	bool ReadyToReadSimpleTDMA(int currenttime);
	bool ReadyToReadDynamicTDMA(int currenttime);
	bool NewSensorDataReady(int currenttime);
	int GetDataToBeSent(void);
	void ConsumeEnergy(int eunits);
	int GetEmergencyNodeNum(void);
	int GetEmergencySlotInfo(void);

	int GetSlotMissedCount(void);
	int GetTotalSlotCount(void);
	int GetSlotMissCountForSleeping(void);

	void ExtractPullData(Ptr<Packet> p, int currenttime);

	int GetLastHeardTime(void);
	void SetMaxEmNodes(int num);	

	void SetGammaValues(int g, int gmax);

	int GetGammaVal(void);
	int GetGammaMax(void);
	int GetCurGammaVal(void);
	void UpdateCurGamma(bool emval);
	int GetEmBitVal(void);
	int GetNextTxTime(void);
	void UpdateNextTxTime(int curtime, int slotlen, int NUM_NODE);

};


} // namespace ns3


#endif
