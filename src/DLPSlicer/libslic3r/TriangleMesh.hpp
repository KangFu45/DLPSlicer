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

//xy平面投影面是否包含点
inline bool PointInTriangle2(stl_facet face, Pointf3 P)
{
	face.vertex[0].z = 0;
	face.vertex[1].z = 0;
	face.vertex[2].z = 0;
	P.z = 0;
	return PointInTriangle3(face, P);
}

//功能：求出单位圆的点
inline Pointfs CirclePoints(int angle)
{
	Pointfs circle;
	float factor = 0.5;
	for (int i = angle; i < 376; i += angle * 2)
		circle.emplace_back(Pointf(sin(PI / 180 * i) * factor, cos(PI / 180 * i) * factor));
	return circle;
}

//功能：求法向量与xy平面的夹角
inline double aormal_xy_angle(const stl_normal& a) { return -a.z / sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }

//范围是-180°到180°的弧度值,返回弧度值
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
/// 求一条直线与平面的交点
/// </summary>
/// <param name="planeVector">平面的法线向量，长度为3</param>
/// <param name="planePoint">平面经过的一点坐标，长度为3</param>
/// <param name="lineVector">直线的方向向量，长度为3</param>
/// <param name="linePoint">直线经过的一点坐标，长度为3</param>
/// <returns>返回交点坐标，长度为3</returns>
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
	//首先判断直线是否与平面平行
	if (vpt == 0)
	{
		res.x = -1;
		res.y = -1;
		res.z = -1;
	}
	else
	{
		//直线的参数方程与平面的点法式方程联立
		double t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
		res.x = m1 + v1 * t;
		res.y = m2 + v2 * t;
		res.z = m3 + v3 * t;
	}
	return res;
}
//求两点之间的距离
inline float DisPoint2(const Pointf& a, const Pointf& b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)); }
//3维的两点的距离
inline float DisPoint3(const Pointf3& a, const Pointf3& b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z)); }

//求三角面片的面积
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

//xy平面上的法向量在z方向旋转一定角度。
inline void xy_normal_rotate_z(float* norm, float angle)
{
	norm[2] = cosf(angle) * sqrtf(norm[0] * norm[0] + norm[1] * norm[1]);
	norm[1] = cosf(angle) * norm[1];
	norm[0] = cosf(angle) * norm[0];
	//stl_normalize_vector(norm);
}

//判断线段是否穿过三角面片
inline bool isLineCrossTriangle(const stl_facet& f, const Linef3& line) {
	//得到射线方向
	float norm[3] = { line.a.x - line.b.x , line.a.y - line.b.y,line.a.z - line.b.z };
	stl_normalize_vector(norm);

	//得到射线与平面的交点
	Pointf3 inter = CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0])
		, Vectorf3(norm[0], norm[1], norm[2]), line.a);

	//PointInTriangle3()无法判断？？？应用范围有限？？？
	if (inter.x == -1 && inter.y == -1 && inter.z == -1)
		return false;

	//判断交点是否在三角面上
	if (PointInTriangle3(f, inter)) {
		//是否穿过
		if ((inter.z <= line.b.z && inter.z > line.a.z)
			|| (inter.z <= line.a.z && inter.z > line.b.z))
			return true;
	}
	return false;
}

//功能：射线与三角面片求交点。
//参数1：三角面片
//参数2：射线方向
//参数3：射线起点
//参数4：有交点，返回交点
//返回：是否有交点
inline bool Ray2TrianglePoint(const stl_facet& f, const Vectorf3& norm, const Pointf3& origin, Pointf3& inter) {
	//得到射线与平面的交点
	inter = CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0]), norm, origin);

	//PointInTriangle3()无法判断？？？应用范围有限？？？
	if (inter.x == -1 && inter.y == -1 && inter.z == -1)
		return false;

	//判断交点是否在三角面上
	if (PointInTriangle3(f, inter))
		return true;

	return false;
}

//两点之间插入点
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

	//模型变换矩阵
	void transform_matrix(QMatrix4x4 matrix);

	void scale(float factor);
	void scale(const Pointf3& versor);
	void translate(float x, float y, float z);
	void rotate(float angle, const Axis& axis);
	//均输入弧度值
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
	void require_shared_vertices();//add得到共享点和点与周围环绕面的数据关系。
	void reverse_normals();

	//功能：标记特征面。
	void extract_feature_face(int angle);

	//---------------------生成特征支撑-------------------
	//功能：生成待支撑的点。
	//参数1：大倾角面与悬吊面上支撑点的间距。
	//返回：待支撑的点。
	stl_vertexs feature_point(QProgressBar* progress);
	stl_vertexs feature_point_face(float space, QProgressBar* progress);

	//功能：得到悬挂点。
	//返回：悬吊点的索引值。
	int* cliff_point(unsigned short& num);

	//功能：寻找凸包点。
	//参数1:一个悬吊点的索引值。
	//返回：该悬吊点的凸包高度。
	double find_convex_hull(int point);

	//功能：生成特征面。
	//返回：特征面区域的集合。
	std::vector<v_face_struct> generate_feature_faces(char extra,float min_area);
	//参数1：面的角度标签
	std::vector<v_face_struct> gather_feature_faces(char extra);

	//功能：特征面生成特征点。
	//参数1：特征面区域集合。
	//参数2：大倾角面与悬吊面上支撑点的间距。
	//返回：特征面上待支撑的点。
	void feature_face_to_point(std::vector<v_face_struct>& feature_faces, float space, stl_vertexs& feature_faces_point, char extra);

	//功能：种子面发散。
	//参数1:种子面的索引值。
	//参数2：返回同一区域的特征面。
	void seed_face(int f, std::vector<int>& faces, char extra);

	//功能：查找以ab为边另一个三角面片。
	//参数1：一个三角面（这个三角面的边）
	//参数2：边上的一点a
	//参数3：边上的一点b
	//返回：共边的三角面
	int find_common_edge_face(int face, int a, int b);

	//功能：特征面到二维投影。
	//参数1：特征面区域
	//参数2-5：二维的矩形投影区域
	BoundingBoxf face_to_2D_projection(const v_face_struct& faces);

	//功能：点沿Z负方向与模型求交。
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

