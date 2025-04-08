/* Author- Tanzila Choudhury
/* DYNAMIC TDMA WITH PUSH AND PULL CYCLE ENABLED */
/* Added Tracing here*/
/*with bit manipulation for risky data in packet*/
/*included dynamic mac protocol class*/
/* broadcast from interface added */ 
/* emergency=1 or broadcast=2 value added in trace files. first 4 bits of a packet contains that information */


/************** Perfectly scheduing the Pull-push scheme with NanoBattery *********************/ 

#include "all_simulation_headers.h"
//#include "ns3/sensor-node-status.h"
#include "ns3/nano-sensor-battery.h"

 using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("NewDynamicTDMABattery6");


/*constant global values*/
#define SN 50
#define DATASIZE 2000
#define NUM_NODE 10
#define SENSOR_TYPE 5
#define RX_ERROR_RATE 0.9
#define MAX_ENERGY 20000 //for nanobattery

int nodedataval[SENSOR_TYPE][DATASIZE]; //array for reading data from file
int SensedDataArray[SENSOR_TYPE][DATASIZE] = {}; //auxiliary array for keeping track of sensed data
int Rxnodedataval[SENSOR_TYPE][DATASIZE] = {} ; //auxiliary array to keep track of missing and received data
int slotmissed[NUM_NODE]={}; //tx slots missed due to sleeping or reading or lack of energy by a node

/* Declaring other functions here*/
void createScenario(Ptr<Riskcalculator> riskc);
static void RxInfoTrace (Ptr<OutputStreamWrapper> stream, int id, Ptr<InterfaceModule> iface, Ptr<const Packet>  pkt);
static void TxInfoTrace (Ptr<OutputStreamWrapper> stream, int id, Ptr<const Packet>  pkt);
void RunDynamicTDMA(Ptr<Riskcalculator> riskc);
void reset_2D_array(int array[SENSOR_TYPE][DATASIZE]);
void dump_2D_array(int array[SENSOR_TYPE][DATASIZE]);
void count_missed_data(int sensor_array[SENSOR_TYPE][DATASIZE], int rx_array[SENSOR_TYPE][DATASIZE], Ptr<Riskcalculator> riskc);
void scheduleTxDevices(Ptr<SimpleNanoDevice> devA, Ptr<SimpleNanoDevice> devB, Ptr<Riskcalculator> riskc);
void scheduleBroadcast( Ptr<SimpleNanoDevice> interface, Ptr<InterfaceModule> iface);
int senseData(int nodeid);
static void RxDrop (Ptr<Riskcalculator> riskc, Ptr<const Packet> pkt);

void ScheduleDumpInterfaceModule(Ptr<InterfaceModule> iface);

//global variables and objects 
int slotlen=10;
int DataReadinterval = 7*slotlen;
int datasensetime = 1*slotlen;

int numOfSentPackets = 0;
int numOfRxPackets = 0;
int numOfDroppedPackets=0;
int dataindex[NUM_NODE]={};

int numOfRiskyData=0;
int numOfDroppedRiskData=0;
int numOfReceivedRiskData=0;
int numOfSensedRiskyData = 0;
int nextbroadcasttime;
int totalbroadcasttime=0;
int lastbroadcasttime;

int finaltime=10025;
int starttime = 25;

int ENDTIME;

Ptr<NanoSensorBattery> sensor[NUM_NODE]; // Sensor nodes Battery operated


