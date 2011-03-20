/*
 * mcDispatcher.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "mcDispatcher.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "mcCore.h"


#define VALUES_BUFFER_SIZE 1

////////////////////////////////////////////////////////////////////////////////

int MCD_VERBOSE = FALSE;

int mcdGetVerbose() {
	return MCD_VERBOSE;
}

void mcdSetVerbose(const int state) {
	MCD_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////


int dispatch(float* input, float isoValue,
			 float valDistX, float valDistY, float valDistZ,
			 size_t inSizeX, size_t inSizeY, size_t inSizeZ,
			 mcdMemParts** output, size_t* outSize, int useHost) {

//	int err;

	int steps = 32;
	size_t stackSize = inSizeZ;
	size_t stepSize = (inSizeX + 1) * (inSizeY + 1);
	size_t partsCount = 0;

	cl_int2 valuesZBuffers = {{0, 0}};
	cl_float4 valuesDistance = {{valDistX, valDistY, valDistZ, 1.0f}};

	// instead of ceil() from math.h
//	size_t partsCount = stackSize / steps;
//	if(stackSize % steps) partsCount++;
	size_t partsSize = ceil((float)stackSize / steps);
	mcdMemParts* parts = calloc(partsSize, sizeof(mcdMemParts));

	for(int s = 0; s < stackSize; s += steps, partsCount++) {

		if(s <= 0)
			valuesZBuffers.s[0] = 0;
		else
			valuesZBuffers.s[0] = 1;

		if(s >= s + stackSize)
			valuesZBuffers.s[1] = 0;
		else
			valuesZBuffers.s[1] = 1;

		if(s + steps > stackSize)
			steps = stackSize - s;

		cl_uint4 sizes = {{inSizeX, inSizeY, steps, 0}};
		cl_float4 offsets = {{0.0f, 0.0f, (float)s, 0.0f}};
		size_t memAddress = (s - valuesZBuffers.s[0]) * stepSize;

		mcdMemParts part = malloc(sizeof(struct mcdMemParts));

		if(useHost)
			mccHost(&input[memAddress], isoValue, inSizeX, inSizeY, steps,
					valuesDistance, offsets, &part->triangles, &part->normals, &part->size);
		else
			mccCL(&input[memAddress], isoValue, sizes, valuesDistance, offsets, valuesZBuffers,
					&part->trianglesVBO, &part->normalsVBO, &part->size);

		parts[partsCount] = part;

		printf(">>> count: %d, s: %d, size: %d\n", partsCount, s, part->size);

	}

	if(!useHost) mccReleaseCL();

	*output = parts;
	*outSize = partsCount;

//	for(int i = 0; i < *outSize; i++) {
//		printf("%d -> %d\n", i, parts[i]->size);
//		if(parts[i]->size)
//			printf("(%.2f;%.2f;%.2f;%.0f)\n", parts[i]->data[0].s[0], parts[i]->data[0].s[1], parts[i]->data[0].s[2], parts[i]->data[0].s[3]);
//	}

	return 0;
}
