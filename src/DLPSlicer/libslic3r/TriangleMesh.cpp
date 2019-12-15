#pragma once
#include "TriangleMesh.hpp"
#include "ClipperUtils.hpp"
#include "Geometry.hpp"
#include <cmath>
#include <deque>
#include <queue>
#include <set>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/nowide/convert.hpp>
#include "tool.h"
#include "qdebug.h"

#ifdef SLIC3R_DEBUG
#include "SVG.hpp"
#endif

namespace Slic3r {

TriangleMesh::TriangleMesh()
    : repaired(false)
{
    stl_initialize(&this->stl);
}

TriangleMesh::TriangleMesh(const Pointf3s &points, const std::vector<Point3>& facets )
    : repaired(false)
{
    stl_initialize(&this->stl);
    stl_file &stl = this->stl;
    stl.error = 0;
    stl.stats.type = inmemory;

    // count facets and allocate memory
    stl.stats.number_of_facets = facets.size();
    stl.stats.original_num_facets = stl.stats.number_of_facets;
    stl_allocate(&stl);

	qDebug() << "number_of_facets: " << stl.stats.number_of_facets;
    for (int i = 0; i < stl.stats.number_of_facets; i++) {
        stl_facet facet;
        facet.normal.x = 0;
        facet.normal.y = 0;
        facet.normal.z = 0;

        const Pointf3& ref_f1 = points[facets[i].x];
        facet.vertex[0].x = ref_f1.x;
        facet.vertex[0].y = ref_f1.y;
        facet.vertex[0].z = ref_f1.z;

        const Pointf3& ref_f2 = points[facets[i].y];
        facet.vertex[1].x = ref_f2.x;
        facet.vertex[1].y = ref_f2.y;
        facet.vertex[1].z = ref_f2.z;

        const Pointf3& ref_f3 = points[facets[i].z];
        facet.vertex[2].x = ref_f3.x;
        facet.vertex[2].y = ref_f3.y;
        facet.vertex[2].z = ref_f3.z;
        
        facet.extra[0] = 0;
        facet.extra[1] = 0;

        stl.facet_start[i] = facet;
    }
    stl_get_size(&stl);
}

TriangleMesh::TriangleMesh(const TriangleMesh &other)
    : stl(other.stl), repaired(other.repaired)
{
    this->stl.heads = NULL;
    this->stl.tail  = NULL;
    this->stl.error = other.stl.error;
    if (other.stl.facet_start != NULL) {
        this->stl.facet_start = (stl_facet*)calloc(other.stl.stats.number_of_facets, sizeof(stl_facet));
        std::copy(other.stl.facet_start, other.stl.facet_start + other.stl.stats.number_of_facets, this->stl.facet_start);
    }
    if (other.stl.neighbors_start != NULL) {
        this->stl.neighbors_start = (stl_neighbors*)calloc(other.stl.stats.number_of_facets, sizeof(stl_neighbors));
        std::copy(other.stl.neighbors_start, other.stl.neighbors_start + other.stl.stats.number_of_facets, this->stl.neighbors_start);
    }
    if (other.stl.v_indices != NULL) {
        this->stl.v_indices = (v_indices_struct*)calloc(other.stl.stats.number_of_facets, sizeof(v_indices_struct));
        std::copy(other.stl.v_indices, other.stl.v_indices + other.stl.stats.number_of_facets, this->stl.v_indices);
    }
    if (other.stl.v_shared != NULL) {
        this->stl.v_shared = (stl_vertex*)calloc(other.stl.stats.shared_vertices, sizeof(stl_vertex));
        std::copy(other.stl.v_shared, other.stl.v_shared + other.stl.stats.shared_vertices, this->stl.v_shared);
    }
}

TriangleMesh& TriangleMesh::operator= (TriangleMesh other)
{
    this->swap(other);
    return *this;
}

void
TriangleMesh::swap(TriangleMesh &other)
{
    std::swap(this->stl,      other.stl);
    std::swap(this->repaired, other.repaired);
}

TriangleMesh::~TriangleMesh() {
    stl_close(&this->stl);

	if (!this->v_shared_faces.empty())
	{
		auto v = this->v_shared_faces.begin();
		delete (*v).v_shared_face;
		this->v_shared_faces.erase(v);
	}
}

void
TriangleMesh::ReadSTLFile(const std::string &input_file) {
    #ifdef BOOST_WINDOWS
    stl_open(&stl, boost::nowide::widen(input_file).c_str());
    #else
    stl_open(&stl, input_file.c_str());
    #endif
    if (this->stl.error != 0) throw std::runtime_error("Failed to read STL file");
}

void
TriangleMesh::write_ascii(const std::string &output_file)
{
    #ifdef BOOST_WINDOWS
    stl_write_ascii(&this->stl, boost::nowide::widen(output_file).c_str(), "");
    #else
    stl_write_ascii(&this->stl, output_file.c_str(), "");
    #endif
}

void
TriangleMesh::write_binary(const std::string &output_file)
{
    #ifdef BOOST_WINDOWS
    stl_write_binary(&this->stl, boost::nowide::widen(output_file).c_str(), "");
    #else
    stl_write_binary(&this->stl, output_file.c_str(), "");
    #endif
}

void
TriangleMesh::repair() {
    if (this->repaired) return;
    
    // admesh fails when repairing empty meshes
    if (this->stl.stats.number_of_facets == 0) return;
    
    this->check_topology();
    
    // remove_unconnected
    if (stl.stats.connected_facets_3_edge <  stl.stats.number_of_facets) {
        stl_remove_unconnected_facets(&stl);
    }
    
    // fill_holes
    if (stl.stats.connected_facets_3_edge < stl.stats.number_of_facets) {
        stl_fill_holes(&stl);
        stl_clear_error(&stl);
    }
    
    // normal_directions
    stl_fix_normal_directions(&stl);
    
    // normal_values
    stl_fix_normal_values(&stl);
    
    // always calculate the volume and reverse all normals if volume is negative
    (void)this->volume();

    // neighbors
    stl_verify_neighbors(&stl);
    
    this->repaired = true;
}

float
TriangleMesh::volume()
{
    if (this->stl.stats.volume == -1) stl_calculate_volume(&this->stl);
    return this->stl.stats.volume;
}

void
TriangleMesh::check_topology()
{
    // checking exact
    stl_check_facets_exact(&stl);
    stl.stats.facets_w_1_bad_edge = (stl.stats.connected_facets_2_edge - stl.stats.connected_facets_3_edge);
    stl.stats.facets_w_2_bad_edge = (stl.stats.connected_facets_1_edge - stl.stats.connected_facets_2_edge);
    stl.stats.facets_w_3_bad_edge = (stl.stats.number_of_facets - stl.stats.connected_facets_1_edge);
    
    // checking nearby
    //int last_edges_fixed = 0;
    float tolerance = stl.stats.shortest_edge;//公差
    float increment = stl.stats.bounding_diameter / 10000.0;//增量
    int iterations = 2;//迭代
    if (stl.stats.connected_facets_3_edge < stl.stats.number_of_facets) {
        for (int i = 0; i < iterations; i++) {
            if (stl.stats.connected_facets_3_edge < stl.stats.number_of_facets) {
                //printf("Checking nearby. Tolerance= %f Iteration=%d of %d...", tolerance, i + 1, iterations);
                stl_check_facets_nearby(&stl, tolerance);
                //printf("  Fixed %d edges.\n", stl.stats.edges_fixed - last_edges_fixed);
                //last_edges_fixed = stl.stats.edges_fixed;
                tolerance += increment;
            } else {
                break;
            }
        }
    }
}

bool
TriangleMesh::is_manifold() const
{
    return this->stl.stats.connected_facets_3_edge == this->stl.stats.number_of_facets;
}

void
TriangleMesh::reset_repair_stats() {
    this->stl.stats.degenerate_facets   = 0;
    this->stl.stats.edges_fixed         = 0;
    this->stl.stats.facets_removed      = 0;
    this->stl.stats.facets_added        = 0;
    this->stl.stats.facets_reversed     = 0;
    this->stl.stats.backwards_edges     = 0;
    this->stl.stats.normals_fixed       = 0;
}

bool
TriangleMesh::needed_repair() const
{
    return this->stl.stats.degenerate_facets    > 0
        || this->stl.stats.edges_fixed          > 0
        || this->stl.stats.facets_removed       > 0
        || this->stl.stats.facets_added         > 0
        || this->stl.stats.facets_reversed      > 0
        || this->stl.stats.backwards_edges      > 0;
}

size_t
TriangleMesh::facets_count() const
{
    return this->stl.stats.number_of_facets;
}

void
TriangleMesh::WriteOBJFile(const std::string &output_file) {
    stl_generate_shared_vertices(&stl);
    
    #ifdef BOOST_WINDOWS
    stl_write_obj(&stl, boost::nowide::widen(output_file).c_str());
    #else
    stl_write_obj(&stl, output_file.c_str());
    #endif
}

void TriangleMesh::transform_matrix(QMatrix4x4 matrix)
{
	int i, j;
	stl_facet* f;
	QVector4D v;

	if (this->stl.error)
		return;

	for (i = 0; i < this->stl.stats.number_of_facets; ++i) {
		f = &this->stl.facet_start[i];
		for (j = 0; j < 3; ++j) {
			v.setX(f->vertex[j].x);
			v.setY(f->vertex[j].y);
			v.setZ(f->vertex[j].z);
			v.setW(1);

			v = matrix*v;

			f->vertex[j].x = v.x(); 
			f->vertex[j].y = v.y();
			f->vertex[j].z = v.z();
		}
	}

	stl_get_size(&this->stl);
	calculate_normals(&this->stl);
}

void TriangleMesh::scale(float factor)
{
    stl_scale(&(this->stl), factor);
    stl_invalidate_shared_vertices(&this->stl);
}

void TriangleMesh::scale(const Pointf3 &versor)
{
    float fversor[3];
    fversor[0] = versor.x;
    fversor[1] = versor.y;
    fversor[2] = versor.z;
    stl_scale_versor(&this->stl, fversor);
    stl_invalidate_shared_vertices(&this->stl);
}

void TriangleMesh::translate(float x, float y, float z)
{
    stl_translate_relative(&(this->stl), x, y, z);
    stl_invalidate_shared_vertices(&this->stl);
}

void TriangleMesh::rotate(float angle, const Axis &axis)
{
    // admesh uses degrees
    angle = Slic3r::Geometry::rad2deg(angle);
    
    if (axis == X) {
        stl_rotate_x(&(this->stl), angle);
    } else if (axis == Y) {
        stl_rotate_y(&(this->stl), angle);
    } else if (axis == Z) {
        stl_rotate_z(&(this->stl), angle);
    }
    stl_invalidate_shared_vertices(&this->stl);
}

void TriangleMesh::rotate_x(float angle)
{
    this->rotate(angle, X);
}

void TriangleMesh::rotate_y(float angle)
{
    this->rotate(angle, Y);
}

void TriangleMesh::rotate_z(float angle)
{
    this->rotate(angle, Z);
}

void TriangleMesh::mirror(const Axis &axis)
{
    if (axis == X) {
        stl_mirror_yz(&this->stl);
    } else if (axis == Y) {
        stl_mirror_xz(&this->stl);
    } else if (axis == Z) {
        stl_mirror_xy(&this->stl);
    }
    stl_invalidate_shared_vertices(&this->stl);
}

void TriangleMesh::mirror_x()
{
    this->mirror(X);
}

void TriangleMesh::mirror_y()
{
    this->mirror(Y);
}

void TriangleMesh::mirror_z()
{
    this->mirror(Z);
}

void TriangleMesh::align_to_origin()
{
    this->translate(
        -(this->stl.stats.min.x),
        -(this->stl.stats.min.y),
        -(this->stl.stats.min.z)
    );
}

void TriangleMesh::center_around_origin()
{
    this->align_to_origin();
    this->translate(
        -(this->stl.stats.size.x/2),
        -(this->stl.stats.size.y/2),
        -(this->stl.stats.size.z/2)
    );
}

void TriangleMesh::rotate(double angle, Point* center)
{
    this->translate(-center->x, -center->y, 0);
    stl_rotate_z(&(this->stl), (float)angle);
    this->translate(+center->x, +center->y, 0);
}

TriangleMeshPtrs
TriangleMesh::split() const
{
    TriangleMeshPtrs meshes;
    std::set<int> seen_facets;
    
    // we need neighbors
  //  if (!this->repaired) CONFESS("split() requires repair()");
    
    // loop while we have remaining facets
    while (1) {
        // get the first facet
        std::queue<int> facet_queue;
        std::deque<int> facets;
        for (int facet_idx = 0; facet_idx < this->stl.stats.number_of_facets; facet_idx++) {
            if (seen_facets.find(facet_idx) == seen_facets.end()) {
                // if facet was not seen put it into queue and start searching
                facet_queue.push(facet_idx);
                break;
            }
        }
        if (facet_queue.empty()) break;
        
        while (!facet_queue.empty()) {
            int facet_idx = facet_queue.front();
            facet_queue.pop();
            if (seen_facets.find(facet_idx) != seen_facets.end()) continue;
            facets.push_back(facet_idx);
            for (int j = 0; j <= 2; j++) {
                facet_queue.push(this->stl.neighbors_start[facet_idx].neighbor[j]);
            }
            seen_facets.insert(facet_idx);
        }
        
        TriangleMesh* mesh = new TriangleMesh;
        meshes.push_back(mesh);
        mesh->stl.stats.type = inmemory;
        mesh->stl.stats.number_of_facets = facets.size();
        mesh->stl.stats.original_num_facets = mesh->stl.stats.number_of_facets;
        stl_clear_error(&mesh->stl);
        stl_allocate(&mesh->stl);
        
        int first = 1;
        for (std::deque<int>::const_iterator facet = facets.begin(); facet != facets.end(); ++facet) {
            mesh->stl.facet_start[facet - facets.begin()] = this->stl.facet_start[*facet];
            stl_facet_stats(&mesh->stl, this->stl.facet_start[*facet], first);
            first = 0;
        }
    }
    
    return meshes;
}

TriangleMeshPtrs
TriangleMesh::cut_by_grid(const Pointf &grid) const
{
    TriangleMesh mesh = *this;
    const BoundingBoxf3 bb = mesh.bounding_box();            
    const Sizef3 size = bb.size();
    const size_t x_parts = ceil((size.x - EPSILON)/grid.x);
    const size_t y_parts = ceil((size.y - EPSILON)/grid.y);
    
    TriangleMeshPtrs meshes;
    for (size_t i = 1; i <= x_parts; ++i) {
        TriangleMesh curr;
        if (i == x_parts) {
            curr = mesh;
        } else {
            TriangleMesh next;
            TriangleMeshSlicer<X>(&mesh).cut(bb.min.x + (grid.x * i), &next, &curr);
            curr.repair();
            next.repair();
            mesh = next;
        }
        
        for (size_t j = 1; j <= y_parts; ++j) {
            TriangleMesh* tile;
            if (j == y_parts) {
                tile = new TriangleMesh(curr);
            } else {
                TriangleMesh next;
                tile = new TriangleMesh;
                TriangleMeshSlicer<Y>(&curr).cut(bb.min.y + (grid.y * j), &next, tile);
                tile->repair();
                next.repair();
                curr = next;
            }
            
            meshes.push_back(tile);
        }
    }
    return meshes;
}

void
TriangleMesh::merge(const TriangleMesh &mesh)
{
    // reset stats and metadata
    int number_of_facets = this->stl.stats.number_of_facets;
    stl_invalidate_shared_vertices(&this->stl);
    this->repaired = false;
    
    // update facet count and allocate more memory
    this->stl.stats.number_of_facets = number_of_facets + mesh.stl.stats.number_of_facets;
    this->stl.stats.original_num_facets = this->stl.stats.number_of_facets;
    stl_reallocate(&this->stl);
    
    // copy facets
    std::copy(mesh.stl.facet_start, mesh.stl.facet_start + mesh.stl.stats.number_of_facets, this->stl.facet_start + number_of_facets);
    std::copy(mesh.stl.neighbors_start, mesh.stl.neighbors_start + mesh.stl.stats.number_of_facets, this->stl.neighbors_start + number_of_facets);
    
    // update size
    stl_get_size(&this->stl);
}

/* this will return scaled ExPolygons */
ExPolygons
TriangleMesh::horizontal_projection() const
{
    Polygons pp;
    pp.reserve(this->stl.stats.number_of_facets);
    for (int i = 0; i < this->stl.stats.number_of_facets; i++) {
        stl_facet* facet = &this->stl.facet_start[i];
        Polygon p;
        p.points.resize(3);
        p.points[0] = Point(facet->vertex[0].x / SCALING_FACTOR, facet->vertex[0].y / SCALING_FACTOR);
        p.points[1] = Point(facet->vertex[1].x / SCALING_FACTOR, facet->vertex[1].y / SCALING_FACTOR);
        p.points[2] = Point(facet->vertex[2].x / SCALING_FACTOR, facet->vertex[2].y / SCALING_FACTOR);
        p.make_counter_clockwise();  // do this after scaling, as winding order might change while doing that
        pp.push_back(p);
    }
    
    // the offset factor was tuned using groovemount.stl
    return union_ex(offset(pp, 0.01 / SCALING_FACTOR), true);
}

Polygon
TriangleMesh::convex_hull()
{
    this->require_shared_vertices();
    Points pp;
    pp.reserve(this->stl.stats.shared_vertices);
    for (int i = 0; i < this->stl.stats.shared_vertices; i++) {
        stl_vertex* v = &this->stl.v_shared[i];
        pp.push_back(Point(v->x / SCALING_FACTOR, v->y / SCALING_FACTOR));
    }
    return Slic3r::Geometry::convex_hull(pp);
}

BoundingBoxf3
TriangleMesh::bounding_box() const
{
    BoundingBoxf3 bb;
    bb.min.x = this->stl.stats.min.x;
    bb.min.y = this->stl.stats.min.y;
    bb.min.z = this->stl.stats.min.z;
    bb.max.x = this->stl.stats.max.x;
    bb.max.y = this->stl.stats.max.y;
    bb.max.z = this->stl.stats.max.z;
    return bb;
}

void
TriangleMesh::require_shared_vertices()
{
    if (!this->repaired) this->repair();
    if (this->stl.v_shared == NULL) stl_generate_shared_vertices(&(this->stl));
}

//得到共享点和点与周围环绕面的数据关系
void TriangleMesh::require_shared_vertices_faces()
{
	if (!this->repaired) this->repair();
	if (this->stl.v_shared == NULL)
	{
		//分配内存
		v_face_struct* _v_shared_faces = new v_face_struct[this->stl.stats.number_of_facets];

		//重定义的，在原来生成共享面点关系的基础上增加了点面关系。
		stl_generate_shared_vertices_faces(&(this->stl), _v_shared_faces);

		if (!this->v_shared_faces.empty())
		{
			auto v = this->v_shared_faces.begin();
			delete (*v).v_shared_face;
			this->v_shared_faces.erase(v);
		}

		for (int i = 0; i < this->stl.stats.number_of_facets; ++i)
			this->v_shared_faces.push_back(_v_shared_faces[i]);

		delete[] _v_shared_faces;
	}
}

//标记特征面
//参数1：特征面的角度
void TriangleMesh::extract_feature_face(int angle)
{
	//归零
	for (int i = 0; i < this->stl.stats.number_of_facets; i++) {
		stl_facet* face = &this->stl.facet_start[i];
		face->extra[0] = 128;
		face->extra[1] = 128;
	}

	int aaa = 0;
	int bbb = 0;
	for (int i = 0; i < this->stl.stats.number_of_facets; i++) {
		stl_facet* face = &this->stl.facet_start[i];
		//得到法向量的角度值
		double a = 180 / PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(face->normal.x, face->normal.y, face->normal.z));
		//a==1,三角面片为0度，为悬吊面，将标记值设为90
		if (a == 0) {
			this->stl.facet_start[i].extra[0] = 90;
		}
		//标记角度值小于临界角度值的三角面片，将标记值设为45
		else if (a < angle&&a > 1) {
			this->stl.facet_start[i].extra[0] = 45;
		}
		//法向量向下且大于临界角度值得三角面片，标记为89
		else if (a > 0 && a <= 1)
			this->stl.facet_start[i].extra[0] = 89;

	}
}

