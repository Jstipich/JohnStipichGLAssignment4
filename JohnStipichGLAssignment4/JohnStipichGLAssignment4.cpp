// JohnStipichGLAssignment4.cpp 
//

#include <iostream>
#include <fstream>
#include <gl/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <vector>
#include "ply.h"

bool rayTriangleIntersect(const glm::vec3& origin, const glm::vec3& direction,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
	float& t);
glm::vec3 calcPointLight(glm::vec3 normal, glm::vec3 fragPos, glm::vec3 objectColor);

int num_elems;
Vertex** vlist;
Face** flist;
std::vector<glm::vec3> worldVects;

void read_test(char* filename)
{
	int i, j, k;
	PlyFile* ply;
	int nelems;
	char** elist;
	int file_type;
	float version;
	int nprops;
	PlyProperty** plist;
	char* elem_name;
	int num_comments;
	char** comments;
	int num_obj_info;
	char** obj_info;

	/* open a PLY file for reading */
	ply = ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);

	/* print what we found out about the file */
	printf("version %f\n", version);
	printf("type %d\n", file_type);

	/* go through each kind of element that we learned is in the file */
	/* and read them */

	for (i = 0; i < nelems; i++) {

		/* get the description of the first element */
		elem_name = elist[i];
		plist = ply_get_element_description(ply, elem_name, &num_elems, &nprops);

		/* print the name of the element, for debugging */
		printf("element %s %d\n", elem_name, num_elems);

		/* if we're on vertex elements, read them in */
		if (equal_strings((char*)"vertex", elem_name)) {

			/* create a vertex list to hold all the vertices */
			vlist = (Vertex * *)malloc(sizeof(Vertex*) * num_elems);

			/* set up for getting vertex elements */

			ply_get_property(ply, elem_name, &vert_props[0]);
			ply_get_property(ply, elem_name, &vert_props[1]);
			ply_get_property(ply, elem_name, &vert_props[2]);

			/* grab all the vertex elements */
			for (j = 0; j < num_elems; j++) {

				/* grab and element from the file */
				vlist[j] = (Vertex*)malloc(sizeof(Vertex));
				ply_get_element(ply, (void*)vlist[j]);

				/* print out vertex x,y,z for debugging */
			   // printf("vertex: %g %g %g\n", vlist[j]->x, vlist[j]->y, vlist[j]->z);
			}
		}

		/* if we're on face elements, read them in */
		if (equal_strings((char*)"face", elem_name)) {

			/* create a list to hold all the face elements */
			flist = (Face * *)malloc(sizeof(Face*) * num_elems);

			/* set up for getting face elements */

			ply_get_property(ply, elem_name, &face_props[0]);
			ply_get_property(ply, elem_name, &face_props[1]);

			/* grab all the face elements */
			for (j = 0; j < num_elems; j++) {

				/* grab and element from the file */
				flist[j] = (Face*)malloc(sizeof(Face));
				ply_get_element(ply, (void*)flist[j]);

				/* print out face info, for debugging */
			   // printf("face: %d, list = ", flist[j]->intensity);
			   // for (k = 0; k < flist[j]->nverts; k++)
			   //     printf("%d ", flist[j]->verts[k]);
			   // printf("\n");
			}
		}

		/* print out the properties we got, for debugging */
		for (j = 0; j < nprops; j++)
			printf("property %s\n", plist[j]->name);
	}

	/* grab and print out the comments in the file */
	comments = ply_get_comments(ply, &num_comments);
	for (i = 0; i < num_comments; i++)
		printf("comment = '%s'\n", comments[i]);

	/* grab and print out the object information */
	obj_info = ply_get_obj_info(ply, &num_obj_info);
	for (i = 0; i < num_obj_info; i++)
		printf("obj_info = '%s'\n", obj_info[i]);

	/* close the PLY file */
	ply_close(ply);
}

template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
	assert(!(hi < lo));
	return (v < lo) ? lo : (hi < v) ? hi : v;
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
unsigned int subwindowWidth;
unsigned int subwindowHeight;
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

