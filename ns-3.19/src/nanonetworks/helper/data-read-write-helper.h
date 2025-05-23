/*Author: Tanzila Choudhury
 Added for helping in file read-write mechanism and functions
purpose: helping to create scenario of reading sensor data */


#ifndef DATA_READ_WRITE_HELPER_H
#define DATA_READ_WRITE_HELPER_H



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

/* helper class for helping generating random data in a file and reading that data from file */

class DataReadWriteHelper
{

private:
	int datasize;

public: 

	DataReadWriteHelper(void);
	DataReadWriteHelper(int size);
	~ DataReadWriteHelper(void);
	int GetDataSize(void);
	void GenerateRandomData(int dataval[], double min, double max);
	void Print1DArray(int array[], int size);
	void WriteDataIntoFile(std:: string filename, int dataval[]);
	void ReadDataFromFile(std:: string filename, int dataval[]);
	



}; //end of class 



} /* namespace ns3 */

#endif /* DATA_READ_WRITE_HELPER_H */