void
TriangleMesh::reverse_normals()
{
    stl_reverse_all_facets(&this->stl);
    if (this->stl.stats.volume != -1) this->stl.stats.volume *= -1.0;
}

void
TriangleMesh::extrude_tin(float offset)
{
    calculate_normals(&this->stl);
    
    const int number_of_facets = this->stl.stats.number_of_facets;
    if (number_of_facets == 0)
        throw std::runtime_error("Error: file is empty");
    
    const float z = this->stl.stats.min.z - offset;
    
    for (int i = 0; i < number_of_facets; ++i) {
        const stl_facet &facet = this->stl.facet_start[i];
        
        if (facet.normal.z < 0)
            throw std::runtime_error("Invalid 2.5D mesh: at least one facet points downwards.");
        
        for (int j = 0; j < 3; ++j) {
            if (this->stl.neighbors_start[i].neighbor[j] == -1) {
                stl_facet new_facet;
                float normal[3];
                
                // first triangle
                new_facet.vertex[0] = new_facet.vertex[2] = facet.vertex[(j+1)%3];
                new_facet.vertex[1] = facet.vertex[j];
                new_facet.vertex[2].z = z;
                stl_calculate_normal(normal, &new_facet);
                stl_normalize_vector(normal);
                new_facet.normal.x = normal[0];
                new_facet.normal.y = normal[1];
                new_facet.normal.z = normal[2];
                stl_add_facet(&this->stl, &new_facet);
                
                // second triangle
                new_facet.vertex[0] = new_facet.vertex[1] = facet.vertex[j];
                new_facet.vertex[2] = facet.vertex[(j+1)%3];
                new_facet.vertex[1].z = new_facet.vertex[2].z = z;
                new_facet.normal.x = normal[0];
                new_facet.normal.y = normal[1];
                new_facet.normal.z = normal[2];
                stl_add_facet(&this->stl, &new_facet);
            }
        }
    }
    stl_get_size(&this->stl);
    
    this->repair();
}

// Generate the vertex list for a cube solid of arbitrary size in X/Y/Z.
TriangleMesh
TriangleMesh::make_cube(double x, double y, double z) {
    Pointf3 pv[8] = { 
        Pointf3(x, y, 0), Pointf3(x, 0, 0), Pointf3(0, 0, 0), 
        Pointf3(0, y, 0), Pointf3(x, y, z), Pointf3(0, y, z), 
        Pointf3(0, 0, z), Pointf3(x, 0, z) 
    };
    Point3 fv[12] = { 
        Point3(0, 1, 2), Point3(0, 2, 3), Point3(4, 5, 6), 
        Point3(4, 6, 7), Point3(0, 4, 7), Point3(0, 7, 1), 
        Point3(1, 7, 6), Point3(1, 6, 2), Point3(2, 6, 5), 
        Point3(2, 5, 3), Point3(4, 0, 3), Point3(4, 3, 5) 
    };

    std::vector<Point3> facets(&fv[0], &fv[0]+12);
    Pointf3s vertices(&pv[0], &pv[0]+8);

    TriangleMesh mesh(vertices ,facets);
    return mesh;
}

