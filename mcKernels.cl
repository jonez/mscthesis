/*
 * mcKernels.cl
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */

#define VOXEL_VERTICES 16 // vertices per voxel
#define VOXEL_TRIANGLES 5 // triangles per voxel
#define TRIANGLE_VERTICES 3 // vertices per triangle

// get 1D position from 3D position
size_t getPosition(uint sX, uint sY, int x, int y, int z) {
	
	return (size_t)x + y * sX + z * sX * sY;
	
}

// get a single value from 3D data set 'values'
float getValue(	global float* values, uint sX, uint sY,
				int x, int y, int z) {
	
	return values[x + y * sX + z * sX * sY];	
	
}

// get value's address from 3D data set 'values'
global float* getAddress(	global float* values, uint sX, uint sY,
							int x, int y, int z) {
	
	return &values[x + y * sX + z * sX * sY];
	
}


// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'results1' (integer) containing the number 
// of vertices per voxel and 'results2' (boolean) which indicates if a voxel
// is relevant (1) or not (0)
kernel void classification(	global float * values, float isoValue,
							//float4 valuesDistances, uint4 valuesOffset,
							global uchar * results1, global uchar * results2,
							constant uchar * tTable, constant uchar * vTable) {
	
	// get coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	
	// work-item and data set sizes
	int sX = get_global_size(0);
	int sY = get_global_size(1);
	int siX = sX + 1;
	int siY = sY + 1;
	
	size_t position = getPosition(sX, sY, x, y ,z);
	
	// values of voxel vertices
	float corners[8];
	corners[0] = getValue(values, siX, siY,	x,		y,		z		);
	corners[1] = getValue(values, siX, siY,	x + 1,	y,		z		);
	corners[2] = getValue(values, siX, siY,	x + 1,	y + 1,	z		);
	corners[3] = getValue(values, siX, siY,	x,		y + 1,	z		);
	corners[4] = getValue(values, siX, siY,	x,		y,		z + 1	);
	corners[5] = getValue(values, siX, siY,	x + 1,	y,		z + 1	);
	corners[6] = getValue(values, siX, siY,	x + 1,	y + 1,	z + 1	);
	corners[7] = getValue(values, siX, siY,	x,		y + 1,	z + 1	);
	
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
	results2[position] = (vertices > 0) ? 1 : 0;
	
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
float4 triangleNormal(float4 * t) {
	
	float4 e1 = t[1] - t[0];
	float4 e2 = t[2] - t[0];
	
	return cross(e1, e2);
	
}

// classify all voxels created from 'values', using 'isoValue' as reference;
// the output result is two arrays, 'results1' (float4) containing all iso- 
// surface vertices in groups of 3 and 'results2' (float4) containing normal
// vectors for each triangle
kernel void generation(	global float * values, float isoValue,
							float4 valuesDistance, uint4 valuesOffset,
							global float4 * results1, global float4 * results2,
							constant uchar * tTable, constant uchar * vTable) {
	
	// get coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	
	// proper coordinates in float data type
	float xf = x + valuesOffset.x;
	float yf = y + valuesOffset.y;
	float zf = z + valuesOffset.z;
	
	float4 vd = valuesDistance;
	
	// work-item and data set sizes
	int sX = get_global_size(0);
	int sY = get_global_size(1);
	int siX = sX + 1;
	int siY = sY + 1;
	
	size_t position = getPosition(sX, sY, x, y ,z);
	
	// coordinates and values of voxel vertices
	float4 corners[8];
	corners[0] = (float4)(	xf,			yf,			zf,			getValue(values, siX, siY,	x,		y,		z		));
	corners[1] = (float4)(	xf + vd.x,	yf,			zf,			getValue(values, siX, siY,	x + 1,	y,		z		));
	corners[2] = (float4)(	xf + vd.x,	yf + vd.y,	zf,			getValue(values, siX, siY,	x + 1,	y + 1,	z		));
	corners[3] = (float4)(	xf,			yf + vd.y,	zf,			getValue(values, siX, siY,	x,		y + 1,	z		));
	corners[4] = (float4)(	xf,			yf,			zf + vd.z,	getValue(values, siX, siY,	x,		y,		z + 1	));
	corners[5] = (float4)(	xf + vd.x,	yf,			zf + vd.z,	getValue(values, siX, siY,	x + 1,	y,		z + 1	));
	corners[6] = (float4)(	xf + vd.x,	yf + vd.y,	zf + vd.z,	getValue(values, siX, siY,	x + 1,	y + 1,	z + 1	));
	corners[7] = (float4)(	xf,			yf + vd.y,	zf + vd.z,	getValue(values, siX, siY,	x,		y + 1,	z + 1	));
	
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
	size_t basePosition = position * VOXEL_TRIANGLES;
	for(uint v = 0, n = 0; v < voxelVertices; v += TRIANGLE_VERTICES, n++) {
	
		size_t vertexPosition = basePosition * TRIANGLE_VERTICES + v;
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
	
		results2[basePosition + n] = triangleNormal(triangle);
//		results2[basePosition + n] = triangleNormal(triangle[0], triangle[1], triangle[2]);
	}
	
}
 