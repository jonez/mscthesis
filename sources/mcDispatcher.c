/*
 * mcDispatcher.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "mcDispatcher.h"

#include <stdlib.h>
#include <stdio.h>
//#include <math.h>

#include "common.h"
#include "clHelper.h"
#include "mcCore.h"


////////////////////////////////////////////////////////////////////////////////

int MCD_VERBOSE = FALSE;

int mcdGetVerbose() {
	return MCD_VERBOSE;
}

void mcdSetVerbose(const int state) {
	MCD_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////


int dispatch(float* input, float isoValue, cl_float4 valuesDistance,
			 size_t inSizeX, size_t inSizeY, size_t inSizeZ,
			 mcdMemParts** output, size_t* outSize, int useHost) {

//	int err;

	int steps = 32;
	size_t stackSize = inSizeZ;
	size_t stepSize = (inSizeX + 1) * (inSizeY + 1);
//	size_t memSize = (steps + 1) * stepSize * sizeof(cl_float);
	size_t count = 0;

//	size_t partsCount = ceil((double)stackSize / steps);
	size_t partsCount = stackSize / steps;
	if(stackSize % steps) partsCount++;
	mcdMemParts* parts = calloc(partsCount, sizeof(mcdMemParts));

	for(int s = 0; s < stackSize; s += steps, count++) {

		if(s + steps > stackSize)
//			memSize = (stackSize - s) * stepSize * sizeof(cl_float);
			steps = stackSize - s;

		cl_int4 offset = {{0, 0, s, 0}};

		mcdMemParts part = malloc(sizeof(struct _mcdMemParts));

//		runCL(&input[s * stepSize], isoValue, inSizeX, inSizeY, steps,
//				valuesDistance, offset, &part->data, vbo, &part->size,
//				resoruces, program, trianglesTableBuffer, verticesTableBuffer,
//				classificationKernel, compactionKernel, generationKernel);
//		printf("%d-%.0f\n", s * stepSize, input[s * stepSize]);

		if(useHost)
			mcHost(&input[s * stepSize], isoValue, inSizeX, inSizeY, steps,
					valuesDistance, offset, &part->triangles, &part->normals, &part->size);
		else
			mcCL(&input[s * stepSize], isoValue, inSizeX, inSizeY, steps,
					valuesDistance, offset, &part->trianglesVBO, &part->normalsVBO, &part->size);

		parts[count] = part;

		printf(">>> count: %d, s: %d, size: %d\n", count, s, part->size);

	}

	if(!useHost) mcReleaseCL();

	*output = parts;
	*outSize = count;

//	for(int i = 0; i < *outSize; i++) {
//		printf("%d -> %d\n", i, parts[i]->size);
//		if(parts[i]->size)
//			printf("(%.2f;%.2f;%.2f;%.0f)\n", parts[i]->data[0].s[0], parts[i]->data[0].s[1], parts[i]->data[0].s[2], parts[i]->data[0].s[3]);
//	}

	return 0;
}
