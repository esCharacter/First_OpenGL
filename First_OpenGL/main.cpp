#include <iostream>
#include <freeglut.h>
#include "modelToolkit.h"

using namespace std;

#define ROOT_PATH	"ply\\shape_sphere2.ply"
//#define ROOT_PATH	"ply\\bun_zipper.ply"

int mouseX, mouseY;			//��ǰ���λ��
bool isMouseDown = false;	//����Ƿ���
float rotateX = 0, rotateY = 0;	//��ʼ����ת�Ƕ�
float zoom = -2.5f;			//��ͼ����
float translateX = 0.0f;	//λ��X
float translateY = 0.0f;	//λ��Y
float scale = 1.0;			//���ű���

GLsizei winWidth = 640;//���ڳߴ�
GLsizei winHeight = 480;

ModelTools ply;			//���ڻ�ȡply�ļ�����

double* vertexArray;//��������
double* colorArray;	//��ɫ����
double* normals;	//ģ����ķ�����

int vertexNum;

//�������ʱ�Ļص�����,ˢ�´���
void idle()
{
	glutPostRedisplay();//���»��Ƶ�ǰ���� 
}

//���ڴ�С�ı�ʱ�ص�����������ʱҲִ�У�
void resize(GLsizei iWidth, GLsizei iHeight)
{
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)winWidth / (GLfloat)winHeight, 0.1, 50);	//ͶӰ��ʽ��͸��ͶӰ
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//******************************************����******************************************
//��������Ӧ����
//��갴��
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
//����ƶ�
void mouseMotion1(int x, int y)
{
	int deltX, deltY;
	// ��������ƶ����룬��תͼ��
	deltY = x - mouseX;
	deltX = y - mouseY;
	// ���µ�ǰ���λ�ã�ʹͼ��ʵʱ��ת
	mouseX = x;
	mouseY = y;
	if (isMouseDown) {
		// rotate  
		rotateX += deltX;
		rotateY += deltY;
		glutPostRedisplay();
	}
}
//������
void mouseWheel(int button, int dir, int x, int y)
{
	const float ZM_SPEED = 0.01;	//΢���ٶ�
	if (dir < 0)					//������󻬶�
	{
		zoom -= ZM_SPEED;
	}
	else
	{
		zoom += ZM_SPEED;
	}
}

//������Ӧ����  
void keyboardAction(unsigned char key, int x, int y)
{
	const float TR_SPEED = 0.01;//XYƽ��λ���ٶ�
	const float SC_SPEED = 0.01;//ģ�������ٶ�
	const float ZM_SPEED = 0.1;	//�����ƶ��ٶ�
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

//******************************************��ͼ******************************************
//ÿ�λ���һ���棬ÿ�����Ӧһ�����֣���������ʰȡѡ��
void drawModelImmediate()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);		// ƽ�� 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// ��Y����ת
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// ��X����ת
	glScaled(scale, scale, scale);					// ����

	int v = 0, c = 0;//��������ָ�����ɫ����ָ��

	for (int i = 0; i < vertexNum / 9; i++, v += 9, c += 9)
	{
		glBegin(GL_TRIANGLES);					// ��ʼ���Ƹ�����
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

//����ͼ�񣬶�������,
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

	glTranslatef(translateX, translateY, zoom);			// ƽ�� 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// ��Y����ת
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// ��X����ת
	glScaled(scale, scale, scale);						// ����

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glNormalPointer(GL_FLOAT, 0, n);
	glColorPointer(3, GL_DOUBLE, 0, colorArray);
	glVertexPointer(3, GL_DOUBLE, 0, vertexArray);//���������ͼ
	glDrawArrays(GL_TRIANGLES, 0, vertexNum / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();
	delete[] n;
}

//��������
void drawSolidSphere()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);			// ƽ�� 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// ��Y����ת
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// ��X����ת
	glScaled(scale, scale, scale);						// ����

	glColor3f(0.3f, 0.3f, 0.3f);//ģ����ɫ
	glutSolidSphere(0.5, 32, 32);

	glPopMatrix();
}

