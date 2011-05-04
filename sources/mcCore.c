/*
 * mcCore.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: * Use async calls with events.
 * 		  * When possible reuse resources like buffers, instead of creating a
 * 		  new one each run; most resources aren't being reused, the reused ones
 * 		  most be initialized first and then reuse them until no longer needed,
 *		  at that time they most be released.
 */

#include "mcCore.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <CL/cl_gl.h>

#include "common.h"
#include "mcTables.h"
#include "clHelper.h"
#include "clScan.h"

////////////////////////////////////////////////////////////////////////////////

#define KERNELS_FILE "mc.cl"

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

// print something

int MCC_VERBOSE = FALSE;

int mccGetVerbose() {
	return MCC_VERBOSE;
}

void mccSetVerbose(const int state) {
	MCC_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

static int initialized = FALSE;

clhResources resources;

cl_program program;
cl_kernel classificationKernel;
cl_kernel compactionKernel;
cl_kernel generationKernel;
size_t* classificationMaxWGSs;
size_t* compactionMaxWGSs;
size_t* generationMaxWGSs;

cl_mem trianglesTableBuffer;
cl_mem verticesTableBuffer;
cl_image_format dataSetFormat;
cl_sampler inputTextureSampler;

////////////////////////////////////////////////////////////////////////////////

cl_int mccInit(const clhResources initializedResources) {

	cl_int retErr;

	if(initializedResources)
		resources = initializedResources;
	else {
		resources = clhInitResources(NULL, 0, 0, &retErr);
		if(retErr) return retErr;
	}
	
	program = clhBuildProgramFromFile(KERNELS_FILE, resources, &retErr);
	if(retErr) return retErr;

	// Now create the kernel "objects" that we want to use
	//	cl_kernel kernels[3];
	//
	//	for(int i = 0; i < kernelCount; i++)
	classificationKernel =
			clCreateKernel(program, kernelNames[MC_CLASSIFICATION], &retErr);
	clhErrorInfo(retErr, "create kernel classification", __FILE__);

//	classificationMaxWGS = clhGetKernelMaxWorkGroupSize(classificationKernel, resources->devices[0], &retErr);
//	printf("max classification wgs: %zu\n", classificationMaxWGS);

	compactionKernel =
			clCreateKernel(program, kernelNames[MC_COMPACTION], &retErr);
	clhErrorInfo(retErr, "create kernel compaction", __FILE__);

//	compactionMaxWGS = clhGetKernelMaxWorkGroupSize(compactionKernel, resources->devices[0], &retErr);
//	printf("max compation wgs: %zu\n", compactionMaxWGS);

	generationKernel =
			clCreateKernel(program, kernelNames[MC_GENERATION], &retErr);
	clhErrorInfo(retErr, "create kernel generation", __FILE__);

//	generationMaxWGS = clhGetKernelMaxWorkGroupSize(generationKernel, resources->devices[0], &retErr);
//	printf("max generation wgs: %zu\n", generationMaxWGS);

	// info about kernel max work group size
	classificationMaxWGSs = calloc(resources->devCount, sizeof(size_t));
	compactionMaxWGSs = calloc(resources->devCount, sizeof(size_t));
	generationMaxWGSs = calloc(resources->devCount, sizeof(size_t));
	
	for(int i = 0; i < resources->devCount; i++) {
		classificationMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(classificationKernel, resources->devices[i], &retErr);
		printf("dev%d max classification wgs: %zu\n", i, classificationMaxWGSs[i]);
		
		compactionMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(compactionKernel, resources->devices[i], &retErr);
		printf("dev%d max compation wgs: %zu\n", i, compactionMaxWGSs[i]);
		
		generationMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(generationKernel, resources->devices[i], &retErr);
		printf("dev%d max generation wgs: %zu\n", i, generationMaxWGSs[i]);
	}

	size_t trianglesTableSize, verticesTableSize;
	// Allocate memory on device to hold lookup tables
	trianglesTableSize = sizeof(trianglesTable);
	trianglesTableBuffer =
			clCreateBuffer(resources->context, CL_MEM_READ_ONLY |
			CL_MEM_COPY_HOST_PTR, trianglesTableSize, trianglesTable, &retErr);
	// profiling proposes
//	trianglesTableBuffer =
//			clCreateBuffer(resources->context, CL_MEM_READ_ONLY,
//					trianglesTableSize, NULL, &retErr);
//	retErr |= clEnqueueWriteBuffer(resources->cmdQueues[0], trianglesTableBuffer,
//			CL_TRUE, 0, trianglesTableSize, trianglesTable, 0, NULL, NULL);
	clhErrorInfo(retErr, "create triangles lookup table", __FILE__);

	verticesTableSize = sizeof(verticesTable);
	verticesTableBuffer =
			clCreateBuffer(resources->context, CL_MEM_READ_ONLY |
			CL_MEM_COPY_HOST_PTR, verticesTableSize, verticesTable, &retErr);
	// profiling proposes
//	verticesTableBuffer =
//			clCreateBuffer(resources->context, CL_MEM_READ_ONLY,
//					verticesTableSize, NULL, &retErr);
//	retErr |= clEnqueueWriteBuffer(resources->cmdQueues[0], verticesTableBuffer,
//			CL_TRUE, 0, verticesTableSize, verticesTable, 0, NULL, NULL);
	clhErrorInfo(retErr, "create vertices lookup table", __FILE__);

	//### DEBUG ###
//	unsigned char* vTable = malloc(verticesTableSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], verticesTableBuffer, CL_TRUE, 0, verticesTableSize, vTable, 0, NULL, NULL);
//	clhErrorInfo(err, "read vTable buffer", "mcCore");
//	for(int i = 0; i < verticesTableSize; i++)
//		printf("%d,", vTable[i]);
//	printf("\n");

	// create texture format and sampler
	dataSetFormat.image_channel_order = CL_R;
	dataSetFormat.image_channel_data_type = CL_FLOAT;

	inputTextureSampler = clCreateSampler(resources->context, CL_FALSE,
			CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &retErr);
	clhErrorInfo(retErr, "create sampler", __FILE__);

	// Now create the kernel "objects" that we want to use
	//	cl_kernel kernel;
	//	char* kernelName = "generation";
	//
	//	printf("Creating kernel '%s' from file '%s'\n", kernelName, KERNELS_SOURCE_FILE);
	//	kernel = clCreateKernel(program, kernelName, &err);
	//	clhErrorInfo(err, "create kernel", "mcCore");

	initialized = TRUE;

	printf( "Init memory allocation\n"
			" triangles table buffer: %zuB\n"
			" vertices table buffer: %zuB\n",
					trianglesTableSize,
					verticesTableSize);
					
	return retErr;

}

/*
 * run marching cubes algorithm with OpenCL
 *
 * Notes: ? - add option to keep output result in memory device
 */
cl_int mccRunCL(const cl_uint deviceIndex, cl_float* values, cl_float isoValue,
				cl_uint4 valuesSizes, cl_float4 valuesDistances,
				cl_float4 valuesOffsets, cl_int2 valuesZBuffers,
				GLuint* trianglesVBO, GLuint* normalsVBO, size_t* outSize/*,
				cl_float4** triangles, cl_float4** normals,
				clhResources resources, cl_program program,
				cl_mem trianglesTableBuffer, cl_mem verticesTableBuffer,
				cl_kernel classificationKernel, cl_kernel compactionKernel,
				cl_kernel generationKernel*/) {

	size_t arg;
	cl_int err;
	cl_event event;
	char infoStr[SMALL_STRING_SIZE];

	cl_context context = resources->context;
	cl_command_queue cmdQueue = resources->cmdQueues[deviceIndex];
	size_t generationMaxWGS = generationMaxWGSs[deviceIndex];

	if(!initialized) {
		printf("mcCore wasn't initialized before running!\n");
		mccInit(NULL);
	}
	
//	cl_device_id dev;
//	clGetCommandQueueInfo(cmdQueue, CL_QUEUE_DEVICE,
//			sizeof(cl_device_id), &dev, NULL);
//	printf("device:%p\n", infoStr);

	// Allocate memory on the device to hold our data and store the results into

	// create texture from dataset and get info

	size_t zBufferSize = valuesZBuffers.s[0] + valuesZBuffers.s[1];
	cl_float4 valuesBuffers = {{0.0f, 0.0f, valuesZBuffers.s[0], 0.0f}};

	//TODO: correct texture region on corner cases (done!?)
	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {valuesSizes.s[0] + 1,
						valuesSizes.s[1] + 1,
						valuesSizes.s[2] + zBufferSize + 1};
						
	cl_mem inputTexture = clCreateImage3D(context, CL_MEM_READ_ONLY,
			&dataSetFormat, valuesSizes.s[0] + 1, valuesSizes.s[1] + 1,
			valuesSizes.s[2] + zBufferSize + 1, 0, 0, NULL, &err);

//	printf("%d %zu %zu %zu %p %p %p\n", err, region[0], region[1], region[2], values, inputTexture, resources->cmdQueues[0]);
//	fflush(NULL);

	err |= clEnqueueWriteImage(cmdQueue, inputTexture, CL_TRUE, origin, region,
			0, 0, values, 0, NULL, &event);
	clhErrorInfo(err, "create and fill input texture", __FILE__);
	
	clhGetEventProfilingInfo(event, sizeof(infoStr), infoStr, &err);
	printf("Profiling event 'fill input texture'\n%s", infoStr);

//	size_t infos;
//	err = clGetImageInfo(inputTexture, CL_IMAGE_ELEMENT_SIZE, sizeof(infos), &infos, NULL);
//	printf("elem. size: %d\n", infos);
//	err |= clGetImageInfo(inputTexture, CL_IMAGE_ROW_PITCH, sizeof(infos), &infos, NULL);
//	printf("row size: %d\n", infos);
//	err |= clGetImageInfo(inputTexture, CL_IMAGE_SLICE_PITCH, sizeof(infos), &infos, NULL);
//	printf("slice size: %d\n", infos);
//	clhErrorInfo(err, "get texture info", __FILE__);


	// Input and results array
	size_t voxelsCount = valuesSizes.s[0] * valuesSizes.s[1] * valuesSizes.s[2];

	size_t inputTextureSize = sizeof(cl_float) *
			((valuesSizes.s[0] + 1) * (valuesSizes.s[1] + 1) * (valuesSizes.s[2] + zBufferSize + 1));
//	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
//			CL_MEM_COPY_HOST_PTR, inputBufferSize, dataSet, &err);
	// profiling proposes
//	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
//			inputBufferSize, NULL, &err);
//	err |= clEnqueueWriteBuffer(resources->cmdQueues[0], inputBuffer,
//			CL_TRUE, 0, inputBufferSize, dataSet, 0, NULL, NULL);
//	clhErrorInfo(err, "create and fill input buffer", "mcCore");

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

	size_t classificationBufferSize = sizeof(cl_float) * voxelsCount;
	cl_mem verticesBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			classificationBufferSize, NULL, &err);
	clhErrorInfo(err, "create vertices buffer", "mcCore");
	cl_mem occupiedBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			classificationBufferSize, NULL, &err);
	clhErrorInfo(err, "create occupied buffer", "mcCore");



	// Get all of the stuff written and allocated
	clFinish(cmdQueue);

	// Now setup the arguments to our kernel
	arg = 0;
	err  = clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_mem), &inputTexture);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_sampler), &inputTextureSampler);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_float4), &valuesBuffers);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_float), &isoValue);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_mem), &verticesTableBuffer);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_mem), &verticesBuffer);
	err |= clSetKernelArg(classificationKernel, arg++,
			sizeof(cl_mem), &occupiedBuffer);
	clhErrorInfo(err, "set classification kernel arguments", "mcCore");

	// Run the calculation by enqueuing it and forcing the
	// command queue to complete the task
	printf("Launching classification kernel configuration: %u * %u * %u = %zu\n",
			valuesSizes.s[0], valuesSizes.s[1], valuesSizes.s[2], voxelsCount);
	size_t classificationGWS[3] = {valuesSizes.s[0], valuesSizes.s[1], valuesSizes.s[2]};

	err = clEnqueueNDRangeKernel(cmdQueue, classificationKernel, 3, NULL,
			classificationGWS, NULL, 0, NULL, &event);
	clhErrorInfo(err, "enqueue kernel range", "mcCore");

	printf("wait for classification kernel to finish\n");
	err = clWaitForEvents(1, &event);
	clhErrorInfo(err, "wait for classification kernel", "mcCore");

	// Get all of the stuff written and allocated
	clFinish(cmdQueue);
	
	clhGetEventProfilingInfo(event, sizeof(infoStr), infoStr, &err);
	printf("Profiling event 'classification kernel'\n%s", infoStr);

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

	float occupiedVoxelsCount = 0;
	cl_mem scannedOccupiedBuffer = clsScanFromDevice(resources, deviceIndex,
			occupiedBuffer, voxelsCount, &occupiedVoxelsCount, &err);

	//### DEBUG ###
