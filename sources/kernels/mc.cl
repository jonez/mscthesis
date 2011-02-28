/*
 * mc.cl
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
inline size_t getPosition(const uint sX, const uint sY,
				   const int x, const int y, const int z) {
	
	return (size_t)x + y * sX + z * sX * sY;
}

// get a single value from 3D data set 'values'
inline float getValue(global float* values, const uint sX, const uint sY,
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
// the output result is two float arrays, 'vVoxels' containing the number of 
// vertices per voxel and 'oVoxels' which indicates if a voxel is relevant/
// occupied (1) or not relevant/empty (0)
kernel void mcClassification(global float* values, const float isoValue,
							 //const float4 valuesDistances, const uint4 valuesOffset,
							 constant uchar* tTable, constant uchar* vTable,
							 global float* vVoxels, global float* oVoxels) {
	
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
	vVoxels[position] = vertices;
	oVoxels[position] = (vertices > 0) ? OCCUPIED_VOXEL : EMPTY_VOXEL;
	
}

// create an array with relevant data only (occupied voxels)
kernel void mcCompaction(global float* values, global float* scannedValues,
						 /*const size_t size, */global size_t* result) {
	
	// get coordinates
	size_t position = get_global_id(0);
	size_t newPosition = scannedValues[position];

	if (values[position] >= 1/* && (getPosition < size)*/)
		result[newPosition] = position;
	
}

// get 3D coordinates from 1D position
inline int4 getCoordinates(const size_t position, const uint2 sizes) {
	
	int4 coordinates;
	int area = sizes.x * sizes.y;
	
	coordinates.x = position % sizes.x;
	coordinates.y = (position % area) / sizes.x;
	coordinates.z = position / area;
	
	return coordinates;
	
}

// cumpute linear interpolation of two vertices
inline float4 vertexInterpolation(float iso, float4 v1, float4 v2) {

	float r = (iso - v1.w) / (v2.w - v1.w);
	return (float4)(mix(v1, v2, r).xyz, 1.0f);

}

//inline float4 triangleNormal(float4 v1, float4 v2, float4 v3) {
//	
//	float4 e1 = v2 - v1;
//	float4 e2 = v3 - v1;
//	
//	return cross(e1, e2);
//	
//}

// compute normal vector of triangle 't'
inline float4 triangleNormal(float4* t) {
	
	float4 e1 = t[1] - t[0];
	float4 e2 = t[2] - t[0];
	
	return cross(e1, e2);
	
}

