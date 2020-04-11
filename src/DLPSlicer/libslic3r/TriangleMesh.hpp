#pragma once

#include "libslic3r.h"
#include <admesh/stl.h>
#include <vector>
#include <boost/thread.hpp>
#include "BoundingBox.hpp"
#include "Line.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "ExPolygon.hpp"
#include "qmatrix4x4.h"
#include "qprogressbar.h"

namespace DLPSlicer {

typedef std::vector<stl_vertex> stl_vertexs;

inline bool 
operator==(const Pointf3& p, const stl_vertex& v)
{
    if (p.x == v.x && p.y == v.y && p.z == v.z)
        return true;
    return false;
}

inline bool
operator==(const stl_vertex& v1, const stl_vertex& v2)
{
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
		return true;
	return false;
}

inline bool
operator==(const stl_facet& f1, const stl_facet& f2)
{
	if (f1.vertex[0] == f2.vertex[0] && f1.vertex[1] == f2.vertex[1] && f1.vertex[2] == f2.vertex[2])
		return true;
	return false;
}

inline bool equal_vertex(const stl_vertex& v1, const stl_vertex& v2)
{
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
		return true;
	return false;
}

inline bool equal_edge(const stl_edge& edge1, const stl_edge& edge2)
{
	if ((equal_vertex(edge1.p1, edge2.p1) && equal_vertex(edge1.p2, edge2.p2))
		|| (equal_vertex(edge1.p1, edge2.p2) && equal_vertex(edge1.p2, edge2.p1)))
		return true;
	return false;
}

inline Pointf3 Ver2Pf3(const stl_vertex& v) { return Pointf3(v.x, v.y, v.z); }
inline stl_vertex Pf32Ver(const Pointf3& p) { return stl_vertex{ (float)p.x,(float)p.y,(float)p.z }; }
inline Vectorf3 Nor2Vt3(const stl_normal& n) { return Vectorf3(n.x, n.y, n.z); }

// Determine whether two vectors v1 and v2 point to the same direction
	// v1 = Cross(AB, AC)
	// v2 = Cross(AB, AP)
inline bool SameSide(const Pointf3& A, const Pointf3& B, const  Pointf3& C, const Pointf3& P)
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
inline bool PointInTriangle3(const Pointf3& A, const Pointf3& B, const Pointf3& C, const Pointf3& P)
{
	return SameSide(A, B, C, P) &&
		SameSide(B, C, A, P) &&
		SameSide(C, A, B, P);
}

inline bool PointInTriangle3(const stl_facet& face, const Pointf3& P)
{
	return PointInTriangle3(Ver2Pf3(face.vertex[0]), Ver2Pf3(face.vertex[1]), Ver2Pf3(face.vertex[2]), P);
}

//xyƽ��ͶӰ���Ƿ������
inline bool PointInTriangle2(stl_facet face, Pointf3 P)
{
	face.vertex[0].z = 0;
	face.vertex[1].z = 0;
	face.vertex[2].z = 0;
	P.z = 0;
	return PointInTriangle3(face, P);
}

//���ܣ������λԲ�ĵ�
inline Pointfs CirclePoints(int angle)
{
	Pointfs circle;
	float factor = 0.5;
	for (int i = angle; i < 376; i += angle * 2)
		circle.emplace_back(Pointf(sin(PI / 180 * i) * factor, cos(PI / 180 * i) * factor));
	return circle;
}

//���ܣ���������xyƽ��ļн�
inline double aormal_xy_angle(const stl_normal& a) { return -a.z / sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }

//��Χ��-180�㵽180��Ļ���ֵ,���ػ���ֵ
inline double vector_angle_3(const Vectorf3& v1, const Vectorf3& v2)
{
	return acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
		/ (sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z)
			* sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z)));
}

inline double vector_angle_2(const Vectorf& v1, const Vectorf& v2)
{
	return acos((v1.x * v2.x + v1.y * v2.y)
		/ (sqrt(v1.x * v1.x + v1.y * v1.y)
			* sqrt(v2.x * v2.x + v2.y * v2.y)));
}