//	float* scan = malloc(classificationSize);
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], scannedOccupiedBuffer, CL_TRUE, 0, classificationSize, scan, 0, NULL, NULL);
//	clhErrorInfo(err, "read scan buffer", "mcCore");
//	for(int i = 0; i < 20; i++)
//		printf("%.0f,", scan[i]);
//	printf("\n");

	size_t compactionBufferSize = (size_t)occupiedVoxelsCount * 4;//sizeof(size_t);

	if(compactionBufferSize > 0) {

		cl_mem compactedInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				compactionBufferSize, NULL, &err);
		clhErrorInfo(err, "create compacted input buffer", "mcCore");

		// Get all of the stuff written and allocated
		clFinish(cmdQueue);

		// Now setup the arguments to our kernel
		arg = 0;
		err  = clSetKernelArg(compactionKernel, arg++,
				sizeof(cl_mem), &occupiedBuffer);
		err |= clSetKernelArg(compactionKernel, arg++,
				sizeof(cl_mem), &scannedOccupiedBuffer);
		err |= clSetKernelArg(compactionKernel, arg++,
				sizeof(cl_mem), &compactedInputBuffer);

		printf("Launching compaction kernel configuration: %zu\n", voxelsCount);

		size_t compactionGWS[1] = {voxelsCount};

		err = clEnqueueNDRangeKernel(cmdQueue, compactionKernel, 1, NULL,
				compactionGWS, NULL, 0, NULL, &event);
		clhErrorInfo(err, "enqueue compaction kernel range", "mcCore");

		printf("wait for compaction kernel to finish\n");
		err = clWaitForEvents(1, &event);
		clhErrorInfo(err, "wait for compaction kernel", "mcCore");

		clReleaseMemObject(occupiedBuffer);
		clReleaseMemObject(scannedOccupiedBuffer);

		// Get all of the stuff written and allocated
		clFinish(cmdQueue);
		
		clhGetEventProfilingInfo(event, sizeof(infoStr), infoStr, &err);
		printf("Profiling event 'compaction kernel'\n%s", infoStr);

		float vertexCount = 0;
		cl_mem scannedVerticesBuffer = clsScanFromDevice(resources, deviceIndex,
				verticesBuffer, voxelsCount, &vertexCount, &err);