// Generate the mesh for a cylinder and return it, using 
// the generated angle to calculate the top mesh triangles.
// Default is 360 sides, angle fa is in radians.
TriangleMesh
TriangleMesh::make_cylinder(double r, double h, double fa) {
    Pointf3s vertices;
    std::vector<Point3> facets;

    // 2 special vertices, top and bottom center, rest are relative to this
    vertices.push_back(Pointf3(0.0, 0.0, 0.0));
    vertices.push_back(Pointf3(0.0, 0.0, h));

    // adjust via rounding to get an even multiple for any provided angle.
    double angle = (2*PI / floor(2*PI / fa));

    // for each line along the polygon approximating the top/bottom of the
    // circle, generate four points and four facets (2 for the wall, 2 for the
    // top and bottom.
    // Special case: Last line shares 2 vertices with the first line.
    unsigned id = vertices.size() - 1;
    vertices.push_back(Pointf3(sin(0) * r , cos(0) * r, 0));
    vertices.push_back(Pointf3(sin(0) * r , cos(0) * r, h));
    for (double i = 0; i < 2*PI; i+=angle) {
        Pointf3 b(0, r, 0);
        Pointf3 t(0, r, h);
        b.rotate(i, Pointf3(0,0,0)); 
        t.rotate(i, Pointf3(0,0,h));
        vertices.push_back(b);
        vertices.push_back(t);
        id = vertices.size() - 1;
        facets.push_back(Point3( 0, id - 1, id - 3)); // top
        facets.push_back(Point3(id,      1, id - 2)); // bottom
        facets.push_back(Point3(id, id - 2, id - 3)); // upper-right of side
        facets.push_back(Point3(id, id - 3, id - 1)); // bottom-left of side
    }
    // Connect the last set of vertices with the first.
    facets.push_back(Point3( 2, 0, id - 1));
    facets.push_back(Point3( 1, 3,     id));
    facets.push_back(Point3(id, 3,      2));
    facets.push_back(Point3(id, 2, id - 1));
    
    TriangleMesh mesh(vertices, facets);
    return mesh;
}

// Generates mesh for a sphere centered about the origin, using the generated angle
// to determine the granularity. 
// Default angle is 1 degree.
TriangleMesh
TriangleMesh::make_sphere(double rho, double fa) {
    Pointf3s vertices;
    std::vector<Point3> facets;

    // Algorithm: 
    // Add points one-by-one to the sphere grid and form facets using relative coordinates.
    // Sphere is composed effectively of a mesh of stacked circles.

    // adjust via rounding to get an even multiple for any provided angle.
    double angle = (2*PI / floor(2*PI / fa));

    // Ring to be scaled to generate the steps of the sphere
    std::vector<double> ring;
    for (double i = 0; i < 2*PI; i+=angle) {
        ring.push_back(i);
    }
    const size_t steps = ring.size(); 
    const double increment = (double)(1.0 / (double)steps);

    // special case: first ring connects to 0,0,0
    // insert and form facets.
    vertices.push_back(Pointf3(0.0, 0.0, -rho));
    size_t id = vertices.size();
    for (size_t i = 0; i < ring.size(); i++) {
        // Fixed scaling 
        const double z = -rho + increment*rho*2.0;
        // radius of the circle for this step.
        const double r = sqrt(std::abs(rho*rho - z*z));
        Pointf3 b(0, r, z);
        b.rotate(ring[i], Pointf3(0,0,z)); 
        vertices.push_back(b);
        if (i == 0) {
            facets.push_back(Point3(1, 0, ring.size()));
        } else {
            facets.push_back(Point3(id, 0, id - 1));
        }
        id++;
    }

    // General case: insert and form facets for each step, joining it to the ring below it.
    for (size_t s = 2; s < steps - 1; s++) {
        const double z = -rho + increment*(double)s*2.0*rho;
        const double r = sqrt(std::abs(rho*rho - z*z));

        for (size_t i = 0; i < ring.size(); i++) {
            Pointf3 b(0, r, z);
            b.rotate(ring[i], Pointf3(0,0,z)); 
            vertices.push_back(b);
            if (i == 0) {
                // wrap around
                facets.push_back(Point3(id + ring.size() - 1 , id, id - 1)); 
                facets.push_back(Point3(id, id - ring.size(),  id - 1)); 
            } else {
                facets.push_back(Point3(id , id - ring.size(), (id - 1) - ring.size())); 
                facets.push_back(Point3(id, id - 1 - ring.size() ,  id - 1)); 
            }
            id++;
        } 
    }


    // special case: last ring connects to 0,0,rho*2.0
    // only form facets.
    vertices.push_back(Pointf3(0.0, 0.0, rho));
    for (size_t i = 0; i < ring.size(); i++) {
        if (i == 0) {
            // third vertex is on the other side of the ring.
            facets.push_back(Point3(id, id - ring.size(),  id - 1));
        } else {
            facets.push_back(Point3(id, id - ring.size() + i,  id - ring.size() + (i - 1)));
        }
    }
    id++;
    TriangleMesh mesh(vertices, facets);
    return mesh;
}


/*-------------------------------------------------------生成特特征支撑-----------------------------------------------------------*/

std::vector<stl_vertex> TriangleMesh::feature_point(QProgressBar* progress)
{
	std::vector<stl_vertex> feature_point;

	//得到局部最低点
	std::vector<int> nadir;//存储的是最低点的id
	nadir = cliff_point();
	progress->setValue(25);

	
	//将特征点加入特征面的点中
	for (std::vector<int>::iterator p = nadir.begin(); p != nadir.end(); ++p) {
		stl_vertex v = stl.v_shared[*p];
		feature_point.push_back(v);
	}

	return feature_point;
}

std::vector<stl_vertex> TriangleMesh::feature_point_face(float space, QProgressBar* progress)
{
	//生成陡峭面
	std::vector<v_face_struct*> feature_faces_45 = generate_feature_faces_45();

	//生成悬吊面
	std::vector<v_face_struct*> feature_faces_90 = generate_feature_faces_90();
	progress->setValue(30);

	//特征面生成点
	std::vector<stl_vertex> feature_faces_point;//存储的是特征面的区域需要支撑点的值
	feature_face_to_point(feature_faces_45, space, feature_faces_point, 45);

	feature_face_to_point(feature_faces_90, space, feature_faces_point, 90);

	//释放特征面的内存
	if (!feature_faces_45.empty())
	{
		auto f = feature_faces_45.begin();
		free((*f)->v_shared_face);
		delete *f;
		feature_faces_45.erase(f);
	}
	feature_faces_45.clear();

	if (!feature_faces_90.empty())
	{
		auto f = feature_faces_90.begin();
		free((*f)->v_shared_face);
		delete *f;
		feature_faces_90.erase(f);
	}
	feature_faces_90.clear();

	progress->setValue(35);

	return feature_faces_point;
}

std::vector<int> TriangleMesh::cliff_point()//得到悬挂点
{
	int aaa = 0;
	std::vector<int> nadir;//局部最低点
	//依次判断每个共享点
	for (int i = 0; i < this->stl.stats.shared_vertices; ++i) {
		//得到包围当前点的环绕面
		v_face_struct v_face = this->v_shared_faces[i];
		//根据id得到点的值
		const stl_vertex v = this->stl.v_shared[i];

		//1.排除接近平面的点
		if (v.z < 0.05)
			continue;

		//2.排除悬吊面上的点
		for (int k = 0; k < v_face.num; ++k) {
			//取出一个环绕面标记值
			if (this->stl.facet_start[v_face.v_shared_face[k]].extra[0] == 90) {
				goto Next;
			}
		}

		//3.筛选凸包点
		for (int k = 0; k < v_face.num; ++k) {
			//取出一个环绕面的点id
			int* p = this->stl.v_indices[v_face.v_shared_face[k]].vertex;
			for (int j = 0; j < 3; ++j) {
				int temp = p[j];
				//根据id得到点的值
				stl_vertex v2 = this->stl.v_shared[temp];
				//根据高度值的比较，判断筛选凸包。
				if (v2.z < v.z)
					goto Next;
			}
		}

		//4.悬挂点向下延伸相交的三角面的个数，奇数不是，偶数是
		int num = 0;
		float bb = 0;
		stl_facet ff;
		for (int j = 0; j < this->stl.stats.number_of_facets; ++j) {
			stl_facet f = this->stl.facet_start[j];
			Pointf3 bb1;

			//排除包含该点的三角面片
			if ((f.vertex[0].x == v.x&&f.vertex[0].y == v.y&&f.vertex[0].z == v.z) ||
				(f.vertex[1].x == v.x&&f.vertex[1].y == v.y&&f.vertex[1].z == v.z) ||
				(f.vertex[2].x == v.x&&f.vertex[2].y == v.y&&f.vertex[2].z == v.z))
				continue;
		
			//三角面片在其上方
			if (f.vertex[0].z >= v.z&&f.vertex[1].z >= v.z&&f.vertex[2].z >= v.z)
				continue;
		
			//测试点在其包围盒中
			if ((v.x > f.vertex[0].x&&v.x > f.vertex[1].x&&v.x > f.vertex[2].x) ||
				(v.x < f.vertex[0].x&&v.x < f.vertex[1].x&&v.x < f.vertex[2].x) ||
				(v.y > f.vertex[0].y&&v.y > f.vertex[1].y&&v.y > f.vertex[2].y) ||
				(v.y < f.vertex[0].y&&v.y < f.vertex[1].y&&v.y < f.vertex[2].y) ||
				(v.z < f.vertex[0].z&&v.z < f.vertex[1].z&&v.z < f.vertex[2].z))
				continue;

			//在平面上判断点向下的延长线是否与三角面片相交
			//对相交次数计数
			if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, 0), Pointf3(f.vertex[1].x, f.vertex[1].y, 0), 
				Pointf3(f.vertex[2].x, f.vertex[2].y, 0), Pointf3(v.x,v.y,0)))
				++num;

			//if (line_to_triangle_point(f, Vectorf3(0, 0, -1), Pointf3(v.x, v.y, v.z), bb1)) {
			//	if (PointinTriangle1(Pointf3(f.vertex[0].x, f.vertex[0].y, 0), Pointf3(f.vertex[1].x, f.vertex[1].y, 0),
			//		Pointf3(f.vertex[2].x, f.vertex[2].y, 0), Pointf3(v.x, v.y, 0))) {
			//		if (bb1.z > bb) {
			//			ff = f;
			//			bb = bb1.z;
			//		}
			//		++num;
			//	}
			//}

		}
		//次数为奇数跳出
		if (num % 2 != 0)
			continue;

		//5.寻找需要受力支撑的点（凸包点）
		double min = find_convex_hull(i);

		//凸包点高度判断（精度值为0.05）
		if (min < 0.05) 
			continue;

		int count = 0;
		for (int k = 0; k < v_face.num; ++k) {
			stl_facet* face = &this->stl.facet_start[v_face.v_shared_face[k]];
			//得到法向量的角度值
			double a = 180 / PI*vector_angle_3D(Vectorf3(0, 0, -1), Vectorf3(face->normal.x, face->normal.y, face->normal.z));
			if (a < 90)
				++count;
		
			//取出一个环绕面标记值
		}

		if (count <= 2)
			goto Next;

		//if (aa > 0) {
		//	int cc1 = 0;//count
		//	int cc2 = 0;
		//	for (int k = 0; k < v_face.num; ++k) 
		//	{
		//		stl_facet* f1 = &this->stl.facet_start[v_face.v_shared_face[k]];
		//		for (int j = k + 1; j < v_face.num; ++j) 
		//		{
		//			stl_facet* f2 = &this->stl.facet_start[v_face.v_shared_face[j]];
		//			for (int f = 0; f < 3; ++f) 
		//			{
		//				if (!(f1->vertex[f].x == v.x&&f1->vertex[f].y == v.y&&f1->vertex[f].z == v.z))
		//				{
		//					for (int z = 0; z < 3; ++z)
		//					{
		//						if (!(f2->vertex[z].x == v.x&&f2->vertex[z].y == v.y&&f2->vertex[z].z == v.z))
		//						{
		//							if (f1->vertex[f].x == f2->vertex[z].x&&f1->vertex[f].y == f2->vertex[z].y&&f1->vertex[f].z == f2->vertex[z].z) 
		//							{
		//								//两个面共边
		//								++cc1;
		//								//向下的边
		//								if ((f1->normal.z + f2->normal.z) < 0) 
		//								{
		//									++cc2;
		//								}
		//							}
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}
		//	//qDebug() << cc1 << "   " << cc2;
		//	if (cc2 * 2 < cc1)
		//		continue;
		//}

		if (min == 10)
			++aaa;
		
		nadir.push_back(i);
		Next:;//用goto时注意不要声明变量时初始化变量？？？
	}
	qDebug() << "min==10 :   " << aaa;
	qDebug() << "nadir:  " << nadir.size();
	return nadir;
}

