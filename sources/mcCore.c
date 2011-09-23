/*
 * mcCore.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: > Use async calls with events.
 * 		  > When possible reuse resources like buffers, instead of creating a
 * 		  new one each run; most resources aren't being reused, the reused ones
 * 		  most be initialized first and then reuse them until no longer needed,
 *		  at that time they most be released.
 */

#include "mcCore.h"

//#include <stdio.h>
#include <math.h>
#if USE_GL
#include <CL/cl_gl.h>
#endif /* USE_GL */

#include "utilities.h"
#include "clHelper.h"
#include "mcTables.h"
#include "clScan.h"

////////////////////////////////////////////////////////////////////////////////

// logging macros
#define _clhLogErr_(m) clhLogErr_(clErr, m, deviceIndex)
#define _clhLogErr__(m) clhLogErr__(clErr, m)

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

#define KERNELS_FILE "mc.cl"		
//static const char* KERNELS_FILE = "mc.cl";

////////////////////////////////////////////////////////////////////////////////

// print something

static int MCC_VERBOSE = DISABLE;

int mccGetVerbose() {
	return MCC_VERBOSE;
}

void mccSetVerbose(const int state) {
	MCC_VERBOSE = state;
}

static int MCC_PROFILING = DISABLE;

int mccGetProfiling() {
	return MCC_PROFILING;
}

void mccSetProfiling(const int state) {
	MCC_PROFILING = state;
}

////////////////////////////////////////////////////////////////////////////////

int mccInitializeCL(const clhResources initializedResources);
int mccShutdownCL();

int mccMarchCL(const cl_uint deviceIndex,
			   const cl_float* values,
			   const cl_float isoValue,
			   const cl_uint4 valuesSizes,
			   const cl_float4 valuesDistances,
			   const cl_float4 valuesOffsets,
			   const cl_int2 valuesZBuffers,
			   size_t* outSizeRet,
#if USE_GL
			   GLuint* trianglesVBORet,
			   GLuint* normalsVBORet,
#endif /* USE_GL */
			   cl_mem* trianglesBufferRet,
			   cl_mem* normalsBufferRet);

////////////////////////////////////////////////////////////////////////////////

static int clInitialized = FALSE;

static clhResources clResources;

static cl_program program;
static cl_kernel classificationKernel;
static cl_kernel compactionKernel;
static cl_kernel generationKernel;
static size_t* classificationMaxWGSs;
static size_t* compactionMaxWGSs;
static size_t* generationMaxWGSs;

static cl_mem trianglesTableBuffer;
static cl_mem verticesTableBuffer;
static cl_image_format dataSetFormat;
static cl_sampler inputTextureSampler;

////////////////////////////////////////////////////////////////////////////////