//		size_t trianglesCount = verticeCount / 3;

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

		//TODO: back to vbo buffer
		size_t trianglesBufferSize = vertexCount * sizeof(cl_float4);
//		cl_mem trianglesBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
//				trianglesBufferSize, NULL, &err);
		GLuint trianglesVBORet;
		cl_mem trianglesBuffer = clhCreateGLCLBuffer(context, trianglesBufferSize, &trianglesVBORet, &err);
		clhErrorInfo(err, "create triangles buffer", "mcCore");

		//TODO: back to vbo buffer
		size_t normalsBufferSize = vertexCount * sizeof(cl_float4);
//		cl_mem normalsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
//				normalsBufferSize, NULL, &err);
		GLuint normalsVBORet;
		cl_mem normalsBuffer = clhCreateGLCLBuffer(context, normalsBufferSize, &normalsVBORet, &err);
		clhErrorInfo(err, "create normals buffer", "mcCore");


		clFinish(cmdQueue);

		cl_uint2 size = {{valuesSizes.s[0], valuesSizes.s[1]}};
		cl_uint voxelCountUInt = occupiedVoxelsCount;

		// Now setup the arguments to our kernel
		arg = 0;
		err  = clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &inputTexture);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_sampler), &inputTextureSampler);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_uint), &voxelCountUInt);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_uint2), &size);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_float), &isoValue);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_float4), &valuesDistances);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_float4), &valuesOffsets);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_float4), &valuesBuffers);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &trianglesTableBuffer);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &verticesTableBuffer);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &scannedVerticesBuffer);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &compactedInputBuffer);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &trianglesBuffer);
		err |= clSetKernelArg(generationKernel, arg++,
				sizeof(cl_mem), &normalsBuffer);
		clhErrorInfo(err, "set generation kernel arguments", "mcCore");

		//TODO: add back acquire GL objects
	    err = clEnqueueAcquireGLObjects(cmdQueue, 1, &trianglesBuffer, 0, NULL, NULL);
	    err |= clEnqueueAcquireGLObjects(cmdQueue, 1, &normalsBuffer, 0, NULL, NULL);
	    clhErrorInfo(err, "aquire gl objects", "mcCore");


	    size_t globalWorkSize = ceil(occupiedVoxelsCount / generationMaxWGS)
	    		* generationMaxWGS;

	    // Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		printf("Launching generation kernel configuration: %zu (%zu)\n",
				(size_t)occupiedVoxelsCount, globalWorkSize);
		size_t generationGWS[1] = {globalWorkSize};
		size_t generationLWS[1] = {generationMaxWGS};

		err = clEnqueueNDRangeKernel(cmdQueue, generationKernel, 1, NULL,
				generationGWS, generationLWS, 0, NULL, &event);
		clhErrorInfo(err, "enqueue generation kernel range", "mcCore");

		printf("wait for generationKernel to finish\n");
		err = clWaitForEvents(1, &event);
		clhErrorInfo(err, "wait for generation kernel", "mcCore");

		//TODO: add back release GL objects
	    err = clEnqueueReleaseGLObjects(cmdQueue, 1, &trianglesBuffer, 0, NULL, NULL);
	    err |= clEnqueueReleaseGLObjects(cmdQueue, 1, &normalsBuffer, 0, NULL, NULL);
	    clhErrorInfo(err, "release gl objects", "mcCore");

		clhGetEventProfilingInfo(event, sizeof(infoStr), infoStr, &err);
		printf("Profiling event 'generation kernel'\n%s", infoStr);
		
		// Once finished read back the results from the answer
		// array into the results array
		//### DEBUG ###