/// <summary>
/// ��һ��ֱ����ƽ��Ľ���
/// </summary>
/// <param name="planeVector">ƽ��ķ�������������Ϊ3</param>
/// <param name="planePoint">ƽ�澭����һ�����꣬����Ϊ3</param>
/// <param name="lineVector">ֱ�ߵķ�������������Ϊ3</param>
/// <param name="linePoint">ֱ�߾�����һ�����꣬����Ϊ3</param>
/// <returns>���ؽ������꣬����Ϊ3</returns>
inline Pointf3 CalPlaneLineIntersectPoint(const Vectorf3& planeVector
	, const Pointf3& planePoint, const Vectorf3& lineVector, const Pointf3& linePoint)
{
	Pointf3 res;
	double vp1 = planeVector.x;
	double vp2 = planeVector.y;
	double vp3 = planeVector.z;
	double n1 = planePoint.x;
	double n2 = planePoint.y;
	double n3 = planePoint.z;
	double v1 = lineVector.x;
	double v2 = lineVector.y;
	double v3 = lineVector.z;
	double m1 = linePoint.x;
	double m2 = linePoint.y;
	double m3 = linePoint.z;
	double vpt = v1 * vp1 + v2 * vp2 + v3 * vp3;
	//�����ж�ֱ���Ƿ���ƽ��ƽ��
	if (vpt == 0)
	{
		res.x = -1;
		res.y = -1;
		res.z = -1;
	}
	else
	{
		//ֱ�ߵĲ���������ƽ��ĵ㷨ʽ��������
		double t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
		res.x = m1 + v1 * t;
		res.y = m2 + v2 * t;
		res.z = m3 + v3 * t;
	}
	return res;
}
//������֮��ľ���
inline float DisPoint2(const Pointf& a, const Pointf& b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)); }
//3ά������ľ���
inline float DisPoint3(const Pointf3& a, const Pointf3& b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z)); }

//��������Ƭ�����
inline double TriArea(const stl_facet& f)
{
	double a = sqrt((f.vertex[0].x - f.vertex[1].x) * (f.vertex[0].x - f.vertex[1].x) +
		(f.vertex[0].y - f.vertex[1].y) * (f.vertex[0].y - f.vertex[1].y) + (f.vertex[0].z - f.vertex[1].z) * (f.vertex[0].z - f.vertex[1].z));

	double b = sqrt((f.vertex[0].x - f.vertex[2].x) * (f.vertex[0].x - f.vertex[2].x) +
		(f.vertex[0].y - f.vertex[2].y) * (f.vertex[0].y - f.vertex[2].y) + (f.vertex[0].z - f.vertex[2].z) * (f.vertex[0].z - f.vertex[2].z));

	double c = sqrt((f.vertex[2].x - f.vertex[1].x) * (f.vertex[2].x - f.vertex[1].x) +
		(f.vertex[2].y - f.vertex[1].y) * (f.vertex[2].y - f.vertex[1].y) + (f.vertex[2].z - f.vertex[1].z) * (f.vertex[2].z - f.vertex[1].z));

	double p = (a + b + c) / 2;
	return sqrt((p * (p - a) * (p - b) * (p - c)));
}

//xyƽ���ϵķ�������z������תһ���Ƕȡ�
inline void xy_normal_rotate_z(float* norm, float angle)
{
	norm[2] = cosf(angle) * sqrtf(norm[0] * norm[0] + norm[1] * norm[1]);
	norm[1] = cosf(angle) * norm[1];
	norm[0] = cosf(angle) * norm[0];
	//stl_normalize_vector(norm);
}

//�ж��߶��Ƿ񴩹�������Ƭ
inline bool isLineCrossTriangle(const stl_facet& f, const Linef3& line) {
	//�õ����߷���
	float norm[3] = { line.a.x - line.b.x , line.a.y - line.b.y,line.a.z - line.b.z };
	stl_normalize_vector(norm);

	//�õ�������ƽ��Ľ���
	Pointf3 inter = CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0])
		, Vectorf3(norm[0], norm[1], norm[2]), line.a);

	//PointInTriangle3()�޷��жϣ�����Ӧ�÷�Χ���ޣ�����
	if (inter.x == -1 && inter.y == -1 && inter.z == -1)
		return false;

	//�жϽ����Ƿ�����������
	if (PointInTriangle3(f, inter)) {
		//�Ƿ񴩹�
		if ((inter.z <= line.b.z && inter.z > line.a.z)
			|| (inter.z <= line.a.z && inter.z > line.b.z))
			return true;
	}
	return false;
}

