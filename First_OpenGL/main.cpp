#include <iostream>
#include <freeglut.h>
#include "modelToolkit.h"

using namespace std;

#define ROOT_PATH	"ply\\shape_sphere2.ply"
//#define ROOT_PATH	"ply\\bun_zipper.ply"

int mouseX, mouseY;			//当前鼠标位置
bool isMouseDown = false;	//鼠标是否按下
float rotateX = 0, rotateY = 0;	//初始化旋转角度
float zoom = -2.5f;			//视图焦距
float translateX = 0.0f;	//位移X
float translateY = 0.0f;	//位移Y
float scale = 1.0;			//缩放比例

GLsizei winWidth = 640;//窗口尺寸
GLsizei winHeight = 480;

ModelTools ply;			//用于获取ply文件数据

double* vertexArray;//顶点数组
double* colorArray;	//颜色数组
double* normals;	//模型面的法向量

int vertexNum;

//程序空闲时的回调函数,刷新窗口
void idle()
{
	glutPostRedisplay();//重新绘制当前窗口 
}

//窗口大小改变时回调（程序启动时也执行）
void resize(GLsizei iWidth, GLsizei iHeight)
{
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)winWidth / (GLfloat)winHeight, 0.1, 50);	//投影方式：透视投影
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//******************************************交互******************************************
//添加鼠标响应函数
//鼠标按下
void mouseButton1(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			mouseX = x;
			mouseY = y;

			isMouseDown = true;
		}
		else
		{
			isMouseDown = false;
		}
	}

}
//鼠标移动
void mouseMotion1(int x, int y)
{
	int deltX, deltY;
	// 计算鼠标移动距离，旋转图像
	deltY = x - mouseX;
	deltX = y - mouseY;
	// 更新当前鼠标位置，使图像实时旋转
	mouseX = x;
	mouseY = y;
	if (isMouseDown) {
		// rotate  
		rotateX += deltX;
		rotateY += deltY;
		glutPostRedisplay();
	}
}
//鼠标滚轮
void mouseWheel(int button, int dir, int x, int y)
{
	const float ZM_SPEED = 0.01;	//微调速度
	if (dir < 0)					//滚轮向后滑动
	{
		zoom -= ZM_SPEED;
	}
	else
	{
		zoom += ZM_SPEED;
	}
}

//键盘响应函数  
void keyboardAction(unsigned char key, int x, int y)
{
	const float TR_SPEED = 0.01;//XY平面位移速度
	const float SC_SPEED = 0.01;//模型缩放速度
	const float ZM_SPEED = 0.1;	//焦距移动速度
	switch (key) {
	case 'z':
	case 'Z':
		zoom += ZM_SPEED;
		break;
	case 'x':
	case 'X':
		zoom -= ZM_SPEED;
		break;
	case 'c':
	case 'C':
		scale += SC_SPEED;
		break;
	case 'v':
	case 'V':
		scale = (scale - SC_SPEED) <= 0 ? 0.005 : (scale - SC_SPEED);
		break;
	case 'w':
	case 'W':
		translateY += TR_SPEED;
		break;
	case 's':
	case 'S':
		translateY -= TR_SPEED;
		break;
	case 'a':
	case 'A':
		translateX -= TR_SPEED;
		break;
	case 'd':
	case 'D':
		translateX += TR_SPEED;
		break;
	}
}

//******************************************绘图******************************************
//每次绘制一个面，每个面对应一个名字，名字用于拾取选择
void drawModelImmediate()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);		// 平移 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// 绕Y轴旋转
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// 绕X轴旋转
	glScaled(scale, scale, scale);					// 缩放

	int v = 0, c = 0;//顶点数组指针和颜色数组指针

	for (int i = 0; i < vertexNum / 9; i++, v += 9, c += 9)
	{
		glBegin(GL_TRIANGLES);					// 开始绘制各个面
		glNormal3f(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
		glColor3f(colorArray[c + 0], colorArray[c + 1], colorArray[c + 2]);
		glVertex3f(vertexArray[v], vertexArray[v + 1], vertexArray[v + 2]);
		glColor3f(colorArray[c + 3], colorArray[c + 4], colorArray[c + 5]);
		glVertex3f(vertexArray[v + 3], vertexArray[v + 4], vertexArray[v + 5]);
		glColor3f(colorArray[c + 6], colorArray[c + 7], colorArray[c + 8]);
		glVertex3f(vertexArray[v + 6], vertexArray[v + 7], vertexArray[v + 8]);
		glEnd();
	}

	glPopMatrix();
}

