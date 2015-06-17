	#if defined(_WIN32)
		#include <windows.h>
	#endif
	#include "glut.h"
	#include "lib/DeltaTimer.h"
	#include "lib/generate_grid.h"
	#include "lib/generate_mountain.h"
	#include "lib/Keyboard.h"
	#include "lib/Mouse.h"
	#include <assert.h>
	#include <glm/gtc/constants.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/type_ptr.hpp>
	#include <glm/mat4x4.hpp>
	#include <glm/vec2.hpp>
	#include <glm/vec3.hpp>
	#include <glm/vec4.hpp>
	#include <iostream>
	#include <math.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <vector>

	using glm::ivec2;
	using glm::ivec3;
	using glm::mat4;
	using glm::vec2;
	using glm::vec3;
	using glm::vec4;
	using std::cout;
	using std::endl;
	using std::vector;

	DeltaTimer deltaTimer;
	Keyboard keyboard(256);
	Keyboard special(256);
	Mouse mouse;

	// Axes

	void drawAxes(float length = 1) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glColor3f(1, 1, 1);
		glutSolidSphere(length/10.0f, 8, 8);
		glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(length, 0, 0);

			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, length, 0);

			glColor3f(0, 0, 1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, length);
		glEnd();
		glPopAttrib();
	}

	// Lights.

	vector<vec4> lights;
	vector<vec4> lightsDiffuse;
	vector<float> lightIntensities;

	void initializeLights() {
		lights.resize(3);
		lights[0] = vec4( 1.0f, 1.0f, 3.0f, 1.0f);
		lights[1] = vec4(-2.0f, 4.0f, 5.0f, 1.0f);
		lights[2] = vec4(4.0f, 0.0f, 0.5f, 1.0f);
		lightsDiffuse.resize(3);
		lightsDiffuse[0] = vec4( 1, 1, 0.8f, 1 );
		lightsDiffuse[1] = vec4( 0, 0.1f, 0.3f, 1 );
		lightsDiffuse[2] = vec4( 1.0f, 0.9f, 0.4f, 1 );
		lightIntensities.resize(3, 1.0f);

		glLighti(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.0f);
		glLighti(GL_LIGHT2, GL_LINEAR_ATTENUATION, 1.0f);
		// glLighti(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.0f);
	}

	void drawLight(const vec3 & color) {
		glPushAttrib(GL_LIGHTING);
			glDisable(GL_LIGHTING);
			glColor3fv(glm::value_ptr(color));
			glutSolidSphere(1, 8, 8);
		glPopAttrib();
	}

	void renderLights(const mat4 & transform) {
		glLoadMatrixf(glm::value_ptr(transform));
		for (vector<vec3>::size_type il = 0; il < lights.size(); il++) {
			vec4 diffuse = lightIntensities[il]*lightsDiffuse[il];
			vec4 light = lights[il];
			if (light.w == 0.0f) return;
			vec3 pos = vec3(light)/light.w;
			glPushMatrix();
				glTranslatef(pos.x, pos.y, pos.z);
				drawLight(vec3(diffuse));
			glPopMatrix();
			glLightfv(GL_LIGHT0 + il, GL_DIFFUSE, glm::value_ptr(diffuse));
			glLightfv(GL_LIGHT0 + il, GL_POSITION, glm::value_ptr(light));
		}
	}

	// Surface.

	vec3 surfacePosition(0.0f, 0.0f, 0.0f);
	vec3 surfaceScale(5.0f, 16.0f, 4.0f);
	vec2 surfaceView(0.0f, 0.0f);
	ivec2 surfaceSize(50, 160);
	ivec2 surfaceHeightsSize(51, 801);
	vector<float> surfaceHeights;
	vector<vec3> surfaceVertices;
	vector<ivec3> surfaceTriangles;
	vector<vec3> surfaceNormals;
	vector<vec3> surfaceColors;

	void initializeSurface() {
		surfaceVertices.resize((surfaceSize.x + 1)*(surfaceSize.y + 1));
		surfaceColors.resize((surfaceSize.x + 1)*(surfaceSize.y + 1));
		surfaceTriangles = generate_grid(surfaceSize.x, surfaceSize.y);

		vector<MountainGenerationLevel> levels(4);
		levels[0].granularity = 0.05f; levels[0].min =  0.00f; levels[0].max = 1.00f;
		levels[1].granularity = 0.20f; levels[1].min = -0.20f; levels[1].max = 0.20f;
		levels[2].granularity = 0.50f; levels[2].min = -0.04f; levels[2].max = 0.04f;
		levels[3].granularity = 1.00f; levels[3].min = -0.02f; levels[3].max = 0.02f;
		surfaceHeights = generate_mountain(surfaceHeightsSize.x, surfaceHeightsSize.y, levels);
	}

	void drawSurface() {
		glBegin(GL_TRIANGLES);
		for (vector<ivec3>::size_type it = 0; it < surfaceTriangles.size(); it++) {
			ivec3 ivs = surfaceTriangles[it];
			for (int iv = 0; iv < 3; iv++) {
				glColor3fv(glm::value_ptr(surfaceColors[ivs[iv]]));
				glNormal3fv(glm::value_ptr(surfaceNormals[ivs[iv]]));
				glVertex3fv(glm::value_ptr(surfaceVertices[ivs[iv]]));
			}
		}
		glEnd();
	}

	void renderSurface(const mat4 & transform) {

		// Set vertex positions and colors.
		for (int ix = 0; ix <= surfaceSize.x; ix++) {
			for (int iy = 0; iy <= surfaceSize.y; iy++) {
				// Compute vertex index.
				int iv = iy + ix*(surfaceSize.y + 1);

				// Compute view coordinates.
				int ixview = (int) surfaceView.x;
				int iyview = (int) surfaceView.y;
				float offsetx = (surfaceView.x - ixview)/surfaceSize.x;
				float offsety = (surfaceView.y - iyview)/surfaceSize.y;

				// Compute height map index.
				int ixh = (ix + ixview) % surfaceHeightsSize.x;
				int iyh = (iy + iyview) % surfaceHeightsSize.y;
				int ih = iyh + ixh*surfaceHeightsSize.y;

				// Compute x and y coordinates relative to the surface.
				float rsx = (float) ix/surfaceSize.x;
				float rsy = (float) iy/surfaceSize.y;

				float c = (cos(rsx*glm::pi<float>()) + 1.0f)/2.0f; // Flatten 1 side.
				float z = c*surfaceHeights[ih];
				if (z < 0.0f) z = 0.0f;
				surfaceVertices[iv] = vec3(rsx - offsetx - 0.5, rsy - offsety - 0.5, z);

				if (z < 0.2) surfaceColors[iv] = vec3(113, 139, 77)/256.0f;
				else if (z < 0.4) surfaceColors[iv] = vec3(79, 101, 58)/256.0f;
				else if (z < 0.6) surfaceColors[iv] = vec3(166, 136, 100)/256.0f;
				else if (z < 0.8) surfaceColors[iv] = vec3(116, 118, 141)/256.0f;
				else surfaceColors[iv] = vec3(1, 1, 1);
			}
		}

		// Clear normals.
		surfaceNormals.assign((surfaceSize.x + 1)*(surfaceSize.y + 1), vec3(0.0f));

		// Compute normals per triangle.
		for (vector<ivec3>::size_type it = 0; it < surfaceTriangles.size(); it++) {
			ivec3 ivs = surfaceTriangles[it];
			vec3 v1 = surfaceVertices[ivs[0]];
			vec3 v2 = surfaceVertices[ivs[1]];
			vec3 v3 = surfaceVertices[ivs[2]];
			vec3 n = glm::cross(v2 - v3, v2 - v1);
			surfaceNormals[ivs[0]] += n;
			surfaceNormals[ivs[1]] += n;
			surfaceNormals[ivs[2]] += n;
		}

		// Normalize normals.
		for (vector<vec3>::size_type iv = 0; iv < surfaceNormals.size(); iv++) {
			surfaceNormals[iv] = glm::normalize(surfaceNormals[iv]);
		}

		mat4 local = glm::translate(transform, surfacePosition);
		local = glm::scale(local, surfaceScale);
		glLoadMatrixf(glm::value_ptr(local));
		drawSurface();
	}

	// Floor.

	vec3 floorPosition(4.0f, 0.0f, -0.0001f);
	vec3 floorScale(6.0f, 16.0f, 1.0f);
	vec3 floorColor = vec3(113, 139, 77)/256.0f;
	ivec2 floorSize(60, 160);
	vector<vec3> floorVertices;
	vector<ivec3> floorTriangles;

	void initializeFloor() {
		floorVertices.resize((floorSize.x + 1)*(floorSize.y + 1));
		floorTriangles = generate_grid(floorSize.x, floorSize.y);

		// Set vertex positions and colors.
		for (int ix = 0; ix <= floorSize.x; ix++) {
			for (int iy = 0; iy <= floorSize.y; iy++) {
				// Compute vertex index.
				int iv = iy + ix*(floorSize.y + 1);

				// Compute x and y coordinates relative to the surface.
				float rsx = (float) ix/floorSize.x;
				float rsy = (float) iy/floorSize.y;
				floorVertices[iv] = vec3(rsx - 0.5f, rsy - 0.5f, 0);
			}
		}
	}

	void drawFloor() {
		glColor3fv(glm::value_ptr(floorColor));
		glNormal3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_TRIANGLES);
		for (vector<ivec3>::size_type it = 0; it < floorTriangles.size(); it++) {
			ivec3 ivs = floorTriangles[it];
			for (int iv = 0; iv < 3; iv++) {
				glVertex3fv(glm::value_ptr(floorVertices[ivs[iv]]));
			}
		}
		glEnd();
	}

	void renderfloor(const mat4 & transform) {
		mat4 local = glm::translate(transform, floorPosition);
		local = glm::scale(local, floorScale);
		glLoadMatrixf(glm::value_ptr(local));
		glColor3fv(glm::value_ptr(floorColor));
		drawFloor();
	}

	// Camera.

	vec3 camera_position_velocity(0.0f, 0.0f, 0.0f);
	vec3 camera_rotation_velocity(0.0f, 0.0f, 0.0f);
	mat4 camera(1.0f);

	void initializeCamera() {
		camera = glm::rotate(camera, glm::two_thirds<float>()*glm::pi<float>(), vec3(1.0f, 1.0f, 1.0f));
		camera = glm::inverse(camera);
		camera = glm::rotate(camera, 0.1f, vec3(0.0f, 1.0f, 0.0f)); // Pitch forward.
		camera = glm::translate(camera, vec3(-7.0f, 0.0f, -1.0f)); // Move back and up.
		camera = glm::inverse(camera);
	}

	// Rendering

	float daynight = 0;

	void render() {
		vec3 day = vec3(94, 127, 232)/256.0f;
		vec3 night = vec3(5, 12, 22)/256.0f;
		vec3 clear = glm::mix(night, day, (sin(daynight) + 1.0f)/2.0f);
		glClearColor(clear.r, clear.g, clear.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

		mat4 transform = glm::inverse(camera);
		glLoadMatrixf(glm::value_ptr(transform));

		renderLights(transform);

		glLoadMatrixf(glm::value_ptr(transform));
		drawAxes();

		renderSurface(transform);

		renderfloor(transform);

		glutSwapBuffers();
		glutPostRedisplay();
	}

	// Animation.

	void animate() {
		float delta = deltaTimer.update();
		keyboard.update();
		special.update();
		mouse.update();

		// surfacePosition += 1.0f*vec3(0.0f, delta, 0.0f);
		surfaceView += 2.0f*vec2(0.0f, delta);

		daynight += delta*0.4f;
		lights[0] = vec4(0.0f, 8*cos(daynight), 8*sin(daynight), 1.0f);
		lightIntensities[0] = (sin(daynight) + 1.0f)/2.0f;

		vec3 crot(0.0f, 0.0f, 0.0f);
		if (keyboard.held('l')) crot.z = -delta;
		if (keyboard.held('j')) crot.z = delta;
		if (keyboard.held('i')) crot.x = delta;
		if (keyboard.held('k')) crot.x = -delta;
		if (keyboard.held('u')) crot.y = delta;
		if (keyboard.held('o')) crot.y = -delta;
		camera_rotation_velocity += crot;
		camera_rotation_velocity *= 0.9f;
		crot += 0.2f*camera_rotation_velocity;

		vec3 cmov(0.0f, 0.0f, 0.0f);
		if (keyboard.held('d')) cmov.x = delta;
		if (keyboard.held('a')) cmov.x = -delta;
		if (keyboard.held('w')) cmov.z = -delta;
		if (keyboard.held('s')) cmov.z = delta;
		if (keyboard.held('q')) cmov.y = delta;
		if (keyboard.held('z')) cmov.y = -delta;
		camera_position_velocity += cmov;
		camera_position_velocity *= 0.9f;
		cmov += 5.0f*camera_position_velocity;

		mat4 mcam(1.0f);
		mcam = glm::translate(mcam, cmov);
		mcam = glm::rotate(mcam, crot.x, vec3(1.0f, 0.0f, 0.0f));
		mcam = glm::rotate(mcam, crot.y, vec3(0.0f, 1.0f, 0.0f));
		mcam = glm::rotate(mcam, crot.z, vec3(0.0f, 0.0f, 1.0f));

		camera = camera*mcam;
	}

	void onKeyDown(unsigned char key, int x, int y) {
		keyboard.press(key);

		const char ESC = 27;

		if (key == ESC) exit(0);
		if (key == '1') glEnable(GL_LIGHT0);
		if (key == '!') glDisable(GL_LIGHT0);
		if (key == '2') glEnable(GL_LIGHT1);
		if (key == '@') glDisable(GL_LIGHT1);
		if (key == '3') glEnable(GL_LIGHT2);
		if (key == '#') glDisable(GL_LIGHT2);
	}

	void onKeyUp(unsigned char key, int x, int y) {
		keyboard.release(key);
	}

	void onSpecialKeyDown(int key, int x, int y) {
		special.press(key);
	}

	void onSpecialKeyUp(int key, int x, int y) {
		special.release(key);
	}

	void onMouse(int button, int state, int x, int y) {
		mouse.state(button, state);

		if (state == 1) {
			vec3 cmov(-0.007f*mouse.dx, 0.007f*mouse.dy, 0.0f);
			camera_position_velocity = cmov;
		}
	}


	void onMotion(int x, int y) {
		mouse.position(x, y);

		vec3 cmov(-0.007f*mouse.dx, 0.007f*mouse.dy, 0.0f);

		mat4 mcam(1.0f);
		mcam = glm::translate(mcam, cmov);

		camera = camera*mcam;
	}

	void onPassiveMotion(int x, int y) {
		mouse.position(x, y);
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
		// Initialize glut.
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutInitWindowPosition(200, 100);
		glutInitWindowSize(800,600);
		glutCreateWindow(argv[0]);

		// Initialize opengl.
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		glPolygonMode(GL_FRONT,GL_FILL);
		glPolygonMode(GL_BACK,GL_LINE);

		glShadeModel(GL_SMOOTH);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Initialize resources.
		initializeLights();
		initializeSurface();
		initializeFloor();
		initializeCamera();

		// Configure glut.
		glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(onKeyDown);
		glutKeyboardUpFunc(onKeyUp);
		glutSpecialFunc(onSpecialKeyDown);
		glutSpecialUpFunc(onSpecialKeyUp);
		glutMouseFunc(onMouse);
		glutMotionFunc(onMotion);
		glutPassiveMotionFunc(onPassiveMotion);
		glutDisplayFunc(render);
		glutIdleFunc(animate);

		// Kickstart the main loop.
		glutMainLoop();

		return 0;
	}
