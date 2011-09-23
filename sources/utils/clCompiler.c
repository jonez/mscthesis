/*
 * compiler.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */


#include "../common.h"
#include "../clHelper.h"

////////////////////////////////////////////////////////////////////////////////

static int CLC_VERBOSE = TRUE;

int clcGetVerbose() {
	return CLC_VERBOSE;
}

void clcSetVerbose(const int state) {
	CLC_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	
	cl_int clErr;
	
	char* sourceFile = argv[1];
	
	clhResources resources = clhInitResources(NULL, CL_DEVICE_TYPE_GPU, 0, &clErr);
	if(clErr || CLC_VERBOSE) {
		clhErrorInfo(clErr, "initializing resources", __FILE__);

		if(clErr) return ERROR;
	}
	
	cl_program program = clhBuildProgramFromFile(sourceFile, resources, FALSE, FALSE, &clErr);
	if(clErr || CLC_VERBOSE) {
		clhErrorInfo(clErr, "compiling source file", __FILE__);

		if(clErr) return ERROR;
	}

	return SUCCESS;
}
