/*
 * mcKernels.cl
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 *
 * Notes: use built-in vectores when possible
 *		  use inline functions
 *		  use local memory
 */

#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable

#define VOXEL_VERTICES 16 // vertices per voxel
#define VOXEL_TRIANGLES 5 // triangles per voxel
#define TRIANGLE_VERTICES 3 // vertices per triangle

#define EMPTY_VOXEL 0 // non relevant voxel
#define OCCUPIED_VOXEL 1 // relevant voxel

// get 1D position from 3D position
size_t getPosition(const uint sX, const uint sY,
				   const int x, const int y, const int z) {
	
	return (size_t)x + y * sX + z * sX * sY;
}

// get a single value from 3D data set 'values'
float getValue(global float* values, const uint sX, const uint sY,
			   const int x, const int y, const int z) {
	
	return values[x + y * sX + z * sX * sY];	
}

// get value's address from 3D data set 'values'
global float* getAddress(global float* values,
						 const uint sX, const uint sY,
						 const int x, const int y, const int z) {
	
	return values + (x + y * sX + z * sX * sY);
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'results1' (integer) containing the number 
// of vertices per voxel and 'results2' (boolean) which indicates if a voxel
// is relevant (1) or not (0)
kernel void mcClassification(global float* values, const float isoValue,
							 //const float4 valuesDistances, const uint4 valuesOffset,
							 constant uchar* tTable, constant uchar* vTable,
							 global float* results1, global float* results2) {
	
	// get coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	int xi = x + 1;
	int yi = y + 1;
	int zi = z + 1;
	
	// work-item and data set sizes
	uint sX = get_global_size(0);
	uint sY = get_global_size(1);
	uint siX = sX + 1;
	uint siY = sY + 1;
	
	size_t position = getPosition(sX, sY, x, y ,z);
	
	// values of voxel vertices
	float corners[8];
	corners[0] = getValue(values, siX, siY,	x,	y,	z	);
	corners[1] = getValue(values, siX, siY,	xi,	y,	z	);
	corners[2] = getValue(values, siX, siY,	xi,	yi,	z	);
	corners[3] = getValue(values, siX, siY,	x,	yi,	z	);
	corners[4] = getValue(values, siX, siY,	x,	y,	zi	);
	corners[5] = getValue(values, siX, siY,	xi,	y,	zi	);
	corners[6] = getValue(values, siX, siY,	xi,	yi,	zi	);
	corners[7] = getValue(values, siX, siY,	x,	yi,	zi	);
	
	// voxel index combination
	uchar combination = 0;
	combination = (uint)(corners[0] < isoValue); 
	combination += (uint)(corners[1] < isoValue) * 2; 
	combination += (uint)(corners[2] < isoValue) * 4; 
	combination += (uint)(corners[3] < isoValue) * 8; 
	combination += (uint)(corners[4] < isoValue) * 16; 
	combination += (uint)(corners[5] < isoValue) * 32; 
	combination += (uint)(corners[6] < isoValue) * 64; 
	combination += (uint)(corners[7] < isoValue) * 128;
	
	// output two arrays
	uchar vertices = vTable[combination];
	results1[position] = vertices;
	results2[position] = (vertices > 0) ? OCCUPIED_VOXEL : EMPTY_VOXEL;
//	results1[position] = corners[0];
//	results2[position] = corners[7];
	
}

// create an array only with relevant data
kernel void mcCompaction(global float* values, global float* scannedValues,
						 /*const size_t size, */global size_t* result) {
	
	// get coordinates
	size_t position = get_global_id(0);
	size_t newPosition = scannedValues[position];

	if (values[position] >= 1/* && (getPosition < size)*/)
		result[newPosition] = position;
	
}

int4 getCoordinates(const size_t position, const uint2 sizes) {
	
	int4 coordinates;
	int area = sizes.x * sizes.y;
	
	coordinates.x = position % sizes.x;
	coordinates.y = (position % area) / sizes.x;
	coordinates.z = position / area;
	
	return coordinates;
	
}

// cumpute linear interpolation of two vertices
float4 vertexInterpolation(float iso, float4 v1, float4 v2) {

	float r = (iso - v1.w) / (v2.w - v1.w);
	return mix(v1, v2, r);

}

//float4 triangleNormal(float4 v1, float4 v2, float4 v3) {
//	
//	float4 e1 = v2 - v1;
//	float4 e2 = v3 - v1;
//	
//	return cross(e1, e2);
//	
//}

// computes normal vector of triangle 't'
float4 triangleNormal(float4* t) {
	
	float4 e1 = t[1] - t[0];
	float4 e2 = t[2] - t[0];
	
	return cross(e1, e2);
	
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'results1' (float4) containing all iso- 
// surface vertices in groups of 3 and 'results2' (float4) containing normal
// vectors for each triangle
kernel void mcGeneration(global float* values, uint2 sizes, float isoValue,
						 float4 valuesDistance, int4 valuesOffset,
						 constant uchar* tTable, constant uchar* vTable,
						 global float* scanned, global size_t* compacted,
						 global float4* results1/*, global float4* results2*/) {
	
	size_t position = get_global_id(0);
	size_t rawPosition = compacted[position];
	int4 coordinates = getCoordinates(rawPosition, sizes);
	
	// get kernel coordinates
	int x = coordinates.x;
	int y = coordinates.y;
	int z = coordinates.z;
	int xi = x + 1;
	int yi = y + 1;
	int zi = z + 1;
	
	// voxel coordinates in float data type
	float xf = x + valuesOffset.x;
	float yf = y + valuesOffset.y;
	float zf = z + valuesOffset.z;
	float xfi = xf + valuesDistance.x;
	float yfi = yf + valuesDistance.y;
	float zfi = zf + valuesDistance.z;
	
	// work-item and dataset sizes
	uint sX = sizes.x;
	uint sY = sizes.y;
	uint siX = sX + 1;
	uint siY = sY + 1;
	
	// coordinates and values of voxel vertices
	float4 corners[8];
	corners[0] = (float4)(xf,	yf,		zf,		getValue(values, siX, siY,	x,	y,	z	));
	corners[1] = (float4)(xfi,	yf,		zf,		getValue(values, siX, siY,	xi,	y,	z	));
	corners[2] = (float4)(xfi,	yfi,	zf,		getValue(values, siX, siY,	xi,	yi,	z	));
	corners[3] = (float4)(xf,	yfi,	zf,		getValue(values, siX, siY,	x,	yi,	z	));
	corners[4] = (float4)(xf,	yf,		zfi,	getValue(values, siX, siY,	x,	y,	zi	));
	corners[5] = (float4)(xfi,	yf,		zfi,	getValue(values, siX, siY,	xi,	y,	zi	));
	corners[6] = (float4)(xfi,	yfi,	zfi,	getValue(values, siX, siY,	xi,	yi,	zi	));
	corners[7] = (float4)(xf,	yfi,	zfi,	getValue(values, siX, siY,	x,	yi,	zi	));
	
	// voxel index combination
	uchar combination = 0;
	combination = (uint)(corners[0].w < isoValue);
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
	uint voxelVertices = vTable[combination];
	constant uchar * edges = &tTable[combination * VOXEL_VERTICES]; // all edge values should be in private (or at least local) memory
	
	size_t basePosition = scanned[rawPosition];

	for(uint v = 0; v < voxelVertices; v += TRIANGLE_VERTICES) {
	
		size_t vertexPosition = basePosition + v;
		uchar triangleEdges[3] = {edges[v], edges[v + 1], edges[v + 2]}; // debug purposes
		float4 triangle[3] = {vertices[triangleEdges[0]], vertices[triangleEdges[1]], vertices[triangleEdges[2]]}; // debug purposes
//		float4 triangle[3] = {vertices[edges[v]], vertices[edges[v + 1]], vertices[edges[v + 2]]};
	
//		triangle[0].w = triangleEdges[0]; // debug purposes
		triangle[0].w = vertexPosition; // debug purposes
		results1[vertexPosition] = triangle[0];
		
//		triangle[1].w = triangleEdges[1]; // debug purposes
		triangle[1].w = combination; // debug purposes
		results1[vertexPosition + 1] = triangle[1];
		
//		triangle[2].w = triangleEdges[2]; // debug purposes
		triangle[2].w = 1; // debug purposes
		results1[vertexPosition + 2] = triangle[2];
		
//		results1[vertexPosition] = (float4)(v); // debug purposes
//		results1[vertexPosition + 1] = (float4)(v+1); // debug purposes
//		results1[vertexPosition + 2] = (float4)(v+2); // debug purposes
	
//		results2[basePosition + n] = triangleNormal(triangle);
//		results2[basePosition + n] = triangleNormal(triangle[0], triangle[1], triangle[2]);
	}
	
//	results1[get_global_id(0)].x = x;
//	results1[get_global_id(0)].y = y;
//	results1[get_global_id(0)].z = z;
//	results1[get_global_id(0)].w = scanned[rawPosition];
	
}