inline float4 vertexNormal(global float* val, uint2 sz, int4 pos) {
	
	float4 n;
	
//	if(pos.x == 0 || pos.x == sz.x - 1 || pos.y == 0 || pos.y == sz.y - 1 || pos.z == 0 || pos.z == sz.y - 1) 
//		n = (float4)(1, 1, 1, getValue(val, sz.x, sz.y, pos.x, pos.y, pos.z));
//	else {
		n.x = getValue(val, sz.x, sz.y, pos.x + 1, pos.y, pos.z) -
				getValue(val, sz.x, sz.y, pos.x - 1, pos.y, pos.z);
		n.y = getValue(val, sz.x, sz.y, pos.x, pos.y + 1, pos.z) -
				getValue(val, sz.x, sz.y, pos.x, pos.y - 1, pos.z);
		n.z = getValue(val, sz.x, sz.y, pos.x, pos.y, pos.z + 1) -
				getValue(val, sz.x, sz.y, pos.x, pos.y, pos.z - 1);
		n.w = 0;
//	}
	
//	return (float4)(1, 0, 0, getValue(val, sz.x, sz.y, pos.x, pos.y, pos.z));
	return (float4)(-normalize(n).xyz, getValue(val, sz.x, sz.y, pos.x, pos.y, pos.z));
	
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'tOutput' (float4) containing all iso- 
// surface vertices in groups of 3 (a triangle) and 'nOutput' (float4)
// containing a normal vector for each triangle (or 3 vertices)
kernel void mcGeneration(global float* values, uint2 size, float isoValue,
						 float4 valuesDistance, int4 valuesOffset, uint count,
						 constant uchar* tTable, constant uchar* vTable,
						 global float* scanned, global size_t* compacted,
						 global float4* tOutput, global float4* nOutput) {
	
	size_t position = get_global_id(0);
	
	if(position < count) {
	
		size_t rawPosition = compacted[position];
		int4 coordinates = getCoordinates(rawPosition, size);
		
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
		uint sX = size.x;
		uint sY = size.y;
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
		
		float4 cornersNormal[8];
		cornersNormal[0] = vertexNormal(values, (uint2)(siX, siY), (int4)(x,	y,	z,	0));
		cornersNormal[1] = vertexNormal(values, (uint2)(siX, siY), (int4)(xi,	y,	z,	0));
		cornersNormal[2] = vertexNormal(values, (uint2)(siX, siY), (int4)(xi,	yi,	z,	0));
		cornersNormal[3] = vertexNormal(values, (uint2)(siX, siY), (int4)(x,	yi,	z,	0));
		cornersNormal[4] = vertexNormal(values, (uint2)(siX, siY), (int4)(x,	y,	zi,	0));
		cornersNormal[5] = vertexNormal(values, (uint2)(siX, siY), (int4)(xi,	y,	zi,	0));
		cornersNormal[6] = vertexNormal(values, (uint2)(siX, siY), (int4)(xi,	yi,	zi,	0));
		cornersNormal[7] = vertexNormal(values, (uint2)(siX, siY), (int4)(x,	yi,	zi,	0));
		
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
		
		float4 verticesNormal[12];
		verticesNormal[0] = vertexInterpolation(isoValue, cornersNormal[0], cornersNormal[1]);
		verticesNormal[1] = vertexInterpolation(isoValue, cornersNormal[1], cornersNormal[2]);
		verticesNormal[2] = vertexInterpolation(isoValue, cornersNormal[2], cornersNormal[3]);
		verticesNormal[3] = vertexInterpolation(isoValue, cornersNormal[3], cornersNormal[0]);

		verticesNormal[4] = vertexInterpolation(isoValue, cornersNormal[4], cornersNormal[5]);
		verticesNormal[5] = vertexInterpolation(isoValue, cornersNormal[5], cornersNormal[6]);
		verticesNormal[6] = vertexInterpolation(isoValue, cornersNormal[6], cornersNormal[7]);
		verticesNormal[7] = vertexInterpolation(isoValue, cornersNormal[7], cornersNormal[4]);

		verticesNormal[8] = vertexInterpolation(isoValue, cornersNormal[4], cornersNormal[0]);
		verticesNormal[9] = vertexInterpolation(isoValue, cornersNormal[1], cornersNormal[5]);
		verticesNormal[10] = vertexInterpolation(isoValue, cornersNormal[2], cornersNormal[6]);
		verticesNormal[11] = vertexInterpolation(isoValue, cornersNormal[3], cornersNormal[7]);
		
		// fill output data in form of triangles and their normal vector
		// each iteration creates one triangle 
		uint voxelVertices = vTable[combination];
		constant uchar * edges = &tTable[combination * VOXEL_VERTICES]; // all edge values should be in private (or at least local) memory
		
		size_t basePosition = scanned[rawPosition];
	
		for(uint v = 0, n = 0; v < voxelVertices; v += TRIANGLE_VERTICES, n++) {
		
			size_t trianglePosition = basePosition + v;
			size_t normalPosition = (basePosition / TRIANGLE_VERTICES) + n;
			
	//		uchar triangleEdges[3] = {edges[v], edges[v + 1], edges[v + 2]}; // debug purposes
	//		float4 triangle[3] = {vertices[triangleEdges[0]], vertices[triangleEdges[1]], vertices[triangleEdges[2]]}; // debug purposes
			float4 triangle[3] = {vertices[edges[v]], vertices[edges[v + 1]], vertices[edges[v + 2]]};
		
	//		triangle[0].w = triangleEdges[0]; // debug purposes
	//		triangle[0].w = trianglePosition; // debug purposes
			tOutput[trianglePosition] = triangle[0];
			nOutput[trianglePosition] = verticesNormal[edges[v]];
			
	//		triangle[1].w = triangleEdges[1]; // debug purposes
	//		triangle[1].w = combination; // debug purposes
			tOutput[trianglePosition + 1] = triangle[1];
			nOutput[trianglePosition + 1] = verticesNormal[edges[v + 1]];
			
	//		triangle[2].w = triangleEdges[2]; // debug purposes
	//		triangle[2].w = 1; // debug purposes
			tOutput[trianglePosition + 2] = triangle[2];
			nOutput[trianglePosition + 2] = verticesNormal[edges[v + 2]];
			
	//		tOutput[trianglePosition] = (float4)(v); // debug purposes
	//		tOutput[trianglePosition + 1] = (float4)(v+1); // debug purposes
	//		tOutput[trianglePosition + 2] = (float4)(v+2); // debug purposes
		
//			nOutput[normalPosition] = triangleNormal(triangle);
	//		nOutput[normalPosition] = triangleNormal(triangle[0], triangle[1], triangle[2]);
		}
		
	//	tOutput[get_global_id(0)].x = x;
	//	tOutput[get_global_id(0)].y = y;
	//	tOutput[get_global_id(0)].z = z;
	//	tOutput[get_global_id(0)].w = scanned[rawPosition];
		
	}
	
}