int mccInitializeCL(const clhResources initializedResources) {

	
//	printf("%s %sinitialized.\n", __FILE__, initializedResources ? "" : "auto ");

//	if(initializedResources)
//		clResources = initializedResources;
//	else {
//		clResources = clhInitResources(NULL, 0, 0, &retErr);
//		if(retErr) return retErr;
//	}

	if(clInitialized) {
		logMsg_("ERROR: illegal mccInitializeCL() reinitialization attempt");
		return ERROR;
	}

	if(initializedResources)
		clResources = initializedResources;
	else {
		logMsg_("ERROR: invalid resources");
		return ERROR;
	}
		
	cl_int clErr;
	cl_context context = clResources->context;
	uint devCount = clResources->devCount;
	cl_device_id* devices = clResources->devices;
	
	program = clhBuildProgramFromFile_(KERNELS_FILE, clResources, TRUE, TRUE, &clErr);
	if(clErr) return ERROR;
	
	// Now create the kernel "objects" that we want to use
	//	cl_kernel kernels[3];
	//
	//	for(int i = 0; i < kernelCount; i++)
	classificationKernel =
			clCreateKernel(program, kernelNames[MC_CLASSIFICATION], &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create kernel classification");

//	classificationMaxWGS = clhGetKernelMaxWorkGroupSize(classificationKernel, devices[0], &retErr);
//	printf("max classification wgs: %zu\n", classificationMaxWGS);

	compactionKernel =
			clCreateKernel(program, kernelNames[MC_COMPACTION], &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create kernel compaction");

//	compactionMaxWGS = clhGetKernelMaxWorkGroupSize(compactionKernel, devices[0], &retErr);
//	printf("max compation wgs: %zu\n", compactionMaxWGS);

	generationKernel =
			clCreateKernel(program, kernelNames[MC_GENERATION], &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create kernel generation");

//	generationMaxWGS = clhGetKernelMaxWorkGroupSize(generationKernel, devices[0], &retErr);
//	printf("max generation wgs: %zu\n", generationMaxWGS);

	// info about kernel max work group size
	classificationMaxWGSs = calloc(devCount, sizeof(size_t));
	compactionMaxWGSs = calloc(devCount, sizeof(size_t));
	generationMaxWGSs = calloc(devCount, sizeof(size_t));
	
	for(int i = 0; i < devCount; i++) {
		classificationMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(
				classificationKernel, devices[i], &clErr);
		if(MCC_VERBOSE)
			printf("dev%d max classification wgs: %zu\n", i, classificationMaxWGSs[i]);
		
		compactionMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(compactionKernel,
				devices[i], &clErr);
		if(MCC_VERBOSE)
			printf("dev%d max compation wgs: %zu\n", i, compactionMaxWGSs[i]);
		
		generationMaxWGSs[i] = clhGetKernelMaxWorkGroupSize(generationKernel,
				devices[i], &clErr);
		if(MCC_VERBOSE)
			printf("dev%d max generation wgs: %zu\n", i, generationMaxWGSs[i]);
	}

	size_t trianglesTableSize, verticesTableSize;
	// Allocate memory on device to hold lookup tables
	trianglesTableSize = sizeof(trianglesTable);
	trianglesTableBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
			CL_MEM_COPY_HOST_PTR, trianglesTableSize, trianglesTable, &clErr);
	// profiling proposes
//	trianglesTableBuffer =
//			clCreateBuffer(clResources->context, CL_MEM_READ_ONLY,
//					trianglesTableSize, NULL, &retErr);
//	retErr |= clEnqueueWriteBuffer(clResources->cmdQueues[0], trianglesTableBuffer,
//			CL_TRUE, 0, trianglesTableSize, trianglesTable, 0, NULL, NULL);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create triangles lookup table");

	verticesTableSize = sizeof(verticesTable);
	verticesTableBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
			CL_MEM_COPY_HOST_PTR, verticesTableSize, verticesTable, &clErr);
	// profiling proposes
//	verticesTableBuffer =
//			clCreateBuffer(clResources->context, CL_MEM_READ_ONLY,
//					verticesTableSize, NULL, &retErr);
//	retErr |= clEnqueueWriteBuffer(clResources->cmdQueues[0], verticesTableBuffer,
//			CL_TRUE, 0, verticesTableSize, verticesTable, 0, NULL, NULL);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create vertices lookup table");

	//### DEBUG ###
//	unsigned char* vTable = malloc(verticesTableSize);
//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], verticesTableBuffer, CL_TRUE, 0, verticesTableSize, vTable, 0, NULL, NULL);
//	_clhLogErr_("read vTable buffer", "mcCore");
//	for(int i = 0; i < verticesTableSize; i++)
//		printf("%d,", vTable[i]);
//	printf("\n");

	// create texture format and sampler
	dataSetFormat.image_channel_order = CL_R;
	dataSetFormat.image_channel_data_type = CL_FLOAT;

	inputTextureSampler = clCreateSampler(context, CL_FALSE,
			CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("create sampler");

	// Now create the kernel "objects" that we want to use
	//	cl_kernel kernel;
	//	char* kernelName = "generation";
	//
	//	printf("Creating kernel '%s' from file '%s'\n", kernelName, KERNELS_SOURCE_FILE);
	//	kernel = clCreateKernel(program, kernelName, &clErr);
	//	_clhLogErr_("create kernel", "mcCore");

	clErr = clsInit(clResources);
	if(clErr) return ERROR;

	clInitialized = TRUE;

	if(MCC_VERBOSE) {
		logMsg_("SUCCESS: initialized!");
		printf( "Init memory allocation\n"
				" triangles table buffer: %zuB\n"
				" vertices table buffer: %zuB\n",
				trianglesTableSize,
				verticesTableSize);
	}
					
	return SUCCESS;
}

int mccShutdownCL() {

	if(!clInitialized) {
		logMsg_("ERROR: not initialized, call mccInitialize(...) before");
		return ERROR;
	}

	cl_int clErr;

	clInitialized = FALSE;

	clsFree();

	clErr = clhReleaseResources(clResources);
	clErr |= clReleaseSampler(inputTextureSampler);
	clErr |= clReleaseMemObject(trianglesTableBuffer);
	clErr |= clReleaseMemObject(verticesTableBuffer);
	clErr |= clReleaseKernel(classificationKernel);
	clErr |= clReleaseKernel(compactionKernel);
	clErr |= clReleaseKernel(generationKernel);
	clErr |= clReleaseProgram(program);
	if(clErr || MCC_VERBOSE)
		_clhLogErr__("releasing resources");
		
	free(classificationMaxWGSs);
	free(compactionMaxWGSs);
	free(generationMaxWGSs);

	if(MCC_VERBOSE) logMsg_("SUCCESS: resources released!");
	
	return SUCCESS;
}

/*
 * run marching cubes algorithm with OpenCL
 *
 * Notes: ? - add option to keep output result in memory device
 *		  ? - add mutex to serialize concurrent access
 *			- review sync points
 */
int mccMarchCL(const cl_uint deviceIndex,
			   const cl_float* values,
			   const cl_float isoValue,
			   const cl_uint4 valuesSizes,
			   const cl_float4 valuesDistances,
			   const cl_float4 valuesOffsets,
			   const cl_int2 valuesZBuffers,
			   size_t* outSizeRet,
#if USE_GL
			   GLuint* trianglesVBORet,
			   GLuint* normalsVBORet,
#endif /* USE_GL */
			   cl_mem* trianglesBufferRet,
			   cl_mem* normalsBufferRet) {

	if(!clInitialized) {
		logMsg_("ERROR: not initialized, call mccInitialize(...) before");
		return ERROR;
	}
	
	size_t clArg;
	cl_int clErr;
	cl_event clEvent;
	char infoStr[SMALL_STRING_SIZE];
	
	cl_context context = clResources->context;
	cl_command_queue cmdQueue = clResources->cmdQueues[deviceIndex];
	size_t generationMaxWGS = generationMaxWGSs[deviceIndex];

//	cl_device_id dev;
//	clGetCommandQueueInfo(cmdQueue, CL_QUEUE_DEVICE,
//			sizeof(cl_device_id), &dev, NULL);
//	printf("device:%p\n", infoStr);

	// Allocate memory on the device to hold our data and store the results into

	// create texture from dataset and get info

	cl_uint4 voxelsSizes = {{valuesSizes.s[X] - 1,
							 valuesSizes.s[Y] - 1,
							 valuesSizes.s[Z] - 1,
							 0.0f}};

	size_t zBufferSize = valuesZBuffers.s[TOP] + valuesZBuffers.s[BOTTOM];
	cl_float4 valuesBuffersOffset = {{0.0f, 0.0f, valuesZBuffers.s[TOP], 0.0f}};

	//TODO: correct texture region on corner cases (done!?)
	size_t textureOrigin[3] = {0, 0, 0};
	size_t textureRegion[3] = {valuesSizes.s[X],
							   valuesSizes.s[Y],
							   valuesSizes.s[Z] + zBufferSize};

	if(MCC_PROFILING)
		printf("INFO: [%u] textureRegion[Z]= %zu (buffers[Z]= %d + %d)\n",
				deviceIndex, textureRegion[Z], valuesZBuffers.s[TOP],
				valuesZBuffers.s[BOTTOM]);
						
	cl_mem inputTexture = clCreateImage3D(context, CL_MEM_READ_ONLY,
			&dataSetFormat, textureRegion[X], textureRegion[Y],
			textureRegion[Z], 0, 0, NULL, &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("create input texture");
//	fflush(NULL);

//	printf("%d %zu %zu %zu %p %p %p\n", clErr, textureRegion[0], textureRegion[1], textureRegion[2], values, inputTexture, clResources->cmdQueues[0]);
//	fflush(NULL);

	clErr = clEnqueueWriteImage(cmdQueue, inputTexture, CL_TRUE, textureOrigin,
			textureRegion, 0, 0, values, 0, NULL, &clEvent);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("fill input texture");
//	fflush(NULL);
	
//	clFinish(cmdQueue);
	
	if(MCC_PROFILING) {
		clhGetEventProfilingInfo(clEvent, sizeof(infoStr), infoStr, &clErr);
		printf("PROFILING: [%u] 'fill input texture'\n%s", deviceIndex, infoStr);
	}

//	size_t infos;
//	clErr = clGetImageInfo(inputTexture, CL_IMAGE_ELEMENT_SIZE, sizeof(infos), &infos, NULL);
//	printf("elem. size: %d\n", infos);
//	clErr |= clGetImageInfo(inputTexture, CL_IMAGE_ROW_PITCH, sizeof(infos), &infos, NULL);
//	printf("row size: %d\n", infos);
//	clErr |= clGetImageInfo(inputTexture, CL_IMAGE_SLICE_PITCH, sizeof(infos), &infos, NULL);
//	printf("slice size: %d\n", infos);
//	_clhLogErr_("get texture info");


	// Input and results array
	size_t voxelsCount = voxelsSizes.s[X] * voxelsSizes.s[Y] * voxelsSizes.s[Z];

	size_t inputTextureSize = sizeof(cl_float) *
			textureRegion[X] * textureRegion[Y] * textureRegion[Z];
	
	
//	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
//			CL_MEM_COPY_HOST_PTR, inputBufferSize, dataSet, &clErr);
	// profiling proposes
//	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
//			inputBufferSize, NULL, &clErr);
//	clErr |= clEnqueueWriteBuffer(clResources->cmdQueues[0], inputBuffer,
//			CL_TRUE, 0, inputBufferSize, dataSet, 0, NULL, NULL);
//	_clhLogErr_("create and fill input buffer", "mcCore");

//	### DEBUG ###
//	float* in = malloc(inputSize);
//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], inputBuffer, CL_TRUE, 0, inputSize, in, 0, NULL, NULL);
//	_clhLogErr_("read input buffer", "mcCore");
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

//	inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, inputSize, NULL, &clErr);
//	mclErrorInfo(clErr, "create input buffer", "mcCore");
//
//	clErr = clEnqueueWriteBuffer(cmd_queue, inputBuffer, CL_TRUE, 0, inputSize,
//			(void*)input, 0, NULL, &clEvent);
//	mclErrorInfo(clErr, "fill input buffer", "mcCore");

	size_t classificationBufferSize = sizeof(cl_float) * voxelsCount;
	cl_mem verticesBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			classificationBufferSize, NULL, &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("create vertices buffer");
	cl_mem occupiedBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			classificationBufferSize, NULL, &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("create occupied buffer");



	// Get all of the stuff written and allocated
	clEnqueueBarrier(cmdQueue);

	// Now setup the arguments to our kernel
	clArg = 0;
	clErr  = clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_mem), &inputTexture);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_sampler), &inputTextureSampler);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_float4), &valuesBuffersOffset);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_float), &isoValue);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_mem), &verticesTableBuffer);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_mem), &verticesBuffer);
	clErr |= clSetKernelArg(classificationKernel, clArg++,
			sizeof(cl_mem), &occupiedBuffer);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("set classification kernel arguments");

	// Run the calculation by enqueuing it and forcing the
	// command queue to complete the task
	if(MCC_PROFILING)
		printf("INFO: [%u] '%s' kernel configuration: %u * %u * %u = %zu\n",
				deviceIndex, kernelNames[MC_CLASSIFICATION], voxelsSizes.s[X],
				voxelsSizes.s[Y], voxelsSizes.s[Z], voxelsCount);
	
	size_t classificationGWS[3] = 
			{voxelsSizes.s[X], voxelsSizes.s[Y], voxelsSizes.s[Z]};

	clErr = clEnqueueNDRangeKernel(cmdQueue, classificationKernel, 3, NULL,
			classificationGWS, NULL, 0, NULL, &clEvent);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("enqueue kernel range");
	
	
	
	