int mainWindow, subWindow1;
int w, h, border = 6;
float sOffset1 = 0.0f, sOffset2 = 0.0f, sOffset3 = 0.0f, sScale = 1.0f, tValX = 0.0f, tValY = 0.0f;
bool clicked = false;
float sPos1 = 300.0f, sPos2 = 300.0f, sPos3 = 300.0f, sPos4 = 684.0f, sPos5 = 300.0f, sPos6 = 250.0f;

void setProjection(int w1, int h1) {
	float ratio;
	//prevent divide by zero, when window is too short
	ratio = 1.0f * w1 / h1;
	//reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//set viewport
	glViewport(0, 0, w1, h1);

	gluOrtho2D(0.0, w, 0.0, h);
	glMatrixMode(GL_MODELVIEW);

	projection = glm::perspective(glm::radians(90.0f), (float)w1 / (float)h1, 1.0f, 100.0f);
}

void changeSize(int w1, int h1) {
	glClearColor(0.5, 0.4, 0.7, 0.0);

	//prevent divide by zero, when window is too short 
	if (h1 == 0)
		h1 = 1;

	w = w1;
	h = h1;

	// set subwindow 1 as the active window
	glutSetWindow(subWindow1);
	// resize and reposition the sub window
	glutPositionWindow(border, border);
	glutReshapeWindow(w / 2 - border * 3 / 2, h / 2 - border * 2);
	subwindowWidth = (w / 2 - border * 3 / 2);
	subwindowHeight = (h / 1.5 - border * 2);
	setProjection(w / 1.4 - border * 3 / 2, h - border * 2);
	//w - 2 * border

	glutSetWindow(mainWindow);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, w, 0.0, h);
	glMatrixMode(GL_MODELVIEW);

}

int sNum;

void mouseButton(int button, int state, int x, int y) { //something is broken here, lets try and fix that
	//only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		//when the button is released
		if (state == GLUT_UP) {
			clicked = false;
		}
		else { //state = GLUT_DOWN
			if (x > 594 && x < 614 && y < (sPos1 + 5) && y >(sPos1 - 5))//glTranslatef(604.0f, 300.0f, 0.0f); vertical
			{
				sNum = 1;
				clicked = true;
			}
			else if (x > 674 && x < 694 && y < (sPos2 + 5) && y >(sPos2 - 5)) {//glTranslatef(684.0f, 300.0f, 0.0f);
				sNum = 2;
				clicked = true;
			}
			else if (x > 754 && x < 774 && y < (sPos3 + 5) && y >(sPos3 - 5)) {//glTranslatef(764.0f, 300.0f, 0.0f);
				sNum = 3;
				clicked = true;
			}
			else if (y > 100 && y < 120 && x < (sPos4 + 5) && x >(sPos4 - 5)) {//glTranslatef(684.0f, 490.0f, 0.0f);
				sNum = 4;
				clicked = true;
			}
			else if (x > 90 && x < 110 && y < (sPos5 + 5) && y > (sPos5 - 5)) {//glTranslatef(100.0f, 150.0f, 0.0f); vertical
				sNum = 5;
				clicked = true;
			}
			else if (y > 140 && y < 160 && x < (sPos6 + 5) && x > (sPos6 - 5)) {//glTranslatef(250.0f, 150.0f, 0.0f); 
				sNum = 6;
				clicked = true;
			}
		}
	}

}

float PtoD = 3.789; //pixels to degrees 
float maxOfVal = 359.955; //maximum offset value 
float minOfVal = -359.955; //minimum offset value
float maxScale = 2.0f;
float minScale = 0.0f;
float maxT = 10.0f;
float minT = -10.0f;