//绘制图像，顶点数组,
void drawModelArray()
{
	float* n = new float[vertexNum];
	int i;
	for (i = 0; i < vertexNum / 9; i++)
	{
		for (int j = 0; j < 9; j += 3)
		{
			n[i * 9 + j + 0] = normals[i * 3 + 0];
			n[i * 9 + j + 1] = normals[i * 3 + 1];
			n[i * 9 + j + 2] = normals[i * 3 + 2];
		}
	}

	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);			// 平移 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// 绕Y轴旋转
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// 绕X轴旋转
	glScaled(scale, scale, scale);						// 缩放

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glNormalPointer(GL_FLOAT, 0, n);
	glColorPointer(3, GL_DOUBLE, 0, colorArray);
	glVertexPointer(3, GL_DOUBLE, 0, vertexArray);//顶点数组绘图
	glDrawArrays(GL_TRIANGLES, 0, vertexNum / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();
	delete[] n;
}

//绘制球体
void drawSolidSphere()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);			// 平移 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// 绕Y轴旋转
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// 绕X轴旋转
	glScaled(scale, scale, scale);						// 缩放

	glColor3f(0.3f, 0.3f, 0.3f);//模型颜色
	glutSolidSphere(0.5, 32, 32);

	glPopMatrix();
}

//绘制基本图形
void drawBase()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);			// 平移 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// 绕Y轴旋转
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// 绕X轴旋转
	glScaled(scale, scale, scale);						// 缩放

	glBegin(GL_TRIANGLES);					// 绘制
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.8, 0.8, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(-0.8, 0.8, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.8, -0.8, 0.0);
	glEnd();

	glPopMatrix();
}

const GLfloat factor = 0.1f;
const GLfloat Pi = 3.1415926536f;
GLfloat num = 0.0f;
const int n = 400;
void drawWater()
{
	GLfloat x;
	//清除颜色，也就是将整个窗口清除为同一种颜色（黑色）
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0.5, 0.4, 0, 0, 0, 0, -0.5, 0.5);
	glBegin(GL_LINES);
	//设置线条的颜色
	glColor3f(0.2f, 0.5f, 0.6f);
	GLfloat eachPiece = 2 * Pi / n;
	//画n个线条来组成一个曲面，同时实现平移动画效果（num的平移）
	for (int i = 0; i < n; i += 1.0) {
		for (x = 0; x < 1.0f / factor; x += 0.01f) {
			glVertex3f(x*factor*cos(eachPiece * i), 2 * (1.0 - x*factor)*sin(4 * x - num)*factor, x*factor*sin(eachPiece * i));
		}
	}
	glEnd();
}

void redisplay() 
{
	num += 0.1;
	glutPostRedisplay();
}

// 绘制并显示图像,回调函数。
void displayModel()
{
	//glClearColor(1.0, 1.0, 1.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//drawModelImmediate();
	//drawModelArray();
	//drawSolidSphere();
	//drawBase();
	drawWater();
	glutSwapBuffers();
}

