/*
 * mcDispatcher.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 *	Notes: > reuse mcdThread instead of creating new ones each run
 */

#include "mcDispatcher.h"

#include <math.h>
#include <limits.h>
#include <pthread.h>

#include "mcCore.h"
#if USE_GL
#include "glHelper.h"
#endif /* USE_GL */
#include "utilities.h"

////////////////////////////////////////////////////////////////////////////////

static int MCD_VERBOSE = DISABLE;

int mcdGetVerbose() {
	return MCD_VERBOSE;
}

void mcdSetVerbose(const int state) {
	MCD_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

mcdMemEntry* newMemEntry(mcdMemEntry* previousEntry) {

	mcdMemEntry* newEntry = (mcdMemEntry*)malloc(sizeof(mcdMemEntry));
	
	newEntry->size = EMPTY;
#if USE_GL
	newEntry->trianglesVBO = EMPTY;
	newEntry->normalsVBO = EMPTY;
#endif /* USE_GL */
	newEntry->trianglesBuffer = NULL;
	newEntry->normalsBuffer = NULL;
	newEntry->next = NULL;

	if(previousEntry) previousEntry->next = newEntry;
	
	return newEntry;
}

void freeMemList(mcdMemEntry* headEntry) {

//	if(MCD_VERBOSE) printf("freeMemList()\n");

	cl_int clErr;

	mcdMemEntry* currentEntry = headEntry;
	
	while(currentEntry) {
	
		mcdMemEntry* nextEntry = currentEntry->next;
		
#if USE_GL
		glDeleteBuffers(currentEntry->trianglesVBO);
		glDeleteBuffers(currentEntry->normalsVBO);
#endif /* USE_GL */
		clErr = clReleaseMemObject(currentEntry->trianglesBuffer);
		clErr = clReleaseMemObject(currentEntry->normalsBuffer);

		free(currentEntry);
		
//		if(MCD_VERBOSE) printf("free = %s\n", clErr ? "nok" : "ok");
		
		currentEntry = nextEntry;
	}
}

void printMemListInfo(mcdMemEntry* headEntry) {

	uint pos = 0;
	size_t sum = 0;
	for(mcdMemEntry* memEntry = headEntry;
		memEntry;
		memEntry = memEntry->next, pos++) {

#if USE_GL
		printf("%d: %zu (%zuKB [%d] + %zuKB [%d])\n",
				pos, memEntry->size,
				memEntry->size * sizeof(cl_float4) / KB, memEntry->trianglesVBO,
				memEntry->size * sizeof(cl_float4) / KB, memEntry->normalsVBO);
#else /* USE_GL */
		printf("%d: %zu (%zuKB [%p] + %zuKB [%p])\n",
				pos, memEntry->size,
				memEntry->size * sizeof(cl_float4) / KB, memEntry->trianglesBuffer,
				memEntry->size * sizeof(cl_float4) / KB, memEntry->normalsBuffer);
#endif /* USE_GL */
		sum += memEntry->size * 2;
	}
	printf("sum (v+n): %zu (%zuMB)\n", sum, sum * sizeof(cl_float4) / MB);
}

uint getMemListEntriesCount(mcdMemEntry* headEntry) {

	uint count = 0;
	for(mcdMemEntry* memEntry = headEntry;
		memEntry;
		memEntry = memEntry->next, count++);
	
	return count;
}

size_t getMemListEntriesSizesSum(mcdMemEntry* headEntry) {

	size_t sum = 0;
	for(mcdMemEntry* memEntry = headEntry;
		memEntry;
		memEntry = memEntry->next) {

		sum += memEntry->size * 2;
	}
	
	return sum;
}

////////////////////////////////////////////////////////////////////////////////

#define STATIC_SIZE 32
#define AUTO_SIZE 32
#define AUTO_SIZE_MIN 4 * MB
#define AUTO_SIZE_MAX 128 * MB
#define AUTO_SIZE_MEM_RATIO 30
// = 6 (* input, average runtime demand) * (runtime fraction, 1 /) 5

static const int DATASET_VALUES_BUFFER_SIZE = 1;

////////////////////////////////////////////////////////////////////////////////

typedef struct _threadRets threadRets;

struct _threadRets {
	cl_int			err;
};

typedef struct _threadArgs threadArgs;

struct _threadArgs {
	uint 					device;
#if USE_GL
	int						multiDevices;
#endif /* USE_GL */
	int						useHost;
	mcDataset*				dataset;
	pthread_mutex_t*		threadsMutex;
	size_t*					currentSlice;
	mcdMemEntry**			currentMemEntry;
	threadRets*				threadStatus;
};

////////////////////////////////////////////////////////////////////////////////

int mcdInitialize			(const clhResources initializedResources);
int mcdShutdown();

int mcdDispatchSingle		(const mcDataset* dataset,
							 const uint device,
							 const int useHost,
							 mcdMemEntry** output);
int mcdDispatchMulti		(const mcDataset* dataset,
							 const int devicesCount,
							 const uint* devicesList,
							 const int useHost,
							 mcdMemEntry** output);

static void mcdThread		(void* arg);

////////////////////////////////////////////////////////////////////////////////

static int initialized = FALSE;

static clhResources resources = NULL;

////////////////////////////////////////////////////////////////////////////////

int mcdInitialize(const clhResources initializedResources) {

#if PROFILING
		printf("### mcdInit() - %dms ###\n", getCurrentTimeInMili());
		fflush(NULL);
#endif /* PROFILING */
	
	if(initialized) {
		logMsg_("ERROR: illegal mcdInitialize() reinitialization attempt");
		return ERROR;
	}

	if(initializedResources)
		resources = initializedResources;
	else {
		logMsg_("ERROR: invalid resources");
		return ERROR;
	}

	mccInitializeCL(resources);
	
	initialized = TRUE;
	logMsg_("SUCCESS: initialized!");
		
	return SUCCESS;
}

int mcdShutdown() {

	if(!initialized) {
		logMsg_("ERROR: not initialized, call mcdInitialize(...) before\n");
		return ERROR;
	}
	
	initialized = FALSE;

	mccShutdownCL();
#if USE_GL
	
#endif /* USE_GL */
	
	if(MCD_VERBOSE)
		logMsg_("SUCCESS: resources released!");
		
	return SUCCESS;
}

int mcdDispatchSingle(const mcDataset* dataset,
					  const uint device,
					  const int useHost,
					  mcdMemEntry** output) {

	if(!initialized) {
		logMsg_("ERROR: not initialized, call mcdInitialize(...) before\n");
		return ERROR;
	}

	threadArgs args;
	threadRets rets;
	
	pthread_mutex_t threadsMutex;
	pthread_mutex_init(&threadsMutex, NULL);
	
	size_t currentSlice = 0;
	mcdMemEntry* memList = newMemEntry_();
	mcdMemEntry* currentMemEntry = memList;	
	
	args.device = device;
#if USE_GL
	args.multiDevices = FALSE;
#endif /* USE_GL */
	args.useHost = useHost;
	args.dataset = (mcDataset*)dataset;
	args.threadsMutex = &threadsMutex;
	args.currentSlice = &currentSlice;
	args.currentMemEntry = &currentMemEntry;
	args.threadStatus = &rets;

	if(MCD_VERBOSE)
		printf("Single thread for device %u.\n", device);
	
	mcdThread((void*)&args);
	
	if(MCD_VERBOSE)
		printf("Completed single thread having a status of %d.\n", rets.err);
	
	pthread_mutex_destroy(&threadsMutex);
	
	if(output) *output = memList;

	return SUCCESS;
}

int mcdDispatchMulti(const mcDataset* dataset,
					 const int devicesCount,
					 const uint* devicesList,
					 const int useHost,
					 mcdMemEntry** output) {

	if(!initialized) {
		logMsg_("ERROR: not initialized, call mcdInitialize(...) before\n");
		return ERROR;
	}

	int			err;
	cl_uint		devCount = devicesCount ? devicesCount : resources->devCount;

	//TESTING//
//	devCount = 4;
	//TESTING//

	pthread_attr_t threadsAttr;
	pthread_attr_init(&threadsAttr);
	pthread_attr_setdetachstate(&threadsAttr, PTHREAD_CREATE_JOINABLE);
	
	pthread_mutex_t threadsMutex;
	pthread_mutex_init(&threadsMutex, NULL);

	pthread_t		threads[devCount];
	threadArgs		threadsArgs[devCount];
	threadRets		threadsStatus[devCount];
	
	size_t currentSlice = 0;
	mcdMemEntry* memList = newMemEntry(NULL);
	mcdMemEntry* currentMemEntry = memList;	
	
	for(uint t = 0; t < devCount; t++){
		
		threadsArgs[t].device = devicesCount ? devicesList[t] : t;		
#if USE_GL
		threadsArgs[t].multiDevices = TRUE;
#endif /* USE_GL */
		threadsArgs[t].useHost = useHost;
		threadsArgs[t].dataset = (mcDataset*)dataset;
		threadsArgs[t].threadsMutex = &threadsMutex;
		threadsArgs[t].currentSlice = &currentSlice;
		threadsArgs[t].currentMemEntry = &currentMemEntry;
		threadsArgs[t].threadStatus = &threadsStatus[t];
		
		if(MCD_VERBOSE)
			printf("Creating thread %u.\n", t);
		err = pthread_create(&threads[t], &threadsAttr, (void*)&mcdThread,
				(void*)&threadsArgs[t]);
		if(err) {
			printf("ERROR: return code from pthread_create() is %d.\n", err);
			return ERROR;
		}
	}
	
//	threadRets* threadsStatus;
	for(uint t = 0; t < devCount; t++) {
	
		err = pthread_join(threads[t], NULL /* (void*)&threadsStatus */);
		if(err) {
			printf("ERROR: return code from pthread_join() is %d.\n", err);
			return ERROR;
		}
		
		if(MCD_VERBOSE)
			printf("Completed join with thread %u, having a status of %d.\n",
				t, threadsStatus[t].err);
		
//		free(threadStatus);
	}
	
	pthread_attr_destroy(&threadsAttr);
	pthread_mutex_destroy(&threadsMutex);
	
	if(output) *output = memList;

//	*output = parts;
//	*outSize = partsCount;

//	for(int i = 0; i < *outSize; i++) {
//		printf("%d -> %d\n", i, parts[i]->size);
//		if(parts[i]->size)
//			printf("(%.2f;%.2f;%.2f;%.0f)\n", parts[i]->data[0].s[0], parts[i]->data[0].s[1], parts[i]->data[0].s[2], parts[i]->data[0].s[3]);
//	}

	return SUCCESS;
}

static void mcdThread(void* arg) {

	threadArgs* args = (threadArgs*)arg;
	
	if(MCD_VERBOSE) printf("Thread (dev: %u)!\n", args->device);
	
#if USE_GL
	GLXContext newGLContext = NULL;

	if(args->multiDevices) {
	
//	#if PROFILING
//		printf("### XLockDisplay() called - %ldms... ", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
		XLockDisplay(resources->glDisplay);
//	#if PROFILING
//		printf("done! - %ldms ###\n", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */

			newGLContext = glXCreateContext(resources->glDisplay,
					resources->glVisual, resources->glContext, TRUE);
			
	//		printf("currentGLContext=%p, newGLContext=%p - %ldms\n",
	//				resources->glContext, newGLContext, getCurrentTimeInMili());
	//		fflush(NULL);
		
			Bool isCtxSet = glXMakeCurrent(resources->glDisplay,
					resources->glWindow, newGLContext);
	//		Bool isCtxSet = glXMakeCurrent(resources->glDisplay,
	//				resources->glWindow, resources->glContext);
			printf("glXMakeCurrent(ctx) = %s - %ldms\n", isCtxSet ? "true" : "false",
					getCurrentTimeInMili());
			fflush(NULL);
		
	//		GLenum errCode;
	//		const GLubyte *errString;
	//
	//		if((errCode = glGetError()) != GL_NO_ERROR) {
	//			errString = gluErrorString(errCode);
	//			fprintf (stderr, "OpenGL Error: %s\n", errString);
	//		}
	
	//		printf("glXIsDirect=%s - %ldms\n", glXIsDirect(resources->glDisplay,
	//				newGLContext) ? "true" : "false", getCurrentTimeInMili());
	//		fflush(NULL);
	
//	#if PROFILING
//		printf("### XUnlockDisplay() called - %ldms... ", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
		XUnlockDisplay(resources->glDisplay);
//	#if PROFILING
//		printf("done! - %ldms ###\n", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
		
//		if(!isCtxSet)
//			exit(0);
			
	}
#endif /* USE_GL */

	pthread_mutex_t* mutex = args->threadsMutex;
	
	uint device = args->device;
	size_t sizeX = args->dataset->sizeX, sizeY = args->dataset->sizeY;

	size_t stackSize = args->dataset->sizeZ;
	size_t sliceSize = sizeX * sizeY;
	size_t currentSlice = 0;
	
#if AUTO_SIZE
#define _size slices * sliceSize * sizeof(cl_float)
#define _slices(s) s / (sliceSize * sizeof(cl_float))
#define _memRatio resources->memSizes[device] / AUTO_SIZE_MEM_RATIO

	size_t slices = AUTO_SIZE;

	if(_size < AUTO_SIZE_MIN)
		slices = _slices(AUTO_SIZE_MIN);
	else {
		if(_size > AUTO_SIZE_MAX)
			slices = _slices(AUTO_SIZE_MAX);
		if(_size > _memRatio)
			slices = _slices(_memRatio);
	}
#else /* AUTO_SIZE */
	size_t slices = STATIC_SIZE;
#endif /* AUTO_SIZE */

	cl_int2 valuesZBuffers = {{0, 0}};
	cl_float4 valuesDistance = {{args->dataset->distanceX,
			args->dataset->distanceY, args->dataset->distanceZ, 1.0f}};

	for(uint i = 0; ; i++) {

#if PROFILING	
		printf("new iteration - %ldms", getCurrentTimeInMili());
		fflush(NULL);
#endif /* PROFILING */
		
		pthread_mutex_lock(mutex); //lock(mutex)

#if PROFILING		
			printf(", lock - %ldms", getCurrentTimeInMili());
			fflush(NULL);
#endif /* PROFILING */
		
			currentSlice = *args->currentSlice;
		
			if(currentSlice + slices > stackSize)
				slices = stackSize - currentSlice;
	
			if(slices <= 1) {
				pthread_mutex_unlock(mutex); //unlock(mutex)

#if PROFILING
				printf(", unlock - %ldms\n", getCurrentTimeInMili());
				fflush(NULL);
#endif /* PROFILING */
			
				break;
			}

			*args->currentSlice += slices - 1;
			mcdMemEntry* memEntry = *args->currentMemEntry;
	
			if(currentSlice + slices < stackSize)
				*args->currentMemEntry = newMemEntry(memEntry);

#if PROFILING		
			printf(", currSlice - %ldms", getCurrentTimeInMili());
			fflush(NULL);
#endif /* PROFILING */
		
		pthread_mutex_unlock(mutex); //unlock(mutex)

#if PROFILING		
		printf(", unlock - %ldms\n", getCurrentTimeInMili());
		fflush(NULL);
#endif /* PROFILING */
		
//		if(MCD_VERBOSE)
//			printf("slices: %zu, stackSize: %zu\n", slices, stackSize);

		if(currentSlice <= 0)
			valuesZBuffers.s[TOP] = EMPTY;
		else
			valuesZBuffers.s[TOP] = DATASET_VALUES_BUFFER_SIZE;

		if(currentSlice + slices >= stackSize)
			valuesZBuffers.s[BOTTOM] = EMPTY;
		else
			valuesZBuffers.s[BOTTOM] = DATASET_VALUES_BUFFER_SIZE;

		cl_uint4 sizes = {{sizeX, sizeY, slices, 0}};
		cl_float4 offsets = {{0.0f, 0.0f, (float)currentSlice, 0.0f}};
		size_t memAddress = (currentSlice - valuesZBuffers.s[TOP]) * sliceSize;
		
		clock_t beginTime = getCurrentTimeInMili();
		if(MCD_VERBOSE)
			printf(">>> currentSlice: %zu, slices: %zu, device: %u:%u, begin:"
					"%ldms\n", currentSlice, slices, device, i, beginTime);

//		if(useHost)
//			mccHost(&input[memAddress], isoValue, sizeX, sizeY, slices,
//					valuesDistance, offsets, &part->triangles, &part->normals, &part->size);
//		else
		mccMarchCL(device, &args->dataset->values[memAddress], args->dataset->isoValue,
				sizes, valuesDistance, offsets, valuesZBuffers, &memEntry->size,
#if USE_GL
				&memEntry->trianglesVBO, &memEntry->normalsVBO,
#endif /* USE_GL */
				&memEntry->trianglesBuffer, &memEntry->normalsBuffer);

		clock_t endTime = getCurrentTimeInMili();
		if(MCD_VERBOSE)
			printf("<<< device: %u:%u, size: %zu, end: %ldms, elapsed: %ldms\n",
					device, i, memEntry->size, endTime, endTime - beginTime);
	}
	
#if USE_GL
//#if PROFILING
//	printf("### glFinish() called - %ldms... ", getCurrentTimeInMili());
//	fflush(NULL);
//#endif /* PROFILING */
	glFinish();
//#if PROFILING
//	printf("done! - %ldms ###\n", getCurrentTimeInMili());
//	fflush(NULL);
//#endif /* PROFILING */

	if(args->multiDevices) {

//	#if PROFILING
//		printf("### XLockDisplay() called - %ldms... ", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
		XLockDisplay(resources->glDisplay);
//	#if PROFILING
//		printf("done! - %ldms ###\n", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */

/*			Bool isCtxSet = glXMakeCurrent(resources->glDisplay, None, NULL);
			printf("glXMakeCurrent() = %s - %ldms\n", isCtxSet  ? "true" : "false", getCurrentTimeInMili());
			fflush(NULL);
			glXDestroyContext(resources->glDisplay, newGLContext);*/
	
//	#if PROFILING
//		printf("### XUnlockDisplay() called - %ldms... ", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
		XUnlockDisplay(resources->glDisplay);
//	#if PROFILING
//		printf("done! - %ldms ###\n", getCurrentTimeInMili());
//		fflush(NULL);
//	#endif /* PROFILING */
	
	}
#endif /* USE_GL */
	
	args->threadStatus->err = CL_SUCCESS;
	
//	pthread_exit(NULL);
}