void mouseMove(int x, int y) {
	//this will only be true when the left button is down
	if (clicked) {
		//update slider posiiton
		if (sNum == 1) {
			sPos1 = y;
			sOffset1 = (sPos1 - 300) * PtoD;
			if (sPos1 > 395 && sOffset1 > maxOfVal) { //(695 - 600) * 3.789 = 359.955
				sPos1 = 395;
				sOffset1 = maxOfVal;
			}
			else if (sPos1 < 205 && sOffset1 < minOfVal) {
				sPos1 = 205;
				sOffset1 = minOfVal;
			}
		}
		else if (sNum == 2) {
			sPos2 = y;
			sOffset2 = (sPos2 - 300) * PtoD;
			if (sPos2 > 395 && sOffset2 > maxOfVal) {
				sPos2 = 395;
				sOffset2 = maxOfVal;
			}
			else if (sPos2 < 205 && sOffset2 < minOfVal) {
				sPos2 = 205;
				sOffset2 = minOfVal;
			}
		}
		else if (sNum == 3) {
			sPos3 = y;
			sOffset3 = (sPos3 - 300) * PtoD;
			if (sPos3 > 395 && sOffset3 > maxOfVal) {
				sPos3 = 395;
				sOffset3 = maxOfVal;
			}
			else if (sPos3 < 205 && sOffset3 < minOfVal) {
				sPos3 = 205;
				sOffset3 = minOfVal;
			}
		}
		else if (sNum == 4) {//glTranslatef(684.0f, 490.0f, 0.0f);
			sPos4 = x;
			sScale = (sPos4 - 589) / 95;
			//sScale = 684 / (2 * sPos4);
			if (sPos4 > 779 && sScale > maxScale) {
				sPos4 = 779;
				sScale = maxScale;
			}
			else if (sPos4 < 589 && sScale < minScale) {
				sPos4 = 589;
				sScale = minScale;
			}
		}
		else if (sNum == 5) {
			sPos5 = y;
			tValY = (sPos5 - 150) / 10;

			if (sPos5 > 245 && tValY > maxT) {//150
				sPos5 = 245;
				tValY = maxT;
			}
			else if (sPos5 < 55 && tValY < minT) {
				sPos5 = 55;
				tValY = minT;
			}
		}
		else if (sNum == 6) {
			sPos6 = x;
			tValX = (sPos6 - 250) / 10;
			if (sPos6 > 345 && tValX > maxT) {//250
				sPos6 = 345;
				tValX = maxT;
			}
			else if (sPos6 < 155 && tValX < minT) {
				sPos6 = 155;
				tValX = minT;
			}
		}

	}
}



void drawPBar() {
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glVertex3f(-100.0f, 10.0f, 0.0f);
	glVertex3f(100.0f, 10.0f, 0.0f);
	glVertex3f(100.0f, -10.0f, 0.0f);
	glVertex3f(-100.0f, -10.0f, 0.0f);
	glEnd();
}

void drawSlider() {
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POLYGON);
	glVertex3f(-5.0f, 10.0f, 0.0f);
	glVertex3f(5.0f, 10.0f, 0.0f);
	glVertex3f(5.0f, -10.0f, 0.0f);
	glVertex3f(-5.0f, -10.0f, 0.0f);
	glEnd();
}

void drawPBarV() {
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glVertex3f(-10.0f, 100.0f, 0.0f);
	glVertex3f(10.0f, 100.0f, 0.0f);
	glVertex3f(10.0f, -100.0f, 0.0f);
	glVertex3f(-10.0f, -100.0f, 0.0f);
	glEnd();
}

void drawSliderV() {
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POLYGON);
	glVertex3f(-10.0f, 5.0f, 0.0f);
	glVertex3f(10.0f, 5.0f, 0.0f);
	glVertex3f(10.0f, -5.0f, 0.0f);
	glVertex3f(-10.0f, -5.0f, 0.0f);
	glEnd();
}