/*Tracing function for dropped packets by the receiver*/
static void RxDrop (Ptr<Riskcalculator> riskc, Ptr<const Packet> pkt)
{
	//NS_LOG_UNCOND("\nRxDrop at "<<Simulator::Now().GetMicroSeconds()<<" Packet size: "<< pkt->GetSize() <<" ");
	Ptr<Packet> p = pkt->Copy ();

	NanoMacHeader macHeader;
        p->RemoveHeader (macHeader);
        uint32_t dst = macHeader.GetDestination ();
        uint32_t src = macHeader.GetSource ();
 
	uint8_t buffer[11];
 	p->CopyData(buffer, sizeof(buffer));

	uint32_t packetdata;
	memcpy(&packetdata, buffer, sizeof(uint32_t));
	//std::cout << "packet data " << packetdata <<std::endl;	

	int emergency = packetdata & 0x0000000F;
	//std::cout << "Emergency bit value " << emergency <<std::endl;
	int data = (packetdata>>4) &0x0000FFFF;
	//std::cout << "Data value " << data <<std::endl;
	//std::cout << "packet data " << data <<" from source node: "<<src<< std::endl;

	if(emergency != 2 )
	{
		if(Rxnodedataval[src%SENSOR_TYPE][dataindex[src]] != data)
		{
			if(riskc->IsRisky(src%SENSOR_TYPE, data) == true)  //drop of a risky data packet
			{	
				//Rxnodedataval[src%SENSOR_TYPE][dataindex[src]]=-2; //-2 assigned to the array indicating risky data missed or dropped 
				//numOfDroppedRiskData++;
			}
			else
			{
				//Rxnodedataval[src%SENSOR_TYPE][dataindex[src]]=-1; //-1 assigned to the arary indicating drop of a normal data packet
			}		
		}
		dataindex[src]++; 
		numOfDroppedPackets++;	
	}

	if(emergency == 1 )
	{	
		numOfDroppedRiskData++;
	}

	
}


/*Tracing fuction at receiver end for packet information */
static void RxInfoTrace (Ptr<OutputStreamWrapper> stream, int id, Ptr<InterfaceModule> iface, Ptr<const Packet>  pkt)
{
	
	Ptr<Packet> p = pkt->Copy ();

	NanoMacHeader macHeader;
        p->RemoveHeader (macHeader);
        uint32_t dst = macHeader.GetDestination ();
        uint32_t src = macHeader.GetSource ();
 
	 uint8_t buffer[11];
	p->CopyData(buffer, sizeof(buffer));
	uint32_t packetdata;
	memcpy(&packetdata, buffer, sizeof(uint32_t));
	//std::cout << "packet data " << packetdata <<std::endl;	

	int emergency = packetdata & 0x0000000F;
	//std::cout << "Emergency bit value " << emergency <<std::endl;
	int data = (packetdata>>4) &0x0000FFFF;
	//std::cout << "Data value " << data <<std::endl;

	int curtime = Simulator::Now().GetMicroSeconds();
	int curemslots = 0; 
	bool pushslot=false;
	
	if (emergency == 1 || emergency == 0) 
	{
		curemslots = sensor[src]->GetEmergencyNodeNum();
		if((curtime - lastbroadcasttime)>=(slotlen*(curemslots+1)))
		{
			pushslot = true;
		}
		
	}
	
	
	if(emergency == 1 ) //if the packet is received by interface and emergency bit has 1 in it, then the id of the source of the packet is added for pull slot
	{	
		if(pushslot == true)		
		{		
			iface->AddToEmergencySlots(src, true);
		}
		numOfReceivedRiskData++;
	}
	else if(emergency == 0) 
	{					
		iface->AddToEmergencySlots(src, false);
	}
	else if(emergency == 2) //if the sent packet was broadcast (emergency bit value 2) then the sensors nodes will extract the information regarding next scheduling
	{
		sensor[id]->ExtractPullData(p, curtime);
	}
 	
	if(emergency != 2)
	{
		if(Rxnodedataval[src%SENSOR_TYPE][dataindex[src]] != data)
		{
			Rxnodedataval[src%SENSOR_TYPE][dataindex[src]]=data; //received data value assigned to the array as the data is received successfully
		}
		dataindex[src]++;
		numOfRxPackets++;
	}

	if(emergency == 2)
	{
	*stream->GetStream() <<std::setw(2) << id<<" "<<std::setw(11)<<Simulator::Now().GetMicroSeconds()<<" "<<std::setw(5)<< src << " "<<std::setw(5) << dst << " "<<std::setw(2) << pkt->GetSize() << " " <<std::setw(2)<< p->GetSize()<< " "<<std::setw(8) << data<< " "<<std::setw(3) << emergency<< " "<<std::setw(3) << sensor[id]->GetEmergencySlotInfo() <<" "<<std::setw(3) << sensor[id]->GetEmergencyNodeNum()<<" "<<std::setw(3) << totalbroadcasttime << std::endl;
	}
	else
	{
		*stream->GetStream() <<std::setw(2) << id<<" "<<std::setw(11)<<Simulator::Now().GetMicroSeconds()<<" "<<std::setw(5)<< src << " "<<std::setw(5) << dst << " "<<std::setw(2) << pkt->GetSize() << " " <<std::setw(2)<< p->GetSize()<< " "<<std::setw(8) << data<< " "<<std::setw(3) << emergency << std::endl;
	}
}

