/*******************************************************************
           Multi-Part Model Construction and Manipulation
********************************************************************/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"
#include "Matrix3D.h"


const int meshSize = 32;    // Mesh size of 32x32. You can change this to 64 to make the holes more smooth
const int vWidth = 1024;     // Viewport width in pixels
const int vHeight = 576;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

// Lighting/shading and material properties for submarine - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat submarine_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat submarine_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat submarine_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat submarine_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground / sea floor 
static QuadMesh groundMesh;


//GLfloat model_matrix[16];
static int shoulder = 0;
static int elbow = 0;
static int rotation = 0;
static int shoulderZ = 0;


Matrix3D m;
Vector3D v;


//float position_x;
//float position_y;
//float position_z;

static float glLookAtX = 0.0;
static float glLookAtY = 6.0;
static float glLookAtZ = 11.5;
static float rotationAngle = 0.0;


 static GLfloat theta = 0.0;

 static float fovScale =1.0;
 float heightmap;


// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
Vector3D ScreenToWorld(int x, int y);
//added
void CollisionDetection(void);
void resetView(void);
void help(void);
int main(int argc, char **argv)
{

	
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(300, 30);
    glutCreateWindow(" Assignment 2");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);

    // Start event loop, never returns
    glutMainLoop();

    return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);   // This light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 

	//glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearColor(0.980, 0.922, 0.843,0);
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    // Set up ground/sea floor quad mesh
    Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
    Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
    Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
    groundMesh = NewQuadMesh(meshSize);
    InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

    Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
    Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
    Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
    SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

    // Set up the bounding box of the scene
    // Currently unused. You could set up bounding boxes for your objects eventually.
    //Set(&BBox.min, -8.0f, 0.0, -8.0);
    //Set(&BBox.max, 8.0f, 6.0,  8.0);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up the camera looking at the origin, up along positive y axis
	gluLookAt(glLookAtX, glLookAtY, glLookAtZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glScalef(fovScale,fovScale,fovScale); //Zooming
	glRotatef(theta, 0.0, 1.0, 0.0); //Scene rotation around Y-Axis

    // Set material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);
	glEnable(GL_COLOR_MATERIAL);

	glColor3f(0.439, 0.502, 0.565);
	glPushMatrix();
	glutSolidSphere(1.0,20,20); //Create the base
	//glutWireSphere(2.0, 4, 4);
	glPopMatrix();
	glPushMatrix();
	glRotatef((GLfloat)rotation, 0.0, 1.0, 0.0); //Shoulder yaw rotation 
	glPushMatrix();
	glRotatef((GLfloat)shoulderZ, 1.0, 0.0, 0.0); //shoulder pitch rotation in Z axis
	glRotatef((GLfloat)shoulder, 0.0, 0.0, 1.0); //shoulder pitch rotation
	glTranslatef(0.0, 2.0, 0.0);//Translate the shoulder boom up
	glPushMatrix();
	glScalef(0.5, 4, 0.5);
	glutSolidCube(1.0); //create boom
	glPopMatrix();
	glTranslatef(0.0, 2, 0.0);//Translate the arm up
	glRotatef((GLfloat)elbow, 0.0, 0.0, 1.0);
	glTranslatef(0, 2, 0.0);
	glPushMatrix();
	glScalef(0.5, 4, 0.5);
	glutSolidCube(1.0);//create arm
	glPopMatrix();
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glTranslatef(0, 0, 2);//Translate cone to the top
	glPushMatrix();
	//glGetFloatv(GL_MODELVIEW_MATRIX, model_matrix);
	glPopMatrix();
	glutSolidCone(.25, 0.5, 20, 10); //create drill
	glPopMatrix();
	glPopMatrix();//added
	glPopMatrix();

	//Create new vector representing the point starting at X: 0, Y: 0, Z:0
	v = NewVector3D(0, 0, 0);
	//Load Identity matrix
	m = NewIdentity(); 

	/*
	Keep track of all the transformations. You can use MatrixRightMultiply
	and use the code below in the proper order. However if you use left muliply
	you need to go in reverse order
	*/
	//MatrixLeftMultiplyV(&m, NewRotateY((GLfloat)rotation));
	//MatrixLeftMultiplyV(&m, NewRotateX((GLfloat)shoulderZ));
	//MatrixLeftMultiplyV(&m, NewRotateZ((GLfloat)shoulder));
	//MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	//MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	//MatrixLeftMultiplyV(&m, NewRotateZ((GLfloat)elbow));
	//MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	//MatrixLeftMultiplyV(&m, NewRotateX(-90));
	//MatrixLeftMultiplyV(&m, NewTranslate(0.0, 0.0, 2.0));


	MatrixLeftMultiplyV(&m, NewTranslate(0.0, 0.0, 2.0));
	MatrixLeftMultiplyV(&m, NewRotateX(-90));
	MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	MatrixLeftMultiplyV(&m, NewRotateZ((GLfloat)elbow));
	MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	MatrixLeftMultiplyV(&m, NewTranslate(0.0, 2.0, 0.0));
	MatrixLeftMultiplyV(&m, NewRotateZ((GLfloat)shoulder));
	MatrixLeftMultiplyV(&m, NewRotateX((GLfloat)shoulderZ));
	MatrixLeftMultiplyV(&m, NewRotateY((GLfloat)rotation));
	VectorLeftMultiply(&v, &m);
	//PrintVector(&v);

	//position_x = model_matrix[12];
	//position_y = model_matrix[13];
	//+ 13.1843681
	//position_z = model_matrix[14];
	//system("cls");
	//printf("%s%.6f", "X: ", position_x);
	//printf("\n");
	//printf("%s%.6f", "Y: ", position_y);
	//printf("\n");
	//printf("%s%.6f", "Z: ", position_z);
	//printf("\n");



	//MatrixPrint(&m);
	//printf("\n");

	//PrintVector(&v);

	CollisionDetection();
    // Draw ground/sea floor
	glDisable(GL_COLOR_MATERIAL);

    DrawMeshQM(&groundMesh, meshSize);

    glutSwapBuffers();   // Double buffering, swap buffers
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60*fovScale, (GLdouble)w / h, 0.2, 40.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'a':
		if (shoulder > 85) {
			break;
		}
		shoulder += 2;
		glutPostRedisplay();
		break;
	case 'd':
		if (shoulder <= 90 && shoulder >-70)
			shoulder -= 2;

		glutPostRedisplay();
		break;
	case 'w':
		if (elbow > 115) {
			break;
		}
		elbow += 2;
		glutPostRedisplay();
		break;
	case 's':
		if (elbow <= 120 && elbow >-115)
			elbow -= 2;
		glutPostRedisplay();
		break;
	case 'q':
		rotation = (rotation + 2) % 360;
		glutPostRedisplay();
		break;
	case 'e':
		rotation = (rotation - 2) % 360;
		glutPostRedisplay();
		break;
	case 'z':
		if (shoulderZ > 85) {
			break;
		}
		shoulderZ += 2;
		glutPostRedisplay();
		break;
	case 'x':
		if (shoulderZ <= 90 && shoulderZ >-70)
			shoulderZ -= 2;
		glutPostRedisplay();
		break;
	case '\t':
		resetView();
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay


}


// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
    // Help key
    if (key == GLUT_KEY_F1)
    {
		help();
	}
	else if (key == GLUT_KEY_LEFT) {
		//85?
		theta += 5 %360;

	}
	else if (key == GLUT_KEY_RIGHT) {

		theta += -5 % 360;
	}
	else if (key == GLUT_KEY_UP) {

		fovScale +=.05;

	}
	else if (key == GLUT_KEY_DOWN) {

		fovScale -= .05;
		if (fovScale<0.05) {
			fovScale = 0.05;
		}

	}
    // Do transformations with arrow keys
    //else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
    //{
    //}

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    currentButton = button;
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {

		}
	
        break;
    default:
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
		theta = xMouse*0.3;
    }
	if (currentButton == GLUT_RIGHT_BUTTON)
	{
		if (rotationAngle >= 0.11) {
			rotationAngle = 0.11;
		}
		else {
		rotationAngle += 0.0010;
		//fabs() restricts going below terrain
		glLookAtY = (fabs(yMouse*0.3))*sin(rotationAngle);
		}
	}
    glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
    // you will need to finish this if you use the mouse
    return NewVector3D(0, 0, 0);
}



void CollisionDetection(void) {
	//printf("%.2f", VectorGetY(&v));
	PrintVector(&v);
	heightmap = roundf(VectorGetY(&v) * 100) /100;
	//printf("%.2f\n", heightmap);
	if (heightmap <= 0.30) {
		MakeHole(&groundMesh, VectorGetX(&v), VectorGetZ(&v));
	}
	//glutIdleFunc(NULL);
	//glutPostRedisplay();
}

void help(void) {
	system("cls");
	printf("Move Shoulder Forward (->)         D\n\
Move Shoulder Back    (<-)         A\n\
Move Arm Forward      (->)         S\n\
Move Arm Back         (<-)         W\n\
Rotate Shoulder       (+Y)         Q\n\
Rotate Shoulder       (-Y)         E\n\
Rotate Shoulder       (+X)         Z\n\
Rotate Shoulder       (-X)         X\n\
Rotate Scene	      (-Y | ->)    Right Arrow OR Left-Click (Hold) + Drag Mouse\n\
Rotate Scene	      (+Y | <-)    Left Arrow OR Left-Click (Hold) + Drag Mouse\n\
Zoom In	                           Up Arrow\n\
Zoom Out	                   Up Arrow\n\
Zoom Over                          Right-Click (Hold) + Drag\n\
Reset Camera View                  Tab\n\
");
}

void resetView() {
	fovScale = 1.0;
	theta = 0.0;
	glLookAtX = 0.0;
	glLookAtY = 6.0;
	glLookAtZ = 11.5;
}