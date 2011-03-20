/*
 * mcCore.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef MCCORE_H_
#define MCCORE_H_

//#include <CL/cl.h>
//#include <GL/gl.h>

#include "clHelper.h"


int mccCL(cl_float* values, cl_float isoValue, cl_uint4 valuesSizes,
		  cl_float4 valuesDistances, cl_float4 valuesOffsets, cl_int2 valuesZBuffers,
		  GLuint* trianglesVBO, GLuint* normalsVBO, size_t* outSize/*,
		  cl_float4** triangles, cl_float4** normals,
		  clhResources resources, cl_program program,
		  cl_mem trianglesTableBuffer, cl_mem verticesTableBuffer,
		  cl_kernel classificationKernel, cl_kernel compactionKernel,
		  cl_kernel generationKernel*/);

void mccReleaseCL();


int mccHost(cl_float* dataSet, cl_float isoValue,
		   size_t sizeX, size_t sizeY, size_t sizeZ,
		   cl_float4 valuesDistance, cl_float4 valuesOffset,
		   cl_float4** triangles, cl_float4** normals, size_t* outSize);

//void test();


#endif /* MCCORE_H_ */
