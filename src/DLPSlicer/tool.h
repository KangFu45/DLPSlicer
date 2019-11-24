#include"libslic3r.h"
#include"admesh\stl.h"
#include"Point.hpp"
#include"TriangleMesh.hpp"
#include"qmath.h"
#include"qdebug.h"

namespace Slic3r{

	//********************
	//日期：2017
	//功能：bmp的头文件。
	//********************
	static char head[54] = {
		0x42, //0
		0x4d, //1
		0x66, //2
		0x75, //3
		0x00, //4
		0x00, //5
		0x00, //6
		0x00, //7
		0x00, //8
		0x00, //9
		0x36, //a
		0x00, //b
		0x00, //c
		0x00, //d
		0x28, //e
		0x00,//f
		0x00, //0
		0x00, //1
		0x64, //2
		0x00, //3
		0x00, //4
		0x00, //5
		0x64, //6
		0x00, //7
		0x00, //8
		0x00, //9
		0x01, //a
		0x00, //b
		0x18, //c
		0x00, //d
		0x00, //e
		0x00,//f
		0x00, //0
		0x00, //1
		0x30, //2
		0x75//3
	};

	// Determine whether two vectors v1 and v2 point to the same direction
	// v1 = Cross(AB, AC)
	// v2 = Cross(AB, AP)
	inline bool SameSide(Pointf3 A, Pointf3 B, Pointf3 C, Pointf3 P)
	{

		Pointf3 AB = B - A;
		Pointf3 AC = C - A;
		Pointf3 AP = P - A;

		Pointf3 v1 = AB.Cross(AC);
		Pointf3 v2 = AB.Cross(AP);
		// v1 and v2 should point to the same direction
		return v1.Dot(v2) >= 0;

	}



	// Same side method
	// Determine whether point P in triangle ABC
	inline bool PointinTriangle1(Pointf3 A, Pointf3 B, Pointf3 C, Pointf3 P)
	{
		return SameSide(A, B, C, P) &&
			SameSide(B, C, A, P) &&
			SameSide(C, A, B, P);
	}

	//*************************
	//功能：求法向量。
	//参数1-3：三角面的三个点
	//返回：法向量
	//************************
	inline void getNormal(stl_normal& normal, Pointf3 a, Pointf3 b, Pointf3 c)
	{
		//AB = （a1, a2, a3）
		//AC = (b1, b2, b3)
		//它的法向量为AB×AC = （a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1）

		float a1[3];
		float a2[3];

		a1[0] = a.x - b.x;
		a1[1] = a.y - b.y;
		a1[2] = a.z - b.z;

		a2[0] = a.x - c.x;
		a2[1] = a.y - c.y;
		a2[2] = a.z - c.z;

		normal.x = a1[1] * a2[2] - a1[2] * a2[1];
		normal.y = a1[2] * a2[0] - a1[0] * a2[2];
		normal.z = a1[0] * a2[1] - a1[1] * a2[0];
	}

	//************************
	//日期：2017
	//功能：求出单位圆的点。
	//参数1：单位圆点的集合。
	//************************
	inline void CirclePoints(std::vector<Pointf>& circle,int angle)
	{
		float factor = 0.5;
		for (int i = angle; i<376;){
			float x = sin(PI / 180 * i)*factor;
			float y = cos(PI / 180 * i)*factor;
			circle.push_back(Pointf(x, y));
			i += angle*2;
		}
	}

	//****************************
	//日期：2017
	//功能：求法向量与xy平面的夹角。
	//参数1：法向量
	//返回：夹角值
	//******************************
	inline double normal_angle(stl_normal& a)
	{
		return -a.z / sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	}