/*Tracing fuction at sender end for packet information */
static void TxInfoTrace (Ptr<OutputStreamWrapper> stream, int id, Ptr<const Packet>  pkt)
{
	

	Ptr<Packet> p = pkt->Copy ();

	NanoMacHeader macHeader;
        p->RemoveHeader (macHeader);
        uint32_t dst = macHeader.GetDestination ();
        uint32_t src = macHeader.GetSource ();

	 uint8_t buffer[11];
	p->CopyData(buffer, sizeof(buffer));
	uint32_t packetdata;
	memcpy(&packetdata, buffer, sizeof(uint32_t));
	//std::cout << "packet data " << packetdata <<std::endl;	

	int emergency = packetdata & 0x0000000F;
	//std::cout << "Emergency bit value " << emergency <<std::endl;
	int data = (packetdata>>4) &0x0000FFFF;
	//std::cout << "Data value " << data <<std::endl;
	
	if(emergency == 1)
	{			
		numOfRiskyData++; //keeps track how many total risky data is actually generated and sent 
	}

	if(emergency == 1|| emergency == 0) //if its the sensor nodes
	{
		*stream->GetStream() <<std::setw(2) << id<<" "<<std::setw(11)<<Simulator::Now().GetMicroSeconds()<<" "<<std::setw(5)<< src << " "<<std::setw(5) << dst << " "<<std::setw(2) << pkt->GetSize() << " " <<std::setw(2)<< p->GetSize()<< " "<<std::setw(8) << data<< " "<<std::setw(3) << emergency << " " <<std::setw(11)<< sensor[id]->GetLastHeardTime()<< std::endl;

	numOfSentPackets++;
	}
	else
	{
		*stream->GetStream() <<std::setw(2) << id<<" "<<std::setw(11)<<Simulator::Now().GetMicroSeconds()<<" "<<std::setw(5)<< src << " "<<std::setw(5) << dst << " "<<std::setw(2) << pkt->GetSize() << " " <<std::setw(2)<< p->GetSize()<< " "<<std::setw(8) << data<< " "<<std::setw(3) << emergency << std::endl;
	}
}


/*This function schedules the events in order to create the scenario*/
void createScenario(Ptr<Riskcalculator> riskc)
{
	NS_LOG_UNCOND("\n"<<Simulator::Now().GetMicroSeconds()<<"\t Inside create Scenario function \n ");
	
	DataReadWriteHelper *drwhelper = new DataReadWriteHelper(DATASIZE);
	std::cout<<"\n Datasize is: "<<drwhelper->GetDataSize()<<std::endl;	
	int filedata[SENSOR_TYPE][DATASIZE]={};

	drwhelper->GenerateRandomData(filedata[0], 110, 120); //generating reading for Systolic blood pressure (60-250)
	drwhelper->GenerateRandomData(filedata[1], 75, 80); //generating reading for Dyastolic blood pressure (20-130)
	drwhelper->GenerateRandomData(filedata[2], 95, 105); //generating reading for Heart beat rate (0-210)
	drwhelper->GenerateRandomData(filedata[3], 99, 103); //generating reading for Body temperature in Fahrenheit (94-109)
	drwhelper->GenerateRandomData(filedata[4], 130, 150); //generating reading for Blood Suger Level in mg/dl (70-180)

	riskc->SetNodeRange(0, 90, 140);
	riskc->SetNodeRange(1, 60, 90);
	riskc->SetNodeRange(2, 60, 100); //normal range for adults
	riskc->SetNodeRange(3, 97, 99);
	riskc->SetNodeRange(4, 80, 140);

	for(int i=0; i<SENSOR_TYPE; i++)   //writing generated random data to files  
	{	
		std::stringstream filename;
		filename<<"Data_for_sensor_type_"<<i+1<<".dat";
		drwhelper->WriteDataIntoFile(filename.str(),filedata[i]);
	}

	for(int i=0; i<SENSOR_TYPE; i++)  //reading data from files and keeping in nodedataval array
	{	
		std::stringstream filename;
		filename<<"Data_for_sensor_type_"<<i+1<<".dat";
		drwhelper->ReadDataFromFile(filename.str(),nodedataval[i]);
	} 		
}

