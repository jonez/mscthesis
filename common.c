/*
 * common.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */


// try to use libc functions instead of system calls to guarantee better portability

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

//void mclCheckError(cl_int error) {
//
//}

void mclErrorInfo(cl_int error, char * info) {

	static const char* errorList[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };
    
    const int listSize = sizeof(errorList) / sizeof(errorList[0]);
    const int index = -error;

    if(index >= 0 && index < listSize) 
		printf("%s : %s\n", errorList[index], info);
    else
    	printf("error no %d : %s", error, info);
		
	//if(error != CL_SUCCESS)
	//	exit(0);
}

char * loadSource(const char *path) {

	FILE* fd;
	long fz;
	char* src;

	// open file for reading
	fd = fopen(path, "r");
	if (fd == NULL) {
		printf("Error while opennig %s file.\n", path);
		return NULL;
	}

	// determine file size

	//if(fseek(fd, 0, SEEK_END) == 0) {
	//	printf("Error while operating on %s file.\n", path);
	//	return NULL;
	//}
	fseek(fd, 0, SEEK_END);
	fz = ftell(fd);
	rewind(fd);

	src = (char *) malloc(fz + 1);
	fread(src, fz, 1, fd);
	src[fz] = '\0';

	return src;
}

int * makeIntArray(int size) {

	int * array = (int *) malloc(sizeof(int) * size);

	int i;
	for(i = 0; i < size; i++)
		array[i] = rand() / size;

	return array;
}

int * makeIntMatrix(int size) {

	int * matrix = (int *) malloc(sizeof(int) * size * size);

	int i, s = size * size;
	for(i = 0; i < s; i++)
//		matrix[i] = rand() / size;
		matrix[i] = 2;

	return matrix;
}

float * makeFloatBlock(int size) {

	float * block = (float *) malloc(sizeof(float) * size * size * size);

	int i, s = size * size * size;
	for(i = 0; i < s; i++)
		block[i] = i;
//		block[i] = (float)i;

	return block;
}
