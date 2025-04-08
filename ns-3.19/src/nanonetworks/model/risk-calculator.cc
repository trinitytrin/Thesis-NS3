/*Author: Tanzila Choudhury
 Added for calculating risk of node data */


#include "risk-calculator.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"


NS_LOG_COMPONENT_DEFINE("RiskCalculator");

namespace ns3{

Riskcalculator::Riskcalculator(void)
{
	size = 0;
	saferange=NULL;
	risky=false;  
}
	
Riskcalculator::Riskcalculator(int nsize)
{
	size=nsize;
	risky = false;
		
	saferange = new int*[size];
		
	for(int i=0; i<size; i++)
	{
		saferange[i] = new int[2];
	}
}	


int Riskcalculator::Getsize(void)
{
	return size;
}

void Riskcalculator::SetNodeRange(int nodeid, int min, int max)	//sets safe range of values of data for a particular node
{
	saferange[nodeid][0] = min;
	saferange[nodeid][1] = max;
}

bool Riskcalculator::IsRisky(int nodeid, int value)
{
	risky=false;
	if(value<saferange[nodeid][0] || value>saferange[nodeid][1])
	{
		risky = true;
	}
	return risky;
}
	
Riskcalculator::~Riskcalculator(void)
{	
	if(saferange!=NULL)
	{	
		for(int i=0; i<size; i++)
		{	
			delete[] saferange[i];
		}
		delete[] saferange;		
	}
}




} //namespace ns3