/* function for sensing data by a node (reading from the array)*/
int senseData(int nodeid)
{
	static int nodedataIndex[NUM_NODE]={0};
	int data = nodedataval[nodeid%SENSOR_TYPE][nodedataIndex[nodeid]]; //if numofnode>sensortype, then same data is sensed by multiple nodes of the same sensor type
	nodedataIndex[nodeid]++;
	return data;
}



/*function that and sends packets to destination devices*/
void scheduleTxDevices(Ptr<SimpleNanoDevice> devA, Ptr<SimpleNanoDevice> devB, Ptr<Riskcalculator> riskc)
{
	int n= devA->GetNode()->GetId();
	int i = Simulator::Now().GetMicroSeconds();

	if(sensor[n]->IsBatteryDead() == true)
	{
		std::cout<<"at time "<<i<<": node "<<n<<"'s battery is dead!\n";
		return; 
	}	
	
	if(sensor[n]->ReadyToTxDynamicTDMA(lastbroadcasttime, i , slotlen, NUM_NODE)==true)
	{
		std::cout<<"at time "<<i<<": node "<<n<<" is ready to tx\n";
		Ptr <Packet> p;
		if(riskc->IsRisky(n%SENSOR_TYPE, sensor[n]->GetDataToBeSent()) == true)
		{
			//std::cout<<"\n it's a risky data value\n";
			 p = sensor[n]->TxDynamicData(true);
		}
		else 
		{
			 p = sensor[n]->TxDynamicData(false);					
		}		

	NS_LOG_UNCOND("\n"<<Simulator::Now().GetMicroSeconds()<<"\t Inside scheduleTx function with Node: "<<devA->GetNode()->GetId() <<" LastBroadcastTime: "<<lastbroadcasttime<<" NextBroadcastTime: "<<totalbroadcasttime);
	devA->GetMac()->Send(p, devB->GetNode()->GetId());

	ENDTIME = i;
	
	}

	else{

		if(sensor[n]->ReadyToRead(i)==true)
		{
			std::cout<<"at time "<<i<<": node "<<n<<" is ready to read\n";
			int index = sensor[n]->GetSensingIndex(i);
			sensor[n]->ReadData(nodedataval[n%SENSOR_TYPE][sensor[n]->GetSensingIndex(i)], i);
			int data = sensor[n]->GetDataToBeSent();
			SensedDataArray[n%SENSOR_TYPE][index]=data;
			if(riskc->IsRisky(n%SENSOR_TYPE, sensor[n]->GetDataToBeSent()) == true)
				numOfSensedRiskyData++;
		}
		else
		{
			std::cout<<"at time "<<i<<": node "<<n<<" doing nothing\n"; 
		}
	}		
		
}


/*function that and broadcasts packets from interface devices*/
void scheduleBroadcast( Ptr<SimpleNanoDevice> interface, Ptr<InterfaceModule> iface)
{
	NS_LOG_UNCOND("\n"<<Simulator::Now().GetMicroSeconds()<<"\t Inside scheduleBroadcast function");
	Ptr<Packet> p;
	p = iface->PullInfoPacket();
	int slotlen=10;
	
	interface->GetMac()->BroadcastPacket(p);
	int em_slot_count = iface->GetEmergencyNodeCount();
	nextbroadcasttime = ((1+em_slot_count+NUM_NODE)*slotlen); 
	std::cout<<"\n next broadcast time is after: "<< nextbroadcasttime<< " microseconds "<< std::endl; 
	
	lastbroadcasttime=Simulator::Now().GetMicroSeconds();

	totalbroadcasttime = totalbroadcasttime+nextbroadcasttime;
	std::cout<<"\n total broadcast time: "<< totalbroadcasttime << std::endl; 

	iface->ResetPullSlots(); //after broadcasting the packet the interface must reset otherwise the previous info stays
	
	if(totalbroadcasttime<finaltime)
		Simulator::Schedule(MicroSeconds(nextbroadcasttime), & scheduleBroadcast, interface, iface);	
}


