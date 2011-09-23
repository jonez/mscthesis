/*
 * clHelper.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#include "clHelper.h"

//#include <stdio.h>
#include <string.h>
#if USE_GL
#include <CL/cl_gl.h> //a pig é a maior
#endif /* USE_GL */

//#include "common.h"
#include "utilities.h"

////////////////////////////////////////////////////////////////////////////////

#define _clhLogErr__(m) clhLogErr__(retErr, m)

#define KERNELS_SRC_PATH DEFAULT_KERNELS_SRC_PATH
#define KERNELS_BIN_PATH DEFAULT_KERNELS_BIN_PATH
#define KERNELS_BIN_EXT DEFAULT_KERNELS_BIN_EXT

////////////////////////////////////////////////////////////////////////////////

static int CLH_VERBOSE = DISABLE;

int clhGetVerbose() {
	return CLH_VERBOSE;
}

void clhSetVerbose(const int state) {
	CLH_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

// deprecated, use clhLogErrorMsg instead
/*void clhErrorInfo(const cl_int error, const char* info, const char* source) {

	static const char* errorList[] = {
			"CL_SUCCESS",
			"CL_DEVICE_NOT_FOUND",
			"CL_DEVICE_NOT_AVAILABLE",
			"CL_COMPILER_NOT_AVAILABLE",
			"CL_MEM_OBJECT_ALLOCATION_FAILURE",
			"CL_OUT_OF_RESOURCES",
			"CL_OUT_OF_HOST_MEMORY",
			"CL_PROFILING_INFO_NOT_AVAILABLE",
			"CL_MEM_COPY_OVERLAP",
			"CL_IMAGE_FORMAT_MISMATCH",
			"CL_IMAGE_FORMAT_NOT_SUPPORTED",
			"CL_BUILD_PROGRAM_FAILURE",
			"CL_MAP_FAILURE",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"CL_INVALID_VALUE",
			"CL_INVALID_DEVICE_TYPE",
			"CL_INVALID_PLATFORM",
			"CL_INVALID_DEVICE",
			"CL_INVALID_CONTEXT",
			"CL_INVALID_QUEUE_PROPERTIES",
			"CL_INVALID_COMMAND_QUEUE",
			"CL_INVALID_HOST_PTR",
			"CL_INVALID_MEM_OBJECT",
			"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
			"CL_INVALID_IMAGE_SIZE",
			"CL_INVALID_SAMPLER",
			"CL_INVALID_BINARY",
			"CL_INVALID_BUILD_OPTIONS",
			"CL_INVALID_PROGRAM",
			"CL_INVALID_PROGRAM_EXECUTABLE",
			"CL_INVALID_KERNEL_NAME",
			"CL_INVALID_KERNEL_DEFINITION",
			"CL_INVALID_KERNEL",
			"CL_INVALID_ARG_INDEX",
			"CL_INVALID_ARG_VALUE",
			"CL_INVALID_ARG_SIZE",
			"CL_INVALID_KERNEL_ARGS",
			"CL_INVALID_WORK_DIMENSION",
			"CL_INVALID_WORK_GROUP_SIZE",
			"CL_INVALID_WORK_ITEM_SIZE",
			"CL_INVALID_GLOBAL_OFFSET",
			"CL_INVALID_EVENT_WAIT_LIST",
			"CL_INVALID_EVENT",
			"CL_INVALID_OPERATION",
			"CL_INVALID_GL_OBJECT",
			"CL_INVALID_BUFFER_SIZE",
			"CL_INVALID_MIP_LEVEL",
			"CL_INVALID_GLOBAL_WORK_SIZE",
	};

	const int listSize = sizeof(errorList) / sizeof(errorList[0]);
	const int index = -error;

	if(index >= 0 && index < listSize)
		printf("%s : %s [@%s] - %ldms\n", errorList[index], info, source, getCurrentTimeInMili());
	else
		printf("ID ERROR %d : %s [@%s] - %ldms\n", error, info, source, getCurrentTimeInMili());

}

void clhErrorCheck(const cl_int error, const char* info, const char* source) {

	if(error != CL_SUCCESS) {
		clhErrorInfo(error, info, source);

		exit(EXIT_FAILURE);
	}

}
*/

const char* clhGetErrorInfo(const cl_int error) {

	static const char* errorList[] = {
			"CL_SUCCESS",
			"CL_DEVICE_NOT_FOUND",
			"CL_DEVICE_NOT_AVAILABLE",
			"CL_COMPILER_NOT_AVAILABLE",
			"CL_MEM_OBJECT_ALLOCATION_FAILURE",
			"CL_OUT_OF_RESOURCES",
			"CL_OUT_OF_HOST_MEMORY",
			"CL_PROFILING_INFO_NOT_AVAILABLE",
			"CL_MEM_COPY_OVERLAP",
			"CL_IMAGE_FORMAT_MISMATCH",
			"CL_IMAGE_FORMAT_NOT_SUPPORTED",
			"CL_BUILD_PROGRAM_FAILURE",
			"CL_MAP_FAILURE",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"CL_INVALID_VALUE",
			"CL_INVALID_DEVICE_TYPE",
			"CL_INVALID_PLATFORM",
			"CL_INVALID_DEVICE",
			"CL_INVALID_CONTEXT",
			"CL_INVALID_QUEUE_PROPERTIES",
			"CL_INVALID_COMMAND_QUEUE",
			"CL_INVALID_HOST_PTR",
			"CL_INVALID_MEM_OBJECT",
			"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
			"CL_INVALID_IMAGE_SIZE",
			"CL_INVALID_SAMPLER",
			"CL_INVALID_BINARY",
			"CL_INVALID_BUILD_OPTIONS",
			"CL_INVALID_PROGRAM",
			"CL_INVALID_PROGRAM_EXECUTABLE",
			"CL_INVALID_KERNEL_NAME",
			"CL_INVALID_KERNEL_DEFINITION",
			"CL_INVALID_KERNEL",
			"CL_INVALID_ARG_INDEX",
			"CL_INVALID_ARG_VALUE",
			"CL_INVALID_ARG_SIZE",
			"CL_INVALID_KERNEL_ARGS",
			"CL_INVALID_WORK_DIMENSION",
			"CL_INVALID_WORK_GROUP_SIZE",
			"CL_INVALID_WORK_ITEM_SIZE",
			"CL_INVALID_GLOBAL_OFFSET",
			"CL_INVALID_EVENT_WAIT_LIST",
			"CL_INVALID_EVENT",
			"CL_INVALID_OPERATION",
			"CL_INVALID_GL_OBJECT",
			"CL_INVALID_BUFFER_SIZE",
			"CL_INVALID_MIP_LEVEL",
			"CL_INVALID_GLOBAL_WORK_SIZE",
	};

	const int listSize = sizeof(errorList) / sizeof(errorList[0]);
	const int index = -error;

	if(index >= 0 && index < listSize)
		return errorList[index];
	else
		return NULL;

}

char* clhErrorMessage(const cl_int error,
					  char* info,
					  const size_t infoSize,
					  const char* message,
					  const int device) {

	char errString[TINY_STRING_SIZE / 5];
	const char* errInfo = clhGetErrorInfo(error);
	if(errInfo && strlen(errInfo))
		snprintf(errString, sizeof(errString), "%s", errInfo);
	else
		snprintf(errString, sizeof(errString), "ERROR ID %d", error);
	
	char devString[TINY_STRING_SIZE / 10] = "[-]";
	if(device > NO_DEVICE)
		snprintf(devString, sizeof(devString), "[%d]", device);
	
	snprintf(info, infoSize, "%s: %s %s", errString, devString, message);
	
	return info;
}

char* clhErrorMessageA(const cl_int error,
					   const char* message,
					   const int device) {

	char* outputString = (char*)calloc(SMALL_STRING_SIZE, sizeof(char));
	
	clhErrorMessage(error, outputString, SMALL_STRING_SIZE, message, device);
	
	return outputString;
}

void clhLogErrorMessage(const cl_int error,
						const char* message,
						const char* source,
						const int device,
						FILE* stream) {
						
	char errorMessage[SMALL_STRING_SIZE];
	clhErrorMessage(error, errorMessage, sizeof(errorMessage), message, device);
	
	logMessage(errorMessage, source, stream);
}

////////////////////////////////////////////////////////////////////////////////

// add option to select the platform - currently, the first one is selected
// ? add fallback to device type specified in 'selectedDeviceType'

clhResources clhInitResources(const char* platformName,
							  const cl_device_type deviceType,
							  const int* devicesList,
							  const int devicesListSize,
							  const cl_command_queue_properties cmdQueuesProperties,
							  cl_int* err) {

	cl_int 							retErr;
	cl_platform_id 					platform = NULL;
	cl_device_type 					devType = CL_DEVICE_TYPE_ALL;
	cl_uint 						devCount = 0;
	cl_device_id* 					devices = NULL;
	cl_context 						context = NULL;
	cl_command_queue*				cmdQueues = NULL;
	size_t*							mwgSizes = 0;
	cl_ulong*						memSizes = 0;
#if USE_GL
	Display*						glDisplay = NULL;
	GLXDrawable						glWindow = None;
	GLXContext						glContext = NULL;
	XVisualInfo*					glVisual = NULL;
	GLXFBConfig*					glFBConfig = NULL;
#endif /* USE_GL */
	char 							info[SMALL_STRING_SIZE] = "";

	cl_uint numPlatforms = 0;
	retErr = clGetPlatformIDs(0, NULL, &numPlatforms);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("getting platform count");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}
	printf("Platform count = %d\n", numPlatforms);

	// Find OpenCL platform available
	retErr = clGetPlatformIDs(1, &platform, NULL);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("getting platform");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

	// Get some information about selected platform
	printf("Platform: %s\n", clhGetPlatformInfo(platform, sizeof(info), info, &retErr));
	if(retErr) {
		if(err) *err = retErr;
		return NULL;
	}

	// Find the selected CL device type, this is what we really want
	// If there is no device of selected type fall back to any device type
	if(deviceType)
		devType = deviceType;

	retErr = clGetDeviceIDs(platform, devType, 0, NULL, &devCount);
	printf("Found %d device(s) of selected type\n", devCount);
	if(retErr || devCount < 1 || CLH_VERBOSE) {
		_clhLogErr__("count device(s) of selected type");

		// Can't find any device of selected type
		if(devCount < 1 && devType != CL_DEVICE_TYPE_ALL) {
			devType = CL_DEVICE_TYPE_ALL;
			retErr = clGetDeviceIDs(platform, devType, 0, NULL, &devCount);
			printf("Found %d fallback device(s)\n", devCount);
			if(retErr || CLH_VERBOSE) 
				_clhLogErr__("counting fallback device(s)");
		}

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
		if(devCount < 1) {
			if(err) *err = CL_DEVICE_NOT_FOUND;
			return NULL;
		}
	}

	// Retrieve previously counted devices
	devices = (cl_device_id*)calloc(devCount, sizeof(cl_device_id));
	retErr = clGetDeviceIDs(platform, devType, devCount, devices, NULL);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("getting device(s)");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}
	
	// Select listed devices
	if(devicesListSize > devCount || (devicesListSize > 0 && !devicesList)) {
		if(err) *err = CL_DEVICE_NOT_FOUND;
		return NULL;
	} else {
		cl_device_id* listedDevices = 
				(cl_device_id*)calloc(devicesListSize, sizeof(cl_device_id));
		for(int i = 0; i < devicesListSize; i++)
			listedDevices[i] = devices[devicesList[i]];
		
		free(devices);
		devices = listedDevices;
		devCount = devicesListSize;
	}

	// Get some information about selected devices
	mwgSizes = (size_t*)calloc(devCount, sizeof(size_t));
	memSizes = (cl_ulong*)calloc(devCount, sizeof(cl_ulong));
	for(int i = 0; i < devCount; i++) {
		mwgSizes[i] = clhGetDeviceMaxWorkGroupSize(devices[i], &retErr);
		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}

		memSizes[i] = clhGetDeviceGlobalMemorySize(devices[i], &retErr);
		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

	// Print some information about selected devices
	printf("List of selected devices:\n");
	for(int i = 0; i < devCount; i++) {
		printf("%d: %s\n", i, clhGetDeviceInfo(devices[i], sizeof(info), info, &retErr));

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

#if USE_GL	 
	glDisplay = glXGetCurrentDisplay();
	glWindow = glXGetCurrentDrawable();
	glContext = glXGetCurrentContext();
	
	printf("GL: context=%p, display=%p, window=%s\n", glContext, glDisplay,
			(glWindow != None) ? "ok" : "none");		
	
	printf("equal=%s\n", (glXGetCurrentReadDrawable() == glWindow) ? "true" : "false");

	int attributesList[] =
		{GLX_RGBA,
		GLX_DOUBLEBUFFER,
//		GLX_BUFFER_SIZE, 32,
//		GLX_DEPTH_SIZE, 24,
		None};

	glVisual = glXChooseVisual(glDisplay, XDefaultScreen(glDisplay), attributesList);
	printf("GL: visual=%p\n", glVisual->visual);
	
//	glVisual = XDefaultVisual(glDisplay, XDefaultScreen(glDisplay));
//	printf("GL: visual=%p\n", glVisual->visual);
		
//	glFBConfig = fgChooseFBConfig();
//	glVisual = glXGetVisualFromFBConfig(glDisplay, glFBConfig[0]);
//	printf("GL: visual=%p FBConfig=%p\n", glVisual->visual, glFBConfig[0]);
#endif /* USE_GL */

	// Create context with all devices
	//TODO: add back GL and GLX context
	cl_context_properties contexProperties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
#if USE_GL
			CL_GL_CONTEXT_KHR, (cl_context_properties)glContext,
			CL_GLX_DISPLAY_KHR, (cl_context_properties)glDisplay,
#endif /* USE_GL */
			0
	};

	context = clCreateContext(contexProperties, devCount, devices, NULL, NULL, &retErr);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("creating context");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

	// Create command queues for all context devices
	cmdQueues = (cl_command_queue*)calloc(devCount, sizeof(cl_command_queue));
	for(int i = 0; i < devCount; i++) {
		cmdQueues[i] = clCreateCommandQueue(context, devices[i], cmdQueuesProperties, &retErr);
		if(retErr || CLH_VERBOSE) {
			snprintf(info, SMALL_STRING_SIZE, "creating command queue for device %d", i);
			_clhLogErr__(info);

			if(retErr) {
				for(int j = 0; j < i; j++)
					clReleaseCommandQueue(cmdQueues[j]);
				clReleaseContext(context);
			
				if(err) *err = retErr;
				return NULL;
			}
		}
	}

	clhResources resources = (clhResources)malloc(sizeof(struct _clhResources));
	resources->context = context;
	resources->devCount = devCount;
	resources->devices = devices;
	resources->context = context;
	resources->cmdQueues = cmdQueues;
	resources->mwgSizes = mwgSizes;
	resources->memSizes = memSizes;
