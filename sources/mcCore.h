/*
 * mcCore.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef MCCORE_H_
#define MCCORE_H_

#include "clHelper.h"

////////////////////////////////////////////////////////////////////////////////

int mccGetVerbose();

void mccSetVerbose(const int		/* state */);

int mccGetProfiling();

void mccSetProfiling(const int		/* state */);

////////////////////////////////////////////////////////////////////////////////

int mccInitializeCL(const clhResources initializedResources);

int mccShutdownCL();

cl_int mccMarchCL(const cl_uint			/* deviceIndex */,
				  const cl_float*		/* values */,
				  const cl_float		/* isoValue */,
				  const cl_uint4		/* valuesSizes */,
				  const cl_float4		/* valuesDistances */,
				  const cl_float4		/* valuesOffsets */,
				  const cl_int2 		/* valuesZBuffers */,
				  size_t*				/* outSize */,
#if USE_GL
				  GLuint*				/* trianglesVBO */,
				  GLuint*				/* normalsVBO */,
#endif /* USE_GL */
				  cl_mem* 				/* trianglesBuffer */,
				  cl_mem* 				/* normalsBuffer */);


int mccHost(cl_float* dataSet, cl_float isoValue,
		   size_t sizeX, size_t sizeY, size_t sizeZ,
		   cl_float4 valuesDistance, cl_float4 valuesOffset,
		   cl_float4** triangles, cl_float4** normals, size_t* outSize);


#endif /* MCCORE_H_ */
