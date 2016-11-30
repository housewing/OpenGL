#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

#include <GL\freeglut.h>
#include <opencv2\opencv.hpp>

struct Points{
	float X;
	float Y;
	float Z;
};

struct Normal{
	float NX;
	float NY;
	float NZ;
};

struct Face{
	int V[3];
	float VN[3];
};

class Model
{
public:
	void Load(char*);
	void SearchNeibor();
	void Draw();
	void Write(char*);
	float* calCurvature(std::set<int>, int);

	std::vector<Points> RGB;
	std::vector<Points> V;
	std::vector<Normal> VN;
	std::vector<Face> F;
	std::vector<std::set<int>> Neibor;
};

template<typename T>
Normal calCross(T _u, T _v)
{
	Normal n;

	n.NX = _u.NY * _v.NZ - _u.NZ * _v.NY;
	n.NY = _u.NZ * _v.NX - _u.NX * _v.NZ;
	n.NZ = _u.NX * _v.NY - _u.NY * _v.NX;

	return n;
}

template<typename T>
float calDot(T _u, T _v)
{
	return _u.NX * _v.NX + _u.NY * _v.NY + _u.NZ * _v.NZ;
}

template<typename T>
Normal calNormal(T _v1, T _v2, T _v3)
{
	T u, v;
	u.X = _v1.X - _v2.X;
	u.Y = _v1.Y - _v2.Y;
	u.Z = _v1.Z - _v2.Z;

	v.X = _v1.X - _v3.X;
	v.Y = _v1.Y - _v3.Y;
	v.Z = _v1.Z - _v3.Z;

	Normal n;
	n.NX = u.Y * v.Z - u.Z * v.Y;
	n.NY = u.Z * v.X - u.X * v.Z;
	n.NZ = u.X * v.Y - u.Y * v.X;

	return n;
}

template<typename T>
Normal normalize(T _n)
{
	float unit = std::sqrt(_n.NX * _n.NX + _n.NY * _n.NY + _n.NZ * _n.NZ);

	_n.NX /= unit;
	_n.NY /= unit;
	_n.NZ /= unit;

	return _n;
}

void Model::Load(char* filename)
{
	std::ifstream ifs2(filename, std::ifstream::in);
	std::string line;
	std::string head;
	while (getline(ifs2, line))
	{
		if (line[0] == 'v'){
			Points v;
			std::istringstream in(line);
			in >> head >> v.X >> v.Y >> v.Z;
			V.push_back(v);
		}
		else if (line[0] == 'f'){
			Face f;
			std::istringstream in(line);
			in >> head >> f.V[0] >> f.V[1] >> f.V[2];
			f.V[0] -= 1;
			f.V[1] -= 1;
			f.V[2] -= 1;

			F.push_back(f);
		}
	}
	ifs2.close();
}

void Model::Draw(){
	//float r, g, b;

	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < F.size(); i++)
	{
		//r = (rand() % 1000) / 999.0;
		//g = (rand() % 1000) / 999.0;
		//b = (rand() % 1000) / 999.0;
		//glColor3f(255, 255, 255);
		glNormal3f(VN[F[i].V[0]].NX,VN[F[i].V[0]].NY,VN[F[i].V[0]].NZ);
		glColor3f(RGB[F[i].V[0]].X * 255, RGB[F[i].V[0]].Y * 255, RGB[F[i].V[0]].Z * 255);
		//glNormal3f(F[i].VN[0], F[i].VN[1], F[i].VN[2]);
		glVertex3f(V[F[i].V[0]].X, V[F[i].V[0]].Y, V[F[i].V[0]].Z);

		glNormal3f(VN[F[i].V[1]].NX,VN[F[i].V[1]].NY,VN[F[i].V[1]].NZ);
		glColor3f(RGB[F[i].V[1]].X * 255, RGB[F[i].V[1]].Y * 255, RGB[F[i].V[1]].Z * 255);
		//glNormal3f(F[i].VN[0], F[i].VN[1], F[i].VN[2]);
		glVertex3f(V[F[i].V[1]].X, V[F[i].V[1]].Y, V[F[i].V[1]].Z);

		glNormal3f(VN[F[i].V[2]].NX,VN[F[i].V[2]].NY,VN[F[i].V[2]].NZ);
		glColor3f(RGB[F[i].V[2]].X * 255, RGB[F[i].V[2]].Y * 255, RGB[F[i].V[2]].Z * 255);
		//glNormal3f(F[i].VN[0], F[i].VN[1], F[i].VN[2]);
		glVertex3f(V[F[i].V[2]].X, V[F[i].V[2]].Y, V[F[i].V[2]].Z);
	}
	glEnd();
}