//******************************************功能******************************************
typedef struct{
	float X;
	float Y;
	float Z;
} Normal;
//计算法向量，根据三个点，计算面的法向量。使用叉积计算，n=a×b。
void computeNormal(double v1[], double v2[], double v3[], double* normal)
{
	GLfloat vc1[3], vc2[3];//三角形内两个不平行的向量
	GLfloat a, b, c;
	GLdouble r;
	vc1[0] = v2[0] - v1[0]; vc1[1] = v2[1] - v1[1]; vc1[2] = v2[2] - v1[2];
	vc2[0] = v3[0] - v1[0]; vc2[1] = v3[1] - v1[1]; vc2[2] = v3[2] - v1[2];
	a = vc1[1] * vc2[2] - vc2[1] * vc1[2];
	b = vc2[0] * vc1[2] - vc1[0] * vc2[2];
	c = vc1[0] * vc2[1] - vc2[0] * vc1[1];
	r = sqrt(a * a + b * b + c * c);
	normal[0] = a / r;	//归一化
	normal[1] = b / r;
	normal[2] = c / r;
}

//计算模型所有面的法向量。
void getNormal(double* vertexArray, int vertexNum, double* normals)
{
	for (int i = 0, j = 0; i < vertexNum / 9; i++, j += 9)
	{
		double v1[3], v2[3], v3[3];
		double normal[3];
		int t = 0;
		for (int k = 0; k < 3; k++, t++)
		{
			v1[k] = vertexArray[j + t];
		}
		for (int k = 0; k < 3; k++, t++)
		{
			v2[k] = vertexArray[j + t];
		}
		for (int k = 0; k < 3; k++, t++)
		{
			v3[k] = vertexArray[j + t];
		}

		computeNormal(v1, v2, v3, normal);

		normals[i * 3 + 0] = normal[0];
		normals[i * 3 + 1] = normal[1];
		normals[i * 3 + 2] = normal[2];
	}
}

//******************************************主函数******************************************
//初始化OpenGL，设定参数和状态.
BOOL initOpenGL()
{
	glEnable(GL_DEPTH_TEST);//深度测试，深度值有效
	//glEnable(GL_BLEND);		//颜色混合
	//glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);//启用颜色追踪

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);		//设置深度范围为最大

	GLfloat lightPosition[] = { 1.0, 1.0, 1.0, 0.0 };		//光源位置,xyz,dis
	GLfloat lightAmbient[] = { 0.8, 0.8, 0.8, 1.0 };		//全局光属性,光强度
	GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);//指定需要颜色追踪的材料属性
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);//指定第0号光源的位置 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse); //漫反射后 
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);//镜面反射后
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);

	return TRUE;
}

void readModel()
{
	ply.setPath(ROOT_PATH);	//定义模型对象，读取ply文件
	ply.plyRead();			//初始化模型数据

	vertexNum = ply.vecFaceTriangle.size();

	vertexArray = new double[ply.vecFaceTriangle.size()];//获取顶点数组
	for (int i = 0; i < ply.vecFaceTriangle.size(); i++)
	{
		vertexArray[i] = ply.vecFaceTriangle[i];
	}

	colorArray = new double[ply.vecTriangleColor.size()];//获取颜色数组
	for (int i = 0; i < ply.vecTriangleColor.size(); i++)
	{
		colorArray[i] = ply.vecTriangleColor[i];
	}

	normals = new double[ply.vecFaceTriangle.size() / 3];//获取顶点向量
	getNormal(vertexArray, ply.vecFaceTriangle.size(), normals);
}

int main(int argc, char** argv)
{
	readModel();

	//创建并初始化窗口 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);//窗口显示模式：RGB颜色模式，深度缓冲，双缓冲  
	glutInitWindowSize(winWidth, winHeight);	//窗口大小 
	glutInitWindowPosition(200, 100);			//窗口位置 
	glutCreateWindow("Discrete Surface Demo");	//创建窗口

	initOpenGL();

	glutMouseFunc(mouseButton1);		//鼠标按下，弹起的响应函数  
	glutMotionFunc(mouseMotion1);	//鼠标移动时的响应函数  
	glutMouseWheelFunc(mouseWheel);
	glutKeyboardFunc(keyboardAction);		//键盘按下的响应函数 
	glutIdleFunc(redisplay);				//程序空闲时的处理函数

	//glutReshapeFunc(resize);		//窗口大小发生改变时的响应函数  
	glutDisplayFunc(displayModel);

	glutMainLoop();

	return 0;
}
