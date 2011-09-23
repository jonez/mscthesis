/*
 * clHelper.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef CLHELPER_H_
#define CLHELPER_H_

#include "common.h"

#include <CL/cl.h>
#if USE_GL
 #include <GL/glx.h>
#endif /* USE_GL */

////////////////////////////////////////////////////////////////////////////////

#define OPENCL_COMPILER_OPTIONS ""

////////////////////////////////////////////////////////////////////////////////

#if USE_GL
extern GLXFBConfig* fgChooseFBConfig();
#endif

typedef struct _clhResources s_clhResources;
typedef struct _clhResources* clhResources;

struct _clhResources {
	cl_platform_id 		platform;
	cl_uint 			devCount;
	cl_device_id* 		devices;
	cl_context 			context;
	cl_command_queue* 	cmdQueues;
	size_t*				mwgSizes;
	cl_ulong*			memSizes;
#if USE_GL
	Display*			glDisplay;
	GLXDrawable			glWindow;
	GLXContext			glContext;
	XVisualInfo*		glVisual;
	GLXFBConfig*		glFBConfig;
#endif
};

////////////////////////////////////////////////////////////////////////////////

int clhGetVerbose();

void clhSetVerbose(const int state);

////////////////////////////////////////////////////////////////////////////////

// deprecated, use clhLogErrorMessage instead
#define clhErrorInfo(e, i, s) clhLogErrorMessage(e, i, s, 0, NULL)
//void clhErrorInfo(const cl_int		/* error */,
//				  const char*		/* info */,
//				  const char*		/* source */);

//void clhErrorCheck(const cl_int		/* error */,
//				   const char*		/* info */,
//				   const char*		/* source */);

const char* clhGetErrorInfo(const cl_int error);

char* clhErrorMessage(const cl_int error,
					  char* info,
					  const size_t infoSize,
					  const char* message,
					  const int device);

char* clhErrorMessageA(const cl_int error,
					   const char* message,
					   const int device);

#define clhLogErr_(e, m, d) clhLogErrorMessage(e, m, __FILE__, d, NULL)
#define clhLogErr__(e, m) clhLogErrorMessage(e, m, __FILE__, NO_DEVICE, NULL)
#define clhLogErr___(e, m, d, s) clhLogErrorMessage(e, m, __FILE__, d, s)
void clhLogErrorMessage(const cl_int error,
						const char* message,
						const char* source,
						const int device,
						FILE* stream);
						
////////////////////////////////////////////////////////////////////////////////

#define clhInitResources_(t, e) clhInitResources(NULL, t, NULL, 0, 0, e)
#define clhInitResources__(t, p, e) clhInitResources(NULL, t, NULL, 0, p, e)
#define clhInitResources___() clhInitResources(NULL, CL_DEVICE_TYPE_ALL, NULL, 0, 0, NULL)
clhResources clhInitResources(const char*							/* platformName */,
							  const cl_device_type					/* deviceType */,
							  const int*							/* devicesList */,
							  const int								/* devicesListSize */,
							  const cl_command_queue_properties		/* cmdQueuesProperties */,
							  cl_int* 								/* err */);
							  
cl_int clhFreeResources(clhResources		/* resources */);

cl_int clhRetainResources(clhResources		/* resources */);

cl_int clhReleaseResources(clhResources		/* resources */);

////////////////////////////////////////////////////////////////////////////////

char* clhGetPlatformInfo(const cl_platform_id		/* platform */,
						 const size_t				/* infoSize */,
						 char*						/* info */,
						 cl_int*					/* err */);
						 
char* clhGetDeviceInfo(const cl_device_id		/* device */,
					   const size_t				/* infoSize */,
					   char*					/* info */,
					   cl_int*					/* err */);
					   
////////////////////////////////////////////////////////////////////////////////

cl_ulong clhGetDeviceGlobalMemorySize(const cl_device_id	/* device */,
									  cl_int*				/* err */);
									  
cl_ulong clhGetDeviceMaxMemoryAllocSize(const cl_device_id	 	/* device */,
										cl_int* 				/* err */);
										
cl_uint clhGetDeviceAddressBits(const cl_device_id		/* device */,
								cl_int*					/* err */);

size_t clhGetDeviceMaxWorkGroupSize(const cl_device_id		/* device */,
									cl_int*					/* err */);

size_t clhGetKernelMaxWorkGroupSize(const cl_kernel 		/* kernel */,
									const cl_device_id		/* device */,
									cl_int* 				/* err */);
									
////////////////////////////////////////////////////////////////////////////////

// deprecated in OpenCL 1.1
cl_int clhSetCmdQueueProfilingState(const cl_command_queue 		/* cmdQueue */,
									const cl_bool 				/* enable */);
									
char* clhGetEventProfilingInfo(const cl_event		/* event */,
							   const size_t			/* infoStrSize */,
							   char*				/* infoStr */,
							   cl_int*				/* err */);
							   
////////////////////////////////////////////////////////////////////////////////

cl_int clhBuildProgram(cl_program				/* program */,
					   const clhResources		/* resources */);
					   
cl_program clhBuildProgramFromSource(const char*			/* source */,
									 const clhResources		/* resources */,
									 cl_int*				/* err */);
									 
cl_program clhBuildProgramFromBinaryFile(const char*			/* path */,
										 const clhResources		/* resources */,
										 cl_int*				/* err */);
										 
cl_int clhStoreProgramBinaryFile(const cl_program		/* program */,
								 const char*			/* path */);
								 
char* clhLoadSourceFile(const char*		/* path */); // err?

#define clhBuildProgramFromFile_(f, r, l, s, e) clhBuildProgramFromFile(f, NULL, NULL, NULL, r, l, s, e);
cl_program clhBuildProgramFromFile(const char*				/* sourceFile */,
								   const char*				/* kernelSrcPath */,
								   const char*				/* kernelBinPath */,
   								   const char*				/* kernelBinExt */,
								   const clhResources		/* resources */,
								   const int				/* loadBin */,
								   const int				/* storeBin */,
								   cl_int*					/* err */);
								   
#if USE_GL
////////////////////////////////////////////////////////////////////////////////

GLuint clhCreateVBO(const GLsizei		/* size */);
								   
cl_mem clhBindNewCLBufferToVBO(const cl_context		/* context */,
							   const GLuint			/* vbo */,
							   cl_int*				/* err */);

cl_mem clhCreateGLCLBuffer(const cl_context 		/* context */,
						   const size_t 			/* size */,
						   GLuint*					/* vbo */,
						   cl_int*					/* err */);
						   
#endif /* USE_GL */

#endif /* CLHELPER_H_ */