#if USE_GL
	resources->glContext = glContext;
	resources->glDisplay = glDisplay;
	resources->glWindow = glWindow;
	resources->glVisual = glVisual;
	resources->glFBConfig = glFBConfig;
#endif /* USE_GL */

	if(err) *err = retErr;

	return resources;
}

cl_int clhFreeResources(clhResources resources) {

	cl_int retErr;
	
	retErr = clReleaseContext(resources->context);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("releasing context");
	
	for(int i = 0; i < resources->devCount; i++)
		retErr |= clReleaseCommandQueue(resources->cmdQueues[i]);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("releasing command queues");
	
	free(resources->mwgSizes);
	free(resources->memSizes);
	free(resources->cmdQueues);
	free(resources->devices);
	free(resources);

	return retErr;
}

cl_int clhRetainResources(clhResources resources) {

	cl_int retErr;
	
	retErr = clRetainContext(resources->context);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("retaining context");
	
	for(int i = 0; i < resources->devCount; i++)
		retErr |= clRetainCommandQueue(resources->cmdQueues[i]);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("retaining command queues");
		
	return retErr;
}

cl_int clhReleaseResources(clhResources resources) {

	cl_int retErr;
	
	retErr = clReleaseContext(resources->context);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("releasing context");
	
	for(int i = 0; i < resources->devCount; i++)
		retErr |= clReleaseCommandQueue(resources->cmdQueues[i]);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("releasing command queues");

	return retErr;
}

