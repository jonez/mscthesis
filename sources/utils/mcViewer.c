/*
 * main.c
 *
 *  Created on: Oct 1, 2010
 *      Author: jonez
 */



//#include <stdlib.h>
//#include <stdio.h>
#include <math.h>
#include <unistd.h>

//OpenGL stuff
#include <GL/glew.h>
#include <GL/glut.h>
#if FREEGLUT
 #include <GL/freeglut_ext.h>
#endif /* FREEGLUT */


#include "../common.h"
#include "../utilities.h"
#include "../mcDispatcher.h"
#include "../clHelper.h"
#include "../glHelper.h"
#include "../clScan.h"

////////////////////////////////////////////////////////////////////////////////

extern void mccSetVerbose(const int);
extern void mcdSetVerbose(const int);
extern void clsSetVerbose(const int);
extern void clhSetVerbose(const int);
extern void glSetVerbose(const int);

static int MCV_VERBOSE = DISABLE;

int mcvGetVerbose() {
	return MCV_VERBOSE;
}

void mcvSetVerbose(const int state) {
	MCV_VERBOSE = state;
}

////////////////////////////////////////////////////////////////////////////////

static void mcvInitialize(int argc, char** argv);
static void mcvShutdown();

static void light();
static void reshape(int w, int h);
void display();
static void render();
static void keyboard(unsigned char key, int x, int y);
void mouseButton(int button, int state, int x, int y);
void mouseMove(int x, int y);
void mouseWindow(int state);

////////////////////////////////////////////////////////////////////////////////

// keyboard
#define KEY_A 97
#define KEY_D 100
#define KEY_W 119
#define KEY_S 115
#define KEY_Q 113
#define KEY_E 101
#define KEY_PLUS 43
#define KEY_MINUS 45
#define KEY_ESCAPE 27

#define NAVIGATION_KEYS FALSE

// mouse
#define MOUSE_SCROLL_UP 3
#define MOUSE_SCROLL_DOWN 4

#define DELTA_VALUE 5
//static const int DELTA_VALUE = 5;

////////////////////////////////////////////////////////////////////////////////

mcdMemEntry* datasetModel;
size_t sizeX, sizeY, sizeZ;
int xValue = 0, yValue = 0, zValue = 300;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

	mcvInitialize(argc, argv);

//	clsTestScan();
//	test();

	int device = (argc < 2) ? 0 : atoi(argv[1]);
	int datasetCase = (argc < 3) ? 0 : atoi(argv[2]);

	mcdSetVerbose(ENABLE);
	mccSetVerbose(ENABLE);
	mccSetProfiling(ENABLE);
	clsSetVerbose(DISABLE);
	clhSetVerbose(DISABLE);
	glSetVerbose(ENABLE);
	
