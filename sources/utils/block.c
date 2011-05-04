///*
// * block.c
// *
// *  Created on: Mar 26, 2011
// *      Author: jonez
// */
//
//#define X_AXIS 0
//#define Y_AXIS 1
//#define Z_AXIS 2
//#define MIN_ARGS_NO 6
//#define PATH_ARG_NO 1
//#define ISOVALUE_ARG_NO 2
//#define SIZE_ARGS 3
//#define SIZEX_ARG_NO 3
//#define SIZEY_ARG_NO 4
//#define SIZEZ_ARG_NO 5
//#define CMD_USAGE "path isoValue sizeX sizeY sizeZ"
//
//#include <stdio.h>
//#include <stdlib.h>
//
//static inline size_t getPosition(const int x, const int y, const size_t sizeX) {
//	return x + y * sizeX;
//}
//
//static size_t countOccupiedVoxels(const char *path, const float isoValue, const size_t* sizes) {
//
//	FILE* fd;
//
//	// open file for reading
//	fd = fopen(path, "r");
//	if (fd == NULL) {
//		printf("Error while opennig '%s' file.\n", path);
//		return 0;
//	}
//
//	printf("Loading dataset from '%s' file.\n", path);
//
//	size_t sliceSize = sizes[X_AXIS] * sizes[Y_AXIS];
//	size_t valuesSize = sizes[X_AXIS] * sizes[Y_AXIS] * sizes[Z_AXIS];
//
//	size_t slice, occupied = 0, count = 0;
//
//	unsigned char* upSlice = malloc(sliceSize * sizeof(unsigned char));
//	slice = fread(upSlice, sizeof(unsigned char), sliceSize, fd);
//	count += slice;
//
//	unsigned char* downSlice = malloc(sliceSize * sizeof(unsigned char));
//	slice = fread(downSlice, sizeof(unsigned char), sliceSize, fd);
//	count += slice;
//
////	for(int i = 0; i < slice; i++)
////		printf("%d,%d-",*(downSlice + i), *(upSlice + i));
//
//	while(slice >= sliceSize && count < valuesSize) {
//
//		for(int y = 0; y < sizes[Y_AXIS]; y++)
//			for(int x = 0; x < sizes[X_AXIS]; x++) {
//
//				unsigned char corners[8];
//				corners[0] = *(downSlice + getPosition(x, y, sizes[X_AXIS]));
//				corners[1] = *(downSlice + getPosition(x + 1, y, sizes[X_AXIS]));
//				corners[2] = *(downSlice + getPosition(x + 1, y + 1, sizes[X_AXIS]));
//				corners[3] = *(downSlice + getPosition(x, y + 1, sizes[X_AXIS]));
//				corners[4] = *(upSlice + getPosition(x, y, sizes[X_AXIS]));
//				corners[5] = *(upSlice + getPosition(x + 1, y, sizes[X_AXIS]));
//				corners[6] = *(upSlice + getPosition(x + 1, y + 1, sizes[X_AXIS]));
//				corners[7] = *(upSlice + getPosition(x, y + 1, sizes[X_AXIS]));
//
//				unsigned char combination = 0;
//				combination =  (corners[0] > isoValue);
//				combination += (corners[1] > isoValue) * 2;
//				combination += (corners[2] > isoValue) * 4;
//				combination += (corners[3] > isoValue) * 8;
//				combination += (corners[4] > isoValue) * 16;
//				combination += (corners[5] > isoValue) * 32;
//				combination += (corners[6] > isoValue) * 64;
//				combination += (corners[7] > isoValue) * 128;
//
//				if(!(combination == 0 || combination == 255))
//					occupied++;
//
//			}
//
//		unsigned char* tempSlice;
//		tempSlice = upSlice;
//		upSlice = downSlice;
//		downSlice = tempSlice;
//
//		slice = fread(upSlice, sizeof(unsigned char), sliceSize, fd);
//		count += slice;
//
////		printf("sliceSize=%d, valuesSize=%d, slice=%d, count=%d\n", sliceSize, valuesSize, slice, count);
//	}
//
//	free(upSlice);
//	free(downSlice);
//
//	return occupied;
//}
//
//int main(int argc, char** argv) {
//
//	if(argc < MIN_ARGS_NO) {
//		printf("usage: %s %s\n", argv[0], CMD_USAGE);
//		return -1;
//	}
//
//	char* path = argv[PATH_ARG_NO];
//
//	float isoValue;
//	sscanf(argv[ISOVALUE_ARG_NO], "%f", &isoValue);
//
//	size_t sizes[SIZE_ARGS];
//	sizes[X_AXIS] = atoi(argv[SIZEX_ARG_NO]);
//	sizes[Y_AXIS] = atoi(argv[SIZEY_ARG_NO]);
//	sizes[Z_AXIS] = atoi(argv[SIZEZ_ARG_NO]);
//
//	size_t valuesSize = sizes[X_AXIS] * sizes[Y_AXIS] * sizes[Z_AXIS];
//	size_t voxelsSize = (sizes[X_AXIS] - 1) * (sizes[Y_AXIS] - 1) * (sizes[Z_AXIS] - 1);
//
//	printf("voxels: %d * %d * %d = %d (%d values)\n",
//			sizes[X_AXIS] - 1, sizes[Y_AXIS] - 1, sizes[Z_AXIS] - 1,
//			voxelsSize, valuesSize);
//
//	size_t count = countOccupiedVoxels(path, isoValue, sizes);
//	printf("occupied voxels: %d/%d (isovalue = %.1f)\n", count, voxelsSize, isoValue);
//
//	return 0;
//}

