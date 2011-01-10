/*
 * scan.h
 *
 *  Created on: Nov 4, 2010
 *      Author: jonez
 */

#ifndef SCAN_H_
#define SCAN_H_

#include "clHelper.h"


int clsGetVerbose();

void clsSetVerbose(const int state);

cl_mem clsScanFromDevice(const clhResources		/* resources */,
						 const int				/* deviceIndex */,
						 const cl_mem			/* inputBuffer */,
						 const size_t			/* size */,
						 cl_float*				/* sum */,
						 cl_int*				/* err */);

void clsRelease();

//int testScan();


#endif /* SCAN_H_ */
