/*
 * mcDispatcher.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef MCDISPATCHER_H_
#define MCDISPATCHER_H_

#include "clHelper.h"

//#include <CL/cl.h>
//#if USE_GL
//#include <GL/gl.h>
//#endif /* USE_GL */

////////////////////////////////////////////////////////////////////////////////

typedef struct _mcdMemEntry mcdMemEntry;

struct _mcdMemEntry {
	size_t				size;
#if USE_GL
	GLuint				trianglesVBO;
	GLuint				normalsVBO;
#endif /* USE_GL */
	cl_mem				trianglesBuffer;
	cl_mem				normalsBuffer;
	mcdMemEntry*		next;
};

typedef struct _mcdMemList mcdMemList;

struct _mcdMemList {
	mcdMemEntry*		head;
	mcdMemEntry*		tail;
	unsigned int		size;
};

typedef struct _mcdWorkUnit mcdWorkUnit;

struct _mcdWorkUnit {
	mcDataset*			dataset;
	int					devicesCount;
	int*				devicesList;
	int					useHost;
	mcdMemEntry*		output;
};

////////////////////////////////////////////////////////////////////////////////

#define newMemEntry_() newMemEntry(NULL)
mcdMemEntry* newMemEntry(mcdMemEntry*		/* previousEntry */);

void freeMemList(mcdMemEntry*		/* headEntry */);

void printMemListInfo(mcdMemEntry*		/* headEntry */);

uint getMemListEntriesCount(mcdMemEntry*		/* headEntry */);

size_t getMemListEntriesSizesSum(mcdMemEntry*		/* headEntry */);

////////////////////////////////////////////////////////////////////////////////


int mcdInitialize(const clhResources		/* initializedResources */);

int mcdShutdown();

int mcdDispatchSingle(const mcDataset*		/* dataset */,
					  const uint			/* deviceIndex */,
					  const int				/* useHost */,
					  mcdMemEntry**			/* output */);

#define mcdDispatchMulti_(d, o) mcdDispatchMulti(d, 0, NULL, FALSE, o)
int mcdDispatchMulti(const mcDataset*		/* dataset */,
					 const int				/* devicesCount */,
					 const uint* 			/* devicesList */,
					 const int				/* useHost */,
					 mcdMemEntry**			/* output */);


#endif /* MCDISPATCHER_H_ */