/*function for scheduling interface module information dumping*/
void ScheduleDumpInterfaceModule(Ptr<InterfaceModule> iface)
{
	NS_LOG_UNCOND("\n"<<Simulator::Now().GetSeconds()<<"\t Inside scheduleInterfaceModule function");
	iface->DumpPullSlots();
	std::cout<<"\n total emergency nodes: "<<iface->GetEmergencyNodeCount()<<std::endl;
	//iface->PullInfoPacket();
}


/* function that sets up all the nanonetwork node scenario and sends packet in TDMA manner*/
void RunDynamicTDMA(Ptr<Riskcalculator> riskc)
{
	NS_LOG_UNCOND("\n"<<Simulator::Now().GetSeconds()<<"\t Inside DynamicTDMA with nanobattery function \n ");

	int numOfNodes = NUM_NODE;
	int numOfGateways=1;
	double txRangeNanoNodes = 0.8;
	double txRangeNanoRouter = 0.9;

	int devNch=0;  //count of total number of devices connected in the channel 

	//timers
	Time::SetResolution(Time::FS);
	double duration = 5;
	
	//int slotlen = 10;

	//layout details
	double xrange = 1;
	double yrange = 0.001;
	double zrange = 0.001;

	//physical details
	double pulseEnergy = 100e-12;
	double pulseDuration = 100;
	double pulseInterval = 10000;
	double powerTransmission = pulseEnergy / pulseDuration;

	NodeContainer n_nodes;
	NodeContainer n_gateways;

	NetDeviceContainer d_gateways;
	NetDeviceContainer d_nodes;

	//helper definition
	NanoHelper nano;

	n_nodes.Create(numOfNodes);
	d_nodes = nano.Install(n_nodes);
	n_gateways.Create(numOfGateways);
	d_gateways = nano.Install(n_gateways);

	//mobility
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
			"Bounds", BoxValue (Box (0, xrange, 0, yrange, 0, zrange)),
			"TimeStep", TimeValue (Seconds (0.001)),
			"Alpha", DoubleValue (0),
			"MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=0.2|Max=0.2]"),
			"MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"),
			"MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
			"NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
			"NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
			"NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"));
	mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
			"X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.15]"),//RandomVariableValue (UniformVariable (0, xrange)),
			"Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.001]"),//RandomVariableValue (UniformVariable (0, yrange)),
			"Z", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.001]"));//RandomVariableValue (UniformVariable (0, zrange)));

	mobility.Install (n_nodes);
	mobility.Install (n_gateways);

	//receiver error model
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	em->Reset();
	em->SetRate (RX_ERROR_RATE);
	em->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);	

	int max_em_nodes = NUM_NODE - 1;	
		

	//interface module creating 	
	Ptr<InterfaceModule> i_module = Create<InterfaceModule>(0, NUM_NODE, 2);
	i_module->SetMaxEmNodeVal(max_em_nodes);

	//tracing 
	AsciiTraceHelper asciiTraceHelper;

	std::stringstream file_outRX_s;
	file_outRX_s << "NewDynamicTDMABattery6-Rx_Events";
	std::string file_outRX = file_outRX_s.str();
	Ptr<OutputStreamWrapper> streamRX = asciiTraceHelper.CreateFileStream(file_outRX);

	std::stringstream file_outTX_s;
	file_outTX_s << "NewDynamicTDMABattery6-Tx_Events";
	std::string file_outTX = file_outTX_s.str();
	Ptr<OutputStreamWrapper> streamTX = asciiTraceHelper.CreateFileStream(file_outTX);