////////////////////////////////////////////////////////////////////////////////

// ? handle memory allocation

char* clhGetPlatformInfo(const cl_platform_id platform, const size_t infoSize, char* info, cl_int* err) {

	cl_int 		retErr;
	char 		name[SMALL_STRING_SIZE] = "<platform>";
	char 		vendor[SMALL_STRING_SIZE] = "<vendor>";
	char 		version[SMALL_STRING_SIZE] = "<version>";

//	info = (char*)calloc(SMALL_STRING_SIZE, sizeof(char));

	retErr = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(name), name, NULL);
	retErr |= clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
	retErr |= clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(version), version, NULL);

	if(err) *err = retErr;

	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("getting platform information");
		if(retErr) return info;
	}

	snprintf(info, infoSize, "%s from %s (%s)", name, vendor, version);
	return info;
}

// ? handle memory allocation

char* clhGetDeviceInfo(const cl_device_id device, const size_t infoSize, char* info, cl_int* err) {

	cl_int 			retErr;
	char 			name[SMALL_STRING_SIZE] = "<device>";
	char 			vendor[SMALL_STRING_SIZE] = "<vendor>";
	char 			version[SMALL_STRING_SIZE] = "<version>";
	cl_uint			coresInfo = 0;
	cl_uint			frequencyInfo = 0;
	cl_ulong		globalMem = 0;
	cl_ulong		bufferMem = 0;

//	info = (char *)calloc(SMALL_STRING_SIZE, sizeof(char));

	retErr = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(version), version, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &coresInfo, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &frequencyInfo, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMem, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &bufferMem, NULL);

	if(err) *err = retErr;

	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("getting device information");
		if(retErr) return info;
	}

	snprintf(info, infoSize, "%s from %s (%s, %u computes units at %umhz with %dMB [%dMB])",
			name, vendor, version, coresInfo, frequencyInfo, (int)(globalMem / MB), (int)(bufferMem / MB));
	
	return info;
}

