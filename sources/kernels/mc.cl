/*
 * mc.cl
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: use built-in vectores when possible
 *		  use inline functions or macros
 *		  use local memory to store corner values
 *		  interleave load/save with logical operations
 */

#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable

#define TXT_COORD_OFFSET 0.5f

#define EMPTY_VOXEL 0 // non relevant voxel
#define OCCUPIED_VOXEL 1 // relevant voxel

#define VOXEL_VERTICES 16 // vertices per voxel
#define VOXEL_TRIANGLES 5 // triangles per voxel
#define TRIANGLE_VERTICES 3 // vertices per triangle

// get 1D position from 3D position
//inline size_t getPosition(const int4 coordinates, const uint2 sizes) {
//	
//	return (size_t)(coordinates.x +
//					coordinates.y * sizes.x +
//					coordinates.z * sizes.x * sizes.y);
//
//}
inline size_t getPosition(const int x,
						  const int y,
						  const int z,
						  const size_t sizeX,
						  const size_t sizeY) {
	
	return (size_t)(x + y * sizeX + z * sizeX * sizeY);
}

// get a single value from 3D data set 'values'
//inline float getValue(const float4 coordinates,
//					  read_only image3d_t values, const sampler_t sampler) {
//	
//	float4 adjCoords = coordinates + TXT_COORD_OFFSET;
//	return read_imagef(values, sampler, adjCoords).x;
//}
inline float getValue(read_only image3d_t values,
					  const sampler_t sampler,
					  const float4 buffers,
					  const float x,
					  const float y,
					  const float z) {
	
	float4 coordinates = (float4)(x, y, z, 0.0f);
	coordinates.xyz += TXT_COORD_OFFSET;
	coordinates += buffers;
	
	return read_imagef(values, sampler, coordinates).x;
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two float arrays, 'vVoxels' containing the number of 
// vertices per voxel and 'oVoxels' which indicates if a voxel is relevant/
// occupied (1) or not relevant/empty (0)
kernel void mcClassification(read_only image3d_t values,
							 const sampler_t valuesSampler,
							 const float4 valuesBuffers,
							 const float isoValue,
							 constant uchar* vTable,
							 global float* vVoxels,
							 global float* oVoxels) {
	
	// int4?
	// get coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	
	float xf = x;
	float yf = y;
	float zf = z;
	float xfi = xf + 1.0f;
	float yfi = yf + 1.0f;
	float zfi = zf + 1.0f;
	
	// values of voxel vertices
	float corners[8];
	corners[0] = getValue(values, valuesSampler, valuesBuffers, xf,  yf,  zf );
	corners[1] = getValue(values, valuesSampler, valuesBuffers, xfi, yf,  zf );
	corners[2] = getValue(values, valuesSampler, valuesBuffers, xfi, yfi, zf );
	corners[3] = getValue(values, valuesSampler, valuesBuffers, xf,  yfi, zf );
	corners[4] = getValue(values, valuesSampler, valuesBuffers, xf,  yf,  zfi);
	corners[5] = getValue(values, valuesSampler, valuesBuffers, xfi, yf,  zfi);
	corners[6] = getValue(values, valuesSampler, valuesBuffers, xfi, yfi, zfi);
	corners[7] = getValue(values, valuesSampler, valuesBuffers, xf,  yfi, zfi);
	
	// voxel index combination
	uchar combination = 0;
	combination =  (uint)(corners[0] < isoValue); 
	combination += (uint)(corners[1] < isoValue) * 2; 
	combination += (uint)(corners[2] < isoValue) * 4; 
	combination += (uint)(corners[3] < isoValue) * 8; 
	combination += (uint)(corners[4] < isoValue) * 16; 
	combination += (uint)(corners[5] < isoValue) * 32; 
	combination += (uint)(corners[6] < isoValue) * 64; 
	combination += (uint)(corners[7] < isoValue) * 128;
	
	// ulong2?
	// work-item and data set sizes
	size_t sizeX = get_global_size(0);
	size_t sizeY = get_global_size(1);
	
	size_t outputPosition = getPosition(x, y, z, sizeX, sizeY);
	
	// output two arrays
	uchar vertices = vTable[combination];
	vVoxels[outputPosition] = vertices;
	oVoxels[outputPosition] = (vertices > 0) ? OCCUPIED_VOXEL : EMPTY_VOXEL;
	
}

// create an array with relevant data only (occupied voxels)
kernel void mcCompaction(global float* values,
						 global float* scannedValues,
						 /*const size_t size,*/
						 global size_t* result) {
	
	// get coordinates
//	size_t size = get_global_size(0);
	size_t position = get_global_id(0);
	size_t compactedPosition = scannedValues[position];
	
	if(values[position] != EMPTY_VOXEL/* && (getPosition < size)*/)
		result[compactedPosition] = position;
	
}

// get 3D coordinates from 1D position
inline int4 getCoordinates(const size_t position,
						   const uint2 sizes) {
	
	int4 coordinates;
	uint area = sizes.x * sizes.y;
	
	coordinates.x = position % sizes.x;
	coordinates.y = (position % area) / sizes.x;
	coordinates.z = position / area;
	coordinates.w = 0;
	
	return coordinates;
	
}

// cumpute linear interpolation of two vertices
inline float4 vertexInterpolation(float iso,
								  float4 v1,
								  float4 v2) {

	float r = (iso - v1.w) / (v2.w - v1.w);
	return (float4)(mix(v1, v2, r).xyz, 1.0f);

}

// compute normal vector of v1, v2, v3
//inline float4 triangleNormal(float4 v1, float4 v2, float4 v3) {
//	
//	float4 e1 = v2 - v1;
//	float4 e2 = v3 - v1;
//	
//	return cross(e1, e2);
//	
//}

// compute normal vector of triangle 't'
//inline float4 triangleNormal(float4* t) {
//	
//	float4 e1 = t[1] - t[0];
//	float4 e2 = t[2] - t[0];
//	
//	return cross(e1, e2);
//	
//}

inline float4 vertexNormal(read_only image3d_t values,
							const sampler_t sampler,
							const float4 buffers,
							const float4 v) {
	
	float4 n;

	n.x = getValue(values, sampler, buffers, v.x + 1.0f,  v.y,  v.z) -
			getValue(values, sampler, buffers, v.x - 1.0f,  v.y, v.z);
	n.y = getValue(values, sampler, buffers, v.x, v.y + 1.0f, v.z) -
			getValue(values, sampler, buffers, v.x, v.y - 1.0f, v.z);
	n.z = getValue(values, sampler, buffers, v.x, v.y, v.z + 1.0f) -
			getValue(values, sampler, buffers, v.x, v.y, v.z - 1.0f);
	n.w = 0;
	
	return -normalize(n);
	
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'tOutput' (float4) containing all iso-
// surface vertices in groups of 3 (a triangle) and 'nOutput' (float4)
// containing a normal vector for each triangle (or 3 vertices)
kernel void mcGeneration(read_only image3d_t values, 
						 const sampler_t valuesSampler,
						 const ulong count,
						 const float isoValue,
						 const uint4 voxelSizes,
						 const float4 valuesDistances,
						 const float4 valuesOffsets,
						 const float4 valuesBuffers,
						 constant uchar* tTable,
						 constant uchar* vTable,
						 global float* scanned, 
						 global size_t* compacted,
						 global float4* tOutput,
						 global float4* nOutput) {
	
	
	size_t rawPosition = get_global_id(0);
	
	if(rawPosition < count) {
	
		size_t position = compacted[rawPosition];
		int4 coordinates = getCoordinates(position, voxelSizes.xy);
			
		// kernel and values coordinates
		float xf = coordinates.x;
		float yf = coordinates.y;
		float zf = coordinates.z;
		float xfi = xf + 1.0f;
		float yfi = yf + 1.0f;
		float zfi = zf + 1.0f;
	
		// voxel coordinates in float data type
//		float xfv = xf + valuesOffsets.x;
//		float yfv = yf + valuesOffsets.y;
//		float zfv = zf + valuesOffsets.z;
//		float xfvi = xfv + 1.0f;
//		float yfvi = yfv + 1.0f;
//		float zfvi = zfv + 1.0f;
		
		// work-item and dataset sizes
//		size_t sX = size.x;
//		size_t sY = size.y;
//		size_t siX = sX + 1;
//		size_t siY = sY + 1;
		
		
		// coordinates and values of voxel vertices
		float4 corners[8];
		corners[0] = (float4)(xf,  yf,  zf,  getValue(values, valuesSampler, valuesBuffers, xf,  yf,  zf ));
		corners[1] = (float4)(xfi, yf,  zf,  getValue(values, valuesSampler, valuesBuffers, xfi, yf,  zf ));
		corners[2] = (float4)(xfi, yfi, zf,  getValue(values, valuesSampler, valuesBuffers, xfi, yfi, zf ));
		corners[3] = (float4)(xf,  yfi, zf,  getValue(values, valuesSampler, valuesBuffers, xf,  yfi, zf ));
		corners[4] = (float4)(xf,  yf,  zfi, getValue(values, valuesSampler, valuesBuffers, xf,  yf,  zfi));
		corners[5] = (float4)(xfi, yf,  zfi, getValue(values, valuesSampler, valuesBuffers, xfi, yf,  zfi));
		corners[6] = (float4)(xfi, yfi, zfi, getValue(values, valuesSampler, valuesBuffers, xfi, yfi, zfi));
		corners[7] = (float4)(xf,  yfi, zfi, getValue(values, valuesSampler, valuesBuffers, xf,  yfi, zfi));
		
		
		// voxel index combination
		uchar combination = 0;
		combination =  (uint)(corners[0].w < isoValue);
		combination += (uint)(corners[1].w < isoValue) * 2;
		combination += (uint)(corners[2].w < isoValue) * 4;
		combination += (uint)(corners[3].w < isoValue) * 8;
		combination += (uint)(corners[4].w < isoValue) * 16;
		combination += (uint)(corners[5].w < isoValue) * 32;
		combination += (uint)(corners[6].w < isoValue) * 64;
		combination += (uint)(corners[7].w < isoValue) * 128;
		
		
		// vertices from voxel edges, even if the coordinates are located
		// outside of edges
		float4 vertices[12];
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
		
		
		// fill output data in form of triangles and their normal vector
		// each iteration creates one triangle 
		uint verticesCount = vTable[combination];
		constant uchar* edges = &tTable[combination * VOXEL_VERTICES]; // all edge values should be in private (or at least local) memory
		
		size_t outputPosition = scanned[position];
	
		for(uint v = 0; v < verticesCount; v += TRIANGLE_VERTICES) {
		
			size_t triangleOutputPosition = outputPosition + v;
//			size_t normalPosition = (voxelOutputPosition / TRIANGLE_VERTICES) + n;
			
	//		uchar triangleEdges[3] = {edges[v], edges[v + 1], edges[v + 2]}; // debug purposes
	//		float4 triangle[3] = {vertices[triangleEdges[0]], vertices[triangleEdges[1]], vertices[triangleEdges[2]]}; // debug purposes
//			float4 triangle[3] = {vertices[edges[v]], vertices[edges[v + 1]], vertices[edges[v + 2]]};
		
	//		triangle[0].w = triangleEdges[0]; // debug purposes
	//		triangle[0].w = triangleOutputPosition; // debug purposes
			tOutput[triangleOutputPosition] = vertices[edges[v]] * valuesDistances + valuesOffsets;
//			nOutput[triangleOutputPosition] = verticesNormal[edges[v]];
			nOutput[triangleOutputPosition] = vertexNormal(values, valuesSampler, valuesBuffers, vertices[edges[v]]);
			
	//		triangle[1].w = triangleEdges[1]; // debug purposes
	//		triangle[1].w = combination; // debug purposes
			tOutput[triangleOutputPosition + 1] = vertices[edges[v + 1]] * valuesDistances + valuesOffsets;
//			nOutput[triangleOutputPosition + 1] = verticesNormal[edges[v + 1]];
			nOutput[triangleOutputPosition + 1] = vertexNormal(values, valuesSampler, valuesBuffers, vertices[edges[v + 1]]);
			
	//		triangle[2].w = triangleEdges[2]; // debug purposes
	//		triangle[2].w = 1; // debug purposes
			tOutput[triangleOutputPosition + 2] = vertices[edges[v + 2]] * valuesDistances + valuesOffsets;
//			nOutput[triangleOutputPosition + 2] = verticesNormal[edges[v + 2]];
			nOutput[triangleOutputPosition + 2] = vertexNormal(values, valuesSampler, valuesBuffers, vertices[edges[v + 2]]);
			
	//		tOutput[triangleOutputPosition] = (float4)(v); // debug purposes
	//		tOutput[triangleOutputPosition + 1] = (float4)(v+1); // debug purposes
	//		tOutput[triangleOutputPosition + 2] = (float4)(v+2); // debug purposes
		
//			nOutput[normalPosition] = triangleNormal(triangle);
	//		nOutput[normalPosition] = triangleNormal(triangle[0], triangle[1], triangle[2]);
		}
		
	//	tOutput[get_global_id(0)].x = x;
	//	tOutput[get_global_id(0)].y = y;
	//	tOutput[get_global_id(0)].z = z;
	//	tOutput[get_global_id(0)].w = scanned[position];
		
	}
	
}
