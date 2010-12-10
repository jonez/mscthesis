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
#include "mcCore.h"

int xValue = 0;
int yValue = 0;
int zValue = 150;

cl_float4* results1;
size_t size, sizeX, sizeY, sizeZ;


void reshape(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window

	// Set the correct perspective.
	gluPerspective(45, ratio, 5, 1000);
//	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

}

void draw(void) {

	glColor3f(1.0, 1.0, 1.0);

	glPushMatrix();

		glPushMatrix();
			glScalef(sizeX, sizeY, sizeZ);

			glutWireCube(1.0);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(-(float)sizeX / 2, -(float)sizeY / 2, -(float)sizeZ / 2);

			for(int i = 0; i < size; i += 3) {
				glBegin(GL_LINE_LOOP);
//					glBegin(GL_TRIANGLES);
					glVertex3f(results1[i].s[0], results1[i].s[1], results1[i].s[2]);
					glVertex3f(results1[i + 1].s[0], results1[i + 1].s[1], results1[i + 1].s[2]);
					glVertex3f(results1[i + 2].s[0], results1[i + 2].s[1], results1[i + 2].s[2]);
				glEnd();
			}

		glPopMatrix();

	glPopMatrix();


	glFlush();

}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
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
		yValue++;
		break;
	case 122:			// - key
		yValue--;
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

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

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

//	glLight();

	// Register callbacks:
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	// Turn the flow of control over to GLUT
	glutMainLoop();

}

int main(int argc, char** argv) {

//	clsTestScan();
//	test();

	sizeX = 128;
	sizeY = 128;
	sizeZ = 128;

	cl_float isoValue = 15000;

	cl_float4 distance = {{1.0f, 1.0f, 1.0f, 0.0f}};
	cl_int4 offset = {{0, 0, 0, 0}};

	cl_float* data = makeFloatBlock(sizeX + 1, sizeY + 1, sizeZ + 1);
//	cl_float* data = loadFloatBlock("data.raw", sizeX + 1);
//	cl_float4 *results1 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 15);
//	cl_float4 *results2 = (cl_float4 *)malloc(sizeof(cl_float4) * size * size * size * 5);

	runCL(data, isoValue, sizeX, sizeY, sizeZ, distance, offset, &results1, NULL, &size);


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

	initGL(argc, argv);

	free(data);
	free(results1);
		
	return 0;
	
}
