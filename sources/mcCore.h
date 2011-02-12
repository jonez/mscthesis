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


int mccCL(cl_float* dataSet, cl_float isoValue,
		 size_t sizeX, size_t sizeY, size_t sizeZ,
		 cl_float4 valuesDistance, cl_int4 valuesOffset,
//		 cl_float4** triangles, cl_float4** normals, size_t* outSize/*,
		 GLuint* trianglesVBO, GLuint* normalsVBO, size_t* outSize/*,
		 clhResources resources, cl_program program,
		 cl_mem trianglesTableBuffer, cl_mem verticesTableBuffer,
		 cl_kernel classificationKernel, cl_kernel compactionKernel,
		 cl_kernel generationKernel*/);

void mccReleaseCL();


int mccHost(cl_float* dataSet, cl_float isoValue,
		   size_t sizeX, size_t sizeY, size_t sizeZ,
		   cl_float4 valuesDistance, cl_int4 valuesOffset,
		   cl_float4** triangles, cl_float4** normals, size_t* outSize);

//void test();


#endif /* MCCORE_H_ */
