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
double calculateDiffTimeInMiliSec(clock_t begin, clock_t end);
double calculateDiffTimeInSec(clock_t begin, clock_t end);

int *makeIntArray(int size);
int *makeIntMatrix(int size);
//float *makeFloatBlock(int size);
float* makeFloatBlock(const int sizeX, const int sizeY, const int sizeZ);
float* loadFloatBlock(const char *path, const int size);


#endif /* MCUTILITIES_H_ */
