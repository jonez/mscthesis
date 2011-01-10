/*
 * mcDispatcher.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "clHelper.h"

struct _mcdMemParts {
	cl_float4*		data;
	size_t			size;
};

typedef struct _mcdMemParts* mcdMemParts;

////////////////////////////////////////////////////////////////////////////////////////////////////


int dispatch(cl_float* input, cl_float isoValue, cl_float4 valuesDistance,
			 size_t inSizeX, size_t inSizeY, size_t inSizeZ,
			 GLuint* vbo, mcdMemParts** output, size_t* outSize);
