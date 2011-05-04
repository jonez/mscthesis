/*
 * utilities.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes:	Try to use libc functions instead of system calls to guarantee
 * 			better portability.
 */

#include "utilities.h"

#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

clock_t getCurrentTime() {

	return clock();
}

clock_t getCurrentTimeInMili() {

	return clock() / (CLOCKS_PER_SEC / 1000);
}

long calculateDiffTimeInSec(clock_t begin, clock_t end) {

	return (end - begin) / (CLOCKS_PER_SEC / 1000000);
}

long calculateDiffTimeInMiliSec(clock_t begin, clock_t end) {

	return calculateDiffTimeInSec(begin, end) * 1000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int *makeIntArray(int size) {

	int *array = (int *)malloc(sizeof(int) * size);

	int i;
	for(i = 0; i < size; i++)
		array[i] = rand() / size;

	return array;
}

int *makeIntMatrix(int size) {

	int *matrix = (int *)malloc(sizeof(int) * size * size);

	int i, s = size * size;
	for(i = 0; i < s; i++)
//		matrix[i] = rand() / size;
		matrix[i] = 2;

	return matrix;
}

//float *makeFloatBlock(int size) {
//
//	float *block = (float *)malloc(sizeof(float) * size * size * size);
//
//	int i, s = size * size * size;
//	for(i = 0; i < s; i++)
//		block[i] = i;
////		block[i] = (float)i;
//
//	return block;
//}

float* makeFloatBlock(const int sizeX, const int sizeY, const int sizeZ) {

	float * block = (float *) malloc(sizeof(float) * sizeX * sizeY * sizeZ);

	int x, y, z;
	int xC = sizeX / 2, yC = sizeY / 2, zC = sizeZ / 2;
	for(z = 0; z < sizeZ; z++)
		for(y = 0; y < sizeY; y++)
			for(x = 0; x < sizeX; x++)
				block[x + y * sizeX + z * sizeX * sizeY] =
						((x - xC) * (x - xC) +
						 (y - yC) * (y - yC) +
						 (z - zC) * (z - zC)) * -10;
						 
	printf("Float block.\n");

	return block;
}

float* loadCharBlock(const char *path, const int szX, const int szY, const int szZ) {

	FILE* fd;
	size_t fz = szX * szY * szZ;
//	long slice = szX * szY;
	float* data;

	// open file for reading
	fd = fopen(path, "r");
	if (fd == NULL) {
		printf("Error while opennig '%s' file.\n", path);
		return NULL;
	}

	printf("Loading dataset from '%s' file.\n", path);

	// determine file size

	//if(fseek(fd, 0, SEEK_END) == 0) {
	//	printf("Error while operating on %s file.\n", path);
	//	return NULL;
	//}
//	fseek(fd, 0, SEEK_END);
//	fz = ftell(fd);
//	rewind(fd);

	data = (float*)calloc(fz, sizeof(float));
	int i = 0;
	while((data[i++] = fgetc(fd)) != EOF && i < fz);

//	for(i = 0; i < slice; i++)
//		data[i] = data[slice + i];

	return data;
}