//���ƻ���ͼ��
void drawBase()
{
	glPushMatrix();

	glTranslatef(translateX, translateY, zoom);			// ƽ�� 
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);				// ��Y����ת
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);				// ��X����ת
	glScaled(scale, scale, scale);						// ����

	glBegin(GL_TRIANGLES);					// ����
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
	//�����ɫ��Ҳ���ǽ������������Ϊͬһ����ɫ����ɫ��
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0.5, 0.4, 0, 0, 0, 0, -0.5, 0.5);
	glBegin(GL_LINES);
	//������������ɫ
	glColor3f(0.2f, 0.5f, 0.6f);
	GLfloat eachPiece = 2 * Pi / n;
	//��n�����������һ�����棬ͬʱʵ��ƽ�ƶ���Ч����num��ƽ�ƣ�
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

// ���Ʋ���ʾͼ��,�ص�������
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

//******************************************����******************************************
typedef struct{
	float X;
	float Y;
	float Z;
} Normal;
//���㷨���������������㣬������ķ�������ʹ�ò�����㣬n=a��b��
void computeNormal(double v1[], double v2[], double v3[], double* normal)
{
	GLfloat vc1[3], vc2[3];//��������������ƽ�е�����
	GLfloat a, b, c;
	GLdouble r;
	vc1[0] = v2[0] - v1[0]; vc1[1] = v2[1] - v1[1]; vc1[2] = v2[2] - v1[2];
	vc2[0] = v3[0] - v1[0]; vc2[1] = v3[1] - v1[1]; vc2[2] = v3[2] - v1[2];
	a = vc1[1] * vc2[2] - vc2[1] * vc1[2];
	b = vc2[0] * vc1[2] - vc1[0] * vc2[2];
	c = vc1[0] * vc2[1] - vc2[0] * vc1[1];
	r = sqrt(a * a + b * b + c * c);
	normal[0] = a / r;	//��һ��
	normal[1] = b / r;
	normal[2] = c / r;
}

//����ģ��������ķ�������
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

//******************************************������******************************************
//��ʼ��OpenGL���趨������״̬.
BOOL initOpenGL()
{
	glEnable(GL_DEPTH_TEST);//��Ȳ��ԣ����ֵ��Ч
	//glEnable(GL_BLEND);		//��ɫ���
	//glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);//������ɫ׷��

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);		//������ȷ�ΧΪ���

	GLfloat lightPosition[] = { 1.0, 1.0, 1.0, 0.0 };		//��Դλ��,xyz,dis
	GLfloat lightAmbient[] = { 0.8, 0.8, 0.8, 1.0 };		//ȫ�ֹ�����,��ǿ��
	GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);//ָ����Ҫ��ɫ׷�ٵĲ�������
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);//ָ����0�Ź�Դ��λ�� 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse); //������� 
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);//���淴���
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);

	return TRUE;
}

void readModel()
{
	ply.setPath(ROOT_PATH);	//����ģ�Ͷ��󣬶�ȡply�ļ�
	ply.plyRead();			//��ʼ��ģ������

	vertexNum = ply.vecFaceTriangle.size();

	vertexArray = new double[ply.vecFaceTriangle.size()];//��ȡ��������
	for (int i = 0; i < ply.vecFaceTriangle.size(); i++)
	{
		vertexArray[i] = ply.vecFaceTriangle[i];
	}

	colorArray = new double[ply.vecTriangleColor.size()];//��ȡ��ɫ����
	for (int i = 0; i < ply.vecTriangleColor.size(); i++)
	{
		colorArray[i] = ply.vecTriangleColor[i];
	}

	normals = new double[ply.vecFaceTriangle.size() / 3];//��ȡ��������
	getNormal(vertexArray, ply.vecFaceTriangle.size(), normals);
}

int main(int argc, char** argv)
{
	readModel();

	//��������ʼ������ 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);//������ʾģʽ��RGB��ɫģʽ����Ȼ��壬˫����  
	glutInitWindowSize(winWidth, winHeight);	//���ڴ�С 
	glutInitWindowPosition(200, 100);			//����λ�� 
	glutCreateWindow("Discrete Surface Demo");	//��������

	initOpenGL();

	glutMouseFunc(mouseButton1);		//��갴�£��������Ӧ����  
	glutMotionFunc(mouseMotion1);	//����ƶ�ʱ����Ӧ����  
	glutMouseWheelFunc(mouseWheel);
	glutKeyboardFunc(keyboardAction);		//���̰��µ���Ӧ���� 
	glutIdleFunc(redisplay);				//�������ʱ�Ĵ�����

	//glutReshapeFunc(resize);		//���ڴ�С�����ı�ʱ����Ӧ����  
	glutDisplayFunc(displayModel);

	glutMainLoop();

	return 0;
}
