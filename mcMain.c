/*
 * mc.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "mcTables.h"

#define KB 1024
#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)

#define NAME_SIZE 1024 // 1KB
#define BUILD_LOG_SIZE 102400 // 1024B * 100 = 100KB

#define SIZE 64
#define SOURCE_FILE "mcKernels.cl"


cl_int runCL(	char* sourceFile, cl_float* input, cl_float isoValue,
				cl_float4 valuesDistance, cl_uint4 valuesOffset,
				cl_float4* output1, cl_float4* output2,
				size_t sizeX, size_t sizeY, size_t sizeZ) {

	cl_int err;
	cl_event event;
	
	cl_platform_id platform = NULL;

	err = clGetPlatformIDs(1, &platform, NULL);
	mclErrorInfo(err, "get platform");
	// Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	cl_device_id device = NULL;

	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	mclErrorInfo(err, "get devices");
	if(err != CL_SUCCESS) {
	
		// Find the CPU CL device, as a fallback
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
		mclErrorInfo(err, "No device found!\n");
		
		// Can't find any usable device
		if(err != CL_SUCCESS)
			return err;

	}

	// Get some information about select device
	cl_char vendor_name[NAME_SIZE];
	cl_char device_name[NAME_SIZE];
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), 
			vendor_name, NULL);
	err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), 
			device_name, NULL);
	printf("Using device: %s %s\n", vendor_name, device_name);
	
	
	// Now create a context to perform our calculation with the 
	// specified device
	cl_context context;
	
	context = clCreateContext(0, 1, &device, NULL, NULL, &err);
	mclErrorInfo(err, "create context");
	//if(err != CL_SUCCESS) {
	//
	//	// Can't create a context
	//	printf("Error while creating context!\n");
	//	return -1;
	//	
	//}
	
	
	// And also a command queue for the context
	cl_command_queue cmd_queue;

	cmd_queue = clCreateCommandQueue(context, device, 0, &err);
	mclErrorInfo(err, "create command queue");
	//if(err != CL_SUCCESS) {
	//
	//	// Can't create a command queue
	//	printf("Error while creating command queue!\n");
	//	return -1;
	//
	//}
	
	
	// Load the program source from disk
	char* source = loadSource(sourceFile);

	printf("Creating program from file '%s'\n", sourceFile);
	cl_program program = clCreateProgramWithSource(context, 1, 
			(const char**)&source, NULL, &err);
	mclErrorInfo(err, "create program");
	
	err = clBuildProgram(program, 0, NULL, "-Wall", NULL, NULL);
	mclErrorInfo(err, "build program");
	
	cl_char buildInfo[BUILD_LOG_SIZE];
	err = clGetProgramBuildInfo (program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildInfo), buildInfo, NULL);
	mclErrorInfo(err, (char*)buildInfo);


	// Now create the kernel "objects" that we want to use
	cl_kernel kernel;
	char* kernelName = "generation";

	printf("Creating kernel '%s' from file '%s'\n", kernelName, sourceFile);
	kernel = clCreateKernel(program, kernelName, &err);
	mclErrorInfo(err, "create kernel");
	
	// Allocate memory on device to hold lookup tables
	size_t trianglesTableSize = sizeof(trianglesTable);
	size_t verticesTableSize = sizeof(verticesTable);
	printf(	"Memory allocation\n"
				"\ttriangles table buffer: %iB\n"
				"\tvertices table buffer: %iB\n",
			trianglesTableSize, verticesTableSize);

	cl_mem trianglesTableBuffer, verticesTableBuffer;

	trianglesTableBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR |
			CL_MEM_READ_ONLY, trianglesTableSize, (void*)trianglesTable, &err);
	mclErrorInfo(err, "create triangles lookup table");

	verticesTableBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR |
			CL_MEM_READ_ONLY, verticesTableSize, (void*)verticesTable, &err);
	mclErrorInfo(err, "create vertices lookup table");


	// Allocate memory on the device to hold our data and store the results into
	size_t inputSize = sizeof(cl_float) * ((sizeX + 1) * (sizeY + 1) * (sizeZ + 1));
	size_t outputSize1 = sizeof(cl_float4) * (sizeX * sizeY * sizeZ) * 15;
	size_t outputSize2 = sizeof(cl_float4) * (sizeX * sizeY * sizeZ) * 5;
	printf(	"Memory allocation\n"
				"\tinput buffer: %iKB\n"
				"\toutput buffer: %iMB + %iMB\n",
			inputSize / KB, outputSize1 / MB, outputSize2 / MB);

	cl_mem inputBuffer, output1Buffer, output2Buffer;

	// Input and results array
	inputBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR |
			CL_MEM_READ_ONLY, inputSize, (void*)input, &err);
	mclErrorInfo(err, "create and fill input buffer");

//	inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, inputSize, NULL, &err);
//	mclErrorInfo(err, "create input buffer");
//
//	err = clEnqueueWriteBuffer(cmd_queue, inputBuffer, CL_TRUE, 0, inputSize,
//			(void*)input, 0, NULL, &event);
//	mclErrorInfo(err, "fill input buffer");

	output1Buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputSize1, NULL, &err);
	mclErrorInfo(err, "create output1 buffer");

	output2Buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputSize2, NULL, &err);
	mclErrorInfo(err, "create output2 buffer");

	// Get all of the stuff written and allocated
	clFinish(cmd_queue);

	// Now setup the arguments to our kernel
	err  = clSetKernelArg(kernel,  0, sizeof(cl_mem), &inputBuffer);
	err |= clSetKernelArg(kernel,  1, sizeof(cl_float), &isoValue);
	err |= clSetKernelArg(kernel,  2, sizeof(cl_float4), &valuesDistance);
	err |= clSetKernelArg(kernel,  3, sizeof(cl_uint4), &valuesOffset);
	err |= clSetKernelArg(kernel,  4, sizeof(cl_mem), &output1Buffer);
	err |= clSetKernelArg(kernel,  5, sizeof(cl_mem), &output2Buffer);
	err |= clSetKernelArg(kernel,  6, sizeof(cl_mem), &trianglesTableBuffer);
	err |= clSetKernelArg(kernel,  7, sizeof(cl_mem), &verticesTableBuffer);
	mclErrorInfo(err, "set kernel arguments");

	// Run the calculation by enqueuing it and forcing the
	// command queue to complete the task
	size_t globalWorkSize[3] = {sizeX, sizeY, sizeZ};
	printf("Launching kernel configuration: %i*%i*%i (%i)\n",
			sizeX, sizeY, sizeZ , (sizeX * sizeY * sizeZ));


	err = clEnqueueNDRangeKernel(cmd_queue, kernel, 3, NULL, globalWorkSize,
			NULL, 0, NULL, &event);
	mclErrorInfo(err, "enqueue kernel range");

	printf("wait for kernel to finish\n");
	err = clWaitForEvents(1, &event);
	mclErrorInfo(err, "wait for kernel");

	// Once finished read back the results from the answer
	// array into the results array
	err = clEnqueueReadBuffer(cmd_queue, output1Buffer, CL_TRUE, 0, outputSize1,
			(void*)output1, 0, NULL, &event);
	mclErrorInfo(err, "read output1 buffer");

	err = clEnqueueReadBuffer(cmd_queue, output2Buffer, CL_TRUE, 0, outputSize2,
			(void*)output2, 0, NULL, &event);
	mclErrorInfo(err, "read output1 buffer");

	clFinish(cmd_queue);


	clReleaseMemObject(inputBuffer);
	clReleaseMemObject(output1Buffer);
	clReleaseMemObject(output2Buffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);


	return CL_SUCCESS;
}

 
int main(int argc, char** argv) {

//	FILE* fd;
//	long fz;
//	char* src;
//
//	// open file for reading
//	fd = fopen("teste", "r");
//
//	char c;
//	int x = 0;
//	while((c = fgetc(fd)) != EOF) {
//		printf("%c", c);
//		if(c == '}')
//			if((c = fgetc(fd)) == ',')
//				printf(",\t//%i", x++);
//			else
//				printf("%c", c);
//
//	}
//
//	exit(0);

	size_t size = SIZE;

	cl_float isoValue = rand() / (size * size * size) / 2;

	cl_float4 distance = {1.0f, 1.0f, 1.0f, 0.0f};
	cl_uint4 offset = {0, 0, 0, 0};

	cl_float * data = makeFloatBlock(size + 1);
	cl_float4 * results1 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 15);
	cl_float4 * results2 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 5);
	
	runCL(SOURCE_FILE, data, isoValue, distance, offset, results1, results2, size, size, size);
	
	printf("size: %i, isovalue: %.0f\n\n", size, isoValue);

	int i, j, k;

	printf("input\n");
	for(i = 0; i < ((size + 1 < 3) ? size + 1 : 3) ; i++) {
		for(j = 0; j < ((size + 1 < 3) ? size + 1 : 3); j++) {
			for(k = 0; k < ((size + 1 < 3) ? size + 1 : 3); k++)
				printf("%.0f  ", data[k + j * (size + 1) + i * (size + 1) * (size + 1)]);
			printf("\n");
		}
		printf("^^^ %i ^^^\n", i);
	}

	printf("\noutput1\n");
	for(i = 0; i < 15; i++)
		printf("%.1f;%.1f;%.1f;%.1f | ", results1[i].x, results1[i].y, results1[i].z, results1[i].w);
	printf("\n");
	for(; i < 30; i++)
		printf("%.1f;%.1f;%.1f;%.1f | ", results1[i].x, results1[i].y, results1[i].z, results1[i].w);


	printf("\noutput2\n");
	for(i = 0; i < 5; i++)
		printf("%.1f;%.1f;%.1f;%.1f | ", results2[i].x, results2[i].y, results2[i].z, results2[i].w);
	printf("\n");
	for(; i < 10; i++)
		printf("%.1f;%.1f;%.1f;%.1f | ", results2[i].x, results2[i].y, results2[i].z, results2[i].w);

//	printf("\noutput1\n");
//	for(i = 0; i < ((size < 2) ? size : 2); i++) {
//		for(j = 0; j < ((size < 2) ? size : 2); j++) {
//			for(k = 0; k < ((size < 2) ? size : 2); k++)
//				printf("%.2f  ", result1[k + j * size + i * size * size]);
//			printf("\n");
//		}
//		printf("^^^ %i ^^^\n", i);
//	}
//
//	printf("\noutput2\n");
//		for(i = 0; i < ((size < 2) ? size : 2); i++) {
//			for(j = 0; j < ((size < 2) ? size : 2); j++) {
//				for(k = 0; k < ((size < 2) ? size : 2); k++)
//					printf("%.2f  ", result2[k + j * size + i * size * size]);
//				printf("\n");
//			}
//			printf("^^^ %i ^^^\n", i);
//	}

	free(data);
	free(results1);
	free(results2);
		
	return 0;
	
}
