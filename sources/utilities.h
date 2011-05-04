/*
 * utilities.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <time.h>


clock_t getCurrentTime();
clock_t getCurrentTimeInMili();
long calculateDiffTimeInMiliSec(clock_t begin, clock_t end);
long calculateDiffTimeInSec(clock_t begin, clock_t end);

int *makeIntArray(int size);
int *makeIntMatrix(int size);
//float *makeFloatBlock(int size);
float* makeFloatBlock(const int sizeX, const int sizeY, const int sizeZ);
float* loadCharBlock(const char *path, const int sizeX, const int sizeY, const int sizeZ);
//size_t countFloatBlock(const char *block, const float isoValue, const int szX, const int szY, const int szZ);


#endif /* UTILITIES_H_ */
