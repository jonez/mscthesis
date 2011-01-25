/*
 * clHelper.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */
#include <GL/glew.h>

#include "clHelper.h"

// use with #include <GL/glext.h>
//#define GL_GLEXT_PROTOTYPES

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glx.h>
//#include <GL/glext.h>
//#include <CL/cl_gl.h>

#include "common.h"

////////////////////////////////////////////////////////////////////////////////

int CLH_VERBOSE = FALSE;

int clhGetVerbose() {
	return CLH_VERBOSE;
}

void clhSetVerbose(const int state) {
	CLH_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

void clhErrorInfo(const cl_int error, const char* info, const char* source) {

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
		printf("%s : %s [@%s]\n", errorList[index], info, source);
	else
		printf("ID ERROR %d : %s [@%s]\n", error, info, source);

}

void clhErrorCheck(const cl_int error, const char* info, const char* source) {

	if(error != CL_SUCCESS) {
		clhErrorInfo(error, info, source);

		exit(EXIT_FAILURE);
	}

}

// add option to select the platform - now, the first is selected
// ? add fallback to device type specified in 'selectedDeviceType'

clhResources clhInitResources(const char* platformName, const cl_device_type selectedDeviceType, const cl_command_queue_properties cmdQueuesProperties, cl_int* err) {

	cl_int 							retErr;
	cl_platform_id 					platform = NULL;
	cl_device_type 					devType = CL_DEVICE_TYPE_ALL;
	cl_uint 						devCount = 0;
	cl_device_id 					*devices = NULL;
	cl_context 						context = NULL;
	cl_command_queue				*cmdQueues = NULL;
	size_t*							wgSizes;
	cl_ulong*						memSizes;
	char 							info[SMALL_STRING_SIZE] = "";


	// Find OpenCL platform available
	retErr = clGetPlatformIDs(1, &platform, NULL);
	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "getting platform", "clHelper");

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
	if(selectedDeviceType)
		devType = selectedDeviceType;

	retErr = clGetDeviceIDs(platform, devType, 0, NULL, &devCount);
	printf("Found %d device(s) of selected type\n", devCount);
	if(retErr || !devCount || CLH_VERBOSE) {
		clhErrorInfo(retErr, "count device(s) of selected type", "clHelper");

		// Can't find any device of selected type
		if(!devCount && devType != CL_DEVICE_TYPE_ALL) {
			devType = CL_DEVICE_TYPE_ALL;
			retErr = clGetDeviceIDs(platform, devType, 0, NULL, &devCount);
			printf("Found %d fallback device(s)\n", devCount);
			if(retErr || CLH_VERBOSE) clhErrorInfo(retErr, "counting fallback device(s)", "clHelper");
		}

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
		if(!devCount) {
			if(err) *err = CL_DEVICE_NOT_FOUND;
			return NULL;
		}
	}

	// Retrieve previously counted devices
	devices = (cl_device_id*)calloc(devCount, sizeof(cl_device_id));
	retErr = clGetDeviceIDs(platform, devType, devCount, devices, NULL);
	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "getting device(s)", "clHelper");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

	// Get some information about selected devices
	wgSizes = (size_t*)calloc(devCount, sizeof(size_t));
	memSizes = (cl_ulong*)calloc(devCount, sizeof(cl_ulong));
	for(int i = 0; i < devCount; i++) {
		wgSizes[i] = clhGetDeviceMaxWorkGroupSize(devices[i], &retErr);
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
	printf("List of devices:\n");
	for(int i = 0; i < devCount; i++) {
		printf("%d: %s\n", i, clhGetDeviceInfo(devices[i], sizeof(info), info, &retErr));

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

	// Create context with all devices
	cl_context_properties contexProperties[] = {
			 CL_CONTEXT_PLATFORM, (cl_context_properties)(platform),
			 CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			 CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
			 0
	 };

	context = clCreateContext(contexProperties, devCount, devices, NULL, NULL, &retErr);
	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "creating context", "clHelper");

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
			clhErrorInfo(retErr, info, "clHelper");

			if(retErr) {
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
	resources->wgSizes = wgSizes;
	resources->memSizes = memSizes;

	if(err) *err = retErr;

	return resources;
}


// ? handle memory allocation

char* clhGetPlatformInfo(const cl_platform_id platform, const size_t infoSize, char* info, cl_int* err) {

	cl_int 		retErr;
	char 		name[SMALL_STRING_SIZE] = "";
	char 		vendor[SMALL_STRING_SIZE] = "";
	char 		version[SMALL_STRING_SIZE] = "";

//	info = (char*)calloc(SMALL_STRING_SIZE, sizeof(char));

	retErr = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(name), name, NULL);
	retErr |= clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
	retErr |= clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(version), version, NULL);

	if(err) *err = retErr;

	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "getting platform information", "clHelper");
		if(retErr) return NULL;
	}

	snprintf(info, infoSize, "%s from %s (%s)", name, vendor, version);
	return info;
}

// ? handle memory allocation

