/*
 * cc.cl
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: 
 */

#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable

#define TRUE 1
#define FALSE 0
#define BACKGROUND 0
#define NEIGHBORHOOD 6

inline size_t getAddress(const int x,
						 const int y,
						 const int z,
						 const size_t sizeX,
						 const size_t sizeY) {
	
	return (size_t)(x + y * sizeX + z * sizeX * sizeY);
}

inline int4 getCoordinates(const size_t position,
						   const size_t sizeX,
						   const size_t sizeY) {
	
	int4 coordinates;
	uint area = sizeX * sizeY;
	
	coordinates.x = position % sizeX;
	coordinates.y = (position % area) / sizeX;
	coordinates.z = position / area;
	coordinates.w = 0;
	
	return coordinates;
	
}

inline void compareNeighbors(const uint oSeg,
							 const uint nSeg,
							 uint* labelPtr,
							 local uint* lLabels,
							 const uint nLabelAddr) {

	if(oSeg == nSeg)
		*labelPtr = min(*labelPtr, lLabels[nLabelAddr]);

}

inline uint findRoot(local uint* parentTree,
					uint node) {

	uint parentNode = parentTree[node];
	
	while(node > parentNode) {
		node = parentNode;
		parentNode = parentTree[node];
	}
	
	return node;
}

