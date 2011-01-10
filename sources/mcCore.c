/*
 * mcCore.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: Use async calls with events.
 */

#include "mcCore.h"

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "utilities.h"
#include "mcTables.h"
#include "clHelper.h"
#include "clScan.h"
#include "CL/cl_gl.h"


#define KERNELS_SOURCE_FILE "mc.cl"

////////////////////////////////////////////////////////////////////////////////

enum kernelMethods {

    MC_CLASSIFICATION		= 0,
    MC_COMPACTION			= 1,
    MC_GENERATION			= 2

};

static const char* kernelNames[] = {

    "mcClassification",
    "mcCompaction",
    "mcGeneration"

};

static const unsigned int kernelCount =
		sizeof(kernelNames) / sizeof(kernelNames[0]);

////////////////////////////////////////////////////////////////////////////////

static int initialized = FALSE;

clhResources resources;
cl_program program;
cl_kernel classificationKernel;
cl_kernel compactionKernel;
cl_kernel generationKernel;
cl_mem trianglesTableBuffer;
cl_mem verticesTableBuffer;

////////////////////////////////////////////////////////////////////////////////

/*
 * run marching cubes algorithm with OpenCL
 *
 * Notes: ? - add option to keep output result in memory device
 */
int mcRunCL(cl_float* input, cl_float isoValue,
		  size_t inSizeX, size_t inSizeY, size_t inSizeZ,
		  cl_float4 valuesDistance, cl_int4 valuesOffset,
		  cl_float4** output, GLuint* vbo, size_t* outSize/*,
		  clhResources resources, cl_program program,
		  cl_mem trianglesTableBuffer, cl_mem verticesTableBuffer,
		  cl_kernel classificationKernel, cl_kernel compactionKernel,
		  cl_kernel generationKernel*/) {

	cl_int err;
	cl_event event;

	size_t trianglesTableSize, verticesTableSize;

	if(!initialized) {

		resources = clhInitResources("", CL_DEVICE_TYPE_CPU, 0, &err);
		program = clhBuildProgramFromFile(KERNELS_SOURCE_FILE, resources, &err);

		// Allocate memory on device to hold lookup tables
		trianglesTableSize = sizeof(trianglesTable);
		trianglesTableBuffer = clCreateBuffer(resources->context, CL_MEM_COPY_HOST_PTR |
				CL_MEM_READ_ONLY, trianglesTableSize, (void*)trianglesTable, &err);
		clhErrorInfo(err, "create triangles lookup table", "mcCore");

		verticesTableSize = sizeof(verticesTable);
		verticesTableBuffer = clCreateBuffer(resources->context, CL_MEM_COPY_HOST_PTR |
				CL_MEM_READ_ONLY, verticesTableSize, (void*)verticesTable, &err);
		clhErrorInfo(err, "create vertices lookup table", "mcCore");

		//### DEBUG ###
//		unsigned char* vTable = malloc(verticesTableSize);
//		err = clEnqueueReadBuffer(resources->cmdQueues[0], verticesTableBuffer, CL_TRUE, 0, verticesTableSize, vTable, 0, NULL, NULL);
//		clhErrorInfo(err, "read vTable buffer", "mcCore");
//		for(int i = 0; i < verticesTableSize; i++)
//			printf("%d,", vTable[i]);
//		printf("\n");

		// Now create the kernel "objects" that we want to use
		//	cl_kernel kernels[3];
		//
		//	for(int i = 0; i < kernelCount; i++)
		classificationKernel = clCreateKernel(program, kernelNames[MC_CLASSIFICATION], &err);
		clhErrorInfo(err, "create kernel classification", "mcCore");

		compactionKernel = clCreateKernel(program, kernelNames[MC_COMPACTION], &err);
		clhErrorInfo(err, "create kernel compaction", "mcCore");

		generationKernel = clCreateKernel(program, kernelNames[MC_GENERATION], &err);
		clhErrorInfo(err, "create kernel generation", "mcCore");

		// Now create the kernel "objects" that we want to use
		//	cl_kernel kernel;
		//	char* kernelName = "generation";
		//
		//	printf("Creating kernel '%s' from file '%s'\n", kernelName, KERNELS_SOURCE_FILE);
		//	kernel = clCreateKernel(program, kernelName, &err);
		//	clhErrorInfo(err, "create kernel", "mcCore");

		initialized = TRUE;

		printf(	"Init memory allocation\n"
							"\ttriangles table buffer: %dB\n"
							"\tvertices table buffer: %dB\n",
						trianglesTableSize,
						verticesTableSize);


	}

	// Allocate memory on the device to hold our data and store the results into

	// Input and results array
	size_t inputSize = sizeof(cl_float) * ((inSizeX + 1) * (inSizeY + 1) * (inSizeZ + 1));
	cl_mem inputBuffer = clCreateBuffer(resources->context, CL_MEM_COPY_HOST_PTR |
			CL_MEM_READ_ONLY, inputSize, input, &err);
	clhErrorInfo(err, "create and fill input buffer", "mcCore");

//	### DEBUG ###
//	float* in = malloc(inputSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], inputBuffer, CL_TRUE, 0, inputSize, in, 0, NULL, NULL);
//	clhErrorInfo(err, "read input buffer", "mcCore");
//	float inSum = 0;
//	printf("1:\n");
//	for(int i = 0; i < inputSize / 4; i++)
//		if(in[i] < 15000) printf("%.0f,", in[i]);
//	printf("\n");
//
//	printf("2:\n");
//	for(int i = 0; i < inputSize / 4; i++)
//		if(input[i] < 15000) printf("%.0f,", input[i]);
//	printf("\n");

//	for(int i = 0; i < 20; i++)
//		printf("%.0f,", input[i]);
//	printf("\n");
//
//	int c = 0;
//	for(int i = 0; i < inputSize / sizeof(cl_float); i++)
//		if(input[i] < isoValue) c++;
//	printf("%d\n", c);
//
//	exit(0);

//	printf("%.0f\n", input[0]);

//	inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, inputSize, NULL, &err);
//	mclErrorInfo(err, "create input buffer", "mcCore");
//
//	err = clEnqueueWriteBuffer(cmd_queue, inputBuffer, CL_TRUE, 0, inputSize,
//			(void*)input, 0, NULL, &event);
//	mclErrorInfo(err, "fill input buffer", "mcCore");

	size_t classificationSize = sizeof(cl_float) * (inSizeX * inSizeY * inSizeZ);
	cl_mem occupiedBuffer = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, classificationSize, NULL, &err);
	clhErrorInfo(err, "create occupied buffer", "mcCore");
	cl_mem verticesBuffer = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, classificationSize, NULL, &err);
	clhErrorInfo(err, "create vertices buffer", "mcCore");

	// Get all of the stuff written and allocated
	clFinish(resources->cmdQueues[0]);

	// Now setup the arguments to our kernel
	int arg = 0;
	err  = clSetKernelArg(classificationKernel, arg++, sizeof(cl_mem), &inputBuffer);
	err |= clSetKernelArg(classificationKernel, arg++, sizeof(cl_float), &isoValue);
	err |= clSetKernelArg(classificationKernel, arg++, sizeof(cl_mem), &trianglesTableBuffer);
	err |= clSetKernelArg(classificationKernel, arg++, sizeof(cl_mem), &verticesTableBuffer);
	err |= clSetKernelArg(classificationKernel, arg++, sizeof(cl_mem), &verticesBuffer);
	err |= clSetKernelArg(classificationKernel, arg++, sizeof(cl_mem), &occupiedBuffer);
	clhErrorInfo(err, "set classification kernel arguments", "mcCore");

	// Run the calculation by enqueuing it and forcing the
	// command queue to complete the task
	printf("Launching classification kernel configuration: %d*%d*%d (%d)\n",
			inSizeX, inSizeY, inSizeZ , (inSizeX * inSizeY * inSizeZ));
	size_t classificationGWS[3] = {inSizeX, inSizeY, inSizeZ};

	err = clEnqueueNDRangeKernel(resources->cmdQueues[0], classificationKernel, 3, NULL,
			classificationGWS, NULL, 0, NULL, &event);
	clhErrorInfo(err, "enqueue kernel range", "mcCore");

	printf("wait for classification kernel to finish\n");
	err = clWaitForEvents(1, &event);
	clhErrorInfo(err, "wait for classification kernel", "mcCore");

	// Get all of the stuff written and allocated
	clFinish(resources->cmdQueues[0]);

	//### DEBUG ###