//sensor nodes modules creating  



	int dataindex[NUM_NODE]={};

	
	for(int i=0; i<NUM_NODE; i++)
	{
		sensor[i] = Create<NanoSensorBattery>(i, MAX_ENERGY, true, 1, 0, datasensetime, slotlen); //sleeptime 0 for Dynamic TDMA
		sensor[i]->ResetSensor();
		sensor[i]->SetMaxEmNodes(max_em_nodes);
		sensor[i]->SetSensingStartTime(starttime-10+slotlen*i);
		sensor[i]->SetDataInterval(DataReadinterval);
		std::cout<<sensor[i]->GetSensingIndex(starttime+slotlen*i)<<std::endl;
		sensor[i]->ReadData(nodedataval[i%SENSOR_TYPE][sensor[i]->GetSensingIndex(starttime-10+slotlen*i)], starttime+slotlen*i);
		SensedDataArray[i%SENSOR_TYPE][0] = sensor[i]->GetDataToBeSent();
		
	}

	


//physical channel modelling 

	//for the interface

	for (int x = 0; x < numOfGateways; x++) 
	{
		Ptr<SimpleNanoDevice> interface = d_gateways.Get(x)->GetObject<SimpleNanoDevice> ();
		Ptr<ConstantPositionMobilityModel> m = CreateObject<
				ConstantPositionMobilityModel> ();

		m->SetPosition(Vector(xrange/2, 0.0, 0.0));
		nano.AddMobility(interface->GetPhy(), m);

		Ptr<NanoMacEntity> mac;		
		Ptr<DynamicNanoMacEntity> mac2 = CreateObject<DynamicNanoMacEntity> ();
		mac = mac2;		
		mac->SetDevice(interface);
		interface->SetMac(mac);
		
		interface->SetReceiveErrorModel(em);

		interface->GetPhy()->SetTxPower(powerTransmission);
		interface->GetPhy()->SetTransmissionRange(txRangeNanoRouter);
		interface->GetPhy()->GetObject<TsOokBasedNanoSpectrumPhy> ()->SetPulseDuration(
				FemtoSeconds(pulseDuration));
		interface->GetPhy()->GetObject<TsOokBasedNanoSpectrumPhy> ()->SetPulseInterval(
				FemtoSeconds(pulseInterval));

		interface->GetMac()->TraceConnectWithoutContext ("PhyRxInfo", MakeBoundCallback (&RxInfoTrace, streamRX, interface->GetNode()->GetId(), i_module));
		interface->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, riskc));
		interface->GetMac()->TraceConnectWithoutContext ("PhyTxInfo", MakeBoundCallback (&TxInfoTrace, streamTX, interface->GetNode()->GetId()));

		NS_LOG_UNCOND("\n This interface is node number: "<<interface->GetNode()->GetId()<< "\n ");
	}

       //for the nodes
	for (int i = 0; i < numOfNodes; i++) 
	{
		Ptr<MobilityModel> m = n_nodes.Get(i)->GetObject<MobilityModel> ();
		nano.AddMobility(
				d_nodes.Get(i)->GetObject<SimpleNanoDevice> ()->GetPhy(), m);
		Ptr<SimpleNanoDevice> dev = d_nodes.Get(i)->GetObject<SimpleNanoDevice> ();


		Ptr<NanoMacEntity> mac;		
		Ptr<DynamicNanoMacEntity> mac2 = CreateObject<DynamicNanoMacEntity> ();
		mac = mac2;	
		mac->SetDevice(dev);
		dev->SetMac(mac);

		dev->SetReceiveErrorModel(em);

		dev->GetPhy()->SetTransmissionRange(txRangeNanoNodes);
		dev->GetPhy()->SetTxPower(powerTransmission);
		dev->GetPhy()->GetObject<TsOokBasedNanoSpectrumPhy> ()->SetPulseDuration(
				FemtoSeconds(pulseDuration));
		dev->GetPhy()->GetObject<TsOokBasedNanoSpectrumPhy> ()->SetPulseInterval(
				FemtoSeconds(pulseInterval));
	
		dev->GetMac()->TraceConnectWithoutContext ("PhyTxInfo", MakeBoundCallback (&TxInfoTrace, streamTX, dev->GetNode()->GetId()));
		dev->GetMac()->TraceConnectWithoutContext ("PhyRxInfo", MakeBoundCallback (&RxInfoTrace, streamRX, dev->GetNode()->GetId(), i_module));
		dev->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, riskc));

		NS_LOG_UNCOND("\n This node is node number: "<<dev->GetNode()->GetId()<< "\n ");

	}

		devNch = d_nodes.Get(0)->GetObject<SimpleNanoDevice> ()->GetPhy()->GetChannel()->GetNDevices();
		std::cout<<"Number of devices in this channel : "<<devNch<<"\n";

 