void display() {
	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT);
	//draw bar one
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(604.0f, 300.0f, 0.0f);
	drawPBarV();
	//draw slider one
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	//glTranslatef(sPosX, 200.0f, 0.0f);
	int vertMove1 = -(sPos1 - 300);
	glTranslatef(604.0f, 300 + vertMove1, 0.0f);
	drawSliderV();
	//draw bar 2
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	//glTranslatef(600.0f, 150.0f, 0.0f);
	glTranslatef(684.0f, 300.0f, 0.0f);
	drawPBarV();
	//draw slider 2
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	//glTranslatef(sPosY, 150.0f, 0.0f);
	int vertMove2 = -(sPos2 - 300);
	glTranslatef(684.0f, 300.0f + vertMove2, 0.0f);
	drawSliderV();
	//draw bar 3
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(764.0f, 300.0f, 0.0f);
	drawPBarV();
	//draw slider 3
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	//glTranslatef(sPosZ, 100.0f, 0.0f);
	int vertMove3 = -(sPos3 - 300);
	glTranslatef(764.0f, 300.0f + vertMove3, 0.0f);
	drawSliderV();

	//draw bar 4
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(684.0f, 490.0f, 0.0f);
	drawPBar();
	//draw slider 4
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(sPos4, 490.0f, 0.0f);
	drawSlider();

	//draw bar 5
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(100.0f, 150.0f, 0.0f);
	drawPBarV();
	//draw slider 5
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	//glTranslatef(sPosZ, 100.0f, 0.0f);
	int vertMove5 = -(sPos5 - 300);
	glTranslatef(100.0f, 150.0f + vertMove5, 0.0f);
	drawSliderV();

	//draw bar 6
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(250.0f, 150.0f, 0.0f);
	drawPBar();
	//draw slider 6
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);//set the camera
	glTranslatef(sPos6, 150.0f, 0.0f);
	drawSlider();

	glutSwapBuffers();
}

glm::vec3 e = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 Vd = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 u = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 P;
glm::vec3 V;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(4.0f, 4.0f, 1.0f);

void displayW1() {
	glutSetWindow(subWindow1);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 9.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(tValX, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, tValY, 0.0f));
	model = glm::rotate(model, glm::radians(sOffset1), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(sOffset2), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(sOffset3), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(sScale, sScale, sScale));//sScale
	
	glm::vec3* frameBuffer = new glm::vec3[subwindowWidth * subwindowHeight];
	glm::vec3* pix = frameBuffer;

	for (unsigned int j = 0; j < subwindowHeight; j++) {
		for (unsigned int i = 0; i < subwindowWidth; i++) {
			P = e + Vd * 1.0f + glm::cross(u, Vd) * (float)(i - (subwindowWidth / 2.0f)) * (float)(1.0f / (subwindowWidth / 2.0f)) + u * (j - (subwindowHeight / 2.0f)) * (1.0f / (subwindowHeight / 2.0f));
			V = glm::normalize(P - e);

			
			
			float t = 0.0f;
			//test each triangle in the object and see which triangle it intersects
			//and out of those triangles, which one is the closest
			float tNear = 100.0f; //the nearest t is infinity by default
			glm::vec3 normalNear = glm::vec3(0.0f, 0.0f, 0.0f);
			for (int k = 0; k < num_elems; k++) {
				glm::vec3 v1 = model * glm::vec4(vlist[flist[k]->verts[0]]->x, vlist[flist[k]->verts[0]]->y, vlist[flist[k]->verts[0]]->z, 1);
				glm::vec3 v2 = model * glm::vec4(vlist[flist[k]->verts[1]]->x, vlist[flist[k]->verts[1]]->y, vlist[flist[k]->verts[1]]->z, 1);
				glm::vec3 v3 = model * glm::vec4(vlist[flist[k]->verts[2]]->x, vlist[flist[k]->verts[2]]->y, vlist[flist[k]->verts[2]]->z, 1);
				glm::vec3 nt = glm::normalize(glm::cross(v2 - v1, v3 - v1));

				//intersection with the plane
				t = glm::dot((v1 - e), nt) / glm::dot(V, nt);

				if (t >= 1e-8) {

					//calculate point of intersection
					glm::vec3 S = e + V * glm::dot(v1 - e, nt / glm::dot(V, nt));

					float area = 0.5 * glm::length(glm::cross(v2 - v1, v3 - v1));
					float area1 = 0.5f * glm::length(glm::cross(v1 - S, v3 - S));
					float area2 = 0.5f * glm::length(glm::cross(v3 - S, v2 - S));
					float area3 = 0.5f * glm::length(glm::cross(v1 - S, v2 - S));

					if (((double)area1 + (double)area2 + (double)area3) <= (double)area + 1e-6 && t < tNear) {
						tNear = t;
						normalNear = nt;
					}
				}
			}

			glm::vec3 phit = e + tNear * V;
			//std::cout << phit.x << " " << phit.y << " " << phit.z << std::endl;
			if (tNear != 100.0f)
				*(pix++) = calcPointLight(normalNear, phit, glm::vec3(0.1f, 0.2f, 0.6f));
				//* (pix++) = glm::vec3(0.1f, 0.2f, 0.6f);
			else
				*(pix++) = glm::vec3(1.0f, 1.0f, 1.0f);
		}
	}
	
	//save results to a PPM image
	std::ofstream ofs("./out.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << subwindowWidth << " " << subwindowHeight << "\n255\n";
	for (unsigned int i = 0; i < subwindowHeight * subwindowWidth; i++) {
		char r = (char)(255 * clamp<float>(frameBuffer[i].x, 0, 1));
		char g = (char)(255 * clamp<float>(frameBuffer[i].y, 0, 1));
		char b = (char)(255 * clamp<float>(frameBuffer[i].z, 0, 1));
		ofs << r << g << b;
	}

	ofs.close();
	

	glDrawPixels(subwindowWidth, subwindowHeight, GL_RGB, GL_FLOAT, frameBuffer);

	delete[] frameBuffer;

	glutSwapBuffers();
}