//		cl_float4* trianglesRet = malloc(trianglesBufferSize);
//		err = clEnqueueReadBuffer(resources->cmdQueues[0], trianglesBuffer,
//				CL_TRUE, 0, trianglesBufferSize, trianglesRet, 0, NULL, NULL);
//		clhErrorInfo(err, "read triangles buffer", "mcCore");
//		for(int i = 0; i < 20; i++)
//			printf("(%.2f;%.2f;%.2f;%.0f)", trianglesRet[i].s[0], trianglesRet[i].s[1], trianglesRet[i].s[2], trianglesRet[i].s[3]);
//		printf("\n");
//		cl_float4* normalsRet = malloc(normalsBufferSize);
//		err = clEnqueueReadBuffer(resources->cmdQueues[0], normalsBuffer,
//				CL_TRUE, 0, normalsBufferSize, normalsRet, 0, NULL, NULL);
//		clhErrorInfo(err, "read normals buffer", "mcCore");
//		for(int i = 0; i < 20; i++)
//			printf("(%.2f;%.2f;%.2f;%.0f)", normalsRet[i].s[0], normalsRet[i].s[1], normalsRet[i].s[2], normalsRet[i].s[3]);
//		printf("\n");


		clFinish(cmdQueue);


		clReleaseMemObject(scannedVerticesBuffer);
		clReleaseMemObject(compactedInputBuffer);