//���ܣ�������������Ƭ�󽻵㡣
//����1��������Ƭ
//����2�����߷���
//����3���������
//����4���н��㣬���ؽ���
//���أ��Ƿ��н���
inline bool Ray2TrianglePoint(const stl_facet& f, const Vectorf3& norm, const Pointf3& origin, Pointf3& inter) {
	//�õ�������ƽ��Ľ���
	inter = CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0]), norm, origin);

	//PointInTriangle3()�޷��жϣ�����Ӧ�÷�Χ���ޣ�����
	if (inter.x == -1 && inter.y == -1 && inter.z == -1)
		return false;

	//�жϽ����Ƿ�����������
	if (PointInTriangle3(f, inter))
		return true;

	return false;
}

//����֮������
inline Pointf InterpolationPoint(const Pointf& a,const Pointf& b, float dis)
{
	float v[3] = { b.x - a.x, b.y - a.y,0 };
	stl_normalize_vector(v);
	return Pointf(a.x + (double)dis * v[0], a.y + (double)dis * v[1]);
}
//******************************************************************************88

class TriangleMesh;
template <Axis A> class TriangleMeshSlicer;
typedef std::vector<TriangleMesh> TriangleMeshs;
typedef std::vector<TriangleMesh*> TriangleMeshPtrs;

class TriangleMesh
{
public:
	TriangleMesh();
	TriangleMesh(const Pointf3s& points, const std::vector<Point3>& facets);
	TriangleMesh(const TriangleMesh& other);
	TriangleMesh& operator= (TriangleMesh other);
	void swap(TriangleMesh& other);
	~TriangleMesh();
	void ReadSTLFile(const std::string& input_file);
	void write_ascii(const std::string& output_file);
	void write_binary(const std::string& output_file);
	void repair();
	void check_topology();
	float volume();
	bool is_manifold() const;
	void WriteOBJFile(const std::string& output_file);

	//ģ�ͱ任����
	void transform_matrix(QMatrix4x4 matrix);

	void scale(float factor);
	void scale(const Pointf3& versor);
	void translate(float x, float y, float z);
	void rotate(float angle, const Axis& axis);
	//�����뻡��ֵ
	void rotate_x(float angle);
	void rotate_y(float angle);
	void rotate_z(float angle);
	void mirror(const Axis& axis);
	void mirror_x();
	void mirror_y();
	void mirror_z();
	void align_to_origin();
	void center_around_origin();
	void rotate(double angle, Point* center);
	TriangleMeshPtrs split() const;
	TriangleMeshPtrs cut_by_grid(const Pointf& grid) const;
	void merge(const TriangleMesh& mesh);
	ExPolygons horizontal_projection() const;
	Polygon convex_hull();
	BoundingBoxf3 bounding_box() const;
	void reset_repair_stats();
	bool needed_repair() const;
	size_t facets_count() const;
	void extrude_tin(float offset);
	void require_shared_vertices();//add�õ������͵�����Χ����������ݹ�ϵ��
	void reverse_normals();

	//���ܣ���������档
	void extract_feature_face(int angle);

	//---------------------��������֧��-------------------
	//���ܣ����ɴ�֧�ŵĵ㡣
	//����1�������������������֧�ŵ�ļ�ࡣ
	//���أ���֧�ŵĵ㡣
	stl_vertexs feature_point(QProgressBar* progress);
	stl_vertexs feature_point_face(float space, QProgressBar* progress);

	//���ܣ��õ����ҵ㡣
	//���أ������������ֵ��
	int* cliff_point(unsigned short& num);

	//���ܣ�Ѱ��͹���㡣
	//����1:һ�������������ֵ��
	//���أ����������͹���߶ȡ�
	double find_convex_hull(int point);

