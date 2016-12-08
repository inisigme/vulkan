/*
 * smooth_opengl3.c, based on smooth.c, which is (c) by SGI, see below.
 * This program demonstrates smooth shading in a way which is fully
 * OpenGL-3.1-compliant.
 * A smooth shaded polygon is drawn in a 2-D projection.
 */

/*
 * Original copyright notice from smooth.c:
 *
 * License Applicability. Except to the extent portions of this file are
 * made subject to an alternative license as permitted in the SGI Free
 * Software License B, Version 1.1 (the "License"), the contents of this
 * file are subject only to the provisions of the License. You may not use
 * this file except in compliance with the License. You may obtain a copy
 * of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
 * Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
 * 
 * http://oss.sgi.com/projects/FreeB
 * 
 * Note that, as provided in the License, the Software is distributed on an
 * "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
 * DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
 * CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 * 
 * Original Code. The Original Code is: OpenGL Sample Implementation,
 * Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
 * Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
 * Copyright in any portions created by third parties is as indicated
 * elsewhere herein. All Rights Reserved.
 * 
 * Additional Notice Provisions: The application programming interfaces
 * established by SGI in conjunction with the Original Code are The
 * OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
 * April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
 * 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
 * Window System(R) (Version 1.3), released October 19, 1998. This software
 * was created using the OpenGL(R) version 1.2.1 Sample Implementation
 * published by SGI, but has not been independently verified as being
 * compliant with the OpenGL(R) version 1.2.1 Specification.
 *
 */

#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

/* report GL errors, if any, to stderr */
void checkError(const char *functionName)
{
   GLenum error;
   while (( error = glGetError() ) != GL_NO_ERROR) {
       fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
   }
}

/* extension #defines, types and entries, avoiding a dependency on additional
   libraries like GLEW or the GL/glext.h header */
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif

#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif

#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

typedef ptrdiff_t ourGLsizeiptr;
typedef char ourGLchar;

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef GL_ARB_vertex_array_object
typedef void (APIENTRY *PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRY *PFNGLBINDVERTEXARRAYPROC) (GLuint array);
#endif
#ifndef GL_VERSION_1_5
typedef void (APIENTRY *PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY *PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY *PFNGLBUFFERDATAPROC) (GLenum target, ourGLsizeiptr size, const GLvoid *data, GLenum usage);
#endif
#ifndef GL_VERSION_2_0
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const ourGLchar **string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, ourGLchar *infoLog);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, ourGLchar *infoLog);
typedef GLint (APIENTRY *PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const ourGLchar *name);
typedef void (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const ourGLchar *name);
typedef void (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif

PFNGLGENVERTEXARRAYSPROC gl_GenVertexArrays;
PFNGLBINDVERTEXARRAYPROC gl_BindVertexArray;
PFNGLGENBUFFERSPROC gl_GenBuffers;
PFNGLBINDBUFFERPROC gl_BindBuffer;
PFNGLBUFFERDATAPROC gl_BufferData;
PFNGLCREATESHADERPROC gl_CreateShader;
PFNGLSHADERSOURCEPROC gl_ShaderSource;
PFNGLCOMPILESHADERPROC gl_CompileShader;
PFNGLCREATEPROGRAMPROC gl_CreateProgram;
PFNGLATTACHSHADERPROC gl_AttachShader;
PFNGLLINKPROGRAMPROC gl_LinkProgram;
PFNGLUSEPROGRAMPROC gl_UseProgram;
PFNGLGETSHADERIVPROC gl_GetShaderiv;
PFNGLGETSHADERINFOLOGPROC gl_GetShaderInfoLog;
PFNGLGETPROGRAMIVPROC gl_GetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC gl_GetProgramInfoLog;
PFNGLGETATTRIBLOCATIONPROC gl_GetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC gl_VertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC gl_EnableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC gl_GetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC gl_UniformMatrix4fv;

void initExtensionEntries(void)
{
   gl_GenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) glutGetProcAddress ("glGenVertexArrays");
   gl_BindVertexArray = (PFNGLBINDVERTEXARRAYPROC) glutGetProcAddress ("glBindVertexArray");
   if (!gl_GenVertexArrays || !gl_BindVertexArray)
   {
       fprintf (stderr, "glGenVertexArrays or glBindVertexArray not found");
       exit(1);
   }
   gl_GenBuffers = (PFNGLGENBUFFERSPROC) glutGetProcAddress ("glGenBuffers");
   gl_BindBuffer = (PFNGLBINDBUFFERPROC) glutGetProcAddress ("glBindBuffer");
   gl_BufferData = (PFNGLBUFFERDATAPROC) glutGetProcAddress ("glBufferData");
   if (!gl_GenBuffers || !gl_BindBuffer || !gl_BufferData)
   {
       fprintf (stderr, "glGenBuffers, glBindBuffer or glBufferData not found");
       exit(1);
   }
   gl_CreateShader = (PFNGLCREATESHADERPROC) glutGetProcAddress ("glCreateShader");
   gl_ShaderSource = (PFNGLSHADERSOURCEPROC) glutGetProcAddress ("glShaderSource");
   gl_CompileShader = (PFNGLCOMPILESHADERPROC) glutGetProcAddress ("glCompileShader");
   gl_CreateProgram = (PFNGLCREATEPROGRAMPROC) glutGetProcAddress ("glCreateProgram");
   gl_AttachShader = (PFNGLATTACHSHADERPROC) glutGetProcAddress ("glAttachShader");
   gl_LinkProgram = (PFNGLLINKPROGRAMPROC) glutGetProcAddress ("glLinkProgram");
   gl_UseProgram = (PFNGLUSEPROGRAMPROC) glutGetProcAddress ("glUseProgram");
   gl_GetShaderiv = (PFNGLGETSHADERIVPROC) glutGetProcAddress ("glGetShaderiv");
   gl_GetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) glutGetProcAddress ("glGetShaderInfoLog");
   gl_GetProgramiv = (PFNGLGETPROGRAMIVPROC) glutGetProcAddress ("glGetProgramiv");
   gl_GetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) glutGetProcAddress ("glGetProgramInfoLog");
   gl_GetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) glutGetProcAddress ("glGetAttribLocation");
   gl_VertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) glutGetProcAddress ("glVertexAttribPointer");
   gl_EnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) glutGetProcAddress ("glEnableVertexAttribArray");
   gl_GetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) glutGetProcAddress ("glGetUniformLocation");
   gl_UniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) glutGetProcAddress ("glUniformMatrix4fv");
   if (!gl_CreateShader || !gl_ShaderSource || !gl_CompileShader || !gl_CreateProgram || !gl_AttachShader || !gl_LinkProgram || !gl_UseProgram || !gl_GetShaderiv || !gl_GetShaderInfoLog || !gl_GetProgramiv || !gl_GetProgramInfoLog || !gl_GetAttribLocation || !gl_VertexAttribPointer || !gl_EnableVertexAttribArray || !gl_GetUniformLocation || !gl_UniformMatrix4fv)
   {
       fprintf (stderr, "glCreateShader, glShaderSource, glCompileShader, glCreateProgram, glAttachShader, glLinkProgram, glUseProgram, glGetShaderiv, glGetShaderInfoLog, glGetProgramiv, glGetProgramInfoLog, glGetAttribLocation, glVertexAttribPointer, glEnableVertexAttribArray, glGetUniformLocation or glUniformMatrix4fv not found");
       exit(1);
   }
}