/*	cl_mem inputTexture2 = clCreateImage3D(context, CL_MEM_READ_ONLY,
			&dataSetFormat, textureRegion[X], textureRegion[Y],
			textureRegion[Z], 0, 0, NULL, &clErr);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("create input texture2");

	cl_event clEvent2;
	clErr = clEnqueueWriteImage(cmdQueue, inputTexture2, CL_FALSE, textureOrigin,
			textureRegion, 0, 0, values, 0, NULL, &clEvent2);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("fill input texture2");*/
		
	

//	if(MCC_VERBOSE)
//		printf("wait for classification kernel to finish\n");
	clErr = clWaitForEvents(1, &clEvent);
	if(clErr || MCC_VERBOSE)
		_clhLogErr_("wait for classification kernel");
	
	if(MCC_PROFILING) {
		clhGetEventProfilingInfo(clEvent, sizeof(infoStr), infoStr, &clErr);
		printf("PROFILING: [%u] '%s' kernel\n%s", deviceIndex,
				kernelNames[MC_CLASSIFICATION], infoStr);
	}

	// Get all of the stuff written and allocated
	clEnqueueBarrier(cmdQueue);

	//### DEBUG ###
//	float* class1 = malloc(classificationSize);
//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], verticesBuffer, CL_TRUE, 0, classificationSize, class1, 0, NULL, NULL);
//	_clhLogErr_("read class1 buffer", "mcCore");
//	float classSum = 0;
//	for(int i = 0; i < classificationSize / 4; i++)
//		classSum += class[i];
//	printf("%.0f\n", classSum);
//	for(int i = 0; i < classificationSize / 4; i += 1000)
//		printf("%.0f,", class1[i]);
//	printf("\n");