//	float* class1 = malloc(classificationSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], verticesBuffer, CL_TRUE, 0, classificationSize, class1, 0, NULL, NULL);
//	clhErrorInfo(err, "read class1 buffer", "mcCore");
//	float classSum = 0;
//	for(int i = 0; i < classificationSize / 4; i++)
//		classSum += class[i];
//	printf("%.0f\n", classSum);
//	for(int i = 0; i < classificationSize / 4; i += 1000)
//		printf("%.0f,", class1[i]);
//	printf("\n");

//	float* class2 = malloc(classificationSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], occupiedBuffer, CL_TRUE, 0, classificationSize, class2, 0, NULL, NULL);
//	clhErrorInfo(err, "read class2 buffer", "mcCore");
//	float classSum = 0;
//	for(int i = 0; i < classificationSize / 4; i++)
//		classSum += class[i];
//	printf("%.0f\n", classSum);
//	for(int i = 0; i < classificationSize / 4; i += 1000)
//		printf("%.0f,", class2[i]);
//	printf("\n");

//	exit(0);

	float voxelCount = 0;
	cl_mem scannedOccupiedBuffer = clsScanFromDevice(resources, 0, occupiedBuffer, inSizeX * inSizeY * inSizeZ, &voxelCount, &err);

	//### DEBUG ###
