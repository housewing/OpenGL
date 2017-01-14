#include <GL/glew.h>
#include <GL/freeglut.h>

#include <opencv2\opencv.hpp>

#include <iostream>

#define WINDOW_TITLE_PREFIX "Advanced CG Example"
#define M_PI 3.1415926

using namespace cv;

int CurrentWidth = 800,	CurrentHeight = 600,	WindowHandle = 0;
bool mouseLeftPressed = 0;
int mouseClickX, mouseClickY;
float ratio = 1.0;

Mat teapotPose = Mat::eye(4, 4, CV_32F);
GLuint textureL;
GLuint textureR;
GLuint textureE;

void ResizeFunction(int, int);//2
void RenderFunction(void); //1

class Points
{
public:
	float x;
	float y;
	float z;
};

void MouseFunc(int button, int state, int x, int y)
{
	printf("button = %d, state = %d\n", button, state);
	if (state)
	{
		if (button == 0) { mouseLeftPressed = 0; }
	}
	else
	{
		if (button == 0) { mouseLeftPressed = 1; mouseClickX = x; mouseClickY = y; }
		else if (button == 3) { ratio += 0.01; RenderFunction(); }
		else if (button == 4) { ratio -= 0.01; RenderFunction(); }
	}
}

void MouseMotion(int x, int y)
{
	printf("x = %d , y = %d\n", x, y);
	if (mouseLeftPressed)
	{
		Mat rotateY = Mat::eye(4, 4, CV_32F);
		float dx = float(x - mouseClickX) / 180.0*3.14159;  //移動 1 pixel轉1度
		float cs = cos(dx);
		float sn = sin(dx);
		rotateY.at<float>(0, 0) = cs;		rotateY.at<float>(0, 2) = sn;
		rotateY.at<float>(2, 0) = -sn;		rotateY.at<float>(2, 2) = cs;

		Mat rotateX = Mat::eye(4, 4, CV_32F);
		float dy = float(y - mouseClickY) / 180.0*3.14159;  //移動 1 pixel轉-1度
		cs = cos(dy);
		sn = sin(dy);
		rotateX.at<float>(1, 1) = cs;		rotateX.at<float>(1, 2) = -sn;
		rotateX.at<float>(2, 1) = sn;		rotateX.at<float>(2, 2) = cs;

		teapotPose = rotateX*rotateY*teapotPose;

		mouseClickX = x;  //重新記錄目前位置
		mouseClickY = y;
		RenderFunction();
	}
}

int initTexture(std::string _name){
	Mat img = imread(_name);
	if (img.empty())
	{
		std::cout << "can not found image " << std::endl;
	}

	Mat textureBitmap(img.rows, img.cols, CV_8UC3);
	Mat textureBitmapFlip(img.rows, img.cols, CV_8UC3);
	resize(img, textureBitmap, textureBitmap.size(), 0, 0, INTER_LINEAR);
	flip(textureBitmap, textureBitmapFlip, 0);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, textureBitmapFlip.data);

	return texture;;
}

void initialize()
{
	textureE = initTexture("TextureMapping/R000.jpg");
	textureE = initTexture("TextureMapping/L000.jpg");
	textureE = initTexture("TextureMapping/earth.bmp");

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
	glEnable(GL_RESCALE_NORMAL);
}