// in this part, if it is node n's slot to tx and it misses, count slot_waste++
//when it is new data generation time and node missed it bcoz of sleeping or tx, advace the dataread index so it means that data reading was missed 
	

	nextbroadcasttime = starttime;
	totalbroadcasttime = nextbroadcasttime;
	Simulator::Schedule(MicroSeconds(nextbroadcasttime), & scheduleBroadcast, d_gateways.Get(0)->GetObject<SimpleNanoDevice> (), i_module);
	
	//finaltime=starttime+NUM_NODE*slotlen*DATASIZE;


	for(int i=starttime; i<finaltime; i=i+slotlen)
	{

	
			for(int n=0; n<NUM_NODE; n++)
			{

					Simulator::Schedule(MicroSeconds(i), & scheduleTxDevices, d_nodes.Get(n)->GetObject<SimpleNanoDevice> (), d_gateways.Get(0)->GetObject<SimpleNanoDevice> (), riskc );

			}
	
							
		
	}	
	

	
}


/* the main function */

int main (int argc, char *argv[])
{
	NS_LOG_UNCOND("\n Testing- Inside Main \n ");
	double slotlen = 10;

	reset_2D_array(nodedataval);
	reset_2D_array(Rxnodedataval);

	Ptr<Riskcalculator> riskc = Create<Riskcalculator> (SENSOR_TYPE);
	
	createScenario(riskc);

	RunDynamicTDMA(riskc);

	//Simulator::Schedule(MicroSeconds(finaltime+10), & dump_2D_array, nodedataval);
	//Simulator::Schedule(MicroSeconds(finaltime+14), & dump_2D_array, SensedDataArray);
	//Simulator::Schedule(MicroSeconds(finaltime+22), & dump_2D_array, Rxnodedataval);
	Simulator::Schedule(MicroSeconds(finaltime+24), & count_missed_data, nodedataval, Rxnodedataval, riskc);
	
	

	Simulator::Stop(MicroSeconds(finaltime+30));
	Simulator::Run();
	Simulator::Destroy();
	

	return 0;
}

void dump_2D_array(int array[SENSOR_TYPE][DATASIZE])
{
	std::cout<<"\n"<<Simulator::Now().GetSeconds()<<": Dumping 2D array\n";
	for(int i=0; i<SENSOR_TYPE; i++)
	{
		for(int j=0; j<DATASIZE; j++)
		{
			std::cout<<std::setw(3)<<array[i][j]<<" ";
		}
		std::cout<<"\n";
	} 
}

void reset_2D_array(int array[SENSOR_TYPE][DATASIZE])
{
	std::cout<<"\n"<<Simulator::Now().GetSeconds()<<": Resetting 2D array\n";
	for(int i=0; i<SENSOR_TYPE; i++)
	{
		for(int j=0; j<DATASIZE; j++)
		{
			array[i][j] = 0;
		}		
	} 
}