	//���ܣ����������档
	//���أ�����������ļ��ϡ�
	std::vector<v_face_struct> generate_feature_faces(char extra,float min_area);
	//����1����ĽǶȱ�ǩ
	std::vector<v_face_struct> gather_feature_faces(char extra);

	//���ܣ����������������㡣
	//����1�����������򼯺ϡ�
	//����2�������������������֧�ŵ�ļ�ࡣ
	//���أ��������ϴ�֧�ŵĵ㡣
	void feature_face_to_point(std::vector<v_face_struct>& feature_faces, float space, stl_vertexs& feature_faces_point, char extra);

	//���ܣ������淢ɢ��
	//����1:�����������ֵ��
	//����2������ͬһ����������档
	void seed_face(int f, std::vector<int>& faces, char extra);

	//���ܣ�������abΪ����һ��������Ƭ��
	//����1��һ�������棨���������ıߣ�
	//����2�����ϵ�һ��a
	//����3�����ϵ�һ��b
	//���أ����ߵ�������
	int find_common_edge_face(int face, int a, int b);

	//���ܣ������浽��άͶӰ��
	//����1������������
	//����2-5����ά�ľ���ͶӰ����
	BoundingBoxf face_to_2D_projection(const v_face_struct& faces);

	//���ܣ�����Z��������ģ���󽻡�
	Pointf3 point_model_intersection_Z(Pointf3 p);
	//--------------------------------------------------------------------

	/// Generate a mesh representing a cube with dimensions (x, y, z), with one corner at (0,0,0).
	static TriangleMesh make_cube(double x, double y, double z);

	/// Generate a mesh representing a cylinder of radius r and height h, with the base at (0,0,0). 
	/// param[in] r Radius 
	/// param[in] h Height 
	/// param[in] fa Facet angle. A smaller angle produces more facets. Default value is 2pi / 360.  
	static TriangleMesh make_cylinder(double r, double h, double fa = (2 * PI / 360));

	/// Generate a mesh representing a sphere of radius rho, centered about (0,0,0). 
	/// param[in] rho Distance from center to the shell of the sphere. 
	/// param[in] fa Facet angle. A smaller angle produces more facets. Default value is 2pi / 360.  
	static TriangleMesh make_sphere(double rho, double fa = (2 * PI / 360));

	stl_file stl;

	/// Whether or not this mesh has been repaired.
	bool repaired;

private:
	friend class TriangleMeshSlicer<X>;
	friend class TriangleMeshSlicer<Y>;
	friend class TriangleMeshSlicer<Z>;
};

enum FacetEdgeType { feNone, feTop, feBottom, feHorizontal };

class IntersectionPoint : public Point
{
    public:
    int point_id;
    int edge_id;
    IntersectionPoint() : point_id(-1), edge_id(-1) {};
};

class IntersectionLine : public Line
{
    public:
    int             a_id;
    int             b_id;
    int             edge_a_id;
    int             edge_b_id;
    FacetEdgeType   edge_type;
    bool            skip;
    IntersectionLine() : a_id(-1), b_id(-1), edge_a_id(-1), edge_b_id(-1), edge_type(feNone), skip(false) {};
};
typedef std::vector<IntersectionLine> IntersectionLines;
typedef std::vector<IntersectionLine*> IntersectionLinePtrs;


/// \brief Class for processing TriangleMesh objects. 
template <Axis A>
class TriangleMeshSlicer
{
    public:
    TriangleMesh* mesh;
    TriangleMeshSlicer(TriangleMesh* _mesh);
    ~TriangleMeshSlicer();
    void slice(const std::vector<float> &z, std::vector<Polygons>* layers, size_t threads) const;
    void slice(const std::vector<float> &z, std::vector<ExPolygons>* layers,size_t threads) const;
    void slice(float z, ExPolygons* slices, size_t threads) const;
    void slice_facet(float slice_z, const stl_facet &facet, const int &facet_idx,
        const float &min_z, const float &max_z, std::vector<IntersectionLine>* lines,
        boost::mutex* lines_mutex = NULL) const;
    
