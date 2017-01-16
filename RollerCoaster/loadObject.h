#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <algorithm>

#include <GL\freeglut.h>
#include <opencv2\opencv.hpp>

struct Points{
	float X;
	float Y;
	float Z;
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
	void RadiusOfCircle();
	void toOrigin(Points&);
	void Draw();
	void Write(char*);

	float bounding[6];
	float radius;
	Points center;

	std::vector<Points> V;
	std::vector<Points> VN;
	std::vector<Face> F;
};

template<typename T>
T calCross(T _v1, T _v2)
{
	T t;
	t.X = _v1.Y * _v2.Z - _v1.Z * _v2.Y;
	t.Y = _v1.Z * _v2.X - _v1.X * _v2.Z;
	t.Z = _v1.X * _v2.Y - _v1.Y * _v2.X;

	return t;
}

template<typename T>
T normalize(T _n)
{
	float unit = std::sqrt(_n.X * _n.X + _n.Y * _n.Y + _n.Z * _n.Z);

	_n.X /= unit;
	_n.Y /= unit;
	_n.Z /= unit;

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
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < F.size(); i++)
	{
		glNormal3f(VN[F[i].V[0]].X, VN[F[i].V[0]].Y, VN[F[i].V[0]].Z);
		glVertex3f(V[F[i].V[0]].X, V[F[i].V[0]].Y, V[F[i].V[0]].Z);

		glNormal3f(VN[F[i].V[1]].X, VN[F[i].V[1]].Y, VN[F[i].V[1]].Z);
		glVertex3f(V[F[i].V[1]].X, V[F[i].V[1]].Y, V[F[i].V[1]].Z);

		glNormal3f(VN[F[i].V[2]].X, VN[F[i].V[2]].Y, VN[F[i].V[2]].Z);
		glVertex3f(V[F[i].V[2]].X, V[F[i].V[2]].Y, V[F[i].V[2]].Z);
	}
	glEnd();
}

void Model::SearchNeibor()
{
	Points *pointsNormal = new Points[V.size()];
	//	for (int i = 0; i < V.size(); i++)
	//	{
	//		pointsNormal[i].NX = 0.0;
	//		pointsNormal[i].NY = 0.0;
	//		pointsNormal[i].NZ = 0.0;
	//	}

	for (size_t i = 0; i < F.size(); i++)
	{
		int v1 = F[i].V[0];
		int v2 = F[i].V[1];
		int v3 = F[i].V[2];

		// calculate normal
		Points vecA, vecB;
		vecA.X = V[v1].X - V[v2].X;
		vecA.Y = V[v1].Y - V[v2].Y;
		vecA.Z = V[v1].Z - V[v2].Z;

		vecB.X = V[v1].X - V[v3].X;
		vecB.Y = V[v1].Y - V[v3].Y;
		vecB.Z = V[v1].Z - V[v3].Z;

		Points nor = calCross(vecA, vecB);
		F[i].VN[0] = nor.X;
		F[i].VN[1] = nor.Y;
		F[i].VN[2] = nor.Z;
		//std::cout << nor.NX << " " << nor.NY << " " << nor.NZ << std::endl;

		pointsNormal[v1].X += nor.X;
		pointsNormal[v1].Y += nor.Y;
		pointsNormal[v1].Z += nor.Z;

		pointsNormal[v2].X += nor.X;
		pointsNormal[v2].Y += nor.Y;
		pointsNormal[v2].Z += nor.Z;

		pointsNormal[v3].X += nor.X;
		pointsNormal[v3].Y += nor.Y;
		pointsNormal[v3].Z += nor.Z;
	}

	std::vector<float> xSet, ySet, zSet;
	for (size_t i = 0; i < V.size(); i++)
	{
		//std::cout << "i " << i << " " << pointsNormal[i].NX << " " << pointsNormal[i].NY << " " << pointsNormal[i].NZ << std::endl;
		xSet.push_back(V[i].X);
		ySet.push_back(V[i].Y);
		zSet.push_back(V[i].Z);
		VN.push_back(normalize(pointsNormal[i]));
	}
	std::sort(xSet.begin(), xSet.end());
	std::sort(ySet.begin(), ySet.end());
	std::sort(zSet.begin(), zSet.end());

	bounding[0] = xSet[0];
	bounding[1] = xSet[V.size() - 1];
	bounding[2] = ySet[0];
	bounding[3] = ySet[V.size() - 1];
	bounding[4] = zSet[0];
	bounding[5] = zSet[V.size() - 1];
}

void Model::RadiusOfCircle()
{
	center.X = (bounding[0] + bounding[1]) / 2.0f;
	center.Y = (bounding[2] + bounding[3]) / 2.0f;
	center.Z = (bounding[4] + bounding[5]) / 2.0f;
	//std::cout << "x " << _center.X << " y " << _center.Y << " z " << _center.Z << std::endl;

	float xLength = std::sqrt((center.X - bounding[0]) * (center.X - bounding[0]));
	float yLength = std::sqrt((center.Y - bounding[2]) * (center.Y - bounding[2]));
	float zLength = std::sqrt((center.Z - bounding[5]) * (center.Z - bounding[5]));
	//std::cout << "x " << xLength << " y " << yLength << " z " << zLength << std::endl;

	radius = xLength;
	if (yLength > radius)
	{
		radius = yLength;
	}

	if (zLength > radius)
	{
		radius = zLength;
	}
	//std::cout << "radius " << _radius << std::endl;
}

void Model::toOrigin(Points& _center)
{
	for (auto& v : V)
	{
		Points tmp = v;
		v.X = tmp.X - _center.X;
		v.Y = tmp.Y - _center.Y;
		v.Z = tmp.Z - _center.Z;
	}
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
		fout << "v " << V[i].X << " " << V[i].Y << " " << V[i].Z << std::endl;
	}

	for (size_t i = 0; i < F.size(); i++)
	{
		fout << "f " << F[i].V[0] + 1 << " " << F[i].V[1] + 1 << " " << F[i].V[2] + 1 << std::endl;
	}
	fout.close();
}