//
//           \                         /                                      
//            \                      /                                            
//       ______\___________________/______              
//        |     \  /\            /      |                                             
//       _|______\/  \         /        |                                           
//        |           \      /        大凸包点的高度                                             
//    小凸包点高度     \   /            |                                          
//                      \/______________|_                                                   
//                                                                            
//两种判断方式：
//1.在有限层数下，有环绕点超过局部最低点，则判断该点的凸包高度是否超过精度值。
//2.在有限层数下，局部最低点为最低点时，默认该点需要支撑。
//（主要是解决在宏观上看着需要支撑的点，在微观上有小的起伏凸包，造成漏判的情况。）
//
double TriangleMesh::find_convex_hull(int point)//寻找凸包点
{
	//初始化最小的包
	std::vector<int> ps;//储存多余的点
	std::vector<int> ps_1;//储存同一层级种子点
	std::vector<double> ps_d;//储存同一层级拐点的Z轴的坐标值
	stl_vertex source = this->stl.v_shared[point];//当前点的值
	int num = 0;//记录环绕层的层数
	double min = 0;
	bool once = false, done = false;

	ps.push_back(point);//将当前点存入多余点中
	v_face_struct v_face1 = this->v_shared_faces[point];
	for (int i = 0; i<v_face1.num; ++i){
		int* pp = this->stl.v_indices[v_face1.v_shared_face[i]].vertex;
		for (int j = 0; j<3; ++j){
			int temp = pp[j];
			ps.push_back(temp);//将第一层的环绕点存入多余点中
			ps_1.push_back(temp);//将第一层的环绕点存入种子点中
		}
	}
	ps = delete_repetition(ps);//存储多余点，并删除重复点
	ps_1 = delete_repetition(ps_1);//存储当前层的种子点，并删除重复点

	//不断地在下一层环绕点中筛选拐点
	while (1) {
		++num;
		std::vector<int> ps2;
		for (std::vector<int>::iterator _p1 = ps_1.begin(); _p1 != ps_1.end(); ++_p1) {
			//-----------------得到当前环绕点的下一层的环绕点-------------------
			v_face_struct v_face2 = this->v_shared_faces[*_p1];
			std::vector<int> ps1;
			for (int i = 0; i < v_face2.num; ++i) {
				int* pp = this->stl.v_indices[v_face2.v_shared_face[i]].vertex;
				for (int j = 0; j < 3; ++j) {
					int temp = pp[j];
					ps1.push_back(temp);
				}
			}
			ps1 = delete_repetition(ps1);//删除重复点
			ps1 = delete_twig(ps1, ps);//删除多余点
			//------------------------------------------------------------------

			//判断当前点是否为拐点
			stl_vertex v1 = this->stl.v_shared[*_p1];
			for (std::vector<int>::iterator _p2 = ps1.begin(); _p2 != ps1.end(); ++_p2) {
				ps2.push_back(*_p2);
				stl_vertex v2 = this->stl.v_shared[*_p2];

				//有环绕点小于局部最低点
				if (v2.z <= source.z)
					done = true;

				//存入拐点的值
				if ((v1.z - v2.z)>0.025 && !once) {
					ps_d.push_back(v1.z);
				}
			}
		}

		ps2 = delete_repetition(ps2);
		//存储下一层环绕点
		for (std::vector<int>::iterator p = ps2.begin(); p != ps2.end(); ++p) {
			ps.push_back(*p);
		}

		if (ps_d.size() > 0 && !once) {
			//找到最低的拐点（最多只会寻找一次）
			double temp = -1;
			for (std::vector<double>::iterator _p3 = ps_d.begin(); _p3 != ps_d.end(); ++_p3) {
				if (temp > 0) {
					if (*_p3 < temp)
						temp = *_p3;
				}
				else
					temp = *_p3;
			}
			min = temp - source.z;
			once = true;
		}
		//第一层有拐点后，不会继续到下一层环绕点
		//else {
		//	//开始下一层环绕点
		//	ps_1.clear();
		//	ps_1 = ps2;
		//	//ps2.clear();
		//}

		//第一种判断方法
		if (done)
			return min;

		//第二种判断方法（有限层数为6）
		if (num > 12)
			return 10;

		ps_1 = ps2;
	}
}

std::vector<v_face_struct*> TriangleMesh::generate_feature_faces_45()//生成特征面
{
	std::vector<v_face_struct*> feature_faces_45;
	//陡峭面
	feature_faces_45 = gather_feature_faces(45);

	//排除特征面（还需细化！）
	if (!feature_faces_45.empty()) {
		for (std::vector<v_face_struct*>::iterator _faces = feature_faces_45.begin(); _faces != feature_faces_45.end(); ++_faces) {
			v_face_struct* faces = *_faces;
			float minx, miny, maxx, maxy;
			float rect = 0.0, eare = 0.0;//rect包围矩形的面积，eare特征面区域的总面积
			face_to_2D_projection(faces, minx, miny, maxx, maxy);//特征面到二维投影
			rect = (maxx - minx)*(maxy - miny);

			for (int i = 0; i<faces->num; ++i)
				eare += triangle_area(this->stl.facet_start[faces->v_shared_face[i]]);

			//qDebug() << "rect:  " << rect << "eare:  " << eare << "num:  " << faces->num;

			if (eare < 5) {
				faces->num = 0;
			}
		}
	}
	return feature_faces_45;
}

std::vector<v_face_struct*> TriangleMesh::generate_feature_faces_90()
{
	std::vector<v_face_struct*> feature_faces_90;
	//悬吊面
	feature_faces_90 = gather_feature_faces(90);

	if (!feature_faces_90.empty()) {
		for (std::vector<v_face_struct*>::iterator _faces = feature_faces_90.begin(); _faces != feature_faces_90.end(); ++_faces) {
			v_face_struct* faces = *_faces;
			float minx, miny, maxx, maxy;
			float rect = 0.0, eare = 0.0;//rect包围矩形的面积，eare特征面区域的总面积
			face_to_2D_projection(faces, minx, miny, maxx, maxy);//特征面到二维投影
			rect = (maxx - minx)*(maxy - miny);
	
			for (int i = 0; i<faces->num; ++i)
				eare += triangle_area(this->stl.facet_start[faces->v_shared_face[i]]);

			if (eare < 1) {
				faces->num = 0;
			}
			
		}
	}
	return feature_faces_90;
}

std::vector<v_face_struct*> TriangleMesh::gather_feature_faces(char extra)
{
	std::vector<v_face_struct*> feature_faces;
	//悬吊面
	for (int i = 0; i < this->stl.stats.number_of_facets; ++i) {
		stl_facet* f1 = &this->stl.facet_start[i];
		//根据标记，提取特征面
		if (f1->extra[1] != 1) {//判断是否已判断
			if (f1->extra[0] == extra || f1->extra[0] == extra - 1) {
				std::vector<int>* faces = new std::vector<int>;
				faces->push_back(i);
				seed_face(i, faces,extra);//种子面发散
				*faces = delete_repetition(*faces);
				if (faces->size() == 0) {
					continue;
				}
				else {
					//----------------------特征面区域存入特征面集合----------------------------
					v_face_struct* faces1 = new v_face_struct;//需要手动delete回收
					faces1->num = faces->size();
					faces1->v_shared_face = (int*)malloc(sizeof(int)*faces1->num);
					for (std::vector<int>::iterator f = faces->begin(); f != faces->end(); ++f) {
						int len = std::distance(faces->begin(), f);
						faces1->v_shared_face[len] = *f;
					}
					feature_faces.push_back(faces1);
					//--------------------------------------------------------------------------
				}
				delete faces;
			}
			else
				continue;
		}
		else
			continue;
	}
	return feature_faces;
}

void TriangleMesh::seed_face(int f, std::vector<int>* faces, char extra)//种子面发散
{
	//控制递归的次数，减少堆栈内存的使用。 递归的次数会在10000这个值上下浮动
	if (faces->size() > 10000)
		return;

	v_indices_struct* v_f = &this->stl.v_indices[f];//得到面片的三个点
	int* ps = v_f->vertex;
	for (int i = 0; i < 3; ++i) {
		//查找与当前面的共边面
		int a = ps[i];
		int b = ps[(i + 1) % 3];
		int f1 = find_common_edge_face(f, a, b);
		if (f1 < 0)
			continue;
		stl_facet* f2 = &this->stl.facet_start[f1];

		//判断共边面是否为特征面，是，则存入面集合再继续以共边面为种子面发散
		if (f2->extra[1] != 1) {
			if (f2->extra[0] == extra || f2->extra[0] == extra - 1) {
				faces->push_back(f1);
				f2->extra[1] = 1;//设置为已判断
				seed_face(f1, faces, extra);
			}
		}
		else
			continue;
	}
}

