#include <iostream>
#include <algorithm>
#include <vector>

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <opencv2\opencv.hpp>

#include "loadObject.h"

using namespace cv;

int CurrentWidth = 800,	CurrentHeight = 600,	WindowHandle = 0;

unsigned FrameCount = 0;
unsigned int countIdleTime=0;

void ResizeFunction(int, int);
void RenderFunction(void);
void IdleFunction(void);
void ColorMap(float InValue,float Lowerbound, float Upperbound,float *OutputRGB);

Model obj;

void ResizeFunction(int Width, int Height)
{
 	CurrentWidth = Width;
 	CurrentHeight = Height;
// 	glViewport(0, 0, CurrentWidth, CurrentHeight);
	
}

void RenderFunction(void)
{
	//int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//float M[]={1,0,0,0,  0,1,0,0,   0,0,1,0,  300,0,0,1 };

 	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-float(CurrentWidth) / 2.0, float(CurrentWidth) / 2.0, -float(CurrentHeight) / 2.0, float(CurrentHeight) / 2.0, -CurrentHeight*10.0, CurrentHeight*10.0);

	gluLookAt(0, 0, 1000, 0, 0, 0, 0.0, 1, 0);


	
	//glCallLists(1,GL_UNSIGNED_BYTE,myLists);	//較快 但不能動態改變資料
	
	glMatrixMode(GL_MODELVIEW);
	
	//float rgb[3];

	glDisable(GL_LIGHTING);
	glPushMatrix();

	obj.Draw();

		//glBegin(GL_LINES);
		//	for (float iValue=-10;iValue<10.0;iValue+=0.01)
		//	{
		//		ColorMap(iValue,-12.0,12.0,rgb); //在[-12,12] 範圍內中繪製-10~10的對應顏色
		//		glColor3fv(rgb);
		//		glVertex3f(iValue*30.0,-10,0);
		//		glVertex3f(iValue*30.0,10,0);	
		//		//std::cout << "r " << rgb[0] << " g " << rgb[1] << " b " << rgb[2] << std::endl;
		//	}
		//glEnd();
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
	//glutPostRedisplay();
}

