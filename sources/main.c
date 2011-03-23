/*
 * main.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//OpenGL stuff
#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "common.h"
#include "utilities.h"
#include "mcDispatcher.h"

#include "clHelper.h"
#include "clScan.h"

int xValue = 0;
int yValue = 0;
int zValue = 300;

//GLuint vbo;
//cl_float4 *vertices, *normals;
size_t count, sizeX, sizeY, sizeZ;
mcdMemParts* datasetModel;


void reshape(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

//	normals = NULL;

	// Set the viewport to be the entire window

	// Set the correct perspective.
	gluPerspective(45, ratio, 5, 1000);
//	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

}

void draw(void) {

	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();

		glPushMatrix();
			glScalef(sizeX, sizeY, sizeZ);

			glutWireCube(1.0);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(-(float)sizeX / 2, -(float)sizeY / 2, -(float)sizeZ / 2);

			for(int i = 0; i < count; i++) {
//				for(size_t v = 0, n = 0; v < geometry[i]->size; v += 3, n++) {
//					printf("%.2f", vertices[i]->data[j].s[0]);
//					printf(" %.2f", vertices[i]->data[j].s[1]);
//					printf(" %.2f\n", vertices[i]->data[j].s[2]);
//					fflush(NULL);

//					glBegin(GL_LINE_LOOP);
//					glBegin(GL_TRIANGLES);
//						glNormal3f(geometry[i]->normals[n].s[0], geometry[i]->normals[n].s[1], geometry[i]->normals[n].s[2]);
//						glVertex3f(geometry[i]->triangles[v].s[0], geometry[i]->triangles[v].s[1], geometry[i]->triangles[v].s[2]);
//						glVertex3f(geometry[i]->triangles[v + 1].s[0], geometry[i]->triangles[v + 1].s[1], geometry[i]->triangles[v + 1].s[2]);
//						glVertex3f(geometry[i]->triangles[v + 2].s[0], geometry[i]->triangles[v + 2].s[1], geometry[i]->triangles[v + 2].s[2]);
//					glEnd();

					glBindBuffer(GL_ARRAY_BUFFER, datasetModel[i]->trianglesVBO);
					glVertexPointer(4, GL_FLOAT, 0, 0);
					glEnableClientState(GL_VERTEX_ARRAY);

					glBindBuffer(GL_ARRAY_BUFFER, datasetModel[i]->normalsVBO);
					glNormalPointer(GL_FLOAT, sizeof(float) * 4, 0);
					glEnableClientState(GL_NORMAL_ARRAY);

					glDrawArrays(GL_TRIANGLES, 0, datasetModel[i]->size);
//					glDrawArrays(GL_POINTS, 0, model[i]->size);

					glDisableClientState(GL_VERTEX_ARRAY);
					glDisableClientState(GL_NORMAL_ARRAY);

//				}

			}

		glPopMatrix();

	glPopMatrix();



	glFinish();


}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glColor3f(1.0, 1.0, 1.0);
	glLoadIdentity();

	gluLookAt(xValue, yValue, zValue,
			  0.0f, 0.0f, -1.0f,
			  0.0f, 1.0f, 0.0f);

	// Render the scene
	draw();

	// Make sure changes appear onscreen
	glutSwapBuffers();

}

void keyboard(unsigned char key, int x, int y) {

	switch(key) {
	case 27:             // ESCAPE key
		exit(0);
		break;
	case 115:			// + key
		xValue++;
		break;
	case 97:			// - key
		xValue--;
		break;
	case 119:			// + key
		yValue--;
		break;
	case 122:			// - key
		yValue++;
		break;
	case 43:			// q key
		zValue++;
		break;
	case 45:			// w key
		zValue--;
		break;
	}

	display();

}

void glLight() {

	GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	GLfloat diffuseMaterial[] = { 0.5, 0.5, 0.5, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);

	GLfloat mat_shininess[] = { 50.0 };
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glColorMaterial(GL_FRONT, GL_DIFFUSE);



//	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

}

void initGL(int argc, char** argv) {

	// GLUT Window Initialization:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Marching Cubes-OpenCL");

	if(glewInit() != GLEW_OK)
		fprintf(stderr, "Failed to initialize GLEW.\n");

	if(!glewIsSupported("GL_VERSION_2_0 GL_ARB_vertex_buffer_object"))
		fprintf(stderr, "VBOs not supported.\n");

	glLight();

	// Register callbacks:
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

}

int main(int argc, char** argv) {

	initGL(argc, argv);

//	clsTestScan();
//	test();

	clhSetVerbose(FALSE);
	clsSetVerbose(FALSE);

	float distanceX = 1.0f;
	float distanceY = 1.0f;
	float distanceZ = 1.0f;


//	sizeX = 255;
//	sizeY = 255;
//	sizeZ = 255;
//	float isoValue = 80;
//	float* dataset = loadCharBlock("data/skull.raw", sizeX + 1, sizeY + 1, sizeZ + 1);

//	sizeX = 255;
//	sizeY = 255;
//	sizeZ = 255;
//	float isoValue = -100000;
//	float* dataset = makeFloatBlock(sizeX + 1, sizeY + 1, sizeZ + 1);

	sizeX = 255;
	sizeY = 255;
	sizeZ = 127;
	float isoValue = 100;
	float* dataset = loadCharBlock("data/engine.raw", sizeX + 1, sizeY + 1, sizeZ + 1);

//	sizeX = 63;
//	sizeY = 63;
//	sizeZ = 63;
//	float isoValue = 80;
//	float* dataset = loadCharBlock("data/fuel.raw", sizeX + 1, sizeY + 1, sizeZ + 1);

//	sizeX = 40;
//	sizeY = 40;
//	sizeZ = 40;
//	float isoValue = 80;
//	float* dataset = loadCharBlock("data/sin.raw", sizeX + 1, sizeY + 1, sizeZ + 1);

//	sizeX = 31;
//	sizeY = 31;
//	sizeZ = 31;
//	float isoValue = 80;
//	float* dataset = loadCharBlock("data/bucky.raw", sizeX + 1, sizeY + 1, sizeZ + 1);

//	cl_int4 offset = {{0, 0, 0, 0}};

//	cl_float* data = makeFloatBlock(sizeX + 1, sizeY + 1, sizeZ + 1);

//	for(int i = 0; i < (sizeX + 1) * (sizeY + 1) * (sizeZ + 1); i += (sizeX + 1) * (sizeY + 1) * 16)
//		printf("%d-%.0f,", i, data[i]);
//	printf("\n");
//
//	int c = 0;
//	for(int i = 0; i < (sizeX + 1) * (sizeY + 1) * (sizeZ + 1); i++)
//		if(data[i] < isoValue) c++;
//	printf("%d\n", c);

//	cl_float4 *results1 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 15);
//	cl_float4 *results2 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 5);
//	cl_float4* output;

//	runCL(data, isoValue, sizeX, sizeY, sizeZ, distance, offset, &geometry, &vbo, &count);
	dispatch(dataset, isoValue, distanceX, distanceY, distanceZ,
			sizeX, sizeY, sizeZ, &datasetModel, &count, FALSE);
//	mcRunHost(data, isoValue, sizeX, sizeY, sizeZ, distance, offset, &output, &count);

//	mcdMemParts aux = malloc(sizeof(struct _mcdMemParts));
//	aux->triangles = output;
//	aux->size = count;
//
//	geometry = &aux;
//	count = 1;

	size_t sum = 0;
	for(int i = 0; i < count; i++) {
		printf("%d: %d (%dKB [%d] + %dKB [%d])\n",
				i, datasetModel[i]->size,
				datasetModel[i]->size * sizeof(cl_float4) / KB, datasetModel[i]->trianglesVBO,
				datasetModel[i]->size * sizeof(cl_float4) / KB, datasetModel[i]->normalsVBO);
		sum += datasetModel[i]->size * 2;
	}
	printf("sum: %d (%dMB)\n", sum, sum * sizeof(cl_float4) / MB);


//	printf("size: %i, isovalue: %.0f\n\n", size, isoValue);
//
//	int i, j, k;
//
//	printf("input\n");
//	for(i = 0; i < ((size + 1 < 3) ? size + 1 : 3) ; i++) {
//		for(j = 0; j < ((size + 1 < 3) ? size + 1 : 3); j++) {
//			for(k = 0; k < ((size + 1 < 3) ? size + 1 : 3); k++)
//				printf("%.0f  ", data[k + j * (size + 1) + i * (size + 1) * (size + 1)]);
//			printf("\n");
//		}
//		printf("^^^ %i ^^^\n", i);
//	}
//
//	printf("\noutput1\n");
//	for(i = 0; i < 15; i++)
//		printf("%.1f;%.1f;%.1f;%.1f | ", results1[i].s[0], results1[i].s[1], results1[i].s[2], results1[i].s[3]);
//	printf("\n");
//	for(; i < 30; i++)
//		printf("%.1f;%.1f;%.1f;%.1f | ", results1[i].s[0], results1[i].s[1], results1[i].s[2], results1[i].s[3]);
//
//
//	printf("\noutput2\n");
//	for(i = 0; i < 5; i++)
//		printf("%.1f;%.1f;%.1f;%.1f | ", results2[i].s[0], results2[i].s[1], results2[i].s[2], results2[i].s[3]);
//	printf("\n");
//	for(; i < 10; i++)
//		printf("%.1f;%.1f;%.1f;%.1f | ", results2[i].s[0], results2[i].s[1], results2[i].s[2], results2[i].s[3]);

//	printf("\noutput1\n");
//	for(i = 0; i < ((size < 2) ? size : 2); i++) {
//		for(j = 0; j < ((size < 2) ? size : 2); j++) {
//			for(k = 0; k < ((size < 2) ? size : 2); k++)
//				printf("%.2f  ", result1[k + j * size + i * size * size]);
//			printf("\n");
//		}
//		printf("^^^ %i ^^^\n", i);
//	}
//
//	printf("\noutput2\n");
//		for(i = 0; i < ((size < 2) ? size : 2); i++) {
//			for(j = 0; j < ((size < 2) ? size : 2); j++) {
//				for(k = 0; k < ((size < 2) ? size : 2); k++)
//					printf("%.2f  ", result2[k + j * size + i * size * size]);
//				printf("\n");
//			}
//			printf("^^^ %i ^^^\n", i);
//	}

	// Turn the flow of control over to GLUT
	glutMainLoop();

	free(dataset);
	free(datasetModel);
		
	return 0;
	
}
