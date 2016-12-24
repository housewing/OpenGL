#include <iostream>
#include <vector>
#include <algorithm>

#include <GL\glew.h>
#include <GL\freeglut.h>

#include "loadObject.h"

#define WINDOW_TITLE_PREFIX "Advanced CG Example"

int CurrentWidth = 800, CurrentHeight = 600, WindowHandle = 0;

unsigned FrameCount = 0;
float deg = 0;

void ResizeFunction(int, int);//2
void RenderFunction(void); //1

Model obj;

void initialize()
{
	//GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 }; // 0.1, 0.1, 0.1, 1.0
	//GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 }; // 0.6, 0.6, 0.6, 1
	//GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 }; // 0.7, 0.7, 0.3, 1
	////GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	////glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	////glLightfv(GL_LIGHT0, GL_POSITION, light_position);


	GLfloat MaterialAmbient[] = { 0.2, 0.2, 0.2, 1.0f };
	GLfloat MaterialDiffuse[] = { 1.0, 1.0, 1.0, 1.0f };
	GLfloat MaterialSpecular[] = { 1.0, 1.0, 1.0, 1.0f };
	GLfloat AmbientLightPosition[] = { 1000, -1000, 1000, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, MaterialAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, MaterialDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, MaterialSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPosition);


	glEnable(GL_COLOR_MATERIAL);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // GL_FALSE
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);
	glEnable(GL_RESCALE_NORMAL);
}

int main(int argc, char* argv[])
{
	//初始化 glut
	glutInit(&argc, argv);

	//以下使用 Context 功能
	/*	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);*/

	//設定 glut 畫布尺寸 與color / depth模式
	glutInitWindowSize(CurrentWidth, CurrentHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	//根據已設定好的 glut (如尺寸,color,depth) 向window要求建立一個視窗，接著若失敗則退出程式
	WindowHandle = glutCreateWindow("This is my openGL Example");
	if (WindowHandle < 1) { printf("ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE); }

	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"ResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
	glutIdleFunc(RenderFunction);		  //閒置時...請系統執行"IdleFunction"

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) { fprintf(stderr, "ERROR: %s\n", glewGetErrorString(GlewInitResult));	exit(EXIT_FAILURE); }

	//背景顏色黑
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	initialize();

	//Giraffe  Agrippa_Rainbow  bunny
	obj.Load("gouraudShading/obj/bunny.obj");
	std::cout << "Vertex " << obj.V.size() << " Face " << obj.F.size() << " RGB " << obj.RGB.size() << std::endl;

	obj.SearchNeibor();
	//std::cout << "x min " << bounding[0] << " x max " << bounding[1] << " y min " << bounding[2] << " y max " << bounding[3] << " z min " << bounding[4] << " z max " << bounding[5] << std::endl;

	obj.RadiusOfCircle();
	//std::cout << "x " << center.X << " y " << center.Y << " z " << center.Z << " radius " << radius[0] << std::endl;

	obj.toOrigin(obj.center);

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
}


void RenderFunction(void)
{
	deg += 1.0;
	++FrameCount;
	printf("Render - %d\n", FrameCount);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, CurrentWidth, CurrentHeight);

	//http://stackoverflow.com/questions/2866350/move-camera-to-fit-3d-scene
	float fovy = 45.0f;
	float eyeX = 100.0f, eyeY = -500.0f, eyeZ = 500.0f;
	GLfloat aspect = (GLfloat)CurrentWidth / CurrentHeight;
	double camDis = (obj.radius * 2.0) / tan(fovy * CV_PI / 180.0f / 2.0);
	//std::cout << "camDis " << camDis << std::endl;

	Points newEye;
	newEye.X = eyeX - 0.0;
	newEye.Y = eyeY - 0.0;
	newEye.Z = eyeZ - 0.0;

	float norm = sqrt(newEye.X * newEye.X + newEye.Y * newEye.Y + newEye.Z * newEye.Z);

	newEye.X /= norm;
	newEye.Y /= norm;
	newEye.Z /= norm;
	//std::cout << "x " << newEye.X << " y " << newEye.Y << " z " << newEye.Z << std::endl;

	newEye.X *= camDis;
	newEye.Y *= camDis;
	newEye.Z *= camDis;

	//http://gamedev.stackexchange.com/questions/43588/how-to-rotate-camera-centered-around-the-cameras-position
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect, 1.0f, 1000.0f);
	//gluLookAt(eyeX, eyeY, eyeZ, center.X, center.Y, center.Z, 0, 1, 0);
	//gluLookAt(newEye.X, newEye.Y, newEye.Z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	gluLookAt(camDis * cos(-deg*3.1415 / 180), camDis * sin(-deg*3.1415 / 180), newEye.Z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//glRotatef(-5, 0.0f, 0.0f, 1.0f);
	//glRotatef(-5, 0.0f, 1.0, 0.0f);
	//glRotatef(-5, 1.0f, 0.0f, 0.0f);
	//glTranslatef(-newEye.X, -newEye.Y, -newEye.Z);
	//glTranslatef(camDis * cos(FrameCount), camDis * sin(FrameCount), -newEye.Z); // model rotate

	glPushMatrix();
	//glRotatef(deg, 0.0f, 0.0f, 1.0f);
		obj.Draw();
	glPopMatrix();


	glFlush();
	glutSwapBuffers();
	//printf("count = %d\n",FrameCount);
	// 
}