//	mcdInit();
//	exit(0);

	float distanceX = 1.0f;
	float distanceY = 1.0f;
	float distanceZ = 1.0f;
	
	float isoValue = 0;
	float* dataset = NULL;
	
	switch(datasetCase) {
	
	case 0 :
		sizeX = 255;
		sizeY = 255;
		sizeZ = 255;
		isoValue = 50;
		dataset = loadCharBlock("data/skull.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 1 :
		sizeX = 255;
		sizeY = 255;
		sizeZ = 127;
		isoValue = 80;
		dataset = loadCharBlock("data/engine.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 2 :
		sizeX = 255;
		sizeY = 255;
		sizeZ = 255;
		isoValue = 50;
		dataset = loadCharBlock("data/aneurism.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 3 :
		sizeX = 63;
		sizeY = 63;
		sizeZ = 63;
		isoValue = 80;
		dataset = loadCharBlock("data/fuel.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 4 :
		sizeX = 40;
		sizeY = 40;
		sizeZ = 40;
		isoValue = 80;
		dataset = loadCharBlock("data/sin.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 5 :
		sizeX = 31;
		sizeY = 31;
		sizeZ = 31;
		isoValue = 80;
		dataset = loadCharBlock("data/bucky.raw", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 6 :
		sizeX = 255;
		sizeY = 255;
		sizeZ = 255;
		isoValue = -100000;
		dataset = makeFloatBlock(sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	case 7 :
		sizeX = 511;
		sizeY = 511;
		sizeZ = 511;
		isoValue = 50;
		dataset = loadCharBlockRepeat("data/skull.raw", (sizeX + 1) / 2, (sizeY + 1) / 2, (sizeZ + 1) / 2);
		break;
			
	case 8 :
		sizeX = 1023;
		sizeY = 1023;
		sizeZ = 24;
		isoValue = 50;
		dataset = loadFloatBlock("/media/24D7-1C08/FGM0716_0002.vol", sizeX + 1, sizeY + 1, sizeZ + 1);
		break;
	
	default :
		goto retErr;
		
	}
	
//	printf("Dataset: sizeX = %zu; sizeY = %zu; sizeZ = %zu; isoValue = %0.2f\n", 
//			sizeX + 1, sizeY + 1, sizeZ + 1, isoValue);
			
	mcDataset ds;
	ds.values = dataset;
	ds.isoValue = isoValue;
	ds.sizeX = sizeX + 1;
	ds.sizeY = sizeY + 1;
	ds.sizeZ = sizeZ + 1;
	ds.distanceX = distanceX;
	ds.distanceY = distanceY;
	ds.distanceZ = distanceZ;
	
	
//	printf("%d values >= %f.2\n", countFloatBlock(dataset, isoValue, sizeX, sizeY, sizeZ), isoValue);

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

	cl_int clErr;
	clhResources resources = clhInitResources
			(NULL, CL_DEVICE_TYPE_GPU, &device, 1, EMPTY, &clErr);
	if(clErr || MCV_VERBOSE) {
		clhLogErr__(clErr, "initializing resources");

		if(clErr) goto retErr;
	}

	int err;
	if((err = mcdInitialize(resources)))
		goto retErr;
		
	glInitialize(NULL);
	
	sleep(1);
	setDataset(&ds);
	
//	mcdDispatchSingle(&ds, device, FALSE, &datasetModel);
//	mcdDispatchMulti(&ds, 0, NULL, FALSE, &datasetModel);
//	printMemListInfo(datasetModel);

	// Turn the flow of control over to GLUT
#if PROFILING
	printf("### glutMainLoop() called - %ldms ###\n", getCurrentTimeInMili());
	fflush(NULL);
#endif /* PROFILING */
	glutMainLoop();

	mcvShutdown();
	
	return SUCCESS;
	
retErr :
	
	return ERROR;
}

static void mcvInitialize(int argc, char** argv) {

	printf("XInitThreads() = %s\n", XInitThreads() ? "ok" : "nok");

	// GLUT Window Initialization:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE| GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("mcViewer");


//	printf("GLUT_WINDOW_BUFFER_SIZE: %d\n", glutGet(GLUT_WINDOW_BUFFER_SIZE));
//	printf("GLUT_WINDOW_STENCIL_SIZE: %d\n", glutGet(GLUT_WINDOW_STENCIL_SIZE));
//	printf("GLUT_WINDOW_DEPTH_SIZE: %d\n", glutGet(GLUT_WINDOW_DEPTH_SIZE));

//	printf("VBOs supported: %s\n", glutExtensionSupported("GL_ARB_vertex_buffer_object")
//			? "true" : "false");

	if(glewInit() != GLEW_OK)
		fprintf(stderr, "Failed to initialize GLEW.\n");

	if(!glewIsSupported("GL_VERSION_2_0 GL_ARB_vertex_buffer_object"))
		fprintf(stderr, "VBOs not supported.\n");

	// Register visual callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	// Regiter input callbacks
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
//	glutEntryFunc(mouseWindow);

	// Setup opengl light
	light();
	
#if FREEGLUT
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
#else /* FREEGLUT */
	atexit(mcvShutdown);
#endif /* FREEGLUT */
}


static void mcvShutdown() {

	printf("\nSHUTDOWN!\n\n");

	glShutdown();
	freeMemList(datasetModel);

}

static void light() {

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

static void reshape(int w, int h) {

#if PROFILING
	printf("### reshape() - %ldms ###\n", getCurrentTimeInMili());
	fflush(NULL);
#endif /* PROFILING */

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, ratio, 5, 1000);
//	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);

	glMatrixMode(GL_MODELVIEW);

}

void display() {
	
//#if PROFILING
	printf("### display() - %ldms ###\n", getCurrentTimeInMili());
	fflush(NULL);
//#endif /* PROFILING */

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(xValue, yValue, zValue,
			  0.0f, 0.0f, -1.0f,
			  0.0f, 1.0f, 0.0f);

	// Render the scene
	render();

	// Make sure changes appear onscreen
	glutSwapBuffers();
	
}

static void render() {

	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();

		glPushMatrix();
			glScalef(sizeX, sizeY, sizeZ);

			glutWireCube(1.0);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(-(float)sizeX / 2, -(float)sizeY / 2, -(float)sizeZ / 2);
			
			//datasetModel = getMemList();
//			mcdMemEntry* memEntry = datasetModel;

			for(mcdMemEntry* memEntry = getMemList();
				memEntry;
				memEntry = memEntry->next) {

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

//				}

//				if(memEntry->size) {
				
#if PROFILING
//				printf("### draw vbo() - %ldms ###\n", getCurrentTimeInMili());
//				fflush(NULL);
#endif /* PROFILING */
				if(MCV_VERBOSE)
					printf("\tVBOs: triangles=%u, normals=%u, size=%zu - %ldms\n",
							memEntry->trianglesVBO, memEntry->normalsVBO,
							memEntry->size, getCurrentTimeInMili());

				glBindBuffer(GL_ARRAY_BUFFER, memEntry->trianglesVBO);
				glVertexPointer(4, GL_FLOAT, 0, 0);
				glEnableClientState(GL_VERTEX_ARRAY);

				glBindBuffer(GL_ARRAY_BUFFER, memEntry->normalsVBO);
				glNormalPointer(GL_FLOAT, sizeof(cl_float4), 0);
				glEnableClientState(GL_NORMAL_ARRAY);

				glDrawArrays(GL_TRIANGLES, 0, memEntry->size);
//				glDrawArrays(GL_POINTS, 0, memEntry->size);

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_NORMAL_ARRAY);
					

//				}

			}

		glPopMatrix();

	glPopMatrix();


	glFinish();
}

static void keyboard(unsigned char key, int x, int y) {

	switch(key) {
	
	case KEY_ESCAPE :
#if FREEGLUT
		glutLeaveMainLoop();
#else
		exit(SUCCESS);
#endif
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_UP :
#else
	case KEY_W :
#endif
		yValue += DELTA_VALUE;
		
		glutPostRedisplay();
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_DOWN :
#else
	case KEY_S :
#endif
		yValue -= DELTA_VALUE;
		
		glutPostRedisplay();
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_LEFT :
#else
	case KEY_A :
#endif
		xValue -= DELTA_VALUE;
		
		glutPostRedisplay();
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_RIGHT :
#else
	case KEY_D :
#endif
		xValue += DELTA_VALUE;
		
		glutPostRedisplay();
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_PAGE_DOWN :
#else
	case KEY_Q :
#endif
		zValue -= DELTA_VALUE;
		
		glutPostRedisplay();
	break;

#if NAVIGATION_KEYS
	case GLUT_KEY_PAGE_UP :
#else
	case KEY_E :
#endif
		zValue += DELTA_VALUE;
		
		glutPostRedisplay();
	break;
	
	case KEY_PLUS :
		incIsoValue(DELTA_VALUE);
	break;
	
	case KEY_MINUS :
		incIsoValue(-DELTA_VALUE);
	break;

	}
	
	if(MCV_VERBOSE)
		printf("Keyboard: key = %u\n", key);
}

int prevMousePosX = -1, prevMousePosY = -1;

void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	switch (button) {
	
	case GLUT_LEFT_BUTTON : 
		// when the button is released
		if (state == GLUT_UP)
			prevMousePosX = -1;
		else  {// state = GLUT_DOWN
			prevMousePosX = x;
			prevMousePosY = y;
		}
	break;
	
	case MOUSE_SCROLL_UP :
		zValue -= DELTA_VALUE;
		
		glutPostRedisplay();
	break;
	
	case MOUSE_SCROLL_DOWN :
		zValue += DELTA_VALUE;
		
		glutPostRedisplay();
	break;
	
	default :
		if(MCV_VERBOSE)
			printf("button=%d\n", button);
	}
}

void mouseMove(int x, int y) {

	// this will only be true when the left button is down
	if (prevMousePosX >= 0) {

		int deltaX = x - prevMousePosX;
		int deltaY = y - prevMousePosY;
		
		xValue -= deltaX;
		yValue += deltaY;
		
		prevMousePosX = x;
		prevMousePosY = y;
		
		glutPostRedisplay();
	}
}

void mouseWindow(int state) {

	if (state == GLUT_LEFT)
		prevMousePosX = -1;
	
}

