/*
 * mcDispatcher.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef MCDISPATCHER_H_
#define MCDISPATCHER_H_

#include <CL/cl.h>
#include <GL/gl.h>

//#include "clHelper.h"


//struct _mcdMemParts {
//	cl_float4*		data;
//	size_t			size;
//};

struct mcdMemParts {
	cl_float4*		triangles;
	cl_float4*		normals;
	GLuint			trianglesVBO;
	GLuint			normalsVBO;
	size_t			size;
};

typedef struct mcdMemParts* mcdMemParts;

////////////////////////////////////////////////////////////////////////////////////////////////////


int dispatch(int				/* device */,
			 float*				/* inputDataSet */,
			 float				/* isoValue */,
//			 cl_float4			/* valuesDistance */,
			 float				/* valuesDistanceX */,
			 float				/* valuesDistanceY */,
			 float				/* valuesDistanceZ */,
			 size_t				/* valuesSizeX */,
			 size_t				/* valuesSizeY */,
			 size_t				/* valuesSizeZ */,
			 mcdMemParts**		/* outputGeometry */,
			 size_t* 			/* outputSize */,
			 int				/* useHostToCompute */);


#endif /* MCDISPATCHER_H_ */