//	float* scan = malloc(classificationSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], scannedOccupiedBuffer, CL_TRUE, 0, classificationSize, scan, 0, NULL, NULL);
//	clhErrorInfo(err, "read scan buffer", "mcCore");
//	for(int i = 0; i < 20; i++)
//		printf("%.0f,", scan[i]);
//	printf("\n");

	size_t compactionSize = voxelCount * sizeof(size_t);

	if(compactionSize > 0) {

		cl_mem compactedInputBuffer = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, compactionSize, NULL, &err);
		clhErrorInfo(err, "create compacted input buffer", "mcCore");

		// Get all of the stuff written and allocated
		clFinish(resources->cmdQueues[0]);

		// Now setup the arguments to our kernel
		arg = 0;
		err  = clSetKernelArg(compactionKernel, arg++, sizeof(cl_mem), &occupiedBuffer);
		err |= clSetKernelArg(compactionKernel, arg++, sizeof(cl_mem), &scannedOccupiedBuffer);
		err |= clSetKernelArg(compactionKernel, arg++, sizeof(cl_mem), &compactedInputBuffer);

		printf("Launching compaction kernel configuration: %d\n", inSizeX * inSizeY * inSizeZ);
		size_t compactionGWS[1] = {inSizeX * inSizeY * inSizeZ};

		err = clEnqueueNDRangeKernel(resources->cmdQueues[0], compactionKernel, 1, NULL, compactionGWS,
				NULL, 0, NULL, &event);
		clhErrorInfo(err, "enqueue compaction kernel range", "mcCore");

		printf("wait for compaction kernel to finish\n");
		err = clWaitForEvents(1, &event);
		clhErrorInfo(err, "wait for compaction kernel", "mcCore");

		clReleaseMemObject(occupiedBuffer);
		clReleaseMemObject(scannedOccupiedBuffer);

		// Get all of the stuff written and allocated
		clFinish(resources->cmdQueues[0]);

		float verticeCount = 0;
		cl_mem scannedVerticesBuffer = clsScanFromDevice(resources, 0, verticesBuffer, inSizeX * inSizeY * inSizeZ, &verticeCount, &err);

		clReleaseMemObject(verticesBuffer);

		// Get all of the stuff written and allocated
		clFinish(resources->cmdQueues[0]);


		//### DEBUG ###
	//	size_t* comp = malloc(compactionSize);
	//	err = clEnqueueReadBuffer(resources->cmdQueues[0], compactedInputBuffer, CL_TRUE, 0, compactionSize, comp, 0, NULL, NULL);
	//	clhErrorInfo(err, "read comp buffer", "mcCore");
	//	for(int i = 0; i < 20; i++)
	//		printf("%d,", comp[i]);
	//	printf("\n");

		size_t outputSize = verticeCount * sizeof(cl_float4);
		cl_mem outputBuffer = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, outputSize, NULL, &err);

	//	GLuint retVBO;
	//	cl_mem outputBuffer = clhCreateGLCLBuffer(resources->context, outputSize, &retVBO, &err);
		clhErrorInfo(err, "create output buffer", "mcCore");

		clFinish(resources->cmdQueues[0]);

		cl_uint2 sizes = {{inSizeX, inSizeY}};

		// Now setup the arguments to our kernel
		arg = 0;
		err  = clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &inputBuffer);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_uint2), &sizes);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_float), &isoValue);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_float4), &valuesDistance);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_int4), &valuesOffset);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &trianglesTableBuffer);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &verticesTableBuffer);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &scannedVerticesBuffer);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &compactedInputBuffer);
		err |= clSetKernelArg(generationKernel, arg++, sizeof(cl_mem), &outputBuffer);
		clhErrorInfo(err, "set generation kernel arguments", "mcCore");

	//    err = clEnqueueAcquireGLObjects(resources->cmdQueues[0], 1, &outputBuffer, 0, NULL, NULL);
	//    clhErrorInfo(err, "aquire gl objects", "mcCore");

		// Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		printf("Launching generation kernel configuration: %d\n", (size_t)voxelCount);
		size_t generationGWS[1] = {voxelCount};

		err = clEnqueueNDRangeKernel(resources->cmdQueues[0], generationKernel, 1, NULL, generationGWS,
				NULL, 0, NULL, &event);
		clhErrorInfo(err, "enqueue generation kernel range", "mcCore");

		printf("wait for generationKernel to finish\n");
		err = clWaitForEvents(1, &event);
		clhErrorInfo(err, "wait for generation kernel", "mcCore");

	//    err = clEnqueueReleaseGLObjects(resources->cmdQueues[0], 1, &outputBuffer, 0, NULL, NULL);
	//    clhErrorInfo(err, "release gl objects", "mcCore");

		// Once finished read back the results from the answer
		// array into the results array
		//### DEBUG ###
		cl_float4* outputRet = malloc(outputSize);
		err = clEnqueueReadBuffer(resources->cmdQueues[0], outputBuffer, CL_TRUE, 0, outputSize, outputRet, 0, NULL, NULL);
		clhErrorInfo(err, "read output buffer", "mcCore");