void TriangleMesh::feature_face_to_point(std::vector<v_face_struct*>& feature_faces_45, float space, std::vector<stl_vertex>& feature_faces_point, char extra)//特征面生成特征点
{
	if (extra == 90)
	{
		for (std::vector<v_face_struct*>::iterator _faces = feature_faces_45.begin(); _faces != feature_faces_45.end(); ++_faces) {
			if ((*_faces)->num > 0) {
				//筛选出所有多边形的边
				std::vector<stl_edge> edges;
				stl_facet face;
				stl_edge edge;
				int f;
				for (int i = 0; i < (*_faces)->num; ++i) {
					f = (*_faces)->v_shared_face[i];
					face = this->stl.facet_start[f];
					for (int j = 0; j < 3; ++j) {
						int a = j % 3;
						int b = (j + 1) % 3;
						edge.p1 = face.vertex[a];
						edge.p2 = face.vertex[b];
						edge.facet_number1 = f;
						edge.facet_number2 = -1;

						bool ret = false;
						for (auto e = edges.begin(); e != edges.end(); ++e) {
							if (equal_edge(edge, *e)) {
								if ((*e).facet_number1 == -1) {
									(*e).facet_number1 = f;
									ret = true;
									break;
								}
								if ((*e).facet_number2 == -1) {
									(*e).facet_number2 = f;
									ret = true;
									break;
								}
								qDebug() << "three edges equal!!!";
							}
						}
						if (!ret)
							edges.push_back(edge);
					}
				}

				//筛选洞与边界的边
				std::vector<stl_edge> edges1;
				for (auto e = edges.begin(); e != edges.end(); ++e) {
					size_t count = 0;
					if ((*e).facet_number1 == -1)
						++count;
					if ((*e).facet_number2 == -1)
						++count;
					if (count == 1)
						edges1.push_back(*e);
				}
				//qDebug() << "edges1:   " << edges1.size()<<"edges:   "<<edges.size();

				if (extra == 90)
				{
					bool ret = true;
					Polygons polygons;
					std::vector<std::vector<stl_vertex>> polygons_v;//用来检测contours是否悬吊的数据
					while (ret) {
						std::list<stl_vertex> vertexs;
						for (auto e1 = edges1.begin(); e1 != edges1.end(); ++e1) {
							if ((*e1).facet_number1 != -1 || (*e1).facet_number2 != -1) {
								vertexs.push_back((*e1).p1);
								vertexs.push_back((*e1).p2);
								(*e1).facet_number1 = -1;
								(*e1).facet_number2 = -1;
								look_for_polygon(edges1, vertexs);
								break;
							}

							//退出
							if (e1 == edges1.end() - 1)
								ret = false;
						}

						//检查是否闭合，生成polygon
						//qDebug() << "vertex's size:   " << vertexs.size();
						if (vertexs.size() != 0) {
							if (vertexs.size() > 3) {
								if (equal_vertex(vertexs.back(), vertexs.front())) {
									Points ps;
									std::vector<stl_vertex> polygon_v;
									for (auto v = vertexs.begin(); v != vertexs.end(); ++v) {
										ps.push_back(Point::new_scale((*v).x, (*v).y));
										polygon_v.push_back(*v);
									}
									polygons.push_back(Polygon(ps));
									polygons_v.push_back(polygon_v);
								}
								else
									qDebug() << "polygon don't close!!!";
							}
							else
								qDebug() << "vertex's size <3 !!!";
						}
					}

					size_t dis = 0;
					if (polygons.size() > 1) {
						for (auto p = polygons.begin(); p != polygons.end(); ++p) {
							for (auto p1 = polygons.begin(); p1 != polygons.end(); ++p1) {
								if (p != p1) {
									if ((*p).contains((*p1).points.front())) {
										dis = std::distance(polygons.begin(), p);
										break;
									}
								}
							}
						}
					}

					Polygon contous = *(polygons.begin() + dis);
					Polygons polygons1, polygons2;
					polygons1.push_back(contous);
					polygons2.push_back(contous);

					std::vector<stl_vertex> polygon_v = *(polygons_v.begin() + dis);

					//判断contous是否悬吊
					size_t cools = 0;
					for (auto v = polygon_v.begin(); v != polygon_v.end(); ++v) {
						for (int i = 0; i < this->stl.stats.shared_vertices; ++i) {
							if (equal_vertex(this->stl.v_shared[i], *v)) {
								//得到包围当前点的环绕面
								v_face_struct v_face = this->v_shared_faces[i];
								bool cool = true;

								//3.筛选凸包点
								for (int k = 0; k < v_face.num; ++k) {
									bool ret2 = true;
									for (int i = 0; i < (*_faces)->num; ++i) {
										if ((*_faces)->v_shared_face[i] == v_face.v_shared_face[k])
											ret2 = false;
									}

									if (ret2)
									{
										//取出一个环绕面的点id
										int* p = this->stl.v_indices[v_face.v_shared_face[k]].vertex;
										for (int j = 0; j < 3; ++j) {
											//根据高度值的比较，判断筛选凸包。
											if (this->stl.v_shared[p[j]].z < (*v).z)
												cool = false;
										}
									}
								}

								if (cool)
									++cools;
							}
						}
					}

					float ratio;
					if (cools > polygon_v.size() / 2)
						ratio = 0.5;
					else
						ratio = space / 2 + 1;
					//qDebug() << ratio << "  " << cools << "   " << polygon_v.size();


					//向圆心偏移
					bool ret1 = true;
					size_t aaa = 0;

					Pointfs points;
					//在路径上插入支撑点
					while (ret1) {
						if (aaa == 0) {
							polygons1 = offset(polygons1, -scale_(1 * ratio));
							++aaa;
						}
						else {
							polygons1 = offset(polygons1, -scale_(space));
							++aaa;
						}

						if (!polygons1.empty()) {
							contous = *polygons1.begin();
							Pointf beginP(Pointf(unscale(contous.points.front().x), unscale(contous.points.front().y)));
							Pointf endP;
							points.push_back(beginP);

							float space1 = space;
							contous.points.push_back(contous.points.front());
							for (auto c = contous.points.begin() + 1; c != contous.points.end(); ++c) {
								endP.x = unscale((*c).x);
								endP.y = unscale((*c).y);
								bool ret = true;
								while (ret) {
									float lenght = distance_point(beginP, endP);
									if (lenght > space1) {
										beginP = interpolation_point(beginP, endP, space1);
										points.push_back(beginP);
										space1 = 5;
									}
									else {
										beginP = endP;
										space1 = space1 - lenght;
										ret = false;
									}
								}
							}
						}
						else
						{
							Point p = contous.centroid();
							points.push_back(Pointf(unscale(p.x), unscale(p.y)));
							ret1 = false;
						}

						//删除接近的点
						Pointfs points1;
						int maxxx = 10000;
						for (auto p1 = points.begin(); p1 != points.end(); ++p1) {
							for (auto p2 = points.begin(); p2 != points.end(); ++p2){
								if (p1 != p2 ) {
									if ((*p2).x != maxxx) {
										if (distance_point(*p1, *p2) < space / 2) {
											(*p1).x = maxxx;
											break;
										}
									}
								}
							}
						}
						
						for (auto p1 = points.begin(); p1 != points.end(); ++p1) {
							if ((*p1).x != maxxx) {
								points1.push_back(*p1);
							}
						}

						Pointf3 c;
						stl_vertex c1;
						size_t count = 0;
						//判断采样点是否在特征面区域上
						for (auto _p = points1.begin(); _p != points1.end(); ++_p) {
							Pointf p = *_p;
							for (int i = 0; i < (*_faces)->num; ++i) {
								int num = (*_faces)->v_shared_face[i];
								stl_facet f = this->stl.facet_start[num];

								if ((p.x > f.vertex[0].x&&p.x > f.vertex[1].x&&p.x > f.vertex[2].x) ||
									(p.x < f.vertex[0].x&&p.x < f.vertex[1].x&&p.x < f.vertex[2].x) ||
									(p.y > f.vertex[0].y&&p.y > f.vertex[1].y&&p.y > f.vertex[2].y) ||
									(p.y < f.vertex[0].y&&p.y < f.vertex[1].y&&p.y < f.vertex[2].y))
									continue;
								else
								{
									//xoy平面上的采样点向上延伸求交
									Pointf3 A1(f.vertex[0].x, f.vertex[0].y, 0);
									Pointf3 A2(f.vertex[1].x, f.vertex[1].y, 0);
									Pointf3 A3(f.vertex[2].x, f.vertex[2].y, 0);
									Pointf3 P(p.x, p.y, 0);
									if (PointinTriangle1(A1, A2, A3, P)) {
										c = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), Vectorf3(0, 0, 1), P);
										c1.x = c.x; c1.y = c.y; c1.z = c.z;
										feature_faces_point.push_back(c1);//存储采样点
										++count;
									}
									else
										continue;
								}
							}
						}

						if (count == 0 && aaa == 1)
						{
							if (ratio > 0.1) {
								ratio *= 0.8;
								--aaa;
							}
							else
								ratio = 0.0;
							polygons1 = polygons2;
						}
						points.clear();
					}
				}
			}
		}
	}
	else if (extra == 45)
	{
		Pointf3 c;
		stl_vertex c1;
		if (!feature_faces_45.empty()) {
			for (std::vector<v_face_struct*>::iterator _faces = feature_faces_45.begin(); _faces != feature_faces_45.end(); ++_faces) {
				if ((*_faces)->num > 0) {
					v_face_struct* faces = *_faces;
					float minx, miny, maxx, maxy;
					face_to_2D_projection(faces, minx, miny, maxx, maxy);//特征面到二维投影
		
					std::vector<stl_vertex> points;//存储采样点
					balance_sampling_point(points, space, minx, miny, maxx, maxy);//提取均匀采样点
		
					//判断采样点是否在特征面区域上
					for (std::vector<stl_vertex>::iterator _p = points.begin(); _p != points.end(); ++_p) {
						stl_vertex p = *_p;
						for (int i = 0; i < faces->num; ++i) {
							int num = faces->v_shared_face[i];
							stl_facet f = this->stl.facet_start[num];
		
							if ((p.x > f.vertex[0].x&&p.x > f.vertex[1].x&&p.x > f.vertex[2].x) ||
								(p.x < f.vertex[0].x&&p.x < f.vertex[1].x&&p.x < f.vertex[2].x) ||
								(p.y > f.vertex[0].y&&p.y > f.vertex[1].y&&p.y > f.vertex[2].y) ||
								(p.y < f.vertex[0].y&&p.y < f.vertex[1].y&&p.y < f.vertex[2].y))
								continue;
							else
							{
								//xoy平面上的采样点向上延伸求交
								Pointf3 A1(f.vertex[0].x, f.vertex[0].y, 0);
								Pointf3 A2(f.vertex[1].x, f.vertex[1].y, 0);
								Pointf3 A3(f.vertex[2].x, f.vertex[2].y, 0);
								Pointf3 P(p.x, p.y, 0);
								if (PointinTriangle1(A1, A2, A3, P)) {
									c = CalPlaneLineIntersectPoint(Vectorf3(f.normal.x, f.normal.y, f.normal.z), Pointf3(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z), Vectorf3(0, 0, 1), P);
									c1.x = c.x; c1.y = c.y; c1.z = c.z;
									feature_faces_point.push_back(c1);//存储采样点
								}
								else
									continue;
							}
						}
					}
				}
			}
		}
	}
}

void TriangleMesh::look_for_polygon(std::vector<stl_edge>& edges, std::list<stl_vertex>& vertexs)
{
	if (vertexs.size() >= 2) {
		for (auto e = edges.begin(); e != edges.end(); ++e) {
			if ((*e).facet_number1 != -1 || (*e).facet_number2 != -1) {
				if (equal_vertex(vertexs.front(), (*e).p1)) {
					vertexs.push_front((*e).p2);
					(*e).facet_number1 = -1;
					(*e).facet_number2 = -1;
					look_for_polygon(edges, vertexs);
					break;
				}
				if (equal_vertex(vertexs.front(), (*e).p2)) {
					vertexs.push_front((*e).p1);
					(*e).facet_number1 = -1;
					(*e).facet_number2 = -1;
					look_for_polygon(edges, vertexs);
					break;
				}
				if (equal_vertex(vertexs.back(), (*e).p1)) {
					vertexs.push_back((*e).p2);
					(*e).facet_number1 = -1;
					(*e).facet_number2 = -1;
					look_for_polygon(edges, vertexs);
					break;
				}
				if (equal_vertex(vertexs.back(), (*e).p2)) {
					vertexs.push_back((*e).p1);
					(*e).facet_number1 = -1;
					(*e).facet_number2 = -1;
					look_for_polygon(edges, vertexs);
					break;
				}
			}
		}
	}
	else
		qDebug() << "vertexs's size is least!!!";
}

std::vector<Linef3> TriangleMesh::skeleton_line(std::vector<stl_vertex> feature_faces_point)//生成骨架线
{
	std::vector<Linef3> support_skeleton_line;
	for (auto v = feature_faces_point.begin(); v != feature_faces_point.end(); ++v) {
		Pointf3 p((*v).x, (*v).y, (*v).z);
		support_skeleton_line.push_back(Linef3(point_model_intersection_Z(p), Pointf3(p.x, p.y, p.z + 0.2)));
	}
	return support_skeleton_line;
}

