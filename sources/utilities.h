/*
 * utilities.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <time.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

clock_t getCurrentTime();
clock_t getCurrentTimeInMili();
long calculateDiffTimeInMiliSec(clock_t begin, clock_t end);
long calculateDiffTimeInSec(clock_t begin, clock_t end);

////////////////////////////////////////////////////////////////////////////////

//FILE* setLogStream(FILE* stream);

#define logMsg_(m) logMessage(m, __FILE__, NULL)
#define logMsg__(m, s) logMessage(m, __FILE__, s)
#define logMsg___(m, s) logMessage(m, s, NULL)
void logMessage(const char* message, const char* source, FILE* stream);

////////////////////////////////////////////////////////////////////////////////

int *makeIntArray(int size);
int *makeIntMatrix(int size);
//float *makeFloatBlock(int size);
float* makeFloatBlock(const int sizeX, const int sizeY, const int sizeZ);
float* loadCharBlock(const char *path, const int sizeX, const int sizeY, const int sizeZ);
float* loadFloatBlock(const char *path, const int sizeX, const int sizeY, const int sizeZ);
float* loadCharBlockRepeat(const char *path, const int szX, const int szY, const int szZ);
//size_t countFloatBlock(const char *block, const float isoValue, const int szX, const int szY, const int szZ);


#endif /* UTILITIES_H_ */