//	float* class2 = malloc(classificationSize);
//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], occupiedBuffer, CL_TRUE, 0, classificationSize, class2, 0, NULL, NULL);
//	_clhLogErr_("read class2 buffer", "mcCore");
//	float classSum = 0;
//	for(int i = 0; i < classificationSize / 4; i++)
//		classSum += class[i];
//	printf("%.0f\n", classSum);
//	for(int i = 0; i < classificationSize / 4; i += 1000)
//		printf("%.0f,", class2[i]);
//	printf("\n");

//	exit(0);

	cl_float occupiedVoxelsCountRet = 0;
	cl_mem scannedOccupiedBuffer = clsScanFromDevice(clResources, deviceIndex,
			occupiedBuffer, voxelsCount, &occupiedVoxelsCountRet, &clErr);
	cl_ulong occupiedVoxelsCount = occupiedVoxelsCountRet;

	//### DEBUG ###
//	float* scan = malloc(classificationSize);
//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], scannedOccupiedBuffer, CL_TRUE, 0, classificationSize, scan, 0, NULL, NULL);
//	_clhLogErr_("read scan buffer", "mcCore");
//	for(int i = 0; i < 20; i++)
//		printf("%.0f,", scan[i]);
//	printf("\n");


	if(occupiedVoxelsCount > 0) {

		size_t elementBufferSize =
				clhGetDeviceAddressBits(clResources->devices[deviceIndex], &clErr) / 8;
		size_t compactionBufferSize = occupiedVoxelsCount * elementBufferSize;
			
		cl_mem compactedInputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				compactionBufferSize, NULL, &clErr);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("create compacted input buffer");

		// Get all of the stuff written and allocated
		clEnqueueBarrier(cmdQueue);

		// Now setup the arguments to our kernel
		clArg = 0;
		clErr  = clSetKernelArg(compactionKernel, clArg++,
				sizeof(cl_mem), &occupiedBuffer);
		clErr |= clSetKernelArg(compactionKernel, clArg++,
				sizeof(cl_mem), &scannedOccupiedBuffer);
		clErr |= clSetKernelArg(compactionKernel, clArg++,
				sizeof(cl_mem), &compactedInputBuffer);

		if(MCC_PROFILING)
			printf("INFO: [%u] '%s' kernel configuration: %zu\n",
					deviceIndex, kernelNames[MC_COMPACTION], voxelsCount);

		size_t compactionGWS[1] = {voxelsCount};

		clErr = clEnqueueNDRangeKernel(cmdQueue, compactionKernel, 1, NULL,
				compactionGWS, NULL, 0, NULL, &clEvent);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("enqueue compaction kernel range");