typedef struct {
	GLdouble eyex ;
	GLdouble eyey ;
	GLdouble eyez ;
	GLdouble centerx ;
	GLdouble centery ;
	GLdouble centerz ;
	GLdouble distance ;
	GLdouble angle;
} View;
View getView() {
	View v;
	v.eyex = 0;
	v.eyey = 0;
	v.eyez = 3;
	v.centerx = 0;
	v.centery = 0;
	v.centerz = -100;
	v.distance = 0.1;
	v.angle = 0.2;
	return v;
}
View view;

View goFoward(View v, GLdouble direction) {
	View newV;
	v.distance *= direction;
	GLdouble deltaX, deltaY, deltaZ = 0;
	GLdouble vecX, vecY, vecZ = 0;
	deltaX = v.centerx - v.eyex;
	deltaY = v.centery - v.eyey;
	deltaZ = v.centerz - v.eyez;
	GLdouble len = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ*deltaZ);
	deltaX = deltaX / len*v.distance;
	deltaY = deltaY / len*v.distance;
	deltaZ = deltaZ / len*v.distance;
	newV.eyex = v.eyex + deltaX;
	newV.eyey = v.eyey + deltaY;
	newV.eyez = v.eyez + deltaZ;
	newV.centerx = v.centerx + deltaX;
	newV.centery = v.centery + deltaY;
	newV.centerz = v.centerz + deltaZ;
	newV.distance = v.distance / direction;
	newV.angle = v.angle;
	return newV;
}

View poziomo(View v, GLdouble direction) {
	View newV;
	v.angle *= direction;
	newV.eyex = v.eyex;
	newV.eyey = v.eyey;
	newV.eyez = v.eyez;
	GLdouble deltaX = v.centerx - v.eyex;
	GLdouble deltaZ = v.centerz - v.eyez;
	newV.centerx = v.eyex + deltaX * cos(v.angle) - deltaZ * sin (v.angle);
	newV.centery = v.centery ;
	newV.centerz = v.eyez + deltaX * sin(v.angle) + deltaZ * cos (v.angle);
	newV.distance = v.distance;
	newV.angle = v.angle / direction;
	return newV;
}


enum
{
	FULL_WINDOW, // aspekt obrazu - całe okno
	ASPECT_1_1, // aspekt obrazu 1:1
	EXIT // wyjście
};

