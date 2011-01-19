/*
 * mcCore.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "clHelper.h"

int mcCL(cl_float* dataSet, cl_float isoValue,
		 size_t sizeX, size_t sizeY, size_t sizeZ,
		 cl_float4 valuesDistance, cl_int4 valuesOffset,
		 cl_float4** triangles, cl_float4** normals, size_t* outSize/*,
		 clhResources resources, cl_program program,
		 cl_mem trianglesTableBuffer, cl_mem verticesTableBuffer,
		 cl_kernel classificationKernel, cl_kernel compactionKernel,
		 cl_kernel generationKernel*/);

void mcReleaseCL();


int mcHost(cl_float* dataSet, cl_float isoValue,
		   size_t sizeX, size_t sizeY, size_t sizeZ,
		   cl_float4 valuesDistance, cl_int4 valuesOffset,
		   cl_float4** triangles, cl_float4** normals, size_t* outSize);

//void test();