cl_ulong clhGetDeviceGlobalMemorySize(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	cl_ulong	size = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &size, NULL);
	if(retErr || CLH_VERBOSE)
			_clhLogErr__("getting device information: global memory size");

	if(err) *err = retErr;

	return size;
}

cl_ulong clhGetDeviceMaxMemoryAllocSize(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	cl_ulong	size = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &size, NULL);
	if(retErr || CLH_VERBOSE)
			_clhLogErr__("getting device information: max memory alloc size");

	if(err) *err = retErr;

	return size;
}

cl_uint clhGetDeviceAddressBits(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	cl_uint		addrBits = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &addrBits, NULL);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("getting device information: address bits");

	if(err) *err = retErr;

	return addrBits;
}

size_t clhGetDeviceMaxWorkGroupSize(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	size_t		size = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &size, NULL);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("getting device information: maximum work group size");

	if(err) *err = retErr;

	return size;
}

size_t clhGetKernelMaxWorkGroupSize(const cl_kernel kernel, const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	size_t		size = 0;

	retErr = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &size, NULL);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("getting kernel information: maximum work group size");

	if(err) *err = retErr;

	return size;
}

////////////////////////////////////////////////////////////////////////////////

#if CL_VERSION_1_1
// deprecated in OpenCL 1.1
cl_int clhSetCmdQueueProfilingState(const cl_command_queue cmdQueue, const cl_bool enable) {

	cl_int retErr;
	
	retErr = clSetCommandQueueProperty(cmdQueue, CL_QUEUE_PROFILING_ENABLE, enable, NULL);
	if(retErr || CLH_VERBOSE)
			_clhLogErr__("setting profiling state");
	
	return retErr;
}
#endif /* CL_VERSION_1_1 */

