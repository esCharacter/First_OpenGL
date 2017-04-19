#include "modelToolkit.h"

using namespace std;

ModelTools::ModelTools()
{
	m_totalVertex = 0;
	m_totalFace = 0;
	m_totalVertexProperty = 0;
	m_colorFlag = 0;

	path = "";
}
ModelTools::ModelTools(string path)
{
	m_totalVertex = 0;
	m_totalFace = 0;
	m_totalVertexProperty = 0;
	m_colorFlag = 0;

	this->path = path;
}

ModelTools::~ModelTools()
{
	fin.close();
}

int  ModelTools::getTotalVertex()
{
	return this->m_totalVertex;
}

int  ModelTools::getTotalFace()
{
	return this->m_totalFace;
}

/*
* ��ȡģ���涥������,��������
*/
int  ModelTools::getTotalFaceVertex3()
{
	return this->m_totalFace * 3;
}

/*
* ���ļ�·����
*/
void ModelTools::setPath(string path)
{
	this->path = path;
	this->clear();
}

const char* ModelTools::getPath()
{
	return this->path.data();
}

bool ModelTools::clear()
{
	vecVertex.clear();
	vecTriangleColor.clear();
	vecFaceTriangle.clear();
	vecFaceTriangleIndex.clear();

	return true;
}