glm::vec3 calcPointLight(glm::vec3 normal, glm::vec3 fragPos, glm::vec3 objectColor) {
	//ambient shading
	float ambientStrength = 0.1f;
	glm::vec3 ambient = ambientStrength * lightColor;

	//diffuse shading
	glm::vec3 lightDir = glm::normalize(lightPos - fragPos);
	float diff = glm::max(glm::dot(normal, lightDir), 0.0f);
	glm::vec3 diffuse = diff * lightColor;

	//specular shading
	float specularStrength = 0.5f;
	glm::vec3 viewDir = glm::normalize(e - fragPos);
	glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
	float spec = glm::pow(glm::max(glm::dot(viewDir, reflectDir), 0.0f), 0.0f);
	glm::vec3 specular = specularStrength * spec * lightColor;

	return (ambient + diffuse + specular) * objectColor;
}

bool rayTriangleIntersect(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, float& t) {
	//calculate normal
	glm::vec3 cross1 = glm::cross(v2 - v1, v3 - v1);
	glm::vec3 cross2 = glm::cross(v2 - v1, v3 - v1);
	glm::vec3 nt = cross1 / glm::vec3(fabs(cross2.x), fabs(cross2.y), fabs(cross2.z));

	//intersection with the plane
	t = glm::dot((v1 - origin), nt) / glm::dot(direction, nt);

	if (t < 1e-8) return false;

	//calculate point of intersection
	glm::vec3 S = origin + direction * glm::dot(v1 - origin, nt / glm::dot(direction, nt));

	float area = .5 * glm::length(cross2);
	float area1 = 0.5f * glm::length(glm::cross(v1 - S, v3 - S));
	float area2 = 0.5f * glm::length(glm::cross(v3 - S, v2 - S));
	float area3 = 0.5f * glm::length(glm::cross(v1 - S, v2 - S));
	
	if (((double)area1 + (double)area2 + (double)area3) > (double)area + 1e-8) return false;

	return true;
}

void displayAll() {
	display();
	displayW1();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	mainWindow = glutCreateWindow("bruh moment");

	//callbacks for the main window
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutIdleFunc(displayAll);
	//mouse callbacks
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	//sub windows
	subWindow1 = glutCreateSubWindow(mainWindow, border, (h + border) / 2, w - 2 * border, h / 2 - border * 3 / 2);
	glutDisplayFunc(displayW1);

	//char file[] = "tetrahedron.ply";
	char file[] = "icosahedron.ply";
	read_test(file);

	//for (int i = 0; i < IMAGE_SIZE; i++) {
	//	if(image[i] != 0)
	//		std::cout << (int)image[i];
	//}

	glutMainLoop();

	return 1;
}
