/*
 * common.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <CL/cl.h>

//void mclCheckError(cl_int error);
void mclErrorInfo(cl_int error, char * info);

char * loadSource(const char *path);
int * makeIntArray(int size);
int * makeIntMatrix(int size);
float * makeFloatBlock(int size);


#endif /* COMMON_H_ */
