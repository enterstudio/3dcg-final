#if defined(_WIN32)
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lib/keys.h"
#include "lib/delta.h"
#include <iostream>

using std::cout;
using std::endl;

float LightPos[4] = {2,1,0.4,1};
float LightTargetPos[4] = {2,1,0.4,1};

//function to draw coordinate axes with a certain length (1 as a default)
void drawCoordSystem(float length=1)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(length,0,0);

		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,length,0);

		glColor3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,length);
	glEnd();
	glPopAttrib();
}

void drawLight() {

	glPushAttrib(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	glPushMatrix();
		glColor3f(1,1,0);
		glTranslatef(LightPos[0], LightPos[1], LightPos[2]);
		glutSolidSphere(0.2, 24, 24);
	glPopMatrix();
	glPopAttrib();

}

void render()
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0,0,-4);

	// tbVisuTransform(); // origine et orientation de la scene

	//set the light to the right position
	glLightfv(GL_LIGHT0,GL_POSITION,LightPos);
	drawLight();

	drawCoordSystem();

	glutSwapBuffers();
	glutPostRedisplay();
}

void animate() {
	float delta = delta::update();

	if (keys::held((unsigned char) 'd')) LightTargetPos[0] += 2.0*delta;
	if (keys::held((unsigned char) 'a')) LightTargetPos[0] -= 2.0*delta;
	if (keys::held((unsigned char) 'w')) LightTargetPos[1] += 2.0*delta;
	if (keys::held((unsigned char) 's')) LightTargetPos[1] -= 2.0*delta;
	if (keys::held((unsigned char) 'q')) LightTargetPos[2] += 2.0*delta;
	if (keys::held((unsigned char) 'z')) LightTargetPos[2] -= 2.0*delta;

	LightPos[0] += (LightTargetPos[0] - LightPos[0])/10.0f;
	LightPos[1] += (LightTargetPos[1] - LightPos[1])/10.0f;
	LightPos[2] += (LightTargetPos[2] - LightPos[2])/10.0f;
}

//take keyboard input into account
void onKeyDown(unsigned char key, int x, int y) {
	keys::onDown(key);

	const char ESC = 27;

	if (key == ESC) exit(0);

	if (key == 'L') glEnable(GL_LIGHTING);
	if (key == 'l') glDisable(GL_LIGHTING);
}

void onKeyUp(unsigned char key, int x, int y) {
	keys::onUp(key);
}

void onSpecialKeyDown(int key, int x, int y) {
	keys::onSpecialDown(key);
}

void onSpecialKeyUp(int key, int x, int y) {
	keys::onSpecialUp(key);
}

void onMouse(int button, int state, int x, int y) {
	// cout << "Mouse press " << button << " state " << state << " x: " << x << " y: " << y << endl;
}

void onMotion(int x, int y) {
	// cout << "Mouse motion x: " << x << " y: " << y << endl;
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho (-1.1, 1.1, -1.1,1.1, -1000.0, 1000.0);
	gluPerspective(50, (float)w/h, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowPosition(200, 100);
	glutInitWindowSize(800,600);
	glutCreateWindow(argv[0]);

	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//Draw frontfacing polygons as filled
	glPolygonMode(GL_FRONT,GL_FILL);
	//draw backfacing polygons as outlined
	glPolygonMode(GL_BACK,GL_LINES);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialFunc(onSpecialKeyDown);
	glutSpecialUpFunc(onSpecialKeyUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutDisplayFunc(render);
	glutIdleFunc(animate);

	glutMainLoop();

	return 0;
}
