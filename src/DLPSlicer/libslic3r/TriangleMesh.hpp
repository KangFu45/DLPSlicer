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

	//ģ�ͱ任����
	void transform_matrix(QMatrix4x4 matrix);

    void scale(float factor);
    void scale(const Pointf3 &versor);
    void translate(float x, float y, float z);
    void rotate(float angle, const Axis &axis);
	//�����뻡��ֵ
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
	//���ڣ�2017
	//���ܣ��õ������͵�����Χ����������ݹ�ϵ��
	//********************************************
	void require_shared_vertices_faces();

	//*********************
	//���ڣ�2017
	//���ܣ���������档
	//����1��������ĽǶȡ�
	//***********************
	void extract_feature_face(int angle);

	/*----------------------------------------------------------��������֧��---------------------------------------------------------------*/
	//**************************************
	//���ڣ�2018.4.2
	//���ܣ����ɴ�֧�ŵĵ㡣
	//����1�������������������֧�ŵ�ļ�ࡣ
	//���أ���֧�ŵĵ㡣
	//**************************************
	std::vector<stl_vertex> feature_point(QProgressBar* progress);

	std::vector<stl_vertex> feature_point_face(float space, QProgressBar* progress);

	//*********************
	//���ڣ�2017
	//���ܣ��õ����ҵ㡣
	//���أ������������ֵ��
	//**********************
	std::vector<int> cliff_point();

	//***************************
	//���ڣ�2017
	//���ܣ�Ѱ��͹���㡣
	//����1:һ�������������ֵ��
	//���أ����������͹���߶ȡ�
	//**************************
	double find_convex_hull(int point);

	//*************************
	//���ڣ�2017
	//���ܣ����������档
	//���أ�����������ļ��ϡ�
	//*************************
	std::vector<v_face_struct*> generate_feature_faces_45();

	std::vector<v_face_struct*> generate_feature_faces_90();

	//����1����ĽǶȱ�ǩ
	std::vector<v_face_struct*> gather_feature_faces(char extra);

	//****************************************
	//���ڣ�2017
	//���ܣ����������������㡣
	//����1�����������򼯺ϡ�
	//����2�������������������֧�ŵ�ļ�ࡣ
	//���أ��������ϴ�֧�ŵĵ㡣
	//***************************************
	void feature_face_to_point(std::vector<v_face_struct*>& feature_faces_45, float space, std::vector<stl_vertex>& feature_faces_point,char extra);

	void look_for_polygon(std::vector<stl_edge>& edges, std::list<stl_vertex>& vertexs);

	//****************************
	//���ڣ�2017
	//���ܣ������淢ɢ��
	//����1:�����������ֵ��
	//����2������ͬһ����������档
	//*****************************
	void seed_face(int f, std::vector<int>* faces, char extra);

	//********************
	//���ڣ�2017
	//���ܣ����ɹǼ��ߡ�
	//����1����֧�ŵĵ㡣
	//���أ�֧���ߡ�
	//*******************
	std::vector<Linef3> skeleton_line(std::vector<stl_vertex> feature_faces_point);

	//************************
	//���ڣ�2017
	//���ܣ�ɾ���ظ��㡣
	//����1��������ĵ㼯�ϡ�
	//���أ�ɾ���ظ���ĵ㼯�ϡ�
	//**************************
	std::vector<int> delete_repetition(std::vector<int> ps);

	//************************************
	//���ڣ�2017
	//���ܣ�ɾ������Ĳ���(����ȥ����֦)
	//����1������
	//����2����֦
	//************************************
	std::vector<int> delete_twig(std::vector<int> &bole, std::vector<int> &twig);

	//************************************
	//���ڣ�2017
	//���ܣ�������abΪ����һ��������Ƭ��
	//����1��һ�������棨���������ıߣ�
	//����2�����ϵ�һ��a
	//����3�����ϵ�һ��b
	//���أ����ߵ�������
	//************************************
	int find_common_edge_face(int face, int a, int b);

	//****************************
	//���ڣ�2017
	//���ܣ������浽��άͶӰ��
	//����1������������
	//����2-5����ά�ľ���ͶӰ����
	//*****************************
	void face_to_2D_projection(v_face_struct* faces, float &minx, float &miny, float &maxx, float &maxy);

	//****************************
	//���ڣ�2017
	//���ܣ���ȡ���Ȳ����㡣
	//����1���������ϵ�֧�ŵ�
	//����2�������������ϵ�ļ��
	//����3-6����ά�ľ���ͶӰ����
	//****************************
	void balance_sampling_point(std::vector<stl_vertex>& points, float space, float minx, float miny, float maxx, float maxy);

	//******************************
	//���ڣ�2018.4.4
	//���ܣ�����Z��������ģ���󽻡�
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
	//���ڣ�2017
	//���ܣ��洢�㻷��������ݡ�
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
