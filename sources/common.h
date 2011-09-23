/*
 * common.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////

#define FALSE 0
#define TRUE !FALSE
#define DISABLE 0
#define ENABLE !DISABLE
#define SUCCESS EXIT_SUCCESS
#define ERROR EXIT_FAILURE
#define EMPTY 0
#define EOL -1
#define NO_DEVICE -1

#define X 0
#define Y 1
#define Z 2
#define W 3
#define TOP 0
#define BOTTOM 1

#define KB 1024 // 1024^1
#define MB 1048576 // 1024^2
#define GB 1073741824 // 1024^3

#define TINY_STRING_SIZE 100 * sizeof(char) + 1
#define SMALL_STRING_SIZE 1 * KB * sizeof(char) + 1
#define MEDDIUM_STRING_SIZE 10 * KB * sizeof(char) + 1
#define BIG_STRING_SIZE 100 * KB * sizeof(char) + 1

////////////////////////////////////////////////////////////////////////////////

typedef unsigned int uint;

typedef struct _mcDataset mcDataset;

struct _mcDataset {
	float*					values;
	float					isoValue;
	uint					sizeX;
	uint					sizeY;
	uint					sizeZ;
	float					distanceX;
	float					distanceY;
	float					distanceZ;
};
	
////////////////////////////////////////////////////////////////////////////////

//#define USE_GL FALSE
#define PROFILING FALSE
#define EXIT_ON_ERROR FALSE

#define DEFAULT_LOG_STREAM stdout

#define DEFAULT_KERNELS_SRC_PATH "sources/kernels"
#define DEFAULT_KERNELS_BIN_PATH "kernels"
#define DEFAULT_KERNELS_BIN_EXT "bin"


#endif /* COMMON_H_ */