//		if(MCC_VERBOSE)
//			printf("wait for compaction kernel to finish\n");
		clErr = clWaitForEvents(1, &clEvent);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("wait for compaction kernel");

//		clFinish(cmdQueue);		
		
		if(MCC_PROFILING) {
			clhGetEventProfilingInfo(clEvent, sizeof(infoStr), infoStr, &clErr);
			printf("PROFILING: [%u] '%s' kernel\n%s", deviceIndex,
					kernelNames[MC_COMPACTION], infoStr);
		}

		clReleaseMemObject(occupiedBuffer);
		clReleaseMemObject(scannedOccupiedBuffer);

		// Get all of the stuff written and allocated
		clEnqueueBarrier(cmdQueue);

		cl_float vertexCountRet = 0.0f;
		cl_mem scannedVerticesBuffer = clsScanFromDevice(clResources, deviceIndex,
				verticesBuffer, voxelsCount, &vertexCountRet, &clErr);
		size_t vertexCount = vertexCountRet;

//		size_t trianglesCount = verticeCount / 3;

		clReleaseMemObject(verticesBuffer);

		//### DEBUG ###
	//	size_t* comp = malloc(compactionSize);
	//	clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], compactedInputBuffer, CL_TRUE, 0, compactionSize, comp, 0, NULL, NULL);
	//	_clhLogErr_("read comp buffer", "mcCore");
	//	for(int i = 0; i < 20; i++)
	//		printf("%d,", comp[i]);
	//	printf("\n");

		//TODO: back to vbo buffer
		size_t outputBufferSize = vertexCount * sizeof(cl_float4);