	//范围是-180°到180°的弧度值,返回弧度值
	inline double vector_angle_3D(Vectorf3 v1, Vectorf3 v2)
	{
		return acos((v1.x*v2.x + v1.y*v2.y + v1.z*v2.z) / (sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)*sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z)));
	}

	inline double vector_angle_2D(Vectorf v1, Vectorf v2)
	{
		return acos((v1.x*v2.x + v1.y*v2.y) / (sqrt(v1.x*v1.x + v1.y*v1.y)*sqrt(v2.x*v2.x + v2.y*v2.y )));
	}



	/// <summary>
	/// 求一条直线与平面的交点
	/// </summary>
	/// <param name="planeVector">平面的法线向量，长度为3</param>
	/// <param name="planePoint">平面经过的一点坐标，长度为3</param>
	/// <param name="lineVector">直线的方向向量，长度为3</param>
	/// <param name="linePoint">直线经过的一点坐标，长度为3</param>
	/// <returns>返回交点坐标，长度为3</returns>
	inline Pointf3 CalPlaneLineIntersectPoint(Vectorf3& planeVector, Pointf3& planePoint, Vectorf3& lineVector, Pointf3& linePoint)
	{
		Pointf3 returnResult;
		float vp1, vp2, vp3, n1, n2, n3, v1, v2, v3, m1, m2, m3, t, vpt;
		vp1 = planeVector.x;
		vp2 = planeVector.y;
		vp3 = planeVector.z;
		n1 = planePoint.x;
		n2 = planePoint.y;
		n3 = planePoint.z;
		v1 = lineVector.x;
		v2 = lineVector.y;
		v3 = lineVector.z;
		m1 = linePoint.x;
		m2 = linePoint.y;
		m3 = linePoint.z;
		vpt = v1 * vp1 + v2 * vp2 + v3 * vp3;
		//首先判断直线是否与平面平行
		if (vpt == 0)
		{
			//qDebug() << "hahaha";
			returnResult.x = -1;
			returnResult.y = -1;
			returnResult.z = -1;
		}
		else
		{
			//直线的参数方程与平面的点法式方程联立
			t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
			returnResult.x = m1 + v1 * t;
			returnResult.y = m2 + v2 * t;
			returnResult.z = m3 + v3 * t;
		}
		return returnResult;
	}

	//*************************
	//日期：2017
	//功能：求两点之间的距离。
	//参数1-2：两个二维点
	//返回：距离值
	//************************
	inline float distance_point(Pointf a, Pointf b)
	{
		return sqrtf((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
	}

	//************************
	//日期：2017
	//功能：3维的两点的距离。
	//参数1-2：两个三维点
	//返回：距离值
	//************************
	inline float distance_point_3(Pointf3 a, Pointf3 b)
	{
		return sqrtf((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
	}

	//*************************
	//日期：2017
	//功能：求三角面片的面积。
	//参数1：三角面片
	//返回：面积值
	//************************
	inline double triangle_area(stl_facet f)
	{
		double a = sqrt((f.vertex[0].x - f.vertex[1].x)*(f.vertex[0].x - f.vertex[1].x) +
			(f.vertex[0].y - f.vertex[1].y)*(f.vertex[0].y - f.vertex[1].y) + (f.vertex[0].z - f.vertex[1].z)*(f.vertex[0].z - f.vertex[1].z));

		double b = sqrt((f.vertex[0].x - f.vertex[2].x)*(f.vertex[0].x - f.vertex[2].x) +
			(f.vertex[0].y - f.vertex[2].y)*(f.vertex[0].y - f.vertex[2].y) + (f.vertex[0].z - f.vertex[2].z)*(f.vertex[0].z - f.vertex[2].z));

		double c = sqrt((f.vertex[2].x - f.vertex[1].x)*(f.vertex[2].x - f.vertex[1].x) +
			(f.vertex[2].y - f.vertex[1].y)*(f.vertex[2].y - f.vertex[1].y) + (f.vertex[2].z - f.vertex[1].z)*(f.vertex[2].z - f.vertex[1].z));

		double p = (a + b + c) / 2;

		return sqrt((p*(p - a)*(p - b)*(p - c)));
	}

	//******************************
	//日期：2018.3.15
	//功能：模型添加单个点的三角面片
	//参数1-3：第一个点
	//参数4-6：第二个点
	//参数7-9：第三个点
	//参数10：模型
	//******************************
	inline void stl_add_face(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, TriangleMesh& mesh) {
		stl_facet f;
		f.vertex[0].x = x1; f.vertex[0].y = y1; f.vertex[0].z = z1;
		f.vertex[1].x = x2; f.vertex[1].y = y2; f.vertex[1].z = z2;
		f.vertex[2].x = x3; f.vertex[2].y = y3; f.vertex[2].z = z3;
		stl_add_facet(&mesh.stl, &f);
	}

	//*******************************************
	//日期：2018.4.3
	//功能：xy平面上的法向量在z方向旋转一定角度。
	//*******************************************
	inline void XY_normal_sacle_Z(float norm[], float angle)
	{
		norm[2] = cosf(angle)*sqrtf(norm[0] * norm[0] + norm[1] * norm[1]);
		norm[1] = cosf(angle)*norm[1];
		norm[0] = cosf(angle)*norm[0];
		//stl_normalize_vector(norm);
	}

	//**************************
	//日期：2018.4.8
	//功能：模型添加三角面片。
	//参数1-3：三角面的三个点
	//参数4：模型
	//*************************
	inline void std_add_face1(Pointf3 p1, Pointf3 p2, Pointf3 p3, TriangleMesh& mesh) {
		stl_add_face(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, mesh);
	}

	//******************************
	//日期：2018.4.11
	//功能：判断线段是否穿过三角面片。
	//参数1：三角面片
	//参数2：线段
	//返回：是否相交
	//*******************************
	inline bool line_to_triangle_bool(stl_facet f, Linef3 line) {
		//得到射线方向
		float norm[3];
		norm[0] = line.a.x - line.b.x;
		norm[1] = line.a.y - line.b.y;
		norm[2] = line.a.z - line.b.z;
		stl_normalize_vector(norm);

		//得到射线与平面的交点
		Pointf3 a = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z),
			Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), Vectorf3(norm[0], norm[1], norm[2]), line.a);

		//PointinTriangle1()无法判断？？？应用范围有限？？？
		if (a.x == -1 && a.y == -1 && a.z == -1)
			return false;

		//判断交点是否在三角面上
		if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
			Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
			Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), Pointf3(a.x, a.y, a.z))) {
			//是否穿过
			if ((a.z<=line.b.z&&a.z>line.a.z)|| (a.z <= line.a.z&&a.z>line.b.z)) {
				return true;
			}
		}
		return false;
	}

	//***************************
	//日期：2018.4.11
	//功能：射线与三角面片求交点。
	//参数1：三角面片
	//参数2：射线方向
	//参数3：射线起点
	//参数4：有交点，返回交点
	//返回：是否有交点
	//****************************
	inline bool line_to_triangle_point(stl_facet f, Vectorf3 norm, Pointf3 p,Pointf3& a) {
		//得到射线与平面的交点
		a = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z),
			Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), norm, p);

		//PointinTriangle1()无法判断？？？应用范围有限？？？
		if (a.x == -1 && a.y == -1 && a.z == -1)
			return false;

		//判断交点是否在三角面上
		if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
			Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
			Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), Pointf3(a.x, a.y, a.z))) {
			return true;
		}
		return false;
	}

	inline Pointf interpolation_point(Pointf a, Pointf b, float dis)
	{
		float v[3];
		v[0] = b.x - a.x;
		v[1] = b.y - a.y;
		v[2] = 0;

		stl_normalize_vector(v);

		return Pointf(a.x + dis*v[0], a.y + dis*v[1]);
	}

	inline bool equal_vertex(const stl_vertex& v1, const stl_vertex& v2)
	{
		if (v1.x == v2.x&&v1.y == v2.y&&v1.z == v2.z)
			return true;
		return false;
	}

	inline bool equal_edge(const stl_edge& edge1, const stl_edge& edge2)
	{
		if (equal_vertex(edge1.p1, edge2.p1) && equal_vertex(edge1.p2, edge2.p2))
			return true;

		if (equal_vertex(edge1.p1, edge2.p2) && equal_vertex(edge1.p2, edge2.p1))
			return true;

		return false;
	}

}