//		clReleaseMemObject(trianglesBuffer);
//		clReleaseMemObject(normalsBuffer);

//		if(triangles) *triangles = trianglesRet;
//		if(normals) *normals = normalsRet;
//		if(triangles) *triangles = NULL;
//		if(normals) *normals = NULL;
		if(trianglesVBO) *trianglesVBO = trianglesVBORet;
		if(normalsVBO) *normalsVBO = normalsVBORet;
		if(outSize) *outSize = vertexCount;

		printf( "Memory allocation\n"
				" input buffer: %zuKB\n"
				" classification buffers: 2x %zuKB\n"
				" compacted input buffer: %zuKB (%zu%%)\n"
				" output buffer %zuKB + %zuKB\n",
				inputTextureSize / KB,
				classificationBufferSize / KB,
				compactionBufferSize / KB, (compactionBufferSize * 100) / inputTextureSize,
				trianglesBufferSize / KB, normalsBufferSize / KB);

	} else {

//		if(triangles) *triangles = NULL;
//		if(normals) *normals = NULL;
		if(trianglesVBO) trianglesVBO = NULL;
		if(normalsVBO) normalsVBO = NULL;
		if(outSize) *outSize = 0;

		printf(	"Memory allocation\n"
				" input buffer: %zuKB\n"
				" classification buffers: 2x %zuKB\n"
				" compacted input buffer: %zuKB (%zu%%)\n",
				inputTextureSize / KB,
				classificationBufferSize / KB,
				compactionBufferSize / KB, (compactionBufferSize * 100) / inputTextureSize);

	}

	clReleaseMemObject(inputTexture);

	return CL_SUCCESS;
}

