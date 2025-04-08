/*Author: Tanzila Choudhury
 Added for calculating risk of node data */

#ifndef RISK_CALCULATOR_H
#define RISK_CALCULATOR_H



#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"


namespace ns3 {


class Riskcalculator: public SimpleRefCount<Riskcalculator>
{
private: 
	int size;
	int** saferange;
	bool risky;
public:
	Riskcalculator(void);
	
	Riskcalculator(int nsize);	 

	int Getsize(void);

	void SetNodeRange(int nodeid, int min, int max);

	bool IsRisky(int nodeid, int value);
	
	~Riskcalculator(void);

};


}   //namespace ns3

#endif