void IdleFunction(void)
{
	printf("%d\n",countIdleTime++);
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
	WindowHandle = glutCreateWindow("This is my openGL Example # 5");
	if(WindowHandle < 1) {	fprintf(stderr,"ERROR: Could not create a new rendering window.\n");exit(EXIT_FAILURE);	}
	
	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"AResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
	//glutIdleFunc(IdleFunction);		  //閒置時...請系統執行"IdleFunction"

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK ) {	fprintf(stderr,"ERROR: %s\n",glewGetErrorString(GlewInitResult)	);	exit(EXIT_FAILURE);	}

	//背景顏色黑
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat MaterialAmbient[] = {0.4,0.4,0.4,1.0f};
	GLfloat MaterialDiffuse[] = {0.7,0.7,0.7,1.0f};
	GLfloat MaterialSpecular[] =  { 1.2,1.2,1.2, 1.0f};
	GLfloat AmbientLightPosition[] = {1000,0,0,1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, MaterialAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, MaterialDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, MaterialSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPosition);

	obj.Load("obj/dragon.obj");
	std::cout << "Vertex " << obj.V.size() << " Face " << obj.F.size() << std::endl;

	obj.SearchNeibor();

	//for (size_t i = 0; i < 10; i++)
	//{
	//	std::cout << "i " << i << " " << obj.VN[i].NX << " " << obj.VN[i].NY << " " << obj.VN[i].NZ << std::endl;
	//	for (std::set<int>::iterator j = obj.Neibor[i].begin(); j != obj.Neibor[i].end(); j++)
	//	{
	//		std::cout << " " << *j << " ";
	//	}
	//	std::cout << std::endl;
	//}

	std::vector<float> curvatureSet;
	for (size_t i = 0; i < obj.V.size(); i++)
	{
		float *curvature = obj.calCurvature(obj.Neibor[i], i);
		//	std::cout << "meanCurve " << meanCurve << std::endl;

		curvatureSet.push_back(curvature[0]);
	}

	std::vector<float> sortCurvature;
	sortCurvature.assign(curvatureSet.begin(), curvatureSet.end());
	std::sort(sortCurvature.begin(), sortCurvature.end());
	int median = sortCurvature.size() / 2;
	std::cout << "median " << sortCurvature[median] << std::endl;

	for (size_t i = 0; i < obj.V.size(); i++)
	{
		float rgb[3];

		ColorMap(curvatureSet[i], sortCurvature[median] - 0.25, sortCurvature[median] + 0.25, rgb);

		Points tmp;
		tmp.X = rgb[0];
		tmp.Y = rgb[1];
		tmp.Z = rgb[2];
		obj.RGB.push_back(tmp);
	}

	obj.Write("obj/test.obj");

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void ColorMap(float InValue,float Lowerbound, float Upperbound,float *OutputRGB)
{
	unsigned char RGB_table[][3]={{0,0,8},
	{0,1,14},
	{0,2,20},
	{0,3,26},
	{0,4,32},
	{0,5,38},
	{0,6,44},
	{0,7,50},
	{0,8,56},
	{0,9,62},
	{0,10,68},
	{0,11,74},
	{0,12,80},
	{0,13,86},
	{0,14,92},
	{0,15,98},
	{0,16,104},
	{0,17,110},
	{0,18,116},
	{0,19,122},
	{0,20,128},
	{0,21,134},
	{0,22,140},
	{0,23,146},
	{0,24,152},
	{0,25,158},
	{0,26,164},
	{0,27,170},
	{0,28,176},
	{0,29,182},
	{0,30,188},
	{0,31,194},
	{0,32,200},
	{0,36,199},
	{0,40,198},
	{0,44,197},
	{0,48,196},
	{0,52,195},
	{0,56,194},
	{0,60,193},
	{0,64,192},
	{0,68,191},
	{0,72,190},
	{0,76,189},
	{0,80,188},
	{0,84,187},
	{0,88,186},
	{0,92,185},
	{0,96,184},
	{0,100,183},
	{0,104,182},
	{0,108,181},
	{0,112,180},
	{0,116,179},
	{0,120,178},
	{0,124,177},
	{0,128,176},
	{0,132,175},
	{0,136,174},
	{0,140,173},
	{0,144,172},
	{0,148,171},
	{0,152,170},
	{0,156,169},
	{0,160,168},
	{0,162,165},
	{0,164,162},
	{0,166,159},
	{0,168,156},
	{0,170,153},
	{0,172,150},
	{0,174,147},
	{0,176,144},
	{0,178,141},
	{0,180,138},
	{0,182,135},
	{0,184,132},
	{0,186,129},
	{0,188,126},
	{0,190,123},
	{0,192,120},
	{0,194,117},
	{0,196,114},
	{0,198,111},
	{0,200,108},
	{0,202,105},
	{0,204,102},
	{0,206,99},
	{0,208,96},
	{0,210,93},
	{0,212,90},
	{0,214,87},
	{0,216,84},
	{0,218,81},
	{0,220,78},
	{0,222,75},
	{0,224,70},
	{8,225,68},
	{16,226,66},
	{24,227,64},
	{32,228,62},
	{40,229,60},
	{48,230,58},
	{56,231,56},
	{64,232,54},
	{72,233,52},
	{80,234,50},
	{88,235,48},
	{96,236,46},
	{104,237,44},
	{112,238,42},
	{120,239,40},
	{128,240,38},
	{136,241,36},
	{144,242,34},
	{152,243,32},
	{160,244,30},
	{168,245,28},
	{176,246,26},
	{184,247,24},
	{192,248,22},
	{200,249,20},
	{208,250,18},
	{216,251,16},
	{224,252,14},
	{232,253,12},
	{240,254,10},
	{248,255,8},
	{255,255,0},
	{255,251,0},
	{255,247,0},
	{255,243,0},
	{255,239,0},
	{255,235,0},
	{255,231,0},
	{255,227,0},
	{255,223,0},
	{255,219,0},
	{255,215,0},
	{255,211,0},
	{255,207,0},
	{255,203,0},
	{255,199,0},
	{255,195,0},
	{255,191,0},
	{255,187,0},
	{255,183,0},
	{255,179,0},
	{255,175,0},
	{255,171,0},
	{255,167,0},
	{255,163,0},
	{255,159,0},
	{255,155,0},
	{255,151,0},
	{255,147,0},
	{255,143,0},
	{255,139,0},
	{255,135,0},
	{255,131,0},
	{255,124,0},
	{255,120,2},
	{255,116,4},
	{255,112,6},
	{255,108,8},
	{255,104,10},
	{255,100,12},
	{255,96,14},
	{255,92,16},
	{255,88,18},
	{255,84,20},
	{255,80,22},
	{255,76,24},
	{255,72,26},
	{255,68,28},
	{255,64,30},
	{255,60,32},
	{255,56,34},
	{255,52,36},
	{255,48,38},
	{255,44,40},
	{255,40,42},
	{255,36,44},
	{255,32,46},
	{255,28,48},
	{255,24,50},
	{255,20,52},
	{255,16,54},
	{255,12,56},
	{255,8,58},
	{255,4,60},
	{255,0,62},
	{255,0,64},
	{255,2,70},
	{255,4,76},
	{255,6,82},
	{255,8,88},
	{255,10,94},
	{255,12,100},
	{255,14,106},
	{255,16,112},
	{255,18,118},
	{255,20,124},
	{255,22,130},
	{255,24,136},
	{255,26,142},
	{255,28,148},
	{255,30,154},
	{255,32,160},
	{255,34,166},
	{255,36,172},
	{255,38,178},
	{255,40,184},
	{255,42,190},
	{255,44,196},
	{255,46,202},
	{255,48,208},
	{255,50,214},
	{255,52,220},
	{255,54,226},
	{255,56,232},
	{255,58,238},
	{255,60,244},
	{255,62,250},
	{255,63,255},
	{255,69,255},
	{255,75,255},
	{255,81,255},
	{255,87,255},
	{255,93,255},
	{255,99,255},
	{255,105,255},
	{255,111,255},
	{255,117,255},
	{255,123,255},
	{255,129,255},
	{255,135,255},
	{255,141,255},
	{255,147,255},
	{255,153,255},
	{255,159,255},
	{255,165,255},
	{255,171,255},
	{255,177,255},
	{255,183,255},
	{255,189,255},
	{255,195,255},
	{255,201,255},
	{255,207,255},
	{255,213,255},
	{255,219,255},
	{255,225,255},
	{255,231,255},
	{255,237,255},
	{255,243,255},
	{255,249,255}};
	float tmp;
	if (Lowerbound>Upperbound) 
	{  tmp = Upperbound;   Upperbound = Lowerbound; Lowerbound = tmp; } //Make sure Lowerbound < Upperbound;

	if (InValue<=Lowerbound) {	for (int i=0;i<3;i++)
		OutputRGB[i] = float(RGB_table[0][i]) / 255.0 ;
	return; }
	if (InValue>=Upperbound) {	for (int i=0;i<3;i++)
		OutputRGB[i] = float(RGB_table[255][i]) / 255.0 ;
	return; }
	int index = (int) (255.0*(InValue-Lowerbound)/(Upperbound-Lowerbound));

	if (index<0) index = 0;
	if (index>255) index = 255;

	for (int i=0;i<3;i++)
		OutputRGB[i] = float(RGB_table[index][i]) / 255.0 ;
	return;
}
