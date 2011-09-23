/*
 * glHelper.h
 *
 *  Created on: Jun 21, 2011
 *      Author: jonez
 */

#ifndef GLHELPER_H_
#define GLHELPER_H_

#include "mcDispatcher.h"

////////////////////////////////////////////////////////////////////////////////

int glGetVerbose();

void glSetVerbose(const int		/* state */);

////////////////////////////////////////////////////////////////////////////////

int glInitialize(const clhResources		/* initializedCLResources */);

int glShutdown();

////////////////////////////////////////////////////////////////////////////////

mcDataset* getDataset();
mcDataset* setDataset(mcDataset*		/* dataset */);

float getIsoValue();
float setIsoValue(const float		/* isoValue */);
float incIsoValue(const float		/* incValue */);

uint getDevice();
uint setDevice(uint		/* device */);

mcdMemEntry* getMemList();

//void requestsDispatcher();

//int syncRequestVBOs(const uint			/* threadNo */,
//					const size_t		/* vboSize */,
//					GLuint*				/* vbo1 */,
//					cl_mem*				/* buffers1 */,
//					GLuint*				/* vbo2 */,
//					cl_mem*				/* buffers2 */);


#endif /* COMMON_H_ */