void count_missed_data(int sensor_array[SENSOR_TYPE][DATASIZE], int rx_array[SENSOR_TYPE][DATASIZE], Ptr<Riskcalculator> riskc)
{	
	int num_missed_data = 0;
	int num_risk_data = 0;
	int missed_risky_data=0;
	int total_slots_missed=0;
	int total_slot_count=0;
	int sensedriskydata = 0;
	int rxriskydata = 0; 	
 
	for(int i=0; i<SENSOR_TYPE; i++)
	{	//int rxdataindex=0;
		for(int j=0; j<DATASIZE; j++)
		{
			if(rx_array[i][j]!=sensor_array[i][j])
			{
				num_missed_data++;
				if(rx_array[i][j]==-2)
					missed_risky_data++;
			}

			if(riskc->IsRisky(i, sensor_array[i][j]) == true)
			{
				//std::cout<<"\n data for node "<<i<<" is Risky, value: "<<sensor_array[i][j]<<std::endl;
				num_risk_data++;
			}						
		}
	}

	for(int i=0; i<SENSOR_TYPE; i++)
	{	
		for(int j=0; j<DATASIZE; j++)
		{
			if((riskc->IsRisky(i, SensedDataArray[i][j]) == true) && (SensedDataArray[i][j]>0))
			{
				sensedriskydata++;
			}						
		}
	}

	for(int i=0; i<SENSOR_TYPE; i++)
	{	
		for(int j=0; j<DATASIZE; j++)
		{
			if((riskc->IsRisky(i, rx_array[i][j]) == true) && (rx_array[i][j]>0))
			{
				rxriskydata++;
			}						
		}
	}

	for(int i=0; i<NUM_NODE; i++)
	{
		total_slots_missed = total_slots_missed + sensor[i]->GetSlotMissedCount();
	}
	
	for(int i=0; i<NUM_NODE; i++)
	{
		total_slot_count = total_slot_count + sensor[i]->GetTotalSlotCount();
	}

	if(rxriskydata<sensedriskydata)	
		missed_risky_data = sensedriskydata-rxriskydata;
	 
	float missrate =((float)missed_risky_data/(float)sensedriskydata)*100;
	float slotmissrate =((float)total_slots_missed/(float)total_slot_count)*100;

	//std::cout<<"\nTotal number of Missed data: "<<num_missed_data<<std::endl;
	std::cout<<"\nTotal number of Sensed Risky data by all nodes: "<<numOfSensedRiskyData<< std::endl;	
	std::cout<<"Total number of TXedRisky data: "<<numOfRiskyData<< std::endl;
	
	//std::cout<<"Actually generated RIsky data: "<<num_risk_data<< std::endl; //2nd one gives the correct value
	std::cout<<"Actually Sensed RIsky data: "<<sensedriskydata<< std::endl;
	std::cout<<"Total number of Actually Rxed Risky data: "<<rxriskydata<< std::endl;
	std::cout<<"Total number of Actually MIssed Risky data: "<<missed_risky_data<<std::endl;
	std::cout<<"Risky data miss rate : "<<missrate<<"%"<<std::endl;	
	
	
	std::cout<<"\nTotal number of slots : "<<total_slot_count<<std::endl;
	std::cout<<"Total number of slots missed : "<<total_slots_missed<<std::endl;

	std::cout<<"Successful TX Simulation ended in : "<<ENDTIME<< " microseconds "<<std::endl;

	std::cout<<"\nTotal number of TXed Sensor Data Packets: "<<numOfSentPackets<< std::endl;
	std::cout<<"Total number of RXed Sensor Data Packets: "<<numOfRxPackets<< std::endl;
	std::cout<<"Total number of Dropped Packets: "<<numOfDroppedPackets<< std::endl;
	std::cout<<"Total number of Dropped Risky Data Packets: "<<numOfDroppedRiskData<< std::endl; 

	std::ofstream resultsfile;
	resultsfile.open("Results_DynamicTDMABattery1.txt", std::ofstream::out | std::ofstream::app);
	resultsfile<<"\n\n~~~~~~~~~ Results for Simulation No: "<<SN<<" ~~~~~~~~~~~~~\n\n";

	resultsfile<<"Size of Data = "<<DATASIZE*SENSOR_TYPE<<std::endl;
	resultsfile<<"Channel Error Rate = "<<RX_ERROR_RATE*100<<"%"<<std::endl;
	resultsfile<<"Maximum Energy of Each Battery = "<<MAX_ENERGY<<std::endl;

	resultsfile<<"\nNumber of Nodes = "<<NUM_NODE<<std::endl;
	resultsfile<<"Length of Slots = "<<slotlen<<std::endl;
	resultsfile<<"Data Read Interval Length = "<<DataReadinterval<<std::endl;

	
	resultsfile<<"\nRisky Data Miss Rate = "<<missrate<<"%"<<std::endl;

	resultsfile<<"\nSuccessful TX Simulation ran for: "<<ENDTIME-starttime<< " time units "<<std::endl;	

	resultsfile<<"\nSlot Miss Rate = "<<slotmissrate<<"%"<<std::endl;

	
	resultsfile.close();
}
