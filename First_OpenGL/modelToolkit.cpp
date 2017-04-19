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
* 获取模型面顶点总数,三角网格。
*/
int  ModelTools::getTotalFaceVertex3()
{
	return this->m_totalFace * 3;
}

/*
* 绑定文件路径。
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
* 初试化文件，读取ply文件信息，将数据存至相应变量中。
*/
bool ModelTools::plyRead()
{
	fin.open(this->path, ios::in);//只读的方式打开文件。
	if (!fin)
	{
		//cout << ERROR << "文件打开失败！" << "(" << this->path.data() << ")" << endl;
		return false;
	}

	const int BUFFER_SIZE = 300;
	char buffer[BUFFER_SIZE];	//储存一行数据的buffer。

	//-------------------------------------分析文件头，获取文件信息-------------------------------------
	//获取头文件中顶点数量，面数量，顶点属性个数，颜色属性
	int posFlag = 0;	//文件指针位置，如为1则位于element vertex之后，如为2则位于element face之后；
	while (strncmp("end_header", buffer, strlen("end_header")) != 0 && !fin.eof())
	{
		if (strncmp("element", buffer, strlen("element")) == 0)	//如果取到关键字element;
		{
			int i = strlen("element");
			for (; i < BUFFER_SIZE; i++)	//略去element之后的空格;
			{
				if (buffer[i] != ' ')
					break;
			}
			strcpy_s(buffer, buffer + i);	//删除前i个字符，"element"和空格;

			if (strncmp("vertex", buffer, strlen("vertex")) == 0)	//空格之后是"vertex"，则接收。
			{
				strcpy_s(buffer, buffer + strlen("vertex"));	//删除"vertex";
				sscanf_s(buffer, "%i", &this->m_totalVertex);	//保存顶点个数m_totalVertex;
				posFlag = 1;
			}
			if (strncmp("face", buffer, strlen("face")) == 0)	//空格之后是"face"，则接收。
			{
				strcpy_s(buffer, buffer + strlen("face"));		//删除"face";
				sscanf_s(buffer, "%i", &this->m_totalFace);		//保存面个数m_totalFace;
				posFlag = 2;
			}
		}
		int vPropertyPos = 0;//记录当前为第几个属性
		if (strncmp("property", buffer, strlen("property")) == 0 && posFlag == 1)//如果取到关键字property，且为顶点属性;
		{
			this->m_totalVertexProperty++;//顶点属性个数+1
			vPropertyPos++;

			int i = strlen("property");
			for (; i < BUFFER_SIZE; i++)	//略去property之后的空格;
			{
				if (buffer[i] != ' ')
					break;
			}
			strcpy_s(buffer, buffer + i);	//删除前i个字符，"property"和空格;
			if (strncmp("red", buffer, strlen("red")) == 0)	//空格之后是"red"，则顶点属性包括颜色值RGB。
			{
				m_colorFlag = 3;
				m_colorPropertyPos = vPropertyPos;//记录red在顶点属性中的位置
			}
			if (strncmp("alpha", buffer, strlen("alpha")) == 0)	//空格之后是"alpha"，则包括RGBA。
			{
				m_colorFlag = 4;
			}
		}
		fin.getline(buffer, 300);

		//cout << buffer << endl;
	}
	//-------------------------------------分析顶点坐标，获取顶点信息-------------------------------------
	//将顶点坐标和颜色属性保存至vecVertex。
	int i;//vector序号
	float R = 100 / 255.0f;
	float G = 100 / 255.0f;
	float B = 100 / 255.0f;
	for (i = 0; i < this->m_totalVertex&&!fin.eof(); i++)	//获取顶点数据
	{
		vertex vTemp;
		vTemp.r = R; vTemp.g = G; vTemp.b = B;//顶点默认颜色

		fin.getline(buffer, 300);
		if (this->m_colorFlag == 0)
		{
			sscanf_s(buffer, "%f%f%f", &vTemp.x, &vTemp.y, &vTemp.z);	//取顶点坐标
		}
		else
		{
			sscanf_s(buffer, "%f%f%f%f%f%f", &vTemp.x, &vTemp.y, &vTemp.z, &vTemp.r, &vTemp.g, &vTemp.b);	//取顶点坐标和颜色属性
		}

		vecVertex.push_back(vTemp);
		//cout << vTemp.x << ' ' << vTemp.y << ' ' << vTemp.z << endl;
	}
	if (i < this->m_totalVertex)	//顶点缺少
	{
		//cout << ERROR << "顶点读取错误！" << endl;
		return false;
	}

	//--------------------------------------分析面坐标，获取面信息--------------------------------------
	//将面顶点索引保存至vecFaceTriangleIndex，面顶点坐标保持至vecFaceTriangle，颜色属性保存至vecTriangleColor.
	for (i = 0; i < this->m_totalFace&&!fin.eof(); i++)	//获取面数据
	{
		int total, v[4];	//total为面顶点数，v为面顶点索引(最大为四边形)
		fin.getline(buffer, 300);
		sscanf_s(buffer, "%d%d%d%d%d", &total, &v[0], &v[1], &v[2], &v[3]);	//取面顶点索引
		float c = 0;
		for (int j = 0; j < total; j++, c += 0.2)						//j代表多边形第几个顶点
		{
			vecFaceTriangleIndex.push_back(v[j]);			//顺序保持面顶点索引
			vecFaceTriangle.push_back(vecVertex[v[j]].x);	//保存面顶点坐标
			vecFaceTriangle.push_back(vecVertex[v[j]].y);
			vecFaceTriangle.push_back(vecVertex[v[j]].z);

			//if (this->m_colorFlag == 0)//如果模型文件中没有颜色属性，则使用默认颜色
			//{
			//	vecTriangleColor.push_back(vecVertex[v[j]].r);
			//	vecTriangleColor.push_back(vecVertex[v[j]].g);
			//	vecTriangleColor.push_back(vecVertex[v[j]].b + c);//保存面顶点颜色值，三个顶点颜色不同
			//}
			//else
			//{
				vecTriangleColor.push_back(vecVertex[v[j]].r);
				vecTriangleColor.push_back(vecVertex[v[j]].g);
				vecTriangleColor.push_back(vecVertex[v[j]].b);//保存面顶点颜色值
			//}
		}
	}
	if (i < this->m_totalFace)	//面缺少
	{
		//cout << ERROR << "面数据读取错误！" << endl;
		return false;
	}

	fin.close();

	return true;
}

void ModelTools::plyWrite()
{

}