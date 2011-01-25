/*
 * clHelper.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef CLHELPER_H_
#define CLHELPER_H_

#include <CL/cl.h>
#include <GL/gl.h>
#include "CL/cl_gl.h"


#define OPENCL_COMPILER_OPTIONS "-Wall"


struct _clhResources {
	cl_platform_id 		platform;
	cl_uint 			devCount;
	cl_device_id* 		devices;
	cl_context 			context;
	cl_command_queue* 	cmdQueues;
	size_t*				wgSizes;
	cl_ulong*			memSizes;
};

typedef struct _clhResources* clhResources;

////////////////////////////////////////////////////////////////////////////////////////////////////


int clhGetVerbose();

void clhSetVerbose(const int state);

void clhErrorInfo(const cl_int		/* error */,
				  const char*		/* info */,
				  const char*		/* source */);

void clhErrorCheck(const cl_int		/* error */,
				   const char*		/* info */,
				   const char*		/* source */);

clhResources clhInitResources(const char*							/* platformName */,
							  const cl_device_type					/* selectedDeviceType */,
							  const cl_command_queue_properties		/* cmdQueuesProperties */,
							  cl_int* 								/* err */);

char* clhGetPlatformInfo(const cl_platform_id		/* platform */,
						 const size_t				/* infoSize */,
						 char*						/* info */,
						 cl_int*					/* err */);

char* clhGetDeviceInfo(const cl_device_id		/* device */,
					   const size_t				/* infoSize */,
					   char*					/* info */,
					   cl_int*					/* err */);

size_t clhGetDeviceMaxWorkGroupSize(const cl_device_id		/* device */,
									cl_int*					/* err */);

cl_ulong clhGetDeviceGlobalMemorySize(const cl_device_id	/* device */,
									  cl_int*				/* err */);

size_t clhGetKernelMaxWorkGroupSize(const cl_kernel 		/* kernel */,
									const cl_device_id		/* device */,
									cl_int* 				/* err */);

cl_program clhBuildProgramFromSource(const char*			/* source */,
									 const clhResources		/* resources */,
									 cl_int*				/* err */);

char* clhLoadSourceFile(const char*		/* path */);

cl_program clhBuildProgramFromFile(const char*				/* sourceFile */,
								   const clhResources		/* resources */,
								   cl_int*					/* err */);

cl_mem clhCreateGLCLBuffer(const cl_context 		/* context */,
						   const size_t 			/* size */,
						   GLuint*					/* vbo */,
						   cl_int*					/* err */);


#endif /* CLHELPER_H_ */
