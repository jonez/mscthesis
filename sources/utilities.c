/*
 * utilities.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes:	Try to use libc functions instead of system calls to guarantee
 * 			better portability.
 *			Create generic logging capabilities
 */

#include "utilities.h"

#include "common.h"

////////////////////////////////////////////////////////////////////////////////

clock_t getCurrentTime() {

	return clock();
}

clock_t getCurrentTimeInMili() {

	return clock() / (CLOCKS_PER_SEC / 1000);
}

long calculateDiffTimeInMiliSec(clock_t begin, clock_t end) {

	return (end - begin) / (CLOCKS_PER_SEC / 1000);
}

long calculateDiffTimeInSec(clock_t begin, clock_t end) {

	return calculateDiffTimeInSec(begin, end) / 1000;
}

////////////////////////////////////////////////////////////////////////////////

/*static FILE* logStream = DEFAULT_LOG_STREAM;

FILE* setLogStream(FILE* stream) {

	FILE* oldStream = logStream;
	logStream = stream;
	
	return oldStream;
}*/

void logMessage(const char* message, const char* source, FILE* stream) {
	
	fprintf(stream ? stream : DEFAULT_LOG_STREAM, "»» %ldms @%s » %s\n",
			getCurrentTimeInMili(), source, message);
}

////////////////////////////////////////////////////////////////////////////////

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
	size_t  /* fz = 0 ,*/ bz = szX * szY * szZ;
	float* data;

	// open file for reading
	if (!(fd = fopen(path, "r"))) {
		printf("Error while opening %s file.\n", path);
		return NULL;
	}

	// determine file size
//	if(fseek(fd, 0, SEEK_END)) {
//		printf("Error while operating on %s file.\n", path);
//		return NULL;
//	}
//	fz = ftell(fd);
//	rewind(fd);

	data = (float*)calloc(bz, sizeof(float));
	int i = 0;
	while((data[i++] = fgetc(fd)) != EOF && i < bz);

	return data;
}

float* loadFloatBlock(const char *path, const int szX, const int szY, const int szZ) {

	FILE* fd;
	size_t  fz, bz = szX * szY * szZ;
	float* data;

	// open file for reading
	if (!(fd = fopen(path, "r"))) {
		printf("Error while opening %s file.\n", path);
		return NULL;
	}

	// determine file size
//	if(fseek(fd, 0, SEEK_END)) {
//		printf("Error while operating on %s file.\n", path);
//		return NULL;
//	}
//	fz = ftell(fd);
//	rewind(fd);

	printf("te");
	fflush(NULL);

	data = (float*)calloc(bz, sizeof(float));
	fz = fread(data, sizeof(float), bz, fd);
	
//	printf("te");
//	fflush(NULL);
	
//	for(int i = 0; i < bz; i++)
//		data[i] = (int)data[i];
	
	printf("te");
	fflush(NULL);
	
//	for(int i = 0; i < 10; i++)
//		printf("%0.2f, ", data[i]);
//	printf("%0.1f, ", data[167 + 1024 * 499]); // 104,4
//	printf("%0.1f", data[163 + 1024 * 494]); // -79.6

	return data;
}

float* loadCharBlockRepeat(const char *path, const int szX, const int szY, const int szZ) {

	const int n = 8;

	FILE* fd;
	size_t  fz = 0, bz = szX * szY * szZ * n;
	float* data;

	// open file for reading
	if (!(fd = fopen(path, "r"))) {
		printf("Error while opening %s file.\n", path);
		return NULL;
	}

	// determine file size
	if(fseek(fd, 0, SEEK_END)) {
		printf("Error while operating on %s file.\n", path);
		return NULL;
	}
	fz = ftell(fd);
	rewind(fd);

	data = (float*)calloc(bz, sizeof(float));
	for(int z = 0; z < szZ; z++)
		for(int y = 0; y < szY; y++)
			for(int x = 0; x < szX; x++) {
				int value = fgetc(fd);
				data[x + y * szX * 2 + z * szX * szY * 4] = (float)value;
				data[(x + szX) + y * szX * 2 + z * szX * szY * 4] = (float)value;
				data[x + (y + szY) * szX * 2 + z * szX * szY * 4] = (float)value;
				data[(x + szX) + (y + szY) * szX * 2 + z * szX * szY * 4] = (float)value;
				data[x + y * szX * 2 + (z + szZ) * szX * szY * 4] = (float)value;
				data[(x + szX) + y * szX * 2 + (z + szZ) * szX * szY * 4] = (float)value;
				data[x + (y + szY) * szX * 2 + (z + szZ) * szX * szY * 4] = (float)value;
				data[(x + szX) + (y + szY) * szX * 2 + (z + szZ) * szX * szY * 4] = (float)value;
			}

	return data;
}