void Model::SearchNeibor()
{
	Normal *pointsNormal = new Normal[V.size()];
//#pragma omp parallel for
//	for (int i = 0; i < V.size(); i++)
//	{
//		pointsNormal[i].NX = 0.0;
//		pointsNormal[i].NY = 0.0;
//		pointsNormal[i].NZ = 0.0;
//	}

	std::set<int> *neibor = new std::set<int>[V.size()]; 
	for (size_t i = 0; i < F.size(); i++)
	{
		int v1 = F[i].V[0];
		int v2 = F[i].V[1];
		int v3 = F[i].V[2];

		// search neibor points
		neibor[v1].insert(v2);
		neibor[v1].insert(v3);

		neibor[v2].insert(v1);
		neibor[v2].insert(v3);

		neibor[v3].insert(v1);
		neibor[v3].insert(v2);

		// calculate normal
		Normal nor = calNormal(V[v1], V[v2], V[v3]);
		F[i].VN[0] = nor.NX;
		F[i].VN[1] = nor.NY;
		F[i].VN[2] = nor.NZ;
		//std::cout << nor.NX << " " << nor.NY << " " << nor.NZ << std::endl;

		pointsNormal[v1].NX += nor.NX;
		pointsNormal[v1].NY += nor.NY;
		pointsNormal[v1].NZ += nor.NZ;

		pointsNormal[v2].NX += nor.NX;
		pointsNormal[v2].NY += nor.NY;
		pointsNormal[v2].NZ += nor.NZ;

		pointsNormal[v3].NX += nor.NX;
		pointsNormal[v3].NY += nor.NY;
		pointsNormal[v3].NZ += nor.NZ;
	}

	for (size_t i = 0; i < V.size(); i++)
	{
		//std::cout << "i " << i << " " << pointsNormal[i].NX << " " << pointsNormal[i].NY << " " << pointsNormal[i].NZ << std::endl;
		VN.push_back(normalize(pointsNormal[i]));
		Neibor.push_back(neibor[i]);
	}
}

float* Model::calCurvature(std::set<int> _neibor, int _vertex)
{
	Normal a = VN[_vertex];
	Normal b;
	b.NX = 0.0;
	b.NY = 0.0;
	b.NZ = -1;
	//std::cout << "up " << up.NX << " " << up.NY << " " << up.NZ << std::endl;
	//std::cout << "down " << down.NX << " " << down.NY << " " << down.NZ << std::endl;

	Normal v = calCross(a, b);
	float c = calDot(a, b);

	if (c == -1) 
	{
		c = 10e-4 * -999;
	}

	cv::Mat vMatrix = (cv::Mat_<float>(3, 3) << 0.0, -v.NZ, v.NY, v.NZ, 0.0, -v.NX, -v.NY, v.NX, 0.0);
	cv::Mat identity = (cv::Mat_<float>(3, 3) << 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

	cv::Mat R = cv::Mat::zeros(3, 3, CV_32FC1);
	R = identity + vMatrix + vMatrix * vMatrix * (1.0 / (1.0 + c));

	//Mat matrix1 = cv::Mat::zeros(1, 3, CV_32FC1);
	//matrix1.at<float>(0, 0) = up.NX;
	//matrix1.at<float>(0, 1) = up.NY;
	//matrix1.at<float>(0, 2) = up.NZ;
	//std::cout << "R * up = down " << matrix1 * R << std::endl;

	cv::Mat BIG_U, BIG_d;

	BIG_U = cv::Mat::zeros(_neibor.size(), 3, CV_32FC1);
	BIG_d = cv::Mat::zeros(_neibor.size(), 1, CV_32FC1);

	int i = 0;
	for (std::set<int>::iterator it = _neibor.begin(); it != _neibor.end(); it++)
	{
		cv::Mat translateOrigin = cv::Mat::zeros(3, 1, CV_32FC1);
		translateOrigin.at<float>(0, 0) = V[*it].X - V[_vertex].X;
		translateOrigin.at<float>(1, 0) = V[*it].Y - V[_vertex].Y;
		translateOrigin.at<float>(2, 0) = V[*it].Z - V[_vertex].Z;

		cv::Mat tmp = R * translateOrigin;

		BIG_U.at<float>(i, 0) = tmp.at<float>(0, 0) * tmp.at<float>(0, 0);
		BIG_U.at<float>(i, 1) = 2 * tmp.at<float>(0, 0) * tmp.at<float>(0, 1);
		BIG_U.at<float>(i, 2) = tmp.at<float>(0, 1) * tmp.at<float>(0, 1);

		BIG_d.at<float>(i, 0) = tmp.at<float>(0, 2);
		i++;
	}

	cv::Mat C_matrix;
	C_matrix = (BIG_U.t() * BIG_U).inv() * BIG_U.t() * BIG_d;
	//printf("C = [%f %f %f]^T\n",C_matrix.at<float>(0,0),C_matrix.at<float>(1,0),C_matrix.at<float>(2,0));

	cv::Mat matrix = (cv::Mat_<float>(2, 2) << C_matrix.at<float>(0, 0), C_matrix.at<float>(1, 0), C_matrix.at<float>(1, 0), C_matrix.at<float>(2, 0));
	//std::cout << matrix << std::endl;

	cv::Mat eig;
	cv::eigen(matrix, eig);
	//std::cout << eig << std::endl;

	static float curvature[4];
	curvature[0] = (eig.at<float>(0, 0) + eig.at<float>(1, 0)) / 2.0;
	curvature[1] = eig.at<float>(0, 0) * eig.at<float>(1, 0);
	curvature[2] = eig.at<float>(0, 0);
	curvature[3] = eig.at<float>(1, 0);

	return curvature;
}

void Model::Write(char* _filename)
{
	std::ofstream fout(_filename); // Write File
	if (!fout)
	{
		std::cout << "can't write file!!!" << std::endl;;
	}

	for (size_t i = 0; i < V.size(); i++)
	{
		fout << "v " << V[i].X << " " << V[i].Y << " " << V[i].Z << " " << RGB[i].X << " " << RGB[i].Y << " " << RGB[i].Z << std::endl;
	}

	for (size_t i = 0; i < F.size(); i++)
	{
		fout << "f " << F[i].V[0] + 1 << " " << F[i].V[1] + 1 << " " << F[i].V[2] + 1 << std::endl;
	}
	fout.close();
}