kernel void cclKernel1(global size_t* gLabels,
					   global uint* gSegments) {
	
	int lX = get_local_id(0);
	int lY = get_local_id(1);
	int lZ = get_local_id(2);
	
	uint lSizeX = get_local_size(0);
	uint lSizeY = get_local_size(1);
	uint lSizeZ = get_local_size(2);

	uint lIndex = getAddress(lX + 1, lY + 1, lZ + 1, lSizeX + 2, lSizeY + 2);

	/*int gX = get_global_id(0);
	int gY = get_global_id(1);
	int gZ = get_global_id(2);
	
	uint gSizeX = get_global_size(0);
	uint gSizeY = get_global_size(1);
	uint gSizeZ = get_global_size(2);
	
	size_t gIndex = getAddress(gX, gY, gZ, gSizeX, gSizeY);*/
	size_t gIndex = getAddress(get_global_id(0), get_global_id(1),
			get_global_id(2), get_global_size(0), get_global_size(1))
	
	//args?
	uint lRow = lSizeX + 2;
	uint lSlice = lRow * (lSizeY + 2);
	uint lSize = lSlice * (lSizeZ + 2);
	
	local int lChanged = FALSE;
	local uint lSegments[lSize];
	local uint lLabels[lSize];
	
	if(lX == 0) {
		lSegments[lIndex - 1] = BACKGROUND;
		lLabels[lIndex - 1] = BACKGROUND;
	}
	
	if(lX == lSizeX - 1) {
		lSegments[lIndex + 1] = BACKGROUND;
		lLabels[lIndex + 1] = BACKGROUND;
	}
		
	if(lY == 0) {
		lSegments[lIndex - lRow] = BACKGROUND;
		lLabels[lIndex - lRow] = BACKGROUND;
		
		/*if(lX == 0) {
			lSegments[lIndex - 1 - lRow] = BACKGROUND;
			lLabels[lIndex - 1 - lRow] = BACKGROUND;
			
			if(lZ == 0)
				for(uint i = 0; i < lSlice; i++) {
					lSegments[i] = BACKGROUND;
					lLabels[i] = BACKGROUND;
				}
		}
		
		if(lX == lSizeX - 1) {
			lSegments[lIndex + 1 - lRow] = BACKGROUND;
			lLabels[lIndex + 1 - lRow] = BACKGROUND;
		}*/
	}
	
	if(lY == lSizeY - 1) {
		lSegments[lIndex + lRow] = BACKGROUND;
		lLabels[lIndex + lRow] = BACKGROUND;	
		
		/*if(lX == 0) {
			lSegments[lIndex - 1 + lRow] = BACKGROUND;
			lLabels[lIndex - 1 + lRow] = BACKGROUND;
		}
		
		if(lX == lSizeX - 1) {
			lSegments[lIndex + 1 + lRow] = BACKGROUND;
			lLabels[lIndex + 1 + lRow] = BACKGROUND;
			
			if(lZ == lSizeZ - 1)			
				for(uint i = lSlice; i > 0; i--) {
					lSegments[sharedMemSize - i] = BACKGROUND;
					lLabels[sharedMemSize - i] = BACKGROUND;
				}
		}*/
	}
	
	if(lZ == 0) {
		lSegments[lIndex - lSlice] = BACKGROUND;
		lLabels[lIndex - lSlice] = BACKGROUND;
	}
	
	if(lZ == lSizeZ - 1) {
		lSegments[lIndex + lSlice] = BACKGROUND;
		lLabels[lIndex + lSlice] = BACKGROUND;
	}
	
	uint nSegments[NEIGHBORHOOD];
	uint ownSegment = gSegments[gIndex];

	lSegments[lIndex] = ownSegment;
	
	barrier(CLK_LOCAL_MEM_FENCE);
//	mem_fence(CLK_LOCAL_MEM_FENCE);
	
	if(ownSegment != BACKGROUND) {
		nSegments[0] = lSegments[lIndex + lSlice]; // top
		nSegments[1] = lSegments[lIndex - lRow]; // ahead
		nSegments[2] = lSegments[lIndex + 1]; // right
		nSegments[3] = lSegments[lIndex + lRow]; // behind
		nSegments[4] = lSegments[lIndex - 1]; // left
		nSegments[5] = lSegments[lIndex - lSlice]; // bottom
	}
	
	uint currLabel = lIndex;
	uint prevLabel = BACKGROUND;
	
	while() {
		if(!(lX | lY | lZ)) lChanged = FALSE;
		lLabels[lIndex] = currLabel;
		prevLabel = currLabel;
		
		barrier(CLK_LOCAL_MEM_FENCE);
//		mem_fence(CLK_LOCAL_MEM_FENCE);
		
		if(ownSegment != BACKGROUND) {
			compareNeighbors(ownSegment, nSegments[0], &currLabel, lLabels, lIndex + lSlice);
			compareNeighbors(ownSegment, nSegments[1], &currLabel, lLabels, lIndex - lRow);
			compareNeighbors(ownSegment, nSegments[2], &currLabel, lLabels, lIndex + 1);
			compareNeighbors(ownSegment, nSegments[3], &currLabel, lLabels, lIndex + lRow);
			compareNeighbors(ownSegment, nSegments[4], &currLabel, lLabels, lIndex - 1);
			compareNeighbors(ownSegment, nSegments[5], &currLabel, lLabels, lIndex - lSlice);
		}
		
//		barrier(CLK_LOCAL_MEM_FENCE);
//		mem_fence(CLK_LOCAL_MEM_FENCE);
		
		if(prevLabel > currLabel) {
			atom_min(lLabels + prevLabel, currLabel);
			lChanged = TRUE;
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
//		mem_fence(CLK_LOCAL_MEM_FENCE);
		
		if(lChanged == FALSE) break;
		
		if(ownSegment != BACKGROUND)
			currLabel = findRoot(lLabels, newLabel);
		
		barrier(CLK_LOCAL_MEM_FENCE);
//		mem_fence(CLK_LOCAL_MEM_FENCE);
	}
	
	size_t label = BACKGROUND;
	if(ownSegment != BACKGROUND) {
		uint index = getAddress(lX, lY, lZ, lSizeX, lSizeY);
		
		int4 labelCoordinates = getCoordinates(currLabel, lSizeX + 2, lSizeY + 2);
		uint labelIndex = getAddress(labelCoordinates.x - 1,
				labelCoordinates.y - 1, labelCoordinates.z - 1, lSizeX, lSizeY);
		
		label = gIndex - index + labelIndex + 1; // +1 (shift) because label = 0 => BACKGROUND
	}
	
	gLabels[gIndex] = label;
}

inline void mergeTree() {

}

kernel void cclKernel2(global size_t* gLabels) {

}