	/// \brief Splits the current mesh into two parts.
	/// \param[in] z Coordinate plane to cut along.
	/// \param[out] upper TriangleMesh object to add the mesh > z. NULL suppresses saving this.
	/// \param[out] lower TriangleMesh object to save the mesh < z. NULL suppresses saving this.
    void cut(float z, TriangleMesh* upper, TriangleMesh* lower) const;
    
    private:
    typedef std::vector< std::vector<int> > t_facets_edges;
    t_facets_edges facets_edges;
    stl_vertex* v_scaled_shared;
    void _slice_do(size_t facet_idx, std::vector<IntersectionLines>* lines, boost::mutex* lines_mutex, const std::vector<float> &z) const;
    void _make_loops_do(size_t i, std::vector<IntersectionLines>* lines, std::vector<Polygons>* layers) const;
    void make_loops(std::vector<IntersectionLine> &lines, Polygons* loops) const;
    void make_expolygons(const Polygons &loops, ExPolygons* slices) const;
    void make_expolygons_simple(std::vector<IntersectionLine> &lines, ExPolygons* slices) const;
    void make_expolygons(std::vector<IntersectionLine> &lines, ExPolygons* slices) const;
    
    float& _x(stl_vertex &vertex) const;
    float& _y(stl_vertex &vertex) const;
    float& _z(stl_vertex &vertex) const;
    const float& _x(stl_vertex const &vertex) const;
    const float& _y(stl_vertex const &vertex) const;
    const float& _z(stl_vertex const &vertex) const;
};

template<> inline float& TriangleMeshSlicer<X>::_x(stl_vertex &vertex) const { return vertex.y; }
template<> inline float& TriangleMeshSlicer<X>::_y(stl_vertex &vertex) const { return vertex.z; }
template<> inline float& TriangleMeshSlicer<X>::_z(stl_vertex &vertex) const { return vertex.x; }
template<> inline float const& TriangleMeshSlicer<X>::_x(stl_vertex const &vertex) const { return vertex.y; }
template<> inline float const& TriangleMeshSlicer<X>::_y(stl_vertex const &vertex) const { return vertex.z; }
template<> inline float const& TriangleMeshSlicer<X>::_z(stl_vertex const &vertex) const { return vertex.x; }

template<> inline float& TriangleMeshSlicer<Y>::_x(stl_vertex &vertex) const { return vertex.z; }
template<> inline float& TriangleMeshSlicer<Y>::_y(stl_vertex &vertex) const { return vertex.x; }
template<> inline float& TriangleMeshSlicer<Y>::_z(stl_vertex &vertex) const { return vertex.y; }
template<> inline float const& TriangleMeshSlicer<Y>::_x(stl_vertex const &vertex) const { return vertex.z; }
template<> inline float const& TriangleMeshSlicer<Y>::_y(stl_vertex const &vertex) const { return vertex.x; }
template<> inline float const& TriangleMeshSlicer<Y>::_z(stl_vertex const &vertex) const { return vertex.y; }

template<> inline float& TriangleMeshSlicer<Z>::_x(stl_vertex &vertex) const { return vertex.x; }
template<> inline float& TriangleMeshSlicer<Z>::_y(stl_vertex &vertex) const { return vertex.y; }
template<> inline float& TriangleMeshSlicer<Z>::_z(stl_vertex &vertex) const { return vertex.z; }
template<> inline float const& TriangleMeshSlicer<Z>::_x(stl_vertex const &vertex) const { return vertex.x; }
template<> inline float const& TriangleMeshSlicer<Z>::_y(stl_vertex const &vertex) const { return vertex.y; }
template<> inline float const& TriangleMeshSlicer<Z>::_z(stl_vertex const &vertex) const { return vertex.z; }

//***********************************************************************
inline void stl_add_face(double x1, double y1, double z1
	, double x2, double y2, double z2
	, double x3, double y3, double z3, TriangleMesh& mesh) {
	stl_facet f;
	f.vertex[0].x = x1; f.vertex[0].y = y1; f.vertex[0].z = z1;
	f.vertex[1].x = x2; f.vertex[1].y = y2; f.vertex[1].z = z2;
	f.vertex[2].x = x3; f.vertex[2].y = y3; f.vertex[2].z = z3;
	stl_add_facet(&mesh.stl, &f);
}

}