//点按Z轴负方向延伸求交
Pointf3 TriangleMesh::point_model_intersection_Z(Pointf3 p)
{
	std::vector<float> ps;
	stl_facet f;
	float bottom;
	Pointf3 c;
	for (int j = 0; j < this->stl.stats.number_of_facets; ++j) {
		f = this->stl.facet_start[j];

		//测试点在其包围盒中
		if ((p.x > f.vertex[0].x&&p.x > f.vertex[1].x&&p.x > f.vertex[2].x) ||
			(p.x < f.vertex[0].x&&p.x < f.vertex[1].x&&p.x < f.vertex[2].x) ||
			(p.y > f.vertex[0].y&&p.y > f.vertex[1].y&&p.y > f.vertex[2].y) ||
			(p.y < f.vertex[0].y&&p.y < f.vertex[1].y&&p.y < f.vertex[2].y) ||
			(p.z < f.vertex[0].z&&p.z < f.vertex[1].z&&p.z < f.vertex[2].z))
			continue;

		//排除包含起点的三角面片---计算错误排除了其他三角面
		//Pointf3 A1(f.vertex[0].x, f.vertex[0].y, f.vertex[0].z);
		//Pointf3 A2(f.vertex[1].x, f.vertex[1].y, f.vertex[1].z);
		//Pointf3 A3(f.vertex[2].x, f.vertex[2].y, f.vertex[2].z);
		//if (PointinTriangle1(A1, A2, A3, Pointf3(p.x, p.y, p.z))) {
		//	continue;
		//}

		if (line_to_triangle_point(f, Vectorf3(0, 0, -1), p, c)) {
			//qDebug() << c.z;
			if (p.z - c.z > 0.001)
				ps.push_back(c.z);
		}
	}

	if (ps.size() >= 1) {
		bottom = *(ps.begin());
		for (auto v2 = ps.begin() + 1; v2 != ps.end(); ++v2) {
			if (bottom < *v2)
				bottom = *v2;
		}
		return Pointf3(p.x, p.y, bottom);
	}
	else {
		return Pointf3(p.x, p.y, 0);
	}
}

void TriangleMesh::face_to_2D_projection(v_face_struct* faces, float &minx, float &miny, float &maxx, float &maxy)//特征面到二维投影
{
	for (int i = 0; i<faces->num; ++i) {
		int num = faces->v_shared_face[i];
		stl_facet f = this->stl.facet_start[num];

		if (i == 0) {
			minx = f.vertex[0].x;
			miny = f.vertex[0].y;
			maxx = f.vertex[0].x;
			maxy = f.vertex[0].y;
		}

		for (int j = 0; j<3; ++j) {
			minx = f.vertex[j].x<minx ? f.vertex[j].x : minx;
			miny = f.vertex[j].y<miny ? f.vertex[j].y : miny;
			maxx = f.vertex[j].x>maxx ? f.vertex[j].x : maxx;
			maxy = f.vertex[j].y>maxy ? f.vertex[j].y : maxy;
		}
	}
}

void TriangleMesh::balance_sampling_point(std::vector<stl_vertex>& points, float space, float minx, float miny, float maxx, float maxy)//提取均匀采样点
{
	std::vector<float> line_x, line_y;
	float midline_x = (maxx - minx) / 2 + minx;
	float midline_y = (maxy - miny) / 2 + miny;
	int num_x = (maxx - midline_x) / space;
	int num_y = (maxy - midline_y) / space;

	line_x.push_back(midline_x);
	line_y.push_back(midline_y);
	for (int i = 0; i<num_x; ++i) {
		line_x.push_back(midline_x + space*(i + 1));
		line_x.push_back(midline_x - space*(i + 1));
	}
	for (int j = 0; j<num_y; ++j) {
		line_y.push_back(midline_y + space*(j + 1));
		line_y.push_back(midline_y - space*(j + 1));
	}

	for (std::vector<float>::iterator x = line_x.begin(); x != line_x.end(); ++x) {
		for (std::vector<float>::iterator y = line_y.begin(); y != line_y.end(); ++y) {
			stl_vertex p = { *x, *y, 0 };
			points.push_back(p);
		}
	}
}

int TriangleMesh::find_common_edge_face(int face, int a, int b)//查找以ab为边另一个三角面片
{
	v_face_struct v_face1 = this->v_shared_faces[a];
	v_face_struct v_face2 = this->v_shared_faces[b];
	int* fs1 = v_face1.v_shared_face;
	int* fs2 = v_face2.v_shared_face;
	for (int i = 0; i<v_face1.num; ++i) {
		int f1 = fs1[i];
		if (f1 == face)
			continue;
		for (int j = 0; j<v_face2.num; ++j) {
			int f2 = fs2[j];
			if (f1 == f2)
				return f1;
		}
	}
	return -1;
}


//删除重复点
std::vector<int> TriangleMesh::delete_repetition(std::vector<int> ps)
{
	//删除其中的重复点
	std::vector<int> ps1;
	for (std::vector<int>::iterator _p = ps.begin(); _p != ps.end(); ++_p) {
		if (*_p != -1) {
			ps1.push_back(*_p);
			for (std::vector<int>::iterator _p1 = _p + 1; _p1 != ps.end(); ++_p1) {
				if ((*_p) == (*_p1))
					*_p1 = -1;
			}
		}
	}
	return ps1;
}

