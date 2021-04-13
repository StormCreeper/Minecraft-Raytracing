#pragma once
#include <istream>
#include <ostream>
using namespace std;

/*#include <random>
random_device rd;
mt19937 mt(rd());
uniform_int_distribution<int> DIST(0, 1001);
*/

#define m_pi 3.1415926535f
#define degtorad m_pi / 180.0f

struct v3 {
	float x, y, z;

	float w = 1;

	v3() {}

	v3(float xx, float yy, float zz)
	{
		x = xx; y = yy; z = zz;
	}

	v3 operator + (v3 other) { return { x + other.x, y + other.y, z + other.z }; }
	v3 operator - (v3 other) { return { x - other.x, y - other.y, z - other.z }; }
	v3 operator * (float other) { return { x * other, y * other, z * other }; }
	v3 operator / (float other) { return { x / other, y / other, z / other }; }

	v3 operator += (v3 other) { return { x += other.x, y += other.y, z += other.z }; }
	v3 operator -= (v3 other) { return { x -= other.x, y -= other.y, z -= other.z }; }

	float dot(v3 other) {
		return  x * other.x + y * other.y + z * other.z;
	}

	v3 cross(v3 o)
	{
		return {
			y * o.z - z * o.y,
			z * o.x - x * o.z,
			x * o.y - y * o.x
		};
	}

	float mag()
	{
		return { sqrt((x * x) + (y * y) + (z * z)) };
	}

	float dist(v3 other)
	{
		v3 t = other - *this;

		return t.mag();
	}

	v3 multiply(v3 other)
	{
		return { x * other.x, y * other.y, z * other.z };
	}

	v3 reflect(v3 norm)
	{
		return *this - (norm * ((*this).dot(norm))) * 2;
	}

	v3 norm()
	{
		float t = mag();
		return { x / t,y / t,z / t };
	}

	v3 lerpvf3d(v3 other, float roughness)
	{
		return (*this) * (1 - roughness) + other * roughness;
	}

	/*static v3 randomVf3d()
	{
		float a = (float)(-500 + (DIST(mt))) / 500.0;
		float b = (float)(-500 + (DIST(mt))) / 500.0;
		float c = (float)(-500 + (DIST(mt))) / 500.0;

		v3 temp = { a,b,c };

		return temp;
	}*/

	/*static vf3d FromPixel(olc::Pixel p)
	{
		return vf3d((float)p.r / 255.0,(float)p.g / 255.0 ,(float)p.b / 255.0);
	}*/

	/*static v3 randomVf3d(v3 n)
	{
		float a = (float)(-500 + (DIST(mt))) / 500.0;
		float b = (float)(-500 + (DIST(mt))) / 500.0;
		float c = (float)(-500 + (DIST(mt))) / 500.0;

		v3 temp = { a,b,c };

		if (temp.dot(n) < 0)
		{
			return temp * -1;
		}

		return temp;
	}*/
};

ostream& operator<< (ostream& os, v3 vec);

struct mat44 {
	float m[4][4];

	mat44() {
	}

