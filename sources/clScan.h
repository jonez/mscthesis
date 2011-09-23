/*
 * scan.h
 *
 *  Created on: Nov 4, 2010
 *      Author: jonez
 */

#ifndef CLSCAN_H_
#define CLSCAN_H_

//#include <CL/cl.h>
//#include <GL/gl.h>

#include "clHelper.h"

////////////////////////////////////////////////////////////////////////////////

int clsGetVerbose();

void clsSetVerbose(const int		/* state */);

////////////////////////////////////////////////////////////////////////////////

cl_int clsInit(const clhResources		/* initializedResources */);

void clsFree();

cl_mem clsScanFromDevice(const clhResources		/* resources */,
						 const int				/* deviceIndex */,
						 const cl_mem			/* inputBuffer */,
						 const size_t			/* size */,
						 cl_float*				/* sum */,
						 cl_int*				/* err */);


#endif /* CLSCAN_H_ */