char* clhGetEventProfilingInfo(const cl_event event, const size_t infoStrSize, char* infoStr, cl_int* retErr) {

	cl_int		err;
	cl_int		status = CL_QUEUED;
	cl_ulong	queue = 0;
	cl_ulong	submit = 0;
	cl_ulong	start = 0;
	cl_ulong	end = 0;
	
	err = clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS,
			sizeof(cl_int), &status, NULL);
	if(err || CLH_VERBOSE) {
			clhLogErr__(err, "getting event information");
			
			if(err) {
				if(retErr) *retErr = err;
				snprintf(infoStr, infoStrSize, "Invalid event!\n");
				return infoStr;
			}
	}

	err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
			sizeof(cl_ulong), &queue, NULL);
	err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_SUBMIT,
			sizeof(cl_ulong), &submit, NULL);
	err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
			sizeof(cl_ulong), &start, NULL);
	err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
			sizeof(cl_ulong), &end, NULL);
	if(err || CLH_VERBOSE)
			clhLogErr__(err, "getting event profiling information");

	if(err)
		snprintf(infoStr, infoStrSize, 
				"Error while retrieving event information!\n");
	else
		snprintf(infoStr, infoStrSize, 
				" queued > submited: %lluns\n"
				" submited > started: %lluns\n"
				" started > ended: %lluns\n"
				" queued > ended: %lluns\n"
				" status: %sfinished\n",
				(long long unsigned int)(submit - queue),
				(long long unsigned int)(start - submit),
				(long long unsigned int)(end - start),
				(long long unsigned int)(end - queue),
				(status != CL_COMPLETE) ? "not " : "");

	if(retErr) *retErr = err;

	return infoStr;
}

