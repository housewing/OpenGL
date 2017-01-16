#include <GL\glew.h>
#include <GL\freeglut.h>
#include <opencv2\opencv.hpp>

#include <iostream>

#include "loadObject.h"

#define WINDOW_TITLE_PREFIX "Advanced CG Example"

using namespace cv;

int CurrentWidth = 800,	CurrentHeight = 600,	WindowHandle = 0;
int count = 0;

void ResizeFunction(int, int);//2
void RenderFunction(void); //1

Model car;
Model route;
std::vector<Points> path;

void initialize()
{
	GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };  //  0.1, 0.1, 0.1, 1.0    0.2, 0.2, 0.2, 1.0
	GLfloat light_diffuse[] = { 0.6, 0.6, 0.6, 1.0 };  //  0.6, 0.6, 0.6, 1.0    1.0, 1.0, 1.0, 1.0
	GLfloat light_specular[] = { 0.7, 0.7, 0.3, 1.0 }; //  0.7, 0.7, 0.3, 1.0    1.0, 1.0, 1.0, 1.0
	//GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_COLOR_MATERIAL);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // GL_FALSE
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);
	glEnable(GL_RESCALE_NORMAL);
}

void loadPath(char* _path)
{
	std::ifstream ifs2(_path, std::ifstream::in);
	std::string line;
	while (getline(ifs2, line))
	{
		Points pt;
		std::istringstream in(line);
		in >> pt.X >> pt.Y >> pt.Z;
		path.push_back(pt);
	}
	ifs2.close();
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
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA);
	
	//根據已設定好的 glut (如尺寸,color,depth) 向window要求建立一個視窗，接著若失敗則退出程式
	WindowHandle = glutCreateWindow("This is my openGL Example");
	if(WindowHandle < 1) {	printf("ERROR: Could not create a new rendering window.\n");exit(EXIT_FAILURE);	}
	
	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"ResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
	glutIdleFunc(RenderFunction);		  //閒置時...請系統執行"IdleFunction"

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK ) {	fprintf(stderr,"ERROR: %s\n",glewGetErrorString(GlewInitResult)	);	exit(EXIT_FAILURE);	}

	//背景顏色黑
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	initialize();

	car.Load("RollerCoaster/obj/car_model.obj");
	route.Load("RollerCoaster/obj/route_model.obj");
	loadPath("RollerCoaster/route.txt");

	car.SearchNeibor();
	route.SearchNeibor();

	car.RadiusOfCircle();
	//std::cout << "center " << car.center.X << " " << car.center.Y << " " << car.center.Z << std::endl;

	car.toOrigin(car.center);

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
	count += 1;

	float fovy = 45.0f;
	float eyeX = 0.0f, eyeY = -500.0f, eyeZ = 500.0f;
	GLfloat aspect = (GLfloat)CurrentWidth / CurrentHeight;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect, 1.0f, 1000.0f);
	gluLookAt(eyeX, eyeY, eyeZ, 0, 0, 0, 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int idx = count % (path.size());

	Points a = path[idx];
	Points b = path[idx + 1];
	//std::cout << "idx " << idx << " " << idx + 1 << std::endl;

	Points vecA, vecB, vecC, vecG, tmp, aa;
	aa.X = a.X - b.X;
	aa.Y = a.Y - b.Y;
	aa.Z = a.Z - b.Z;

	vecG.X = 0.0;
	vecG.Y = 0.0;
	vecG.Z = -1.0;

	vecA = normalize(aa);

	tmp = calCross(aa, vecG);
	vecB = normalize(tmp);

	tmp = calCross(aa, vecB);
	vecC = normalize(tmp);

	// pose matrix ax ay az 0
	//             bx by bz 0
	//             cx cy cz 0
	//             tx ty tz 1
	Mat transformMatrix = (Mat_<float>(4, 4) << vecA.X, vecA.Y, vecA.Z, 0.0, vecB.X, vecB.Y, vecB.Z, 0.0, vecC.X, vecC.Y, vecC.Z, 0.0, path[idx].X, path[idx].Y, path[idx].Z, 1.0);

	glPushMatrix();
		glMultMatrixf((float*)transformMatrix.data);
		glRotatef(90, 0.0, 0.0, 1.0);
		car.Draw();
	glPopMatrix();

	glPushMatrix();
		route.Draw();
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
	Sleep(50);
}
