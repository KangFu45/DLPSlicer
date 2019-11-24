#include"libslic3r.h"
#include"admesh\stl.h"
#include"Point.hpp"
#include"TriangleMesh.hpp"
#include"qmath.h"
#include"qdebug.h"

namespace Slic3r{

	//********************
	//���ڣ�2017
	//���ܣ�bmp��ͷ�ļ���
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
	//���ܣ���������
	//����1-3���������������
	//���أ�������
	//************************
	inline void getNormal(stl_normal& normal, Pointf3 a, Pointf3 b, Pointf3 c)
	{
		//AB = ��a1, a2, a3��
		//AC = (b1, b2, b3)
		//���ķ�����ΪAB��AC = ��a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1��

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
	//���ڣ�2017
	//���ܣ������λԲ�ĵ㡣
	//����1����λԲ��ļ��ϡ�
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
	//���ڣ�2017
	//���ܣ���������xyƽ��ļнǡ�
	//����1��������
	//���أ��н�ֵ
	//******************************
	inline double normal_angle(stl_normal& a)
	{
		return -a.z / sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	}

	//��Χ��-180�㵽180��Ļ���ֵ,���ػ���ֵ
	inline double vector_angle_3D(Vectorf3 v1, Vectorf3 v2)
	{
		return acos((v1.x*v2.x + v1.y*v2.y + v1.z*v2.z) / (sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)*sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z)));
	}

	inline double vector_angle_2D(Vectorf v1, Vectorf v2)
	{
		return acos((v1.x*v2.x + v1.y*v2.y) / (sqrt(v1.x*v1.x + v1.y*v1.y)*sqrt(v2.x*v2.x + v2.y*v2.y )));
	}



	/// <summary>
	/// ��һ��ֱ����ƽ��Ľ���
	/// </summary>
	/// <param name="planeVector">ƽ��ķ�������������Ϊ3</param>
	/// <param name="planePoint">ƽ�澭����һ�����꣬����Ϊ3</param>
	/// <param name="lineVector">ֱ�ߵķ�������������Ϊ3</param>
	/// <param name="linePoint">ֱ�߾�����һ�����꣬����Ϊ3</param>
	/// <returns>���ؽ������꣬����Ϊ3</returns>
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
		//�����ж�ֱ���Ƿ���ƽ��ƽ��
		if (vpt == 0)
		{
			//qDebug() << "hahaha";
			returnResult.x = -1;
			returnResult.y = -1;
			returnResult.z = -1;
		}
		else
		{
			//ֱ�ߵĲ���������ƽ��ĵ㷨ʽ��������
			t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
			returnResult.x = m1 + v1 * t;
			returnResult.y = m2 + v2 * t;
			returnResult.z = m3 + v3 * t;
		}
		return returnResult;
	}

	//*************************
	//���ڣ�2017
	//���ܣ�������֮��ľ��롣
	//����1-2��������ά��
	//���أ�����ֵ
	//************************
	inline float distance_point(Pointf a, Pointf b)
	{
		return sqrtf((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
	}

	//************************
	//���ڣ�2017
	//���ܣ�3ά������ľ��롣
	//����1-2��������ά��
	//���أ�����ֵ
	//************************
	inline float distance_point_3(Pointf3 a, Pointf3 b)
	{
		return sqrtf((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
	}

	//*************************
	//���ڣ�2017
	//���ܣ���������Ƭ�������
	//����1��������Ƭ
	//���أ����ֵ
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
	//���ڣ�2018.3.15
	//���ܣ�ģ����ӵ������������Ƭ
	//����1-3����һ����
	//����4-6���ڶ�����
	//����7-9����������
	//����10��ģ��
	//******************************
	inline void stl_add_face(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, TriangleMesh& mesh) {
		stl_facet f;
		f.vertex[0].x = x1; f.vertex[0].y = y1; f.vertex[0].z = z1;
		f.vertex[1].x = x2; f.vertex[1].y = y2; f.vertex[1].z = z2;
		f.vertex[2].x = x3; f.vertex[2].y = y3; f.vertex[2].z = z3;
		stl_add_facet(&mesh.stl, &f);
	}

	//*******************************************
	//���ڣ�2018.4.3
	//���ܣ�xyƽ���ϵķ�������z������תһ���Ƕȡ�
	//*******************************************
	inline void XY_normal_sacle_Z(float norm[], float angle)
	{
		norm[2] = cosf(angle)*sqrtf(norm[0] * norm[0] + norm[1] * norm[1]);
		norm[1] = cosf(angle)*norm[1];
		norm[0] = cosf(angle)*norm[0];
		//stl_normalize_vector(norm);
	}

	//**************************
	//���ڣ�2018.4.8
	//���ܣ�ģ�����������Ƭ��
	//����1-3���������������
	//����4��ģ��
	//*************************
	inline void std_add_face1(Pointf3 p1, Pointf3 p2, Pointf3 p3, TriangleMesh& mesh) {
		stl_add_face(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, mesh);
	}

	//******************************
	//���ڣ�2018.4.11
	//���ܣ��ж��߶��Ƿ񴩹�������Ƭ��
	//����1��������Ƭ
	//����2���߶�
	//���أ��Ƿ��ཻ
	//*******************************
	inline bool line_to_triangle_bool(stl_facet f, Linef3 line) {
		//�õ����߷���
		float norm[3];
		norm[0] = line.a.x - line.b.x;
		norm[1] = line.a.y - line.b.y;
		norm[2] = line.a.z - line.b.z;
		stl_normalize_vector(norm);

		//�õ�������ƽ��Ľ���
		Pointf3 a = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z),
			Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), Vectorf3(norm[0], norm[1], norm[2]), line.a);

		//PointinTriangle1()�޷��жϣ�����Ӧ�÷�Χ���ޣ�����
		if (a.x == -1 && a.y == -1 && a.z == -1)
			return false;

		//�жϽ����Ƿ�����������
		if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z),
			Pointf3(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z),
			Pointf3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z), Pointf3(a.x, a.y, a.z))) {
			//�Ƿ񴩹�
			if ((a.z<=line.b.z&&a.z>line.a.z)|| (a.z <= line.a.z&&a.z>line.b.z)) {
				return true;
			}
		}
		return false;
	}

	//***************************
	//���ڣ�2018.4.11
	//���ܣ�������������Ƭ�󽻵㡣
	//����1��������Ƭ
	//����2�����߷���
	//����3���������
	//����4���н��㣬���ؽ���
	//���أ��Ƿ��н���
	//****************************
	inline bool line_to_triangle_point(stl_facet f, Vectorf3 norm, Pointf3 p,Pointf3& a) {
		//�õ�������ƽ��Ľ���
		a = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z),
			Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), norm, p);

		//PointinTriangle1()�޷��жϣ�����Ӧ�÷�Χ���ޣ�����
		if (a.x == -1 && a.y == -1 && a.z == -1)
			return false;

		//�жϽ����Ƿ�����������
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