// aspekt obrazu

int Aspect = FULL_WINDOW;

// funkcja generująca scenę 3D
void rysujKostke();
void rysujUklad();
GLfloat *tablica;
GLdouble *tablicaZPliku;
size_t result;
void displayFromFile();
void Display()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	

	gluLookAt(view.eyex, view.eyey, view.eyez, view.centerx, view.centery, view.centerz, 0, 1, 0);

	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 1.0f);
	//printf("%i\n", result);
	for (int i = 0; i < result; i++) {
		glVertex3f(tablica[i * 3], tablica[i * 3 + 1], tablica[i * 3 + 2]);
		printf("%i ---------- %f - %f - %f\n", i, tablica[i * 3], tablica[i * 3 + 1], tablica[i * 3 + 2]);
		Sleep(2000);
	}
	glEnd();

	rysujUklad();
	//rysujKostke();


	glFlush();
	glutSwapBuffers();
}


void Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLdouble cos = 0.01;
	glFrustum(-cos, cos, -cos, cos, 0.01, 500.0);
	Display();
}

// obsługa klawiatury

void Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '5':
		view = goFoward(view, 1.0);
		break;
	case '0':
		view = goFoward(view, -1.0);
		break;
	case '4':
		view = poziomo(view, 1.0);
		break;
	case '6':
		view = poziomo(view, -1.0);
		break;
	default:
		break;
	}
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obsługa klawiszy funkcyjnych i klawiszy kursora

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		view.eyex += 0.1;
		break;

		// kursor w górę
	case GLUT_KEY_UP:
		view.eyey -= 0.1;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		view.eyex -= 0.1;
		break;

		// kursor w dół
	case GLUT_KEY_DOWN:
		view.eyey += 0.1;
		break;
	}

	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}


int main(int argc, char * argv[])
{
	displayFromFile();
	view = getView();
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// rozmiary głównego okna programu
	glutInitWindowSize(1024, 768);

	// utworzenie głównego okna programu
	glutCreateWindow("Sześcian 4");

	// dołączenie funkcji generującej scenę 3D
	glutDisplayFunc(Display);

	// dołączenie funkcji wywoływanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// dołączenie funkcji obsługi klawiatury
	glutKeyboardFunc(Keyboard);

	// dołączenie funkcji obsługi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);


	// wprowadzenie programu do obsługi pętli komunikatów
	glutMainLoop();
	return 0;
}



void rysujUklad() {
	glBegin(GL_LINES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0, 0.0, 10000.0);
	glVertex3f(0.0, 0.0, -10000.0);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0, 10000.0, 0.0);
	glVertex3f(0.0, -10000.0, 0.0);
	
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(10000.0, 0.0, 0.0);
	glVertex3f(-10000.0, 0.0, 0.0);

	glEnd();
}


void rysujKostke(){

	glColor3f(0.0, 0.0, 0.0);

	// początek definicji krawędzi sześcianu
	glBegin(GL_LINES);
	// wspólrzędne kolejnych krawędzi sześcianu
	glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(1.0, -1.0, 1.0);

	glVertex3f(1.0, -1.0, 1.0);
	glVertex3f(1.0, -1.0, -1.0);

	glVertex3f(1.0, -1.0, -1.0);
	glVertex3f(1.0, 1.0, -1.0);

	glVertex3f(1.0, 1.0, -1.0);
	glVertex3f(1.0, 1.0, 1.0);

	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0, -1.0, 1.0);

	glVertex3f(-1.0, -1.0, 1.0);
	glVertex3f(-1.0, -1.0, -1.0);

	glVertex3f(-1.0, -1.0, -1.0);
	glVertex3f(-1.0, 1.0, -1.0);

	glVertex3f(-1.0, 1.0, -1.0);
	glVertex3f(-1.0, 1.0, 1.0);	

	glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, 1.0);

	glVertex3f(1.0, -1.0, 1.0);
	glVertex3f(-1.0, -1.0, 1.0);

	glVertex3f(1.0, -1.0, -1.0);
	glVertex3f(-1.0, -1.0, -1.0);

	glVertex3f(1.0, 1.0, -1.0);
	glVertex3f(-1.0, 1.0, -1.0);

	// koniec definicji prymitywu
	glEnd();

}


GLdouble tap[] = { 1.0,0.0,0.0, 2.0, 0.0, 0.0,  5.0, 5.0, -5.0 };
void displayFromFile() {

	FILE *fp = fopen("dane", "rb");
	if (!fp) perror("dane"), exit(1);

	long lSize = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	rewind(fp);

	char *buffer = calloc(1, lSize + 1);
	if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

	if (1 != fread(buffer, lSize, 1, fp))
		fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

	result = lSize / 4 / 3;
	printf("%d result\n", lSize);
	fclose(fp);
	tablica = (GLfloat *)buffer;
}