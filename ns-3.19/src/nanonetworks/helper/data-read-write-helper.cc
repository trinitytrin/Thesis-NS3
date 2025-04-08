/*Author: Tanzila Choudhury
 Added for helping in file read-write mechanism and functions
purpose: helping to create scenario of reading sensor data */


#include "data-read-write-helper.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"


NS_LOG_COMPONENT_DEFINE("DataReadWriteHelper");

namespace ns3 {


DataReadWriteHelper::DataReadWriteHelper(void)
{
	//std::cout<<"Hi!\n";
	datasize = 0;
}

DataReadWriteHelper::DataReadWriteHelper(int size)
{
	//std::cout<<"Hi!\n";
	datasize = size;
}


DataReadWriteHelper::~DataReadWriteHelper(void)
{
	//std::cout<<"Bye!\n";
}

int DataReadWriteHelper::GetDataSize(void)
{
	return datasize;
}

void DataReadWriteHelper::GenerateRandomData(int dataval[], double min, double max)
{
	//NS_LOG_UNCOND("\n\t Inside generateRandomData function \n ");
	
	Ptr<UniformRandomVariable> x = CreateObject <UniformRandomVariable> ();
	x->SetAttribute("Min", DoubleValue(min));
	x->SetAttribute("Max", DoubleValue(max));

	for(int i=0; i<datasize; i++)
	{
		dataval[i]= x->GetInteger();
		//std::cout<<"\nrandom value: "<<dataval[i]<<"\n";
	}
}


void DataReadWriteHelper::Print1DArray(int array[], int size)
{
	std::cout<<"\nprinting 1D array\n";
	for(int i=0; i<size; i++)
	{
		std::cout<<" "<<array[i];
	}
	std::cout<<"\n\n";
}


void DataReadWriteHelper::WriteDataIntoFile(std:: string filename, int dataval[])
{
	std::ofstream dataWriteFile;

	dataWriteFile.open(filename.c_str());

	if(dataWriteFile.is_open())
	{
		for(int i=0; i<datasize; i++)
		{
			dataWriteFile<<dataval[i]<<"\n";

		}
		
		dataWriteFile.close();
	}
	else
	{
		std::cout<<"\nUnable to open dataWriteFile\n";
	}
	
}


/* this function reads data from a file and saves into an 1d array */
void DataReadWriteHelper::ReadDataFromFile(std:: string filename, int dataval[])
{
	int value;
	int dataindex=0;
	std::string line;
	std::ifstream dataReadFile;
	dataReadFile.open(filename.c_str());

	if(dataReadFile.is_open())
	{	
		while(getline(dataReadFile, line))
		{
			value = std::atoi(line.c_str());
			dataval[dataindex++]=value;
			
		}
		dataReadFile.close();		
	}

	else 
	{
		std::cout<<"\nUnable to open dataReadFile\n";
	}			

}


} //namespace ns3 