//删除多余的部分
std::vector<int> TriangleMesh::delete_twig(std::vector<int> &bole, std::vector<int> &twig)
{
	std::vector<int> ps;
	for (std::vector<int>::iterator _p = bole.begin(); _p != bole.end(); ++_p) {
		bool ret = true;
		for (std::vector<int>::iterator _p1 = twig.begin(); _p1 != twig.end(); ++_p1) {
			if (*_p == *_p1&&ret)
				ret = false;
		}
		if (ret)
			ps.push_back(*_p);
	}
	return ps;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/



template <Axis A>
void
TriangleMeshSlicer<A>::slice(const std::vector<float> &z, std::vector<Polygons>* layers, size_t threads) const
{
    /**
       This method gets called with a list of unscaled Z coordinates and outputs
       a vector pointer having the same number of items as the original list.
       Each item is a vector of polygons created by slicing our mesh at the 
       given heights.
       
       This method should basically combine the behavior of the existing
       Perl methods defined in lib/Slic3r/TriangleMesh.pm:
       
       - analyze(): this creates the 'facets_edges' and the 'edges_facets'
            tables (we don't need the 'edges' table)
       
       - slice_facet(): this has to be done for each facet. It generates 
            intersection lines with each plane identified by the Z list.
            The get_layer_range() binary search used to identify the Z range
            of the facet is already ported to C++ (see Object.xsp)
       
       - make_loops(): this has to be done for each layer. It creates polygons
            from the lines generated by the previous step.
        
        At the end, we free the tables generated by analyze() as we don't 
        need them anymore.
        
        NOTE: this method accepts a vector of floats because the mesh coordinate
        type is float.
    */
    
    std::vector<IntersectionLines> lines(z.size());
    {
        boost::mutex lines_mutex;
        parallelize<int>(
            0,
            this->mesh->stl.stats.number_of_facets-1,
            boost::bind(&TriangleMeshSlicer<A>::_slice_do, this, _1, &lines, &lines_mutex, z),
			threads
        );
    }
    
    // v_scaled_shared could be freed here
    
    // build loops
    layers->resize(z.size());
    parallelize<size_t>(
        0,
        lines.size()-1,
        boost::bind(&TriangleMeshSlicer<A>::_make_loops_do, this, _1, &lines, layers),
		threads
    );
}

template <Axis A>
void
TriangleMeshSlicer<A>::_slice_do(size_t facet_idx, std::vector<IntersectionLines>* lines, boost::mutex* lines_mutex, 
    const std::vector<float> &z) const
{
    const stl_facet &facet = this->mesh->stl.facet_start[facet_idx];
    
    // find facet extents
    const float min_z = fminf(_z(facet.vertex[0]), fminf(_z(facet.vertex[1]), _z(facet.vertex[2])));
    const float max_z = fmaxf(_z(facet.vertex[0]), fmaxf(_z(facet.vertex[1]), _z(facet.vertex[2])));
    
    #ifdef SLIC3R_DEBUG
    printf("\n==> FACET %zu (%f,%f,%f - %f,%f,%f - %f,%f,%f):\n", facet_idx,
        _x(facet.vertex[0]), _y(facet.vertex[0]), _z(facet.vertex[0]),
        _x(facet.vertex[1]), _y(facet.vertex[1]), _z(facet.vertex[1]),
        _x(facet.vertex[2]), _y(facet.vertex[2]), _z(facet.vertex[2]));
    printf("z: min = %.2f, max = %.2f\n", min_z, max_z);
    #endif
    
    // find layer extents
    std::vector<float>::const_iterator min_layer, max_layer;
    min_layer = std::lower_bound(z.begin(), z.end(), min_z); // first layer whose slice_z is >= min_z
    max_layer = std::upper_bound(z.begin() + (min_layer - z.begin()), z.end(), max_z) - 1; // last layer whose slice_z is <= max_z
    #ifdef SLIC3R_DEBUG
    printf("layers: min = %d, max = %d\n", (int)(min_layer - z.begin()), (int)(max_layer - z.begin()));
    #endif
    
    for (std::vector<float>::const_iterator it = min_layer; it != max_layer + 1; ++it) {
        std::vector<float>::size_type layer_idx = it - z.begin();
        this->slice_facet(*it / SCALING_FACTOR, facet, facet_idx, min_z, max_z, &(*lines)[layer_idx], lines_mutex);
    }
}

template <Axis A>
void
TriangleMeshSlicer<A>::slice(const std::vector<float> &z, std::vector<ExPolygons>* layers, size_t threads) const
{
    std::vector<Polygons> layers_p;
    this->slice(z, &layers_p,threads);
    
    layers->resize(z.size());
    for (std::vector<Polygons>::const_iterator loops = layers_p.begin(); loops != layers_p.end(); ++loops) {
        #ifdef SLIC3R_DEBUG
        size_t layer_id = loops - layers_p.begin();
        printf("Layer %zu (slice_z = %.2f):\n", layer_id, z[layer_id]);
        #endif
        
        this->make_expolygons(*loops, &(*layers)[ loops - layers_p.begin() ]);
    }
}

template <Axis A>
void
TriangleMeshSlicer<A>::slice(float z, ExPolygons* slices, size_t threads) const
{
    std::vector<float> zz;
    zz.push_back(z);
    std::vector<ExPolygons> layers;
    this->slice(zz, &layers,threads);
    append_to(*slices, layers.front());
}

template <Axis A>
void
TriangleMeshSlicer<A>::slice_facet(float slice_z, const stl_facet &facet, const int &facet_idx,
    const float &min_z, const float &max_z, std::vector<IntersectionLine>* lines,
    boost::mutex* lines_mutex) const
{
    std::vector<IntersectionPoint> points;
    std::vector< std::vector<IntersectionPoint>::size_type > points_on_layer;
    bool found_horizontal_edge = false;
    
    /* reorder vertices so that the first one is the one with lowest Z
       this is needed to get all intersection lines in a consistent order
       (external on the right of the line) */
    int i = 0;
    if (_z(facet.vertex[1]) == min_z) {
        // vertex 1 has lowest Z
        i = 1;
    } else if (_z(facet.vertex[2]) == min_z) {
        // vertex 2 has lowest Z
        i = 2;
    }
    for (int j = i; (j-i) < 3; j++) {  // loop through facet edges
        int edge_id = this->facets_edges[facet_idx][j % 3];
        int a_id = this->mesh->stl.v_indices[facet_idx].vertex[j % 3];
        int b_id = this->mesh->stl.v_indices[facet_idx].vertex[(j+1) % 3];
        stl_vertex* a = &this->v_scaled_shared[a_id];
        stl_vertex* b = &this->v_scaled_shared[b_id];
        
        if (_z(*a) == _z(*b) && _z(*a) == slice_z) {
            // edge is horizontal and belongs to the current layer
            
            stl_vertex &v0 = this->v_scaled_shared[ this->mesh->stl.v_indices[facet_idx].vertex[0] ];
            stl_vertex &v1 = this->v_scaled_shared[ this->mesh->stl.v_indices[facet_idx].vertex[1] ];
            stl_vertex &v2 = this->v_scaled_shared[ this->mesh->stl.v_indices[facet_idx].vertex[2] ];
            IntersectionLine line;
            if (min_z == max_z) {
                line.edge_type = feHorizontal;
                if (_z(this->mesh->stl.facet_start[facet_idx].normal) < 0) {
                    /*  if normal points downwards this is a bottom horizontal facet so we reverse
                        its point order */
                    std::swap(a, b);
                    std::swap(a_id, b_id);
                }
            } else if (_z(v0) < slice_z || _z(v1) < slice_z || _z(v2) < slice_z) {
                line.edge_type = feTop;
                std::swap(a, b);
                std::swap(a_id, b_id);
            } else {
                line.edge_type = feBottom;
            }
            line.a.x    = _x(*a);
            line.a.y    = _y(*a);
            line.b.x    = _x(*b);
            line.b.y    = _y(*b);
            line.a_id   = a_id;
            line.b_id   = b_id;
            if (lines_mutex != NULL) {
                boost::lock_guard<boost::mutex> l(*lines_mutex);
                lines->push_back(line);
            } else {
                lines->push_back(line);
            }
            
            found_horizontal_edge = true;
            
            // if this is a top or bottom edge, we can stop looping through edges
            // because we won't find anything interesting
            
            if (line.edge_type != feHorizontal) return;
        } else if (_z(*a) == slice_z) {
            IntersectionPoint point;
            point.x         = _x(*a);
            point.y         = _y(*a);
            point.point_id  = a_id;
            points.push_back(point);
            points_on_layer.push_back(points.size()-1);
        } else if (_z(*b) == slice_z) {
            IntersectionPoint point;
            point.x         = _x(*b);
            point.y         = _y(*b);
            point.point_id  = b_id;
            points.push_back(point);
            points_on_layer.push_back(points.size()-1);
        } else if ((_z(*a) < slice_z && _z(*b) > slice_z) || (_z(*b) < slice_z && _z(*a) > slice_z)) {
            // edge intersects the current layer; calculate intersection
            
            IntersectionPoint point;
            point.x         = _x(*b) + (_x(*a) - _x(*b)) * (slice_z - _z(*b)) / (_z(*a) - _z(*b));
            point.y         = _y(*b) + (_y(*a) - _y(*b)) * (slice_z - _z(*b)) / (_z(*a) - _z(*b));
            point.edge_id   = edge_id;
            points.push_back(point);
        }
    }
    if (found_horizontal_edge) return;
    
    if (!points_on_layer.empty()) {
        // we can't have only one point on layer because each vertex gets detected
        // twice (once for each edge), and we can't have three points on layer because
        // we assume this code is not getting called for horizontal facets
        assert(points_on_layer.size() == 2);
        assert( points[ points_on_layer[0] ].point_id == points[ points_on_layer[1] ].point_id );
        if (points.size() < 3) return;  // no intersection point, this is a V-shaped facet tangent to plane
        points.erase( points.begin() + points_on_layer[1] );
    }
    
    if (!points.empty()) {
        assert(points.size() == 2); // facets must intersect each plane 0 or 2 times
        IntersectionLine line;
        line.a          = (Point)points[1];
        line.b          = (Point)points[0];
        line.a_id       = points[1].point_id;
        line.b_id       = points[0].point_id;
        line.edge_a_id  = points[1].edge_id;
        line.edge_b_id  = points[0].edge_id;
        if (lines_mutex != NULL) {
            boost::lock_guard<boost::mutex> l(*lines_mutex);
            lines->push_back(line);
        } else {
            lines->push_back(line);
        }
        return;
    }
}

template <Axis A>
void
TriangleMeshSlicer<A>::_make_loops_do(size_t i, std::vector<IntersectionLines>* lines, std::vector<Polygons>* layers) const
{
    this->make_loops((*lines)[i], &(*layers)[i]);
}

template <Axis A>
void
TriangleMeshSlicer<A>::make_loops(std::vector<IntersectionLine> &lines, Polygons* loops) const
{
    /*
    SVG svg("lines.svg");
    svg.draw(lines);
    svg.Close();
    */
    
    // remove tangent edges
    for (IntersectionLines::iterator line = lines.begin(); line != lines.end(); ++line) {
        if (line->skip || line->edge_type == feNone) continue;
        
        /* if the line is a facet edge, find another facet edge
           having the same endpoints but in reverse order */
        for (IntersectionLines::iterator line2 = line + 1; line2 != lines.end(); ++line2) {
            if (line2->skip || line2->edge_type == feNone) continue;
            
            // are these facets adjacent? (sharing a common edge on this layer)
            if (line->a_id == line2->a_id && line->b_id == line2->b_id) {
                line2->skip = true;
                
                /* if they are both oriented upwards or downwards (like a 'V')
                   then we can remove both edges from this layer since it won't 
                   affect the sliced shape */
                /* if one of them is oriented upwards and the other is oriented
                   downwards, let's only keep one of them (it doesn't matter which
                   one since all 'top' lines were reversed at slicing) */
                if (line->edge_type == line2->edge_type) {
                    line->skip = true;
                    break;
                }
            } else if (line->a_id == line2->b_id && line->b_id == line2->a_id) {
                /* if this edge joins two horizontal facets, remove both of them */
                if (line->edge_type == feHorizontal && line2->edge_type == feHorizontal) {
                    line->skip = true;
                    line2->skip = true;
                    break;
                }
            }
        }
    }
    
    // build a map of lines by edge_a_id and a_id
    std::vector<IntersectionLinePtrs> by_edge_a_id, by_a_id;
    by_edge_a_id.resize(this->mesh->stl.stats.number_of_facets * 3);
    by_a_id.resize(this->mesh->stl.stats.shared_vertices);
    for (IntersectionLines::iterator line = lines.begin(); line != lines.end(); ++line) {
        if (line->skip) continue;
        if (line->edge_a_id != -1) by_edge_a_id[line->edge_a_id].push_back(&(*line));
        if (line->a_id != -1) by_a_id[line->a_id].push_back(&(*line));
    }
    
    CYCLE: while (1) {
        // take first spare line and start a new loop
        IntersectionLine* first_line = NULL;
        for (IntersectionLines::iterator line = lines.begin(); line != lines.end(); ++line) {
            if (line->skip) continue;
            first_line = &(*line);
            break;
        }
        if (first_line == NULL) break;
        first_line->skip = true;
        IntersectionLinePtrs loop;
        loop.push_back(first_line);
        
        /*
        printf("first_line edge_a_id = %d, edge_b_id = %d, a_id = %d, b_id = %d, a = %d,%d, b = %d,%d\n", 
            first_line->edge_a_id, first_line->edge_b_id, first_line->a_id, first_line->b_id,
            first_line->a.x, first_line->a.y, first_line->b.x, first_line->b.y);
        */
        
        while (1) {
            // find a line starting where last one finishes
            IntersectionLine* next_line = NULL;
            if (loop.back()->edge_b_id != -1) {
                IntersectionLinePtrs &candidates = by_edge_a_id[loop.back()->edge_b_id];
                for (IntersectionLinePtrs::iterator lineptr = candidates.begin(); lineptr != candidates.end(); ++lineptr) {
                    if ((*lineptr)->skip) continue;
                    next_line = *lineptr;
                    break;
                }
            }
            if (next_line == NULL && loop.back()->b_id != -1) {
                IntersectionLinePtrs &candidates = by_a_id[loop.back()->b_id];
                for (IntersectionLinePtrs::iterator lineptr = candidates.begin(); lineptr != candidates.end(); ++lineptr) {
                    if ((*lineptr)->skip) continue;
                    next_line = *lineptr;
                    break;
                }
            }
            
            if (next_line == NULL) {
                // check whether we closed this loop
                if ((loop.front()->edge_a_id != -1 && loop.front()->edge_a_id == loop.back()->edge_b_id)
                    || (loop.front()->a_id != -1 && loop.front()->a_id == loop.back()->b_id)) {
                    // loop is complete
                    Polygon p;
                    p.points.reserve(loop.size());
                    for (IntersectionLinePtrs::const_iterator lineptr = loop.begin(); lineptr != loop.end(); ++lineptr) {
                        p.points.push_back((*lineptr)->a);
                    }
                    
                    loops->push_back(p);
                    
                    #ifdef SLIC3R_DEBUG
                    printf("  Discovered %s polygon of %d points\n", (p.is_counter_clockwise() ? "ccw" : "cw"), (int)p.points.size());
                    #endif
                    
                    goto CYCLE;
                }
                
                // we can't close this loop!
                //// push @failed_loops, [@loop];
                //#ifdef SLIC3R_DEBUG
                printf("  Unable to close this loop having %d points\n", (int)loop.size());
                //#endif
                goto CYCLE;
            }
            /*
            printf("next_line edge_a_id = %d, edge_b_id = %d, a_id = %d, b_id = %d, a = %d,%d, b = %d,%d\n", 
                next_line->edge_a_id, next_line->edge_b_id, next_line->a_id, next_line->b_id,
                next_line->a.x, next_line->a.y, next_line->b.x, next_line->b.y);
            */
            loop.push_back(next_line);
            next_line->skip = true;
        }
    }
}

class _area_comp {
    public:
    _area_comp(std::vector<double>* _aa) : abs_area(_aa) {};
    bool operator() (const size_t &a, const size_t &b) {
        return (*this->abs_area)[a] > (*this->abs_area)[b];
    }
    
    private:
    std::vector<double>* abs_area;
};

template <Axis A>
void
TriangleMeshSlicer<A>::make_expolygons_simple(std::vector<IntersectionLine> &lines, ExPolygons* slices) const
{
    Polygons loops;
    this->make_loops(lines, &loops);
    
    // cache slice contour area
    std::vector<double> area;
    area.resize(slices->size(), -1);
    
    Polygons cw;
    for (const Polygon &loop : loops) {
        const double a = loop.area();
        if (a >= 0) {
            slices->push_back(ExPolygon(loop));
            area.push_back(a);
        } else {
            cw.push_back(loop);
        }
    }
    
    // assign holes to contours
    for (const Polygon &loop : cw) {
        int slice_idx = -1;
        double current_contour_area = -1;
        for (size_t i = 0; i < slices->size(); ++i) {
            if ((*slices)[i].contour.contains(loop.points.front())) {
                if (area[i] == -1) area[i] = (*slices)[i].contour.area();
                if (area[i] < current_contour_area || current_contour_area == -1) {
                    slice_idx = i;
                    current_contour_area = area[i];
                }
            }
        }
        
        // discard holes which couldn't fit inside a contour as they are probably
        // invalid polygons (self-intersecting)
        if (slice_idx > -1)
            (*slices)[slice_idx].holes.push_back(loop);
    }
}

template <Axis A>
void
TriangleMeshSlicer<A>::make_expolygons(const Polygons &loops, ExPolygons* slices) const
{
    /**
        Input loops are not suitable for evenodd nor nonzero fill types, as we might get
        two consecutive concentric loops having the same winding order - and we have to 
        respect such order. In that case, evenodd would create wrong inversions, and nonzero
        would ignore holes inside two concentric contours.
        So we're ordering loops and collapse consecutive concentric loops having the same 
        winding order.
        \todo find a faster algorithm for this, maybe with some sort of binary search.
        If we computed a "nesting tree" we could also just remove the consecutive loops
        having the same winding order, and remove the extra one(s) so that we could just
        supply everything to offset() instead of performing several union/diff calls.
    
        we sort by area assuming that the outermost loops have larger area;
        the previous sorting method, based on $b->contains($a->[0]), failed to nest
        loops correctly in some edge cases when original model had overlapping facets
    */

    std::vector<double> area;
    std::vector<double> abs_area;
    std::vector<size_t> sorted_area;  // vector of indices
    for (Polygons::const_iterator loop = loops.begin(); loop != loops.end(); ++loop) {
        double a = loop->area();
        area.push_back(a);
        abs_area.push_back(std::fabs(a));
        sorted_area.push_back(loop - loops.begin());
    }
    
    std::sort(sorted_area.begin(), sorted_area.end(), _area_comp(&abs_area));  // outer first

    // we don't perform a safety offset now because it might reverse cw loops
    Polygons p_slices;
    for (std::vector<size_t>::const_iterator loop_idx = sorted_area.begin(); loop_idx != sorted_area.end(); ++loop_idx) {
        /* we rely on the already computed area to determine the winding order
           of the loops, since the Orientation() function provided by Clipper
           would do the same, thus repeating the calculation */
        Polygons::const_iterator loop = loops.begin() + *loop_idx;
        if (area[*loop_idx] > +EPSILON) {
            p_slices.push_back(*loop);
        } else if (area[*loop_idx] < -EPSILON) {
            p_slices = diff(p_slices, *loop);
        }
    }

    // perform a safety offset to merge very close facets (TODO: find test case for this)
    double safety_offset = scale_(0.0499);
    ExPolygons ex_slices = offset2_ex(p_slices, +safety_offset, -safety_offset);
    
    #ifdef SLIC3R_DEBUG
    size_t holes_count = 0;
    for (ExPolygons::const_iterator e = ex_slices.begin(); e != ex_slices.end(); ++e) {
        holes_count += e->holes.size();
    }
    printf("%zu surface(s) having %zu holes detected from %zu polylines\n",
        ex_slices.size(), holes_count, loops.size());
    #endif
    
    // append to the supplied collection
    slices->insert(slices->end(), ex_slices.begin(), ex_slices.end());
}

template <Axis A>
void
TriangleMeshSlicer<A>::make_expolygons(std::vector<IntersectionLine> &lines, ExPolygons* slices) const
{
    Polygons pp;
    this->make_loops(lines, &pp);
    this->make_expolygons(pp, slices);
}

template <Axis A>
void
TriangleMeshSlicer<A>::cut(float z, TriangleMesh* upper, TriangleMesh* lower) const
{
    IntersectionLines upper_lines, lower_lines;
    
    const float scaled_z = scale_(z);
    for (int facet_idx = 0; facet_idx < this->mesh->stl.stats.number_of_facets; facet_idx++) {
        stl_facet* facet = &this->mesh->stl.facet_start[facet_idx];
        
        // find facet extents
        float min_z = fminf(_z(facet->vertex[0]), fminf(_z(facet->vertex[1]), _z(facet->vertex[2])));
        float max_z = fmaxf(_z(facet->vertex[0]), fmaxf(_z(facet->vertex[1]), _z(facet->vertex[2])));
        
        // intersect facet with cutting plane
        IntersectionLines lines;
        this->slice_facet(scaled_z, *facet, facet_idx, min_z, max_z, &lines);
        
        // save intersection lines for generating correct triangulations
        for (IntersectionLines::const_iterator it = lines.begin(); it != lines.end(); ++it) {
            if (it->edge_type == feTop) {
                lower_lines.push_back(*it);
            } else if (it->edge_type == feBottom) {
                upper_lines.push_back(*it);
            } else if (it->edge_type != feHorizontal) {
                lower_lines.push_back(*it);
                upper_lines.push_back(*it);
            }
        }
        
        if (min_z > z || (min_z == z && max_z > min_z)) {
            // facet is above the cut plane and does not belong to it
            if (upper != NULL) stl_add_facet(&upper->stl, facet);
        } else if (max_z < z || (max_z == z && max_z > min_z)) {
            // facet is below the cut plane and does not belong to it
            if (lower != NULL) stl_add_facet(&lower->stl, facet);
        } else if (min_z < z && max_z > z) {
            // facet is cut by the slicing plane
            
            // look for the vertex on whose side of the slicing plane there are no other vertices
            int isolated_vertex;
            if ( (_z(facet->vertex[0]) > z) == (_z(facet->vertex[1]) > z) ) {
                isolated_vertex = 2;
            } else if ( (_z(facet->vertex[1]) > z) == (_z(facet->vertex[2]) > z) ) {
                isolated_vertex = 0;
            } else {
                isolated_vertex = 1;
            }
            
            // get vertices starting from the isolated one
            stl_vertex* v0 = &facet->vertex[isolated_vertex];
            stl_vertex* v1 = &facet->vertex[(isolated_vertex+1) % 3];
            stl_vertex* v2 = &facet->vertex[(isolated_vertex+2) % 3];
            
            // intersect v0-v1 and v2-v0 with cutting plane and make new vertices
            stl_vertex v0v1, v2v0;
            _x(v0v1) = _x(*v1) + (_x(*v0) - _x(*v1)) * (z - _z(*v1)) / (_z(*v0) - _z(*v1));
            _y(v0v1) = _y(*v1) + (_y(*v0) - _y(*v1)) * (z - _z(*v1)) / (_z(*v0) - _z(*v1));
            _z(v0v1) = z;
            _x(v2v0) = _x(*v2) + (_x(*v0) - _x(*v2)) * (z - _z(*v2)) / (_z(*v0) - _z(*v2));
            _y(v2v0) = _y(*v2) + (_y(*v0) - _y(*v2)) * (z - _z(*v2)) / (_z(*v0) - _z(*v2));
            _z(v2v0) = z;
            
            // build the triangular facet
            stl_facet triangle;
            triangle.normal = facet->normal;
            triangle.vertex[0] = *v0;
            triangle.vertex[1] = v0v1;
            triangle.vertex[2] = v2v0;
            
            // build the facets forming a quadrilateral on the other side
            stl_facet quadrilateral[2];
            quadrilateral[0].normal = facet->normal;
            quadrilateral[0].vertex[0] = *v1;
            quadrilateral[0].vertex[1] = *v2;
            quadrilateral[0].vertex[2] = v0v1;
            quadrilateral[1].normal = facet->normal;
            quadrilateral[1].vertex[0] = *v2;
            quadrilateral[1].vertex[1] = v2v0;
            quadrilateral[1].vertex[2] = v0v1;
            
            if (_z(*v0) > z) {
                if (upper != NULL) stl_add_facet(&upper->stl, &triangle);
                if (lower != NULL) {
                    stl_add_facet(&lower->stl, &quadrilateral[0]);
                    stl_add_facet(&lower->stl, &quadrilateral[1]);
                }
            } else {
                if (upper != NULL) {
                    stl_add_facet(&upper->stl, &quadrilateral[0]);
                    stl_add_facet(&upper->stl, &quadrilateral[1]);
                }
                if (lower != NULL) stl_add_facet(&lower->stl, &triangle);
            }
        }
    }
    
    // triangulate holes of upper mesh
    if (upper != NULL) {
        // compute shape of section
        ExPolygons section;
        this->make_expolygons_simple(upper_lines, &section);
        
        // triangulate section
        Polygons triangles;
        for (ExPolygons::const_iterator expolygon = section.begin(); expolygon != section.end(); ++expolygon)
            expolygon->triangulate_p2t(&triangles);
        
        // convert triangles to facets and append them to mesh
        for (Polygons::const_iterator polygon = triangles.begin(); polygon != triangles.end(); ++polygon) {
            Polygon p = *polygon;
            p.reverse();
            stl_facet facet;
            _x(facet.normal) = 0;
            _y(facet.normal) = 0;
            _z(facet.normal) = -1;
            for (size_t i = 0; i <= 2; ++i) {
                _x(facet.vertex[i]) = unscale(p.points[i].x);
                _y(facet.vertex[i]) = unscale(p.points[i].y);
                _z(facet.vertex[i]) = z;
            }
            stl_add_facet(&upper->stl, &facet);
        }
    }
    
    // triangulate holes of lower mesh
    if (lower != NULL) {
        // compute shape of section
        ExPolygons section;
        this->make_expolygons_simple(lower_lines, &section);
        
        // triangulate section
        Polygons triangles;
        for (ExPolygons::const_iterator expolygon = section.begin(); expolygon != section.end(); ++expolygon)
            expolygon->triangulate_p2t(&triangles);
        
        // convert triangles to facets and append them to mesh
        for (Polygons::const_iterator polygon = triangles.begin(); polygon != triangles.end(); ++polygon) {
            stl_facet facet;
            _x(facet.normal) = 0;
            _y(facet.normal) = 0;
            _z(facet.normal) = 1;
            for (size_t i = 0; i <= 2; ++i) {
                _x(facet.vertex[i]) = unscale(polygon->points[i].x);
                _y(facet.vertex[i]) = unscale(polygon->points[i].y);
                _z(facet.vertex[i]) = z;
            }
            stl_add_facet(&lower->stl, &facet);
        }
    }
    
    
    stl_get_size(&(upper->stl));
    stl_get_size(&(lower->stl));
}

template <Axis A>
TriangleMeshSlicer<A>::TriangleMeshSlicer(TriangleMesh* _mesh) : mesh(_mesh), v_scaled_shared(NULL)
{
    // build a table to map a facet_idx to its three edge indices
    this->mesh->require_shared_vertices();
    typedef std::pair<int,int>              t_edge;
    typedef std::vector<t_edge>             t_edges;  // edge_idx => a_id,b_id
    typedef std::map<t_edge,int>            t_edges_map;  // a_id,b_id => edge_idx
    
    this->facets_edges.resize(this->mesh->stl.stats.number_of_facets);
    
    {
        t_edges edges;
        // reserve() instad of resize() because otherwise we couldn't read .size() below to assign edge_idx
        edges.reserve(this->mesh->stl.stats.number_of_facets * 3);  // number of edges = number of facets * 3
        t_edges_map edges_map;
        for (int facet_idx = 0; facet_idx < this->mesh->stl.stats.number_of_facets; facet_idx++) {
            this->facets_edges[facet_idx].resize(3);
            for (int i = 0; i <= 2; i++) {
                int a_id = this->mesh->stl.v_indices[facet_idx].vertex[i];
                int b_id = this->mesh->stl.v_indices[facet_idx].vertex[(i+1) % 3];
                
                int edge_idx;
                t_edges_map::const_iterator my_edge = edges_map.find(std::make_pair(b_id,a_id));
                if (my_edge != edges_map.end()) {
                    edge_idx = my_edge->second;
                } else {
                    /* admesh can assign the same edge ID to more than two facets (which is 
                       still topologically correct), so we have to search for a duplicate of 
                       this edge too in case it was already seen in this orientation */    
                    my_edge = edges_map.find(std::make_pair(a_id,b_id));
                    
                    if (my_edge != edges_map.end()) {
                        edge_idx = my_edge->second;
                    } else {
                        // edge isn't listed in table, so we insert it
                        edge_idx = edges.size();
                        edges.push_back(std::make_pair(a_id,b_id));
                        edges_map[ edges[edge_idx] ] = edge_idx;
                    }
                }
                this->facets_edges[facet_idx][i] = edge_idx;
                
                #ifdef SLIC3R_DEBUG
                printf("  [facet %d, edge %d] a_id = %d, b_id = %d   --> edge %d\n", facet_idx, i, a_id, b_id, edge_idx);
                #endif
            }
        }
    }
    
    // clone shared vertices coordinates and scale them
    this->v_scaled_shared = (stl_vertex*)calloc(this->mesh->stl.stats.shared_vertices, sizeof(stl_vertex));
    std::copy(this->mesh->stl.v_shared, this->mesh->stl.v_shared + this->mesh->stl.stats.shared_vertices, this->v_scaled_shared);
    for (int i = 0; i < this->mesh->stl.stats.shared_vertices; i++) {
        this->v_scaled_shared[i].x /= SCALING_FACTOR;
        this->v_scaled_shared[i].y /= SCALING_FACTOR;
        this->v_scaled_shared[i].z /= SCALING_FACTOR;
    }
}

template <Axis A>
TriangleMeshSlicer<A>::~TriangleMeshSlicer()
{
    if (this->v_scaled_shared != NULL) free(this->v_scaled_shared);
}

template class TriangleMeshSlicer<X>;
template class TriangleMeshSlicer<Y>;
template class TriangleMeshSlicer<Z>;

}