/*
* ���Ի��ļ�����ȡply�ļ���Ϣ�������ݴ�����Ӧ�����С�
*/
bool ModelTools::plyRead()
{
	fin.open(this->path, ios::in);//ֻ���ķ�ʽ���ļ���
	if (!fin)
	{
		//cout << ERROR << "�ļ���ʧ�ܣ�" << "(" << this->path.data() << ")" << endl;
		return false;
	}

	const int BUFFER_SIZE = 300;
	char buffer[BUFFER_SIZE];	//����һ�����ݵ�buffer��

	//-------------------------------------�����ļ�ͷ����ȡ�ļ���Ϣ-------------------------------------
	//��ȡͷ�ļ��ж������������������������Ը�������ɫ����
	int posFlag = 0;	//�ļ�ָ��λ�ã���Ϊ1��λ��element vertex֮����Ϊ2��λ��element face֮��
	while (strncmp("end_header", buffer, strlen("end_header")) != 0 && !fin.eof())
	{
		if (strncmp("element", buffer, strlen("element")) == 0)	//���ȡ���ؼ���element;
		{
			int i = strlen("element");
			for (; i < BUFFER_SIZE; i++)	//��ȥelement֮��Ŀո�;
			{
				if (buffer[i] != ' ')
					break;
			}
			strcpy_s(buffer, buffer + i);	//ɾ��ǰi���ַ���"element"�Ϳո�;

			if (strncmp("vertex", buffer, strlen("vertex")) == 0)	//�ո�֮����"vertex"������ա�
			{
				strcpy_s(buffer, buffer + strlen("vertex"));	//ɾ��"vertex";
				sscanf_s(buffer, "%i", &this->m_totalVertex);	//���涥�����m_totalVertex;
				posFlag = 1;
			}
			if (strncmp("face", buffer, strlen("face")) == 0)	//�ո�֮����"face"������ա�
			{
				strcpy_s(buffer, buffer + strlen("face"));		//ɾ��"face";
				sscanf_s(buffer, "%i", &this->m_totalFace);		//���������m_totalFace;
				posFlag = 2;
			}
		}
		int vPropertyPos = 0;//��¼��ǰΪ�ڼ�������
		if (strncmp("property", buffer, strlen("property")) == 0 && posFlag == 1)//���ȡ���ؼ���property����Ϊ��������;
		{
			this->m_totalVertexProperty++;//�������Ը���+1
			vPropertyPos++;

			int i = strlen("property");
			for (; i < BUFFER_SIZE; i++)	//��ȥproperty֮��Ŀո�;
			{
				if (buffer[i] != ' ')
					break;
			}
			strcpy_s(buffer, buffer + i);	//ɾ��ǰi���ַ���"property"�Ϳո�;
			if (strncmp("red", buffer, strlen("red")) == 0)	//�ո�֮����"red"���򶥵����԰�����ɫֵRGB��
			{
				m_colorFlag = 3;
				m_colorPropertyPos = vPropertyPos;//��¼red�ڶ��������е�λ��
			}
			if (strncmp("alpha", buffer, strlen("alpha")) == 0)	//�ո�֮����"alpha"�������RGBA��
			{
				m_colorFlag = 4;
			}
		}
		fin.getline(buffer, 300);

		//cout << buffer << endl;
	}
	//-------------------------------------�����������꣬��ȡ������Ϣ-------------------------------------
	//�������������ɫ���Ա�����vecVertex��
	int i;//vector���
	float R = 100 / 255.0f;
	float G = 100 / 255.0f;
	float B = 100 / 255.0f;
	for (i = 0; i < this->m_totalVertex&&!fin.eof(); i++)	//��ȡ��������
	{
		vertex vTemp;
		vTemp.r = R; vTemp.g = G; vTemp.b = B;//����Ĭ����ɫ

		fin.getline(buffer, 300);
		if (this->m_colorFlag == 0)
		{
			sscanf_s(buffer, "%f%f%f", &vTemp.x, &vTemp.y, &vTemp.z);	//ȡ��������
		}
		else
		{
			sscanf_s(buffer, "%f%f%f%f%f%f", &vTemp.x, &vTemp.y, &vTemp.z, &vTemp.r, &vTemp.g, &vTemp.b);	//ȡ�����������ɫ����
		}

		vecVertex.push_back(vTemp);
		//cout << vTemp.x << ' ' << vTemp.y << ' ' << vTemp.z << endl;
	}
	if (i < this->m_totalVertex)	//����ȱ��
	{
		//cout << ERROR << "�����ȡ����" << endl;
		return false;
	}

	//--------------------------------------���������꣬��ȡ����Ϣ--------------------------------------
	//���涥������������vecFaceTriangleIndex���涥�����걣����vecFaceTriangle����ɫ���Ա�����vecTriangleColor.
	for (i = 0; i < this->m_totalFace&&!fin.eof(); i++)	//��ȡ������
	{
		int total, v[4];	//totalΪ�涥������vΪ�涥������(���Ϊ�ı���)
		fin.getline(buffer, 300);
		sscanf_s(buffer, "%d%d%d%d%d", &total, &v[0], &v[1], &v[2], &v[3]);	//ȡ�涥������
		float c = 0;
		for (int j = 0; j < total; j++, c += 0.2)						//j�������εڼ�������
		{
			vecFaceTriangleIndex.push_back(v[j]);			//˳�򱣳��涥������
			vecFaceTriangle.push_back(vecVertex[v[j]].x);	//�����涥������
			vecFaceTriangle.push_back(vecVertex[v[j]].y);
			vecFaceTriangle.push_back(vecVertex[v[j]].z);

			//if (this->m_colorFlag == 0)//���ģ���ļ���û����ɫ���ԣ���ʹ��Ĭ����ɫ
			//{
			//	vecTriangleColor.push_back(vecVertex[v[j]].r);
			//	vecTriangleColor.push_back(vecVertex[v[j]].g);
			//	vecTriangleColor.push_back(vecVertex[v[j]].b + c);//�����涥����ɫֵ������������ɫ��ͬ
			//}
			//else
			//{
				vecTriangleColor.push_back(vecVertex[v[j]].r);
				vecTriangleColor.push_back(vecVertex[v[j]].g);
				vecTriangleColor.push_back(vecVertex[v[j]].b);//�����涥����ɫֵ
			//}
		}
	}
	if (i < this->m_totalFace)	//��ȱ��
	{
		//cout << ERROR << "�����ݶ�ȡ����" << endl;
		return false;
	}

	fin.close();

	return true;
}

void ModelTools::plyWrite()
{

}