void mccReleaseCL() {

	initialized = FALSE;

	clsRelease();

	clReleaseSampler(inputTextureSampler);
	clReleaseMemObject(trianglesTableBuffer);
	clReleaseMemObject(verticesTableBuffer);
	clReleaseKernel(classificationKernel);
	clReleaseKernel(compactionKernel);
	clReleaseKernel(generationKernel);
	clReleaseProgram(program);


	printf("%s resources released!\n", __FILE__);

}

////////////////////////////////////////////////////////////////////////////////

#define VOXEL_VERTICES 16 // vertices per voxel
#define VOXEL_TRIANGLES 5 // triangles per voxel
#define TRIANGLE_VERTICES 3 // vertices per triangle

//struct float4{
//	float x;
//	float y;
//	float z;
//	float w;
//};
//
//typedef struct float4 float4;

inline static float getValue(cl_float* values, const size_t sX, const size_t sY,
						 const int x, const int y, const int z) {

	return values[x + y * sX + z * sX * sY];
}

inline static cl_float4 vertexInterpolation(float iso, cl_float4 v1, cl_float4 v2) {

	float r = (iso - v1.s[3]) / (v2.s[3] - v1.s[3]);

	cl_float4 iv;
	iv.s[0] = v1.s[0] + (v2.s[0] - v1.s[0]) * r;
	iv.s[1] = v1.s[1] + (v2.s[1] - v1.s[1]) * r;
	iv.s[2] = v1.s[2] + (v2.s[2] - v1.s[2]) * r;
//	iv.s[3] = 0.0f;

	return iv;

}

inline static cl_float4 triangleNormal(cl_float4* t) {

	cl_float4 e1, e2;

	e1.s[0] = t[1].s[0] - t[0].s[0];
	e1.s[1] = t[1].s[1] - t[0].s[1];
	e1.s[2] = t[1].s[2] - t[0].s[2];
//	e1.s[3] = 0.0f;

	e2.s[0] = t[2].s[0] - t[0].s[0];
	e2.s[1] = t[2].s[1] - t[0].s[1];
	e2.s[2] = t[2].s[2] - t[0].s[2];
//	e2.s[3] = 0.0f;

	return e1;

}