//		size_t trianglesBufferSize = vertexCount * sizeof(cl_float4);
#if USE_GL
		GLuint trianglesVBO;
		
		cl_mem trianglesBuffer = clhCreateGLCLBuffer(context,
				outputBufferSize, &trianglesVBO, &clErr);
		
//		cl_mem trianglesBuffer;
//		syncRequestVBOs(deviceIndex, ouputBufferSize, &trianglesVBO, NULL);
//		trianglesBuffer = clhBindNewCLBufferToVBO(context,
//				trianglesVBO, &clErr);

//		printf("context: %p, ouputBufferSize: %zu, trianglesVBO: %p, clErr: %p\n",
//				context, ouputBufferSize, &trianglesVBO, &clErr);
#else /* USE_GL */
		cl_mem trianglesBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				outputBufferSize, NULL, &clErr);
#endif /* USE_GL */
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("create triangles buffer");


		//TODO: back to vbo buffer
//		size_t normalsBufferSize = vertexCount * sizeof(cl_float4);
#if USE_GL
		GLuint normalsVBO;

		cl_mem normalsBuffer = clhCreateGLCLBuffer(context, outputBufferSize,
				&normalsVBO, &clErr);

//		cl_mem normalsBuffer;
//		syncRequestVBOs(deviceIndex, ouputBufferSize, &trianglesVBO,
//				&trianglesBuffer, &normalsVBO, &normalsBuffer);
//		printf("normals vbo: %u\n", normalsVBO);
//		cl_mem normalsBuffer = clhBindNewCLBufferToVBO(context,
//				normalsVBO, &clErr);
#else /* USE_GL */
		cl_mem normalsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				outputBufferSize, NULL, &clErr);
