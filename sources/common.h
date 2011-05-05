/*
 * common.h
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#ifndef COMMON_H_
#define COMMON_H_

#define FALSE 0
#define TRUE !FALSE

#define DISABLE 0
#define ENABLE !DISABLE

#define EXIT_ON_ERROR 0
#define USE_GL TRUE

#define KB 1024 // 1024^1
#define MB 1048576 // 1024^2
#define GB 1073741824 // 1024^3

#define SMALL_STRING_SIZE 1 * KB
#define MEDDIUM_STRING_SIZE 10 * KB
#define BIG_STRING_SIZE 100 * KB

#define KERNELS_SRC_PATH "sources/kernels"
#define KERNELS_BIN_PATH "kernels"
#define KERNELS_BIN_EXT "bin"


#endif /* COMMON_H_ */
