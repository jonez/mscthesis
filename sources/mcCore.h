/*
 * mcCore.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "clHelper.h"

int runCL(cl_float* input, cl_float isoValue,
		  size_t sizeX, size_t sizeY, size_t sizeZ,
		  cl_float4 valuesDistance, cl_int4 valuesOffset,
		  cl_float4** output1, cl_float4** output2, size_t* size);

void test();