//cl_int mclGetProgramDevices(cl_program program, cl_device_id *devices, cl_int *deviceCount) {
//
//	cl_int err;
//	size_t size = sizeof(cl_device_id);
//
//	err = clGetProgramInfo (program, CL_PROGRAM_DEVICES, 0, NULL, &size);
//	if(VERBOSE || err) {
//		mclErrorInfo(err, "getting program information - program devices", "clHelper");
//		if(err)
//			return err;
//	}
//
//	devices = (cl_device_id *)malloc(size);
//	err = clGetProgramInfo (program, CL_PROGRAM_DEVICES, 0, NULL, &count);
//	if(VERBOSE || err) {
//		mclErrorInfo(err, "getting program information - program devices", "clHelper");
//		if(err)
//			return err;
//	}
//
//}

////////////////////////////////////////////////////////////////////////////////

cl_int clhBuildProgram(cl_program program, const clhResources resources) {

	cl_int				retErr;
	cl_uint				devCount = resources->devCount;
	cl_device_id* 		devices = resources->devices;
	
//	printf("Building program executable\n");
	retErr = clBuildProgram(program, devCount, devices, OPENCL_COMPILER_OPTIONS, NULL, NULL);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("creating program");

		char log[BIG_STRING_SIZE] = "";
		for(int i = 0; i < devCount; i++) {
			printf("Program build log for device %d:\n", i);
			cl_int infoErr = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
			if(!infoErr) printf("%s\n", log);

			if(infoErr || CLH_VERBOSE) 
				clhLogErr__(infoErr, "getting program build info");
		}

	}

	return retErr;
}

cl_program clhBuildProgramFromSource(const char* source, const clhResources resources, cl_int* err) {

	cl_int 			retErr;
	cl_program		program = NULL;

	// Create the compute program from the source
	program = clCreateProgramWithSource(resources->context, 1, (const char**)&source, NULL, &retErr);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("creating program from source");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}
	
	retErr = clhBuildProgram(program, resources);
	
	if(err) *err = retErr;

	return retErr ? NULL : program;
}