//		for(int i = 0; i < 20; i++)
//			printf("(%.2f;%.2f;%.2f;%.0f)", outputRet[i].s[0], outputRet[i].s[1], outputRet[i].s[2], outputRet[i].s[3]);
//		printf("\n");

		clFinish(resources->cmdQueues[0]);



		clReleaseMemObject(scannedVerticesBuffer);
		clReleaseMemObject(compactedInputBuffer);
		clReleaseMemObject(outputBuffer);


		if(output) *output = outputRet;
		if(outSize) *outSize = verticeCount;

		printf(	"Memory allocation\n"
					"\tinput buffer: %dKB\n"
					"\tclassification buffers: 2x %dKB\n"
					"\tcompacted input buffer: %dKB (%d%%)\n"
					"\toutput buffer %dKB\n",
				inputSize / KB,
				classificationSize / KB,
				compactionSize / KB, (compactionSize * 100) / inputSize,
				outputSize / KB);

	}
	else {

		if(output) *output = NULL;
		if(outSize) *outSize = 0;

		printf(	"Memory allocation\n"
						"\tinput buffer: %dKB\n"
						"\tclassification buffers: 2x %dKB\n"
						"\tcompacted input buffer: %dKB (%d%%)\n",
					inputSize / KB,
					classificationSize / KB,
					compactionSize / KB, (compactionSize * 100) / inputSize);

	}

	clReleaseMemObject(inputBuffer);


	//	if(vbo) *vbo = retVBO;


	return CL_SUCCESS;
}