#endif /* USE_GL */
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("create normals buffer");


		// Get all of the stuff written and allocated
		clEnqueueBarrier(cmdQueue);

		// Now setup the arguments to our kernel
		clArg = 0;
		clErr  = clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &inputTexture);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_sampler), &inputTextureSampler);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_ulong), &occupiedVoxelsCount);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_float), &isoValue);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_uint4), &voxelsSizes);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_float4), &valuesDistances);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_float4), &valuesOffsets);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_float4), &valuesBuffersOffset);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &trianglesTableBuffer);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &verticesTableBuffer);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &scannedVerticesBuffer);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &compactedInputBuffer);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &trianglesBuffer);
		clErr |= clSetKernelArg(generationKernel, clArg++,
				sizeof(cl_mem), &normalsBuffer);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("set generation kernel arguments");

#if USE_GL
	    clErr = clEnqueueAcquireGLObjects(cmdQueue, 1, &trianglesBuffer, 0, NULL, NULL);
	    clErr |= clEnqueueAcquireGLObjects(cmdQueue, 1, &normalsBuffer, 0, NULL, NULL);
	    if(clErr || MCC_VERBOSE)
		    _clhLogErr_("aquire gl objects");
#endif /* USE_GL */

		// occupiedVoxelsCountRet == (float)occupiedVoxelsCount
	    size_t globalWorkSize = ceil(occupiedVoxelsCountRet / generationMaxWGS)
	    		* generationMaxWGS;

	    // Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		if(MCC_PROFILING)
			printf("INFO: [%u] '%s' kernel configuration: %zu (%zu)\n", 
					deviceIndex, kernelNames[MC_GENERATION],
					(size_t)occupiedVoxelsCount, globalWorkSize);

		size_t generationGWS[1] = {globalWorkSize};
		size_t generationLWS[1] = {generationMaxWGS};
		clErr = clEnqueueNDRangeKernel(cmdQueue, generationKernel, 1, NULL,
				generationGWS, generationLWS, 0, NULL, &clEvent);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("enqueue generation kernel range");

//		if(MCC_VERBOSE)
//			printf("wait for generation kernel to finish\n");
		clErr = clWaitForEvents(1, &clEvent);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("wait for generation kernel");

//		clFinish(cmdQueue);

		if(MCC_PROFILING) {
			clhGetEventProfilingInfo(clEvent, sizeof(infoStr), infoStr, &clErr);
			printf("PROFILING: [%u] '%s' kernel\n%s", deviceIndex,
					kernelNames[MC_GENERATION], infoStr);
		}

#if USE_GL
		clErr = clEnqueueReleaseGLObjects(cmdQueue, 1, &trianglesBuffer, 0, NULL, NULL);
		clErr |= clEnqueueReleaseGLObjects(cmdQueue, 1, &normalsBuffer, 0, NULL, NULL);
		if(clErr || MCC_VERBOSE)
			_clhLogErr_("release gl objects");
#endif /* USE_GL */
		
		// Once finished read back the results from the answer
		// array into the results array
		//### DEBUG ###
