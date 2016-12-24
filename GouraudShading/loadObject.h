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
	void RadiusOfCircle();
	void toOrigin(Points&);
	void Draw();
	void Write(char*);

	float bounding[6];
	float radius;
	Points center;

	std::vector<Points> RGB;
	std::vector<Points> V;
	std::vector<Normal> VN;
	std::vector<Face> F;
};

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
			Points rgb;
			std::istringstream in(line);
			in >> head >> v.X >> v.Y >> v.Z >> rgb.X >> rgb.Y >> rgb.Z;
			V.push_back(v);
			RGB.push_back(rgb);
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

int degree = 0;
void Model::Draw(){
	degree += 1;
	float m_Emissive[] = { 0.0, 0.0, 0.0, 1.0 };
	float m_Ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	float m_Diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
	float m_Specular[] = { 1.0, 1.0, 1.0, 1.0 };
	float m_fShininess = float(degree % 120 + 1);

	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &m_Emissive[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &m_Ambient[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &m_Diffuse[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &m_Specular[0]);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_fShininess);

	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < F.size(); i++)
	{
		glNormal3f(VN[F[i].V[0]].NX, VN[F[i].V[0]].NY, VN[F[i].V[0]].NZ);
		glColor3f(RGB[F[i].V[0]].X, RGB[F[i].V[0]].Y, RGB[F[i].V[0]].Z);
		glVertex3f(V[F[i].V[0]].X, V[F[i].V[0]].Y, V[F[i].V[0]].Z);

		glNormal3f(VN[F[i].V[1]].NX, VN[F[i].V[1]].NY, VN[F[i].V[1]].NZ);
		glColor3f(RGB[F[i].V[1]].X, RGB[F[i].V[1]].Y, RGB[F[i].V[1]].Z);
		glVertex3f(V[F[i].V[1]].X, V[F[i].V[1]].Y, V[F[i].V[1]].Z);

		glNormal3f(VN[F[i].V[2]].NX, VN[F[i].V[2]].NY, VN[F[i].V[2]].NZ);
		glColor3f(RGB[F[i].V[2]].X, RGB[F[i].V[2]].Y, RGB[F[i].V[2]].Z);
		glVertex3f(V[F[i].V[2]].X, V[F[i].V[2]].Y, V[F[i].V[2]].Z);
	}
	glEnd();
}

void Model::SearchNeibor()
{
	Normal *pointsNormal = new Normal[V.size()];
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
		fout << "v " << V[i].X << " " << V[i].Y << " " << V[i].Z << " " << RGB[i].X << " " << RGB[i].Y << " " << RGB[i].Z << std::endl;
	}

	for (size_t i = 0; i < F.size(); i++)
	{
		fout << "f " << F[i].V[0] + 1 << " " << F[i].V[1] + 1 << " " << F[i].V[2] + 1 << std::endl;
	}
	fout.close();
}