	static mat44 CreateIdentity()
	{
		mat44 temp;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == j) {
					temp.m[i][j] = 1;
				}
				else {
					temp.m[i][j] = 0;
				}
			}
		}

		return temp;
	}

	v3 operator * (v3 o)
	{
		v3 temp;
		float* a = &o.x;
		for (int i = 0; i < 4; i++)
		{
			*a = m[i][0] * o.x + m[i][1] * o.y + m[i][2] * o.z + m[i][3] * o.w;
			a++;
		}
		return temp;
	}

	mat44 operator * (mat44 o)
	{
		mat44 out;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				out.m[i][j] = m[i][0] * o.m[0][j] + m[i][1] * o.m[1][j] + m[i][2] * o.m[2][j] + m[i][3] * o.m[3][j];
			}
		}

		return out;
	}

	mat44 operator * (float o)
	{
		mat44 temp;

		float* d = (float*)(void*)temp.m;
		float* t = (float*)(void*)m;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = m[i][j] * o;
			}
		}
		return temp;
	}

	mat44 operator + (mat44 o)
	{
		mat44 temp;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = m[i][j] + o.m[i][j];
			}
		}

		return temp;
	}

	mat44 Invert()
	{
		mat44 o;

		o.m[0][0] = m[0][0];       o.m[0][1] = m[1][0];     o.m[0][2] = m[2][0];       o.m[0][3] = m[0][3];
		o.m[1][0] = m[0][1];       o.m[1][1] = m[1][1];     o.m[1][2] = m[2][1];      o.m[1][3] = m[1][3];
		o.m[2][0] = m[0][2];       o.m[2][1] = m[1][2];     o.m[2][2] = m[2][2];       o.m[2][3] = m[2][3];
		o.m[3][0] = m[3][0];       o.m[3][1] = m[3][1];     o.m[3][2] = m[3][2];       o.m[3][3] = m[3][3];
		return o;
	}

	static mat44 CreateXRot(float a)
	{
		mat44 o;

		o.m[0][0] = 1; o.m[0][1] = 0; o.m[0][2] = 0; o.m[0][3] = 0;
		o.m[1][0] = 0; o.m[1][1] = cos(a); o.m[1][2] = -sin(a); o.m[1][3] = 0;
		o.m[2][0] = 0; o.m[2][1] = sin(a); o.m[2][2] = cos(a); o.m[2][3] = 0;
		o.m[3][0] = 0; o.m[3][1] = 0; o.m[3][2] = 0; o.m[3][3] = 1;
		return o;
	}
	static mat44 CreateYRot(float a)
	{
		mat44 o;

		o.m[0][0] = cos(a); o.m[0][1] = 0; o.m[0][2] = sin(a); o.m[0][3] = 0;
		o.m[1][0] = 0; o.m[1][1] = 1; o.m[1][2] = 0; o.m[1][3] = 0;
		o.m[2][0] = -sin(a); o.m[2][1] = 0; o.m[2][2] = cos(a); o.m[2][3] = 0;
		o.m[3][0] = 0; o.m[3][1] = 0; o.m[3][2] = 0; o.m[3][3] = 1;
		return o;
	}
	static mat44 CreateZRot(float a)
	{
		mat44 o;

		o.m[0][0] = cos(a); o.m[0][1] = -sin(a); o.m[0][2] = 0; o.m[0][3] = 0;
		o.m[1][0] = sin(a); o.m[1][1] = cos(a); o.m[1][2] = 0; o.m[1][3] = 0;
		o.m[2][0] = 0; o.m[2][1] = 0; o.m[2][2] = 1; o.m[2][3] = 0;
		o.m[3][0] = 0; o.m[3][1] = 0; o.m[3][2] = 0; o.m[3][3] = 1;
		return o;
	}

	static mat44 LookAt(v3 c, v3 point, v3 up)
	{
		v3 z = (point - c).norm();
		v3 x = z.cross(up);

		v3 y = x.cross(z);

		mat44 t;

		t.m[0][0] = x.x; t.m[0][1] = y.x; t.m[0][2] = z.x;
		t.m[1][0] = x.y; t.m[1][1] = y.y; t.m[1][2] = z.y;
		t.m[2][0] = x.z; t.m[2][1] = y.z; t.m[2][2] = z.z;

		/*t.m[0][0] = x.x; t.m[0][1] = x.y; t.m[0][2] = x.z;
		t.m[1][0] = y.x; t.m[1][1] = y.y; t.m[1][2] = y.z;
		t.m[2][0] = z.x; t.m[2][1] = z.y; t.m[2][2] = z.z;*/

		t.m[3][3] = 1;

		return t;
	}

	v3 GetBasisX()
	{
		return { m[0][0],m[1][0],m[2][0] };
	}
	v3 GetBasisY()
	{
		return { m[0][1],m[1][1],m[2][1] };
	}
	v3 GetBasisZ()
	{
		return { m[0][2],m[1][2],m[2][2] };
	}

	void reOrthogonalize()
	{
		v3 x = GetBasisX();
		v3 y = GetBasisY();
		v3 z = GetBasisZ();

		z = y.cross(x).norm();
		x = z.cross(y).norm();
		y = x.cross(z).norm();

		m[0][0] = x.x;
		m[1][0] = x.y;
		m[2][0] = x.z;

		m[0][1] = y.x;
		m[1][1] = y.y;
		m[2][1] = y.z;

		m[0][2] = z.x;
		m[1][2] = z.y;
		m[2][2] = z.z;
	}

	mat44 differentiate(v3 v)
	{
		mat44 temp = mat44::CreateIdentity();

		v3 x = v.cross(GetBasisX());
		v3 y = v.cross(GetBasisY());
		v3 z = v.cross(GetBasisZ());

		temp.m[0][0] = x.x;
		temp.m[1][0] = x.y;
		temp.m[2][0] = x.z;
		temp.m[0][1] = y.x;
		temp.m[1][1] = y.y;
		temp.m[2][1] = y.z;
		temp.m[0][2] = z.x;
		temp.m[1][2] = z.y;
		temp.m[2][2] = z.z;

		return temp;
	}
};
