/*
 * mcDispatcher.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "clHelper.h"
#include "common.h"

//struct _mcdMemParts {
//	cl_float4*		data;
//	size_t			size;
//};

struct _mcdMemParts {
	cl_float4*		triangles;
	cl_float4*		normals;
	GLuint			trianglesVBO;
	GLuint			normalsVBO;
	size_t			size;
};

typedef struct _mcdMemParts* mcdMemParts;

////////////////////////////////////////////////////////////////////////////////////////////////////


int dispatch(float* input, float isoValue, cl_float4 valuesDistance,
			 size_t inSizeX, size_t inSizeY, size_t inSizeZ,
			 mcdMemParts** output, size_t* outSize, int useHost);