void mcReleaseCL() {

	initialized = FALSE;

	clsRelease();

	clReleaseMemObject(trianglesTableBuffer);
	clReleaseMemObject(verticesTableBuffer);
	clReleaseKernel(classificationKernel);
	clReleaseKernel(compactionKernel);
	clReleaseKernel(generationKernel);
	clReleaseProgram(program);
//	clReleaseCommandQueue(resources->cmdQueues[0]);
//	clReleaseContext(resources->context);

	printf("%s resources released!\n", __FILE__);

}

void test() {

	clhSetVerbose(FALSE);
	clsSetVerbose(FALSE);

	cl_int err;
	cl_event event;

	clhResources resources = clhInitResources("", CL_DEVICE_TYPE_CPU, 0, &err);

	cl_program program = clhBuildProgramFromFile(KERNELS_SOURCE_FILE, resources, &err);

	float values[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};

	cl_mem input = clCreateBuffer(resources->context, CL_MEM_COPY_HOST_PTR |
			CL_MEM_READ_ONLY, sizeof(values), values, &err);
	clhErrorInfo(err, "array", "mcCore");

	cl_float sum;

	cl_mem scanned = clsScanFromDevice(resources, 0, input, 10, &sum, &err);
	clhErrorInfo(err, "scan bin", "mcCore");

	printf("sum: %d\n", (int)sum);
	int result[(int)sum];

	cl_mem output = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, sizeof(cl_uint) * sum, NULL, &err);
	clhErrorInfo(err, "array", "mcCore");

	cl_kernel kernel = clCreateKernel(program, kernelNames[MC_COMPACTION], &err);
	clhErrorInfo(err, "create kernel", "mcCore");

	// Get all of the stuff written and allocated
	clFinish(resources->cmdQueues[0]);

	// Now setup the arguments to our kernel
	err  = clSetKernelArg(kernel,  0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel,  1, sizeof(cl_mem), &scanned);
	err |= clSetKernelArg(kernel,  2, sizeof(cl_mem), &output);
	clhErrorInfo(err, "set kernel arguments", "mcCore");

	// Run the calculation by enqueuing it and forcing the
	// command queue to complete the task
	size_t globalWorkSize = 10;

	err = clEnqueueNDRangeKernel(resources->cmdQueues[0], kernel, 1, NULL, &globalWorkSize,
			NULL, 0, NULL, &event);
	clhErrorInfo(err, "enqueue kernel range", "mcCore");

	err = clWaitForEvents(1, &event);
	clhErrorInfo(err, "wait for kernel", "mcCore");

	err = clEnqueueReadBuffer(resources->cmdQueues[0], output, CL_TRUE, 0, sizeof(cl_uint) * sum,
			result, 0, NULL, NULL);

	clFinish(resources->cmdQueues[0]);

	for(int i = 0; i < (int)sum; i++)
		printf("%d ", result[i]);

	clReleaseMemObject(input);
	clReleaseMemObject(output);
	clReleaseCommandQueue(resources->cmdQueues[0]);
	clReleaseContext(resources->context);

}