void drawSphere(double r, int lats, int longs)
{
	//http://stackoverflow.com/questions/16656776/sphere-texture-mapping-opengl-and-c
    int i, j;
	float texY1 = float(0) / float(lats);
    for (i = 1; i <= lats; i++) 
	{
		double lat0 = M_PI * (-0.5 + (double)(i - 1) / lats);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);

		double lat1 = M_PI * (-0.5 + (double)i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		float texY0 = texY1;
		texY1 = float(i) / float(lats);

		glBegin(GL_QUAD_STRIP); //GL_QUAD_STRIP GL_POINTS
		for (j = 0; j <= longs; j++)
		{
			double lng = 2 * M_PI * (double)(j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			float texX = (float(j) / float(longs / 2));

			glTexCoord2f(texX, texY0);
			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glTexCoord2f(texX, texY1);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
}

//https://www.opengl.org/discussion_boards/showthread.php/166271-To-Draw-a-3D-Circle-and-Applying-Texturing
GLvoid draw_circle(const GLfloat radius, const GLuint num_vertex)
{
	GLfloat vertex[4];
	GLfloat texcoord[2];

	const GLfloat delta_angle = 2.0*M_PI / num_vertex;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureL);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_TRIANGLE_FAN);

	//draw the vertex at the center of the circle
	texcoord[0] = 0.5;
	texcoord[1] = 0.5;
	glTexCoord2fv(texcoord);

	vertex[0] = vertex[1] = vertex[2] = 0.0;
	vertex[3] = 1.0;
	glVertex4fv(vertex);

	for (int i = 0; i < num_vertex; i++)
	{
		texcoord[0] = (std::cos(delta_angle*i) + 1.0)*0.5;
		texcoord[1] = (std::sin(delta_angle*i) + 1.0)*0.5;
		glTexCoord2fv(texcoord);

		vertex[0] = std::cos(delta_angle*i) * radius;
		vertex[1] = std::sin(delta_angle*i) * radius;
		vertex[2] = 0.0;
		vertex[3] = 1.0;
		glVertex4fv(vertex);
	}

	texcoord[0] = (1.0 + 1.0)*0.5;
	texcoord[1] = (0.0 + 1.0)*0.5;
	glTexCoord2fv(texcoord);

	vertex[0] = 1.0 * radius;
	vertex[1] = 0.0 * radius;
	vertex[2] = 0.0;
	vertex[3] = 1.0;
	glVertex4fv(vertex);
	glEnd();

	glDisable(GL_TEXTURE_2D);

}

void DrawCircle(float cx, float cy, float r, int num_segments)
{
	float theta = 2 * M_PI / float(num_segments);
	float tangetial_factor = tanf(theta);//calculate the tangential factor 

	float radial_factor = cosf(theta);//calculate the radial factor 

	float x = r;//we start at angle = 0 

	float y = 0;

	glBegin(GL_POINTS);
	for (int ii = 0; ii < num_segments / 8; ii++)
	{
		glVertex2f(x + cx, y + cy);//output vertex 

		//calculate the tangential vector 
		//remember, the radial vector is (x, y) 
		//to get the tangential vector we flip those coordinates and negate one of them 

		float tx = -y;
		float ty = x;

		//add the tangential vector 

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		//correct using the radial factor 

		x *= radial_factor;
		y *= radial_factor;
	}
	glEnd();
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
	//glutIdleFunc(RenderFunction);		  //閒置時...請系統執行"IdleFunction"

	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMotion);

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK ) {	fprintf(stderr,"ERROR: %s\n",glewGetErrorString(GlewInitResult)	);	exit(EXIT_FAILURE);	}

	//背景顏色黑
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	initialize();

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
	float radius = 2;
	float fovy = 45.0f;
	float eyeX = 0.0f, eyeY = 0.0f, eyeZ = 500.0f;
	GLfloat aspect = (GLfloat)CurrentWidth / CurrentHeight;
	double camDis = (radius * ratio) / tan(fovy * CV_PI / 180.0f / 2.0);
	//std::cout << "camDis " << camDis << std::endl;

	Points newEye;
	newEye.x = eyeX - 0.0;
	newEye.y = eyeY - 0.0;
	newEye.z = eyeZ - 0.0;

	float norm = sqrt(newEye.x * newEye.x + newEye.y * newEye.y + newEye.z * newEye.z);

	newEye.x /= norm;
	newEye.y /= norm;
	newEye.z /= norm;
	//std::cout << "x " << newEye.X << " y " << newEye.Y << " z " << newEye.Z << std::endl;

	newEye.x *= camDis;
	newEye.y *= camDis;
	newEye.z *= camDis;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect, 1.0f, 1000.0f);
	gluLookAt(newEye.x, newEye.y, newEye.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//gluLookAt(camDis * cos(-deg*3.1415 / 180), camDis * sin(-deg*3.1415 / 180), newEye.z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	Mat teapotPoseT = teapotPose.t();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureE);
	glPushMatrix();
		glLoadMatrixf((float*)teapotPoseT.data);
		drawSphere(radius, 12, 24);
	glPopMatrix();


	//DrawCircle(0, -0.1, 2, 8);

	//draw_circle(2, 8);

	glFlush();
	glutSwapBuffers();
}