int mccHost(cl_float* dataSet, cl_float isoValue,
		   size_t inSizeX, size_t inSizeY, size_t inSizeZ,
		   cl_float4 valuesDistance, cl_float4 valuesOffset,
		   cl_float4** triangles, cl_float4** normals, size_t* outSize) {

	cl_float* values = dataSet;
	cl_float4* trianglesRet = calloc(inSizeX * inSizeY * inSizeZ, sizeof(cl_float4));

	size_t verticesCount = 0;
	size_t sX = inSizeX, sY = inSizeY, siX = sX + 1, siY = sY + 1;

//	printf("1");
//	fflush(NULL);

	for(int z = 0; z < inSizeZ; z++) {
		for(int y = 0; y < inSizeY; y++) {
			for(int x = 0; x < inSizeX; x++) {

				int xi = x + 1, yi = y + 1, zi = z + 1;

//				printf("\n2 ");
//				printf("%d-%d %d-%d %d-%d", x, inSizeX, y, inSizeY, z, inSizeZ);
//				fflush(NULL);

				float cornersValue[8];
				cornersValue[0] = getValue(values, siX, siY,	x,	y,	z	);
				cornersValue[1] = getValue(values, siX, siY,	xi,	y,	z	);
				cornersValue[2] = getValue(values, siX, siY,	xi,	yi,	z	);
				cornersValue[3] = getValue(values, siX, siY,	x,	yi,	z	);
				cornersValue[4] = getValue(values, siX, siY,	x,	y,	zi	);
				cornersValue[5] = getValue(values, siX, siY,	xi,	y,	zi	);
				cornersValue[6] = getValue(values, siX, siY,	xi,	yi,	zi	);
				cornersValue[7] = getValue(values, siX, siY,	x,	yi,	zi	);

//				for(int i = 0; i < 8; i++)
//					printf(" %.1f", cornersValue[i]);
//				printf("\n3 ");
//				fflush(NULL);

				int combination = 0;
				combination = (cornersValue[0] < isoValue);
				combination += (cornersValue[1] < isoValue) * 2;
				combination += (cornersValue[2] < isoValue) * 4;
				combination += (cornersValue[3] < isoValue) * 8;
				combination += (cornersValue[4] < isoValue) * 16;
				combination += (cornersValue[5] < isoValue) * 32;
				combination += (cornersValue[6] < isoValue) * 64;
				combination += (cornersValue[7] < isoValue) * 128;

//				printf("%d", combination);

				if(combination > 0 && combination < 255) {

//					printf("\n4 ");

					float xf = x + valuesOffset.s[0];
					float yf = y + valuesOffset.s[1];
					float zf = z + valuesOffset.s[2];
					float xfi = xf + valuesDistance.s[0];
					float yfi = yf + valuesDistance.s[1];
					float zfi = zf + valuesDistance.s[2];

//					printf("%.1f %.1f %.1f", xf, yf, zf);

					cl_float4 corners[8];
					corners[0] = (cl_float4){{xf,	yf,		zf,		cornersValue[0]}};
					corners[1] = (cl_float4){{xfi,	yf,		zf,		cornersValue[1]}};
					corners[2] = (cl_float4){{xfi,	yfi,	zf,		cornersValue[2]}};
					corners[3] = (cl_float4){{xf,	yfi,	zf,		cornersValue[3]}};
					corners[4] = (cl_float4){{xf,	yf,		zfi,	cornersValue[4]}};
					corners[5] = (cl_float4){{xfi,	yf,		zfi,	cornersValue[5]}};
					corners[6] = (cl_float4){{xfi,	yfi,	zfi,	cornersValue[6]}};
					corners[7] = (cl_float4){{xf,	yfi,	zfi,	cornersValue[7]}};

					cl_float4 vertices[12];
					vertices[0] = vertexInterpolation(isoValue, corners[0], corners[1]);
					vertices[1] = vertexInterpolation(isoValue, corners[1], corners[2]);
					vertices[2] = vertexInterpolation(isoValue, corners[2], corners[3]);
					vertices[3] = vertexInterpolation(isoValue, corners[3], corners[0]);

					vertices[4] = vertexInterpolation(isoValue, corners[4], corners[5]);
					vertices[5] = vertexInterpolation(isoValue, corners[5], corners[6]);
					vertices[6] = vertexInterpolation(isoValue, corners[6], corners[7]);
					vertices[7] = vertexInterpolation(isoValue, corners[7], corners[4]);

					vertices[8] = vertexInterpolation(isoValue, corners[4], corners[0]);
					vertices[9] = vertexInterpolation(isoValue, corners[1], corners[5]);
					vertices[10] = vertexInterpolation(isoValue, corners[2], corners[6]);
					vertices[11] = vertexInterpolation(isoValue, corners[3], corners[7]);

//					printf("\n5 ");
//					for(int i = 0; i < 12; i++)
//						printf("%.1f %.1f %.1f ", vertices[i].s[0], vertices[i].s[1], vertices[i].s[2]);
//					fflush(NULL);

					unsigned int voxelVertices = verticesTable[combination];
					unsigned char* edges = trianglesTable[combination]; // all edge values should be in private (or at least local) memory

//					size_t basePosition = count;

//					printf("\n6 ");
//					printf("%d %d\n", voxelVertices, count);
//					fflush(NULL);
//					for(int i = 0; i < 15; i++) {
//						printf(" %d", edges[i]);
//					}
//					fflush(NULL);
//
//					printf("\n7 ");
//					fflush(NULL);

					for(size_t v = 0; v < voxelVertices; v += TRIANGLE_VERTICES) {

//						size_t vertexPosition = basePosition + v;
						size_t vertexPosition = verticesCount + v;

						trianglesRet[vertexPosition] = vertices[edges[v]];
						trianglesRet[vertexPosition + 1] = vertices[edges[v + 1]];
						trianglesRet[vertexPosition + 2] = vertices[edges[v + 2]];

//						for(int i = 0; i < 3; i++)
//							printf("%.1f %.1f %.1f  ", vertices[edges[v + i]].s[0], vertices[edges[v + i]].s[1], vertices[edges[v + i]].s[2]);
//						fflush(NULL);

					}

					verticesCount += voxelVertices;

				}

			}

//			exit(0);

		}

	printf("%d\n", z);

	}

//	for(int i = 0; i < 100; i++)
//		printf("%.1f %.1f %.1f\n", outputRet[i].s[0], outputRet[i].s[1], outputRet[i].s[2]);
//	fflush(NULL);

	if(triangles) *triangles = trianglesRet;
//	if(normals) *normals = normalsRet;
	if(outSize) *outSize = verticesCount;


	return CL_SUCCESS;

}