char* clhGetDeviceInfo(const cl_device_id device, const size_t infoSize, char* info, cl_int* err) {

	cl_int 			retErr;
	char 			name[SMALL_STRING_SIZE] = "";
	char 			vendor[SMALL_STRING_SIZE] = "";
	char 			version[SMALL_STRING_SIZE] = "";
	cl_uint			coresInfo = 0;
	cl_uint			frequencyInfo = 0;
	cl_ulong		globalMem = 0;
	cl_ulong		bufferMem = 0;

//	info = (char *)calloc(SMALL_STRING_SIZE, sizeof(char));

	retErr = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(version), version, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(coresInfo), &coresInfo, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequencyInfo), &frequencyInfo, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(globalMem), &globalMem, NULL);
	retErr |= clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(bufferMem), &bufferMem, NULL);

	if(err) *err = retErr;

	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "getting device information", "clHelper");
		if(retErr) return NULL;
	}

	snprintf(info, infoSize, "%s from %s (%s, %u computes units at %umhz with %dMB [%dMB])", name, vendor, version, coresInfo, frequencyInfo, (int)(globalMem / MB), (int)(bufferMem / MB));
	return info;
}

size_t clhGetDeviceMaxWorkGroupSize(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	size_t		size = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size), &size, NULL);
	if(retErr || CLH_VERBOSE)
		clhErrorInfo(retErr, "getting device information: maximum work group size", "clHelper");

	if(err) *err = retErr;

	return size;
}

cl_ulong clhGetDeviceGlobalMemorySize(const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	cl_ulong	size = 0;

	retErr = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size), &size, NULL);
	if(retErr || CLH_VERBOSE)
			clhErrorInfo(retErr, "getting device information: global memory size", "clHelper");

	if(err) *err = retErr;

	return size;
}

size_t clhGetKernelMaxWorkGroupSize(const cl_kernel kernel, const cl_device_id device, cl_int* err) {

	cl_int		retErr;
	size_t		size = 0;

	retErr = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size), &size, NULL);
	if(retErr || CLH_VERBOSE)
		clhErrorInfo(retErr, "getting kernel information: maximum work group size", "clHelper");

	if(err) *err = retErr;

	return size;
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

cl_program clhBuildProgramFromSource(const char* source, clhResources resources, cl_int* err) {

	cl_int 			retErr;
	cl_program		program = NULL;
	char 			log[BIG_STRING_SIZE] = "";

	// Create the compute program from the source
	program = clCreateProgramWithSource(resources->context, 1, (const char **)&source, NULL, &retErr);
	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "creating program from source", "clHelper");

		if(retErr) {
			if(err) *err = retErr;
			return NULL;
		}
	}

//	printf("Building program executable\n");
	retErr = clBuildProgram(program, resources->devCount, resources->devices, OPENCL_COMPILER_OPTIONS, NULL, NULL);
	if(retErr || CLH_VERBOSE) {
		clhErrorInfo(retErr, "creating program", "clHelper");

		for(int i = 0; i < resources->devCount; i++) {
			printf("Program build log for device %d:\n", i);
			cl_int infoErr = clGetProgramBuildInfo(program, resources->devices[i], CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
			if(!infoErr) printf("%s\n", log);

			if(infoErr || CLH_VERBOSE) clhErrorInfo(infoErr, "getting program build info", "clHelper");
		}

	}

	if(err) *err = retErr;

	return program;
}

char* clhLoadSourceFile(const char* fn) {

	FILE* 		fd = NULL;
	size_t 		fz = 0;
	char* 		src = NULL;

	char path[SMALL_STRING_SIZE] = "";
	snprintf(path, sizeof(path), "%s%s", KERNELS_PATH, fn);

	// open file for reading
	fd = fopen(path, "r");
	if (!fd) {
		printf("Error while opening %s file.\n", path);
		return NULL;
	}

	// determine file size

	//if(fseek(fd, 0, SEEK_END) == 0) {
	//	printf("Error while operating on %s file.\n", path);
	//	return NULL;
	//}
	fseek(fd, 0, SEEK_END);
	fz = ftell(fd);
	rewind(fd);

	src = (char*)malloc(fz + 1);
	if(fread(src, sizeof(char), fz, fd) != fz) {
		printf("Error while reading %s file.\n", path);
		return NULL;
	}
	src[fz] = '\0';

	fclose(fd);

	return src;
}

cl_program clhBuildProgramFromFile(const char *sourceFile, clhResources resources, cl_int* err) {

	printf("Creating program from file '%s'.\n", sourceFile);
	char* source = clhLoadSourceFile(sourceFile);
	if(!source){
		printf("Failed to load kernel source from file!\n");

		if(err) *err = CL_INVALID_VALUE;
		return NULL;
	}

//	free(source);

	return clhBuildProgramFromSource(source, resources, err);

}

GLuint clhCreateVBO(GLsizei size) {

	// ID of VBO
	GLuint		id;

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

cl_mem clhBindNewCLBufferToVBO(cl_context context, size_t size, GLuint vbo,
							   cl_int* err) {

	cl_int		retErr;
	cl_mem		vboBuffer;

	vboBuffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, vbo, &retErr);
	if(retErr || CLH_VERBOSE)
		clhErrorInfo(retErr, "binding new CL buffer to VBO", "clHelper");

	if(err) *err = retErr;

	return vboBuffer;

}

cl_mem clhCreateGLCLBuffer(const cl_context context, const size_t size,
						   GLuint* vbo, cl_int* err) {

	cl_int		retErr;
	GLuint		retVBO;
	cl_mem		buffer;


	retVBO = clhCreateVBO(size);

	buffer = clhBindNewCLBufferToVBO(context, size, retVBO, &retErr);

	if(err) *err = retErr;
	if(vbo) *vbo = retVBO;

	return buffer;

}