cl_program clhBuildProgramFromBinaryFile(const char* path, const clhResources resources, cl_int* err) {

	cl_int 				retErr;
	cl_uint 			devCount = 0;
	cl_program			program = NULL;
	size_t* 			binSizes = NULL;
	unsigned char** 	binaries = NULL;
	
	FILE*				fd = NULL;
	int					szRet = 0;
	
	fd = fopen(path, "rb");
	if (!fd) {
		printf("Error while opening %s file.\n", path);
		retErr = CL_INVALID_VALUE;
		
		goto ret;
	}
	
	szRet = fscanf(fd, "devCount : %u\n", &devCount);
//	printf("devCount : %u\n", devCount);
	binSizes = calloc(devCount, sizeof(size_t));
	binaries = calloc(devCount, sizeof(unsigned char*));
	
	for(int i = 0; i < devCount; i++) {
		szRet = fscanf(fd, "binSize : %zu\n", &binSizes[i]);
//		printf("szRet = %d; binSize : %zu\n", szRet, binSizes[i]);

		binaries[i] = calloc(binSizes[i], sizeof(unsigned char));
		if(fread(binaries[i], sizeof(unsigned char), binSizes[i], fd) != binSizes[i])
			printf("Error while reading %s file.\n", path);
	}
	
	
	// Create the compute program from the source
	program = clCreateProgramWithBinary(resources->context, devCount, resources->devices,
			binSizes, (const unsigned char**)binaries, NULL, &retErr);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("creating program from binary");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}
	
	retErr = clhBuildProgram(program, resources);
	
ret:
	
	free(binSizes);
	for(int i = 0; i < devCount; i++) 
		free(binaries[i]);
	free(binaries);
	
	if(err) *err = retErr;

	return retErr ? NULL : program;
}

cl_int clhStoreProgramToBinaryFile(const cl_program program, const char* path) {
	
	cl_int 				retErr;
	cl_uint 			devCount = 0;
	size_t* 			binSizes;
	unsigned char** 	binaries;

	retErr = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &devCount, NULL);
	if(retErr || CLH_VERBOSE) {
		_clhLogErr__("counting program devices");

		if(retErr) return retErr;
	}
	
	binSizes = calloc(devCount, sizeof(size_t));
	retErr = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
			devCount * sizeof(size_t), binSizes, NULL);
	if(retErr || !binSizes || CLH_VERBOSE) {
		_clhLogErr__("getting program binaries sizes");

		if(retErr) return retErr;
	}
	
	size_t binTotalSize = 0; // is it really necessary?
//	size_t binTotalSizeRet;
	binaries = calloc(devCount, sizeof(unsigned char*));
	for(int i = 0; i < devCount; i++) {
		binaries[i] = calloc(binSizes[i], sizeof(unsigned char));
		binTotalSize += binSizes[i];	
	}
	retErr = clGetProgramInfo(program, CL_PROGRAM_BINARIES, binTotalSize, binaries, NULL /* &binTotalSizeRet */);
	if(retErr || !binaries || CLH_VERBOSE) {
		_clhLogErr__("getting program binaries");

		if(retErr) goto ret;
	}
	
//	printf("devCount = %u, binTotalSize = %zu, binTotalSizeRet = %zu\n", devCount, binTotalSize, binTotalSizeRet);
//	for(int i = 0; i < devCount; i++) {
//		printf("dev%d binSize = %zu\n", i, binSizes[i]);
//		printf("bin=\n%s\n", binaries[i]);
//	}

	
	FILE*		fd = NULL;
	int			szRet = 0;

	fd = fopen(path, "wb");
	if (!fd) {
		printf("Error while creating %s file.\n", path);
		retErr = CL_INVALID_VALUE;
		
		goto ret;
	}
	
	fprintf(fd, "devCount : %u\n", devCount);
	for(int i = 0; i < devCount; i++) {
		fprintf(fd, "binSize : %zu\n", binSizes[i]);
		szRet += fwrite(binaries[i], sizeof(unsigned char), binSizes[i], fd);
	}
	if (szRet < binTotalSize) {
		printf("Error while writing %s file.\n", path);
		retErr = CL_INVALID_VALUE;
	}
	
	fclose(fd);
	
ret:
	
	free(binSizes);
	for(int i = 0; i < devCount; i++) 
		free(binaries[i]);
	free(binaries);
	
	return retErr;
}
	
