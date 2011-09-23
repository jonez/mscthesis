/*
 * glHelper.c
 *
 *  Created on: Jun 21, 2011
 *      Author: jonez
 */

#include "glHelper.h"

#include <stdio.h>
#include <pthread.h>

#include <GL/glut.h>
#if FREEGLUT
	#include <GL/freeglut_ext.h>
#endif

#include "utilities.h"


////////////////////////////////////////////////////////////////////////////////

static int GL_VERBOSE = DISABLE;

int glGetVerbose() {
	return GL_VERBOSE;
}

void glSetVerbose(const int state) {
	GL_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

typedef struct request request;

struct request {
	size_t		size;
};

////////////////////////////////////////////////////////////////////////////////

int					glInitialize(const clhResources initializedCLResources);
int 				glShutdow();

mcDataset*			getDataset();
mcDataset*			setDataset(mcDataset* dataset);
float 				getIsoValue();
float 				setIsoValue(const float	isoValue);
float 				incIsoValue(const float incValue);
uint				getDevice();
uint				setDevice(uint device);
mcdMemEntry*		getMemList();
static void* 		clThreadFunc(void* arg);
void				clNewRun();

/*void				requestsDispatcher();
int					syncRequestVBOs(const uint threadNo, size_t size,
					GLuint* vbo1, cl_mem* buffer1, GLuint* vbo2,
					cl_mem* buffer2);*/

////////////////////////////////////////////////////////////////////////////////

static int 					initialized = FALSE;

static clhResources			resources = NULL;

static pthread_t			clThread;
static pthread_mutex_t		clThreadMutex;
static pthread_cond_t		clThreadCondition;
static mcDataset*			currDataset = NULL;
static mcdMemEntry*			currMemList = NULL;
static uint					currDevice = 0;
/*static pthread_mutex_t*		resourcesMutexes = NULL;
static pthread_cond_t*		resourcesConditions = NULL;
static size_t* 			sizesRequests = NULL;
static GLuint*				fstVBOs = NULL;
static GLuint*				sndVBOs = NULL;
static cl_mem*				fstBuffers = NULL;
static cl_mem*				sndBuffers = NULL;*/

////////////////////////////////////////////////////////////////////////////////

int glInitialize(const clhResources initializedCLResources) {

	if(GL_VERBOSE)
		printf("Initializing gl... ");
			
	pthread_create(&clThread, NULL, clThreadFunc, NULL);
	pthread_mutex_init(&clThreadMutex, NULL);
	pthread_cond_init(&clThreadCondition, NULL);

	resources = initializedCLResources;
	/*uint resourcesCount = resources->devCount;
	
	resourcesMutexes = (pthread_mutex_t*)calloc(resourcesCount, 
			sizeof(pthread_mutex_t));
	resourcesConditions = (pthread_cond_t*)calloc(resourcesCount,
			sizeof(pthread_cond_t));
	sizesRequests = (size_t*)calloc(resourcesCount, sizeof(size_t));
	fstVBOs = (GLuint*)calloc(resourcesCount, sizeof(GLuint));
	sndVBOs = (GLuint*)calloc(resourcesCount, sizeof(GLuint));
	fstBuffers = (cl_mem*)calloc(resourcesCount, sizeof(cl_mem));
	sndBuffers = (cl_mem*)calloc(resourcesCount, sizeof(cl_mem));
	
	for(int i = 0; i < resourcesCount; i++) {
		
		pthread_mutex_init(&resourcesMutexes[i], NULL);
		pthread_cond_init(&resourcesConditions[i], NULL);
		
		sizesRequests[i] = EMPTY;
		fstVBOs[i]	= EMPTY;
		sndVBOs[i] = EMPTY;
		fstBuffers[i] = EMPTY;
		sndBuffers[i] = EMPTY;
	}*/

	initialized = TRUE;
	
	if(GL_VERBOSE)
		printf("done!\n");

	return SUCCESS;
}

int glShutdown() {

	if(GL_VERBOSE)
		printf("Shuting down gl... ");

	initialized = FALSE;
	
	pthread_mutex_destroy(&clThreadMutex);
	/*for(int i = 0; i < resources->devCount; i++) {
		pthread_mutex_destroy(&resourcesMutexes[i]);
		pthread_cond_destroy(&resourcesConditions[i]);
	}

	free(resourcesMutexes);
	free(resourcesConditions);
	free(sizesRequests);
	free(fstVBOs);
	free(sndVBOs);
	free(fstBuffers);
	free(sndBuffers);*/
	
//	free memlist
	freeMemList(currMemList);

	if(GL_VERBOSE)
		printf("done!\n");
	
	return SUCCESS;
}

mcDataset* getDataset() {

	return currDataset;
}

mcDataset* setDataset(mcDataset* dataset) {

	mcDataset* prevDataset = currDataset;
	printf("Dataset: sizeX = %u, sizeY = %u, sizeZ = %u, isoValue = %0.2f\n", 
			dataset->sizeX, dataset->sizeY, dataset->sizeZ, dataset->isoValue);

	currDataset = dataset;
	clNewRun();
	
	return prevDataset;
}

float getIsoValue() {

	return currDataset->isoValue;
}

float setIsoValue(const float isoValue) {

	float prevIsoValue = currDataset->isoValue;
	if(GL_VERBOSE) printf("Dataset: isoValue = %0.2f\n", isoValue);

	currDataset->isoValue = isoValue;
	clNewRun();
	
	return prevIsoValue;
}

float incIsoValue(const float incValue) {

	return setIsoValue(getIsoValue() + incValue);
}

uint getDevice() {

	return currDevice;
}

uint setDevice(uint device) {

	uint prevDevice = currDevice;
	
	currDevice = device;
	
	return prevDevice;
}

mcdMemEntry* getMemList() {

	return currMemList;
}

void clNewRun() {

#if PROFILING
		printf("### clNewRun() - %ldms ###\n", getCurrentTimeInMili());
		fflush(NULL);
#endif /* PROFILING */

	if(currDataset) {
	
		pthread_mutex_lock(&clThreadMutex); //lock(clThreadMutex)
		
			freeMemList(currMemList);
			pthread_cond_signal(&clThreadCondition);
			if(GL_VERBOSE) printf("signal: %ldms\n", getCurrentTimeInMili());
		
		pthread_mutex_unlock(&clThreadMutex); //unlock(clThreadMutex)
	}
}

static void* clThreadFunc(void* arg) {

	while(initialized) {
	
		pthread_mutex_lock(&clThreadMutex); //lock(clThreadMutex)

			if(GL_VERBOSE) printf("wait: %ldms\n", getCurrentTimeInMili());
			pthread_cond_wait(&clThreadCondition, &clThreadMutex);
			if(GL_VERBOSE) printf("condition: %ldms\n", getCurrentTimeInMili());
		
			if(!initialized) {
				pthread_mutex_unlock(&clThreadMutex); //unlock(clThreadMutex)
				break;	
			}
			
//			mcdDispatchSingle(currDataset, currDevice, FALSE, &currMemList);
			mcdDispatchMulti(currDataset, 0, NULL, FALSE, &currMemList);
			if(GL_VERBOSE) printMemListInfo(currMemList);

#if USE_GL
			glutPostRedisplay();
#endif /* USE_GL */
//			glutMainLoopEvent();
					
		pthread_mutex_unlock(&clThreadMutex); //unlock(clThreadMutex)
	}
	
	pthread_exit(NULL);
}

/*static uint count = 0;

void requestsDispatcher(int value) {

//	printf("rd(%d):", resourcesCount);
	count++;

	for(int i = 0; i < resources->devCount; i++) {

		if(sizesRequests[i] > 0) {
		
			pthread_mutex_lock(&resourcesMutexes[i]); //lock(clThreadMutex)

			fstBuffers[i] = clhCreateGLCLBuffer(resources->context,
					sizesRequests[i], &fstVBOs[i], NULL);
			sndBuffers[i] = clhCreateGLCLBuffer(resources->context,
					sizesRequests[i], &sndVBOs[i], NULL);
		
			printf("rd(%u): res=%d;buf1=%p;vbo1=%u;buf2=%p;vbo2=%u\n", count, i,
					fstBuffers[i], fstVBOs[i], sndBuffers[i], sndVBOs[i]);

			sizesRequests[i] = EMPTY;
		
			pthread_cond_signal(&resourcesConditions[i]);
			pthread_mutex_unlock(&resourcesMutexes[i]); //unlock(clThreadMutex)
		}
	}
	
//	printf("\n");
}

int syncRequestVBOs(const uint threadNo, size_t size, GLuint* vbo1,
		cl_mem* buffer1, GLuint* vbo2, cl_mem* buffer2) {

	if(GL_VERBOSE) printf("lock: %ldms\n", getCurrentTimeInMili());
	pthread_mutex_lock(&resourcesMutexes[threadNo]); //lock(resourcesMutexes[threadNo])
	
	sizesRequests[threadNo] = size;
	if(GL_VERBOSE) printf("r: %d(%zu)\n", threadNo, size);

	pthread_cond_wait(&resourcesConditions[threadNo],
			&resourcesMutexes[threadNo]);	
	if(GL_VERBOSE) printf("unlock: %ldms\n", getCurrentTimeInMili());

	if(vbo1) *vbo1 = fstVBOs[threadNo];
	if(buffer1) *buffer1 = fstBuffers[threadNo];
	if(vbo2) *vbo2 = sndVBOs[threadNo];
	if(buffer2) *buffer2 = sndBuffers[threadNo];
	
	pthread_mutex_unlock(&resourcesMutexes[threadNo]); //unlock(resourcesMutexes[threadNo])

	return SUCCESS;
}*/