//void test() {
//
//	clhSetVerbose(FALSE);
//	clsSetVerbose(FALSE);
//
//	cl_int err;
//	cl_event event;
//
//	clhResources resources = clhInitResources("", CL_DEVICE_TYPE_CPU, 0, &err);
//
//	cl_program program = clhBuildProgramFromFile(KERNELS_SOURCE_FILE, resources, &err);
//
//	float values[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};
//
//	cl_mem input = clCreateBuffer(resources->context, CL_MEM_COPY_HOST_PTR |
//			CL_MEM_READ_ONLY, sizeof(values), values, &err);
//	clhErrorInfo(err, "array", "mcCore");
//
//	cl_float sum;
//
//	cl_mem scanned = clsScanFromDevice(resources, 0, input, 10, &sum, &err);
//	clhErrorInfo(err, "scan bin", "mcCore");
//
//	printf("sum: %d\n", (int)sum);
//	int result[(int)sum];
//
//	cl_mem output = clCreateBuffer(resources->context, CL_MEM_READ_WRITE, sizeof(cl_uint) * sum, NULL, &err);
//	clhErrorInfo(err, "array", "mcCore");
//
//	cl_kernel kernel = clCreateKernel(program, kernelNames[MC_COMPACTION], &err);
//	clhErrorInfo(err, "create kernel", "mcCore");
//
//	// Get all of the stuff written and allocated
//	clFinish(resources->cmdQueues[0]);
//
//	// Now setup the arguments to our kernel
//	err  = clSetKernelArg(kernel,  0, sizeof(cl_mem), &input);
//	err |= clSetKernelArg(kernel,  1, sizeof(cl_mem), &scanned);
//	err |= clSetKernelArg(kernel,  2, sizeof(cl_mem), &output);
//	clhErrorInfo(err, "set kernel arguments", "mcCore");
//
//	// Run the calculation by enqueuing it and forcing the
//	// command queue to complete the task
//	size_t globalWorkSize = 10;
//
//	err = clEnqueueNDRangeKernel(resources->cmdQueues[0], kernel, 1, NULL, &globalWorkSize,
//			NULL, 0, NULL, &event);
//	clhErrorInfo(err, "enqueue kernel range", "mcCore");
//
//	err = clWaitForEvents(1, &event);
//	clhErrorInfo(err, "wait for kernel", "mcCore");
//
//	err = clEnqueueReadBuffer(resources->cmdQueues[0], output, CL_TRUE, 0, sizeof(cl_uint) * sum,
//			result, 0, NULL, NULL);
//
//	clFinish(resources->cmdQueues[0]);
//
//	for(int i = 0; i < (int)sum; i++)
//		printf("%d ", result[i]);
//
//	clReleaseMemObject(input);
//	clReleaseMemObject(output);
//	clReleaseCommandQueue(resources->cmdQueues[0]);
//	clReleaseContext(resources->context);
//
//}
