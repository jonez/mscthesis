/*
 * mcBenchmark.c
 *
 *  Created on: Jun 21, 2011
 *      Author: jonez
 */

//#include <stdio.h>

#include "../common.h"
#include "../utilities.h"
#include "../mcDispatcher.h"

////////////////////////////////////////////////////////////////////////////////

extern void mccSetVerbose(const int);
extern void mcdSetVerbose(const int);
extern void clsSetVerbose(const int);
extern void clhSetVerbose(const int);

static int MCB_VERBOSE = DISABLE;

int mcbGetVerbose() {
	return MCB_VERBOSE;
}

void mcbSetVerbose(const int state) {
	MCB_VERBOSE = state;
}

extern void mccSetProfiling(const int);

////////////////////////////////////////////////////////////////////////////////

#define MAX_DEVICES_COUNT 8

#define SWITCH_RUNS 'r'
#define SWITCH_DEVICE 'd'
#define SWITCH_DSCASE 'c'
#define SWITCH_VERBOSE 'v'
#define SWITCH_MODULE_MCB 'b'
#define SWITCH_MODULE_MCD 'd'
#define SWITCH_MODULE_MCC 'c'
#define SWITCH_MODULE_CLS 's'
#define SWITCH_MODULE_CLH 'h'
#define SWITCH_CL_PROFILING 'p'

//#define DEFAULT_RUNS 1
//#define DEFAULT_DSCASE 0
static const int DEFAULT_RUNS = 1;
static const int DEFAULT_DSCASE = 0;