unsigned char* clhLoadFile(const char* path) {

	FILE* 				fd = NULL;
	size_t 				fz = 0;
	unsigned char* 		src = NULL;

	if (!(fd = fopen(path, "r"))) {
		printf("Error while opening %s file.\n", path);
		goto ret;
	}

	// determine file size
	if(fseek(fd, 0, SEEK_END)) {
		printf("Error while operating on %s file.\n", path);
		goto ret;
	}
	fz = ftell(fd);
	rewind(fd);

	src = (unsigned char*)malloc(fz + 1);
	if(fread(src, sizeof(unsigned char), fz, fd) != fz) {
		printf("Error while reading %s file.\n", path);
		free(src);
		goto ret;
	}
	src[fz] = '\0';

ret:

	fclose(fd);

	return src;
}

cl_program clhBuildProgramFromFile(const char* sourceFile,
								   const char* kernelSrcPath,
								   const char* kernelBinPath,
   								   const char* kernelBinExt,
								   const clhResources resources,
								   const int loadBin,
								   const int storeBin,
								   cl_int* err) {

	cl_int			retErr;
	cl_program		program = NULL;
	char 			path[SMALL_STRING_SIZE] = "";

	if(loadBin) {
		snprintf(path, sizeof(path), "%s/%s.%s", kernelBinPath ?
				kernelBinPath : KERNELS_BIN_PATH, sourceFile, kernelBinExt ?
				kernelBinExt : KERNELS_BIN_EXT);
		printf("Creating program from binary file '%s'.\n", path);
		program = clhBuildProgramFromBinaryFile(path, resources, &retErr);
		if(!retErr) goto ret;

		printf("Failed to create program from binary file, using source instead.\n");
	}

	snprintf(path, sizeof(path), "%s/%s", kernelSrcPath ?
			kernelSrcPath : KERNELS_SRC_PATH, sourceFile);
	printf("Creating program from souce file '%s'.\n", path);
	char* source = (char*)clhLoadFile(path);
	if(!source){
		printf("Failed to load source from file!\n");
		retErr = CL_INVALID_VALUE;

		goto ret;
	}

	program = clhBuildProgramFromSource(source, resources, &retErr);
	free(source);
	if(retErr) goto ret;
	
	if(storeBin) {
		snprintf(path, sizeof(path), "%s/%s.%s", KERNELS_BIN_PATH, sourceFile, KERNELS_BIN_EXT);
		if((retErr = clhStoreProgramToBinaryFile(program, path)))
			printf("Failed to store program binary file in '%s'.\n", path);
		else
			printf("Stored program binary file in '%s'.\n", path);
	}
	
ret:
	
	if(err) *err = retErr;
	
	return program;
}

#if USE_GL
////////////////////////////////////////////////////////////////////////////////

GLuint clhCreateVBO(const GLsizei size) {

	// ID of VBO
	GLuint id;

	// generate a new VBO and get the associated ID
//	glGenBuffersARB(1, &id);
	glGenBuffers(1, &id);

	// bind VBO in order to use
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
	glBindBuffer(GL_ARRAY_BUFFER, id);

	// upload data to VBO
//	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);

	// delete VBO when program terminated
//	glDeleteBuffersARB(1, &vboId);
//	glDeleteBuffers(1, &vboId);

	return id;

}

cl_mem clhBindNewCLBufferToVBO(const cl_context context,
							   const GLuint vbo, cl_int* err) {

	cl_int		retErr;
	cl_mem		vboBuffer;

	vboBuffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, vbo, &retErr);
	if(retErr || CLH_VERBOSE)
		_clhLogErr__("binding new CL buffer to VBO");

	if(err) *err = retErr;

	return vboBuffer;

}

cl_mem clhCreateGLCLBuffer(const cl_context context, const size_t size,
						   GLuint* vbo, cl_int* err) {

//	printf("context: %p, size: %zu, vbo: %p, err: %p\n",
//			context, size, vbo, err);
	
	cl_int		retErr;
	GLuint		retVBO;
	cl_mem		buffer;

	retVBO = clhCreateVBO(size);
	
	buffer = clhBindNewCLBufferToVBO(context, retVBO, &retErr);

	if(err) *err = retErr;
	if(vbo) *vbo = retVBO;

	return buffer;

}
#endif /* USE_GL */