//		cl_float4* trianglesBufferRet = malloc(trianglesBufferSize);
//		clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], trianglesBuffer,
//				CL_TRUE, 0, trianglesBufferSize, trianglesBufferRet, 0, NULL, NULL);
//		_clhLogErr_("read triangles buffer", "mcCore");
//		for(int i = 0; i < 20; i++)
//			printf("(%.2f;%.2f;%.2f;%.0f)", trianglesBufferRet[i].s[0], trianglesBufferRet[i].s[1], trianglesBufferRet[i].s[2], trianglesBufferRet[i].s[3]);
//		printf("\n");
//		cl_float4* normalsBufferRet = malloc(normalsBufferSize);
//		clErr = clEnqueueReadBuffer(clResources->cmdQueues[0], normalsBuffer,
//				CL_TRUE, 0, normalsBufferSize, normalsBufferRet, 0, NULL, NULL);
//		_clhLogErr_("read normals buffer", "mcCore");
//		for(int i = 0; i < 20; i++)
//			printf("(%.2f;%.2f;%.2f;%.0f)", normalsBufferRet[i].s[0], normalsBufferRet[i].s[1], normalsBufferRet[i].s[2], normalsBufferRet[i].s[3]);
//		printf("\n");

		clEnqueueBarrier(cmdQueue);

		clReleaseMemObject(scannedVerticesBuffer);
		clReleaseMemObject(compactedInputBuffer);
//		clReleaseMemObject(trianglesBuffer);
//		clReleaseMemObject(normalsBuffer);


		if(outSizeRet) *outSizeRet = vertexCount;
#if USE_GL
		if(trianglesVBORet) *trianglesVBORet = trianglesVBO;
		if(normalsVBORet) *normalsVBORet = normalsVBO;
#endif /* USE_GL */
		if(trianglesBufferRet) *trianglesBufferRet = trianglesBuffer;
		if(normalsBufferRet) *normalsBufferRet = normalsBuffer;

		if(MCC_PROFILING)
			printf( "INFO: [%u] memory allocation\n"
					" input texture: %zuKB\n"
					" classification buffers: 2x %zuKB\n"
					" scanned buffers: 2x %zuKB\n"
	//				" compacted input buffer: %zuKB (%zu%%)\n"
	//				" output buffer: %zuKB + %zuKB\n",
					" compacted buffer: %zuKB\n"
					" output buffer 2x %zuKB\n",
					deviceIndex,
					inputTextureSize / KB,
					classificationBufferSize / KB,
	//				compactionBufferSize / KB, (compactionBufferSize * 100) / inputTextureSize,
	//				trianglesBufferSize / KB, normalsBufferSize / KB);
					classificationBufferSize / KB,
					compactionBufferSize / KB,
					outputBufferSize / KB);
				
	} else {

		clReleaseMemObject(verticesBuffer);
		clReleaseMemObject(occupiedBuffer);
		clReleaseMemObject(scannedOccupiedBuffer);

		if(outSizeRet) *outSizeRet = EMPTY;
#if USE_GL
		if(trianglesVBORet) trianglesVBORet = NULL;
		if(normalsVBORet) normalsVBORet = NULL;
#endif /* USE_GL */
		if(trianglesBufferRet) *trianglesBufferRet = NULL;
		if(normalsBufferRet) *normalsBufferRet = NULL;

		if(MCC_PROFILING)
			printf(	"INFO: [%u] memory allocation\n"
					" input buffer: %zuKB\n"
					" classification buffers: 2x %zuKB\n"
					" scanned buffer: %zuKB\n",
					deviceIndex,
					inputTextureSize / KB,
					classificationBufferSize / KB,
					classificationBufferSize / KB);
	}
	
	clReleaseMemObject(inputTexture);

/*	clReleaseMemObject(inputTexture2);
	_clhLogErr_("wait for generation kernel");	
	clErr = clWaitForEvents(1, &clEvent2);
	_clhLogErr_("wait for generation kernel");
	if(MCC_PROFILING) {
		clhGetEventProfilingInfo(clEvent2, sizeof(infoStr), infoStr, &clErr);
		printf("PROFILING: [%u] 'fill input texture2'\n%s", deviceIndex, infoStr);
	}*/

	return SUCCESS;
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