////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {

	printf("\n\t###>>> mcBenchmark <<<###\n\n");

	int runs = DEFAULT_RUNS;
	int datasetCase = DEFAULT_DSCASE;
	
	int deviceSwitch;
	int devicesCount = 0;
	int devicesList[MAX_DEVICES_COUNT];

	cl_command_queue_properties cmdQueuesProperties = EMPTY;
	
	for(int i = 1; i < argc; i++) {
	
		switch(argv[i][0]) {
		
		case SWITCH_RUNS :
			runs = atoi(argv[i] + 1);
			if(runs < 1) runs = DEFAULT_RUNS;
		break;
			
		case SWITCH_DEVICE :
			deviceSwitch = atoi(argv[i] + 1);
			if(devicesCount < MAX_DEVICES_COUNT)
				devicesList[devicesCount++] = deviceSwitch;
			else
				printf("INFO: Too many selected devices (max: %d), device %d "
						"ignored.\n", MAX_DEVICES_COUNT, deviceSwitch);
		break;
			
		case SWITCH_DSCASE : 
			datasetCase = atoi(argv[i] + 1);
			if(datasetCase < 0 || datasetCase > 9) datasetCase = DEFAULT_DSCASE;
		break;
			
		case SWITCH_VERBOSE :
		
			switch(argv[i][1]) {
			
			case SWITCH_MODULE_MCB :
				mcbSetVerbose(ENABLE);
			break;
				
			case SWITCH_MODULE_MCD :
				mcdSetVerbose(ENABLE);
			break;
			
			case SWITCH_MODULE_MCC :
				mccSetVerbose(ENABLE);
			break;
			
			case SWITCH_MODULE_CLS :
				clsSetVerbose(ENABLE);
			break;
			
			case SWITCH_MODULE_CLH :
				clhSetVerbose(ENABLE);
			break;
			
			default :
				printf("INFO: Invalid verbose switch, '%c' will be ignored.\n",
					argv[i][1]);					
			}
				
		break;
		
		case SWITCH_CL_PROFILING :
			mccSetProfiling(ENABLE);
			cmdQueuesProperties |= CL_QUEUE_PROFILING_ENABLE;
		break;
			
		default :
			printf("INFO: Invalid switch, '%c' will be ignored.\n",
					argv[i][0]);		
		}
	}
		
	printf("Settings: runs = %d; datasetCase = %d; devices = ", runs, datasetCase);
	if(!devicesCount)
		printf("all");
	else
		for(int i = 0; i < devicesCount; i++)
			printf("%d ", devicesList[i]);
	printf("\n");
	
	mcDataset ds;
	ds.distanceX = 1.0f;
	ds.distanceY = 1.0f;
	ds.distanceZ = 1.0f;
		
	
	switch(datasetCase) {
	
	case 0 :
		ds.sizeX = 256;
		ds.sizeY = 256;
		ds.sizeZ = 256;
		ds.isoValue = 50;
		ds.values = loadCharBlock("data/skull.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 1 :
		ds.sizeX = 256;
		ds.sizeY = 256;
		ds.sizeZ = 128;
		ds.isoValue = 80;
		ds.values = loadCharBlock("data/engine.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 2 :
		ds.sizeX = 256;
		ds.sizeY = 256;
		ds.sizeZ = 256;
		ds.isoValue = 50;
		ds.values = loadCharBlock("data/aneurism.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 3 :
		ds.sizeX = 64;
		ds.sizeY = 64;
		ds.sizeZ = 64;
		ds.isoValue = 80;
		ds.values = loadCharBlock("data/fuel.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 4 :
		ds.sizeX = 41;
		ds.sizeY = 41;
		ds.sizeZ = 41;
		ds.isoValue = 80;
		ds.values = loadCharBlock("data/sin.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 5 :
		ds.sizeX = 32;
		ds.sizeY = 32;
		ds.sizeZ = 32;
		ds.isoValue = 80;
		ds.values = loadCharBlock("data/bucky.raw", ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 6 :
		ds.sizeX = 256;
		ds.sizeY = 256;
		ds.sizeZ = 256;
		ds.isoValue = -100000;
		ds.values = makeFloatBlock(ds.sizeX, ds.sizeY, ds.sizeZ);
	break;
	
	case 7 :
		ds.sizeX = 512;
		ds.sizeY = 512;
		ds.sizeZ = 512;
		ds.isoValue = 70;
		ds.values = loadCharBlockRepeat("data/skull.raw",
				ds.sizeX / 2, ds.sizeY / 2, ds.sizeZ / 2);
	break;
			
/*	case 8 :
		ds.sizeX = 1024;
		ds.sizeY = 1024;
		ds.sizeZ = 25;
		ds.isoValue = 50;
		ds.values = loadFloatBlock("/media/24D7-1C08/FGM0716_0002.vol",
				ds.sizeX, ds.sizeY, ds.sizeZ);
	break;*/
	
	default :
		printf("ERROR: Invalid dataset case!\n");
		goto retErr;
		
	}
	
	if(ds.values)
		printf("Dataset: sizeX = %d; sizeY = %d; sizeZ = %d; isoValue = %0.2f\n",
				ds.sizeX, ds.sizeY, ds.sizeZ, ds.isoValue);
	else {
		printf("ERROR: Invalid dataset (due to missing dataset file?)!\n");
		goto retErr;
	}

	printf("\n");
	
	cl_int clErr;
	clhResources resources = clhInitResources(NULL, CL_DEVICE_TYPE_GPU,
			devicesCount ? devicesList : NULL, devicesCount,
			cmdQueuesProperties, &clErr);
	if(clErr || MCB_VERBOSE) {
		clhLogErr__(clErr, "initializing resources");

		if(clErr) goto retErr;
	}

	int err;
	if((err = mcdInitialize(resources)))
		goto retErr;
	
	mcdMemEntry* memList = NULL;
	uint memListEntriesCount = 0;
	size_t memListEntriesSizesSum = 0;

	printf("\n");
	clock_t beginTime = getCurrentTimeInMili();	
	
	for(int i = 0; i < runs; i++) {
		printf("\n\t>>> RUN %d!!! <<<\n\n", i + 1);
		
//		ds.isoValue -= i * 5;
		
		err = mcdDispatchMulti_(&ds, &memList);
	
		//printMemListInfo(memList);
		memListEntriesCount += getMemListEntriesCount(memList);
		memListEntriesSizesSum += getMemListEntriesSizesSum(memList);
		freeMemList(memList);
	}
	
	clock_t endTime = getCurrentTimeInMili();
	printf("\n");
	
	free(ds.values);

	clock_t elapsedTime = endTime - beginTime;
	clock_t averageTime = elapsedTime / runs;
	uint averageMemListEntriesCount = memListEntriesCount / runs;
	size_t averageMemListEntriesSizesSum = memListEntriesSizesSum / runs;
	
	printf(	"Benchmark:\n"
			" dataset = %u * %u * %u (%zuMB)\n"
			" begin = %ldms, end = %ldms, elapsed = %ldms, avgTime = %ldms\n"
			" avgMemListEntriesCount = %u, avgMemListEntriesSizesSum = %zu (%zuMB)\n",
			ds.sizeX, ds.sizeY, ds.sizeZ,
			ds.sizeX * ds.sizeY * ds.sizeZ * sizeof(cl_float) / MB,
			beginTime, endTime, elapsedTime, averageTime,
			averageMemListEntriesCount, averageMemListEntriesSizesSum,
			averageMemListEntriesSizesSum * sizeof(cl_float4) / MB);
	
	return SUCCESS;	
			
retErr:

	return ERROR;
}

