#pragma once
#ifndef slic3r_TriangleMesh_hpp_
#define slic3r_TriangleMesh_hpp_

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

namespace Slic3r {

class TriangleMesh;
template <Axis A> class TriangleMeshSlicer;
typedef std::vector<TriangleMesh> TriangleMeshs;
typedef std::vector<TriangleMesh*> TriangleMeshPtrs;

class TriangleMesh
{
    public:
    TriangleMesh();
    TriangleMesh(const Pointf3s &points, const std::vector<Point3> &facets);
    TriangleMesh(const TriangleMesh &other);
    TriangleMesh& operator= (TriangleMesh other);
    void swap(TriangleMesh &other);
    ~TriangleMesh();
    void ReadSTLFile(const std::string &input_file);
    void write_ascii(const std::string &output_file);
    void write_binary(const std::string &output_file);
    void repair();
    void check_topology();
    float volume();
    bool is_manifold() const;
    void WriteOBJFile(const std::string &output_file);

	//模型变换矩阵
	void transform_matrix(QMatrix4x4 matrix);

    void scale(float factor);
    void scale(const Pointf3 &versor);
    void translate(float x, float y, float z);
    void rotate(float angle, const Axis &axis);
	//均输入弧度值
    void rotate_x(float angle);
    void rotate_y(float angle);
    void rotate_z(float angle);
    void mirror(const Axis &axis);
    void mirror_x();
    void mirror_y();
    void mirror_z();
    void align_to_origin();
    void center_around_origin();
    void rotate(double angle, Point* center);
    TriangleMeshPtrs split() const;
    TriangleMeshPtrs cut_by_grid(const Pointf &grid) const;
    void merge(const TriangleMesh &mesh);
    ExPolygons horizontal_projection() const;
    Polygon convex_hull();
    BoundingBoxf3 bounding_box() const;
    void reset_repair_stats();
    bool needed_repair() const;
    size_t facets_count() const;
    void extrude_tin(float offset);
	void require_shared_vertices();
    void reverse_normals();

	//*******************************************
	//日期：2017
	//功能：得到共享点和点与周围环绕面的数据关系。
	//********************************************
	void require_shared_vertices_faces();

	//*********************
	//日期：2017
	//功能：标记特征面。
	//参数1：特征面的角度。
	//***********************
	void extract_feature_face(int angle);

	/*----------------------------------------------------------生成特征支撑---------------------------------------------------------------*/
	//**************************************
	//日期：2018.4.2
	//功能：生成待支撑的点。
	//参数1：大倾角面与悬吊面上支撑点的间距。
	//返回：待支撑的点。
	//**************************************
	std::vector<stl_vertex> feature_point(QProgressBar* progress);

	std::vector<stl_vertex> feature_point_face(float space, QProgressBar* progress);

	//*********************
	//日期：2017
	//功能：得到悬挂点。
	//返回：悬吊点的索引值。
	//**********************
	std::vector<int> cliff_point();

	//***************************
	//日期：2017
	//功能：寻找凸包点。
	//参数1:一个悬吊点的索引值。
	//返回：该悬吊点的凸包高度。
	//**************************
	double find_convex_hull(int point);

	//*************************
	//日期：2017
	//功能：生成特征面。
	//返回：特征面区域的集合。
	//*************************
	std::vector<v_face_struct*> generate_feature_faces_45();

	std::vector<v_face_struct*> generate_feature_faces_90();

	//参数1：面的角度标签
	std::vector<v_face_struct*> gather_feature_faces(char extra);

	//****************************************
	//日期：2017
	//功能：特征面生成特征点。
	//参数1：特征面区域集合。
	//参数2：大倾角面与悬吊面上支撑点的间距。
	//返回：特征面上待支撑的点。
	//***************************************
	void feature_face_to_point(std::vector<v_face_struct*>& feature_faces_45, float space, std::vector<stl_vertex>& feature_faces_point,char extra);

	void look_for_polygon(std::vector<stl_edge>& edges, std::list<stl_vertex>& vertexs);

	//****************************
	//日期：2017
	//功能：种子面发散。
	//参数1:种子面的索引值。
	//参数2：返回同一区域的特征面。
	//*****************************
	void seed_face(int f, std::vector<int>* faces, char extra);

	//********************
	//日期：2017
	//功能：生成骨架线。
	//参数1：待支撑的点。
	//返回：支撑线。
	//*******************
	std::vector<Linef3> skeleton_line(std::vector<stl_vertex> feature_faces_point);

	//************************
	//日期：2017
	//功能：删除重复点。
	//参数1：待处理的点集合。
	//返回：删除重复点的点集合。
	//**************************
	std::vector<int> delete_repetition(std::vector<int> ps);

	//************************************
	//日期：2017
	//功能：删除多余的部分(树干去掉树枝)
	//参数1：树干
	//参数2：树枝
	//************************************
	std::vector<int> delete_twig(std::vector<int> &bole, std::vector<int> &twig);

	//************************************
	//日期：2017
	//功能：查找以ab为边另一个三角面片。
	//参数1：一个三角面（这个三角面的边）
	//参数2：边上的一点a
	//参数3：边上的一点b
	//返回：共边的三角面
	//************************************
	int find_common_edge_face(int face, int a, int b);

	//****************************
	//日期：2017
	//功能：特征面到二维投影。
	//参数1：特征面区域
	//参数2-5：二维的矩形投影区域
	//*****************************
	void face_to_2D_projection(v_face_struct* faces, float &minx, float &miny, float &maxx, float &maxy);

	//****************************
	//日期：2017
	//功能：提取均匀采样点。
	//参数1：特征面上的支撑点
	//参数2：特征面区域上点的间距
	//参数3-6：二维的矩形投影区域
	//****************************
	void balance_sampling_point(std::vector<stl_vertex>& points, float space, float minx, float miny, float maxx, float maxy);

	//******************************
	//日期：2018.4.4
	//功能：点沿Z负方向与模型求交。
	//******************************
	Pointf3 point_model_intersection_Z(Pointf3 p);
	/*---------------------------------------------------------------------------------------------------------------------------------------*/
	
	/// Generate a mesh representing a cube with dimensions (x, y, z), with one corner at (0,0,0).
    static TriangleMesh make_cube(double x, double y, double z);
	
	/// Generate a mesh representing a cylinder of radius r and height h, with the base at (0,0,0). 
	/// param[in] r Radius 
	/// param[in] h Height 
	/// param[in] fa Facet angle. A smaller angle produces more facets. Default value is 2pi / 360.  
    static TriangleMesh make_cylinder(double r, double h, double fa=(2*PI/360));
	
	/// Generate a mesh representing a sphere of radius rho, centered about (0,0,0). 
	/// param[in] rho Distance from center to the shell of the sphere. 
	/// param[in] fa Facet angle. A smaller angle produces more facets. Default value is 2pi / 360.  
    static TriangleMesh make_sphere(double rho, double fa=(2*PI/360));
    
    stl_file stl;

	//**************************
	//日期：2017
	//功能：存储点环绕面的数据。
	//***************************
	std::vector<v_face_struct> v_shared_faces;

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

}

#endif
