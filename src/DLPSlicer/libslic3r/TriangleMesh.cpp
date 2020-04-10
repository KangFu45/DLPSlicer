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
#include "qdebug.h"

#ifdef DLPSlicer_DEBUG
#include "SVG.hpp"
#endif

namespace DLPSlicer {

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
    if (other.stl.v_shared_faces != NULL) {
        this->stl.v_shared_faces = (v_face_struct*)calloc(other.stl.stats.shared_vertices, sizeof(v_face_struct));
        std::copy(other.stl.v_shared_faces, other.stl.v_shared_faces + other.stl.stats.shared_vertices, this->stl.v_shared_faces);
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
    stl_generate_shared_vertices_faces(&stl);
    
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
    angle = DLPSlicer::Geometry::rad2deg(angle);
    
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
    return DLPSlicer::Geometry::convex_hull(pp);
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
    if (this->stl.v_shared == NULL) stl_generate_shared_vertices_faces(&(this->stl));
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
		double a = 180 / PI*vector_angle_3(Vectorf3(0, 0, -1), Vectorf3(face->normal.x, face->normal.y, face->normal.z));
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
       Perl methods defined in lib/DLPSlicer/TriangleMesh.pm:
       
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
    
    #ifdef DLPSlicer_DEBUG
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
    #ifdef DLPSlicer_DEBUG
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
        #ifdef DLPSlicer_DEBUG
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
                    
                    #ifdef DLPSlicer_DEBUG
                    printf("  Discovered %s polygon of %d points\n", (p.is_counter_clockwise() ? "ccw" : "cw"), (int)p.points.size());
                    #endif
                    
                    goto CYCLE;
                }
                
                // we can't close this loop!
                //// push @failed_loops, [@loop];
                //#ifdef DLPSlicer_DEBUG
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
    
    #ifdef DLPSlicer_DEBUG
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
                
                #ifdef DLPSlicer_DEBUG
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

//--------------------------生成特特征支撑----------------------------

stl_vertexs TriangleMesh::feature_point(QProgressBar* progress)
{
    stl_vertexs feature_points;

    //得到局部最低点
    unsigned short num = 0;
    int* nadir = cliff_point(num);

    progress->setValue(25);

    for (int i = 0; i < num; ++i)
        feature_points.emplace_back(stl.v_shared[nadir[i]]);

    return feature_points;
}

stl_vertexs TriangleMesh::feature_point_face(float space, QProgressBar* progress)
{
    //生成陡峭面，参数化
    std::vector<v_face_struct> feature_faces_45 = generate_feature_faces(45, 5);
    std::vector<v_face_struct> feature_faces_90 = generate_feature_faces(90, 1);

    progress->setValue(30);

    //特征面生成点
    stl_vertexs feature_faces_point;//存储的是特征面的区域需要支撑点的值
    feature_face_to_point(feature_faces_45, space, feature_faces_point, 45);
    feature_face_to_point(feature_faces_90, space, feature_faces_point, 90);

    //释放特征面的内存
    while (!feature_faces_45.empty())
    {
        delete[] feature_faces_45.begin()->v_shared_face;
        feature_faces_45.erase(feature_faces_45.begin());
    }

    while (!feature_faces_90.empty())
    {
        delete[] feature_faces_90.begin()->v_shared_face;
        feature_faces_90.erase(feature_faces_90.begin());
    }

    progress->setValue(35);

    return feature_faces_point;
}

int* TriangleMesh::cliff_point(unsigned short& num)//得到悬挂点
{
    int* nadir = new int[this->stl.stats.shared_vertices / 2];//局部最低点
    //依次判断每个共享点
    for (int i = 0; i < this->stl.stats.shared_vertices; ++i) {
        //得到包围当前点的环绕面
        v_face_struct v_face = this->stl.v_shared_faces[i];
        //根据id得到点的值
        stl_vertex v = this->stl.v_shared[i];
        unsigned short ret = 0;

        //1.排除接近平面的点
        if (v.z < 0.05)//TODO:参数化
            continue;

        //2.排除悬吊面上的点
        for (int j = 0; j < v_face.num; ++j) {
            //取出一个环绕面标记值
            if (this->stl.facet_start[v_face.v_shared_face[j]].extra[0] == 90) {
                ++ret;
                break;
            }
        }
        if (ret != 0) continue;

        //3.筛选凸包点
        for (int j = 0; j < v_face.num; ++j) {
            for (int k = 0; k < 3; ++k) {
                //取出一个环绕面的点id
                //根据id得到点的值
                //根据高度值的比较，判断筛选凸包。
                if (this->stl.v_shared[this->stl.v_indices[v_face.v_shared_face[j]].vertex[k]].z < v.z) {
                    ++ret;
                    break;
                }
            }
        }
        if (ret != 0) continue;

        //4.悬挂点向下延伸相交的三角面的个数，奇数不是，偶数是
        for (int j = 0; j < this->stl.stats.number_of_facets; ++j) {
            stl_facet f = this->stl.facet_start[j];

            //4.1排除包含该点的三角面片	
            //4.2三角面片在其上方		
            //4.3测试点在其包围盒中
            if ((f.vertex[0] == v || f.vertex[1] == v || f.vertex[2] == v) ||
                (f.vertex[0].z >= v.z && f.vertex[1].z >= v.z && f.vertex[2].z >= v.z) ||
                (v.x > f.vertex[0].x&& v.x > f.vertex[1].x&& v.x > f.vertex[2].x) ||
                (v.x < f.vertex[0].x && v.x < f.vertex[1].x && v.x < f.vertex[2].x) ||
                (v.y > f.vertex[0].y&& v.y > f.vertex[1].y&& v.y > f.vertex[2].y) ||
                (v.y < f.vertex[0].y && v.y < f.vertex[1].y && v.y < f.vertex[2].y) ||
                (v.z < f.vertex[0].z && v.z < f.vertex[1].z && v.z < f.vertex[2].z))
                continue;

            //4.4在平面上判断点向下的延长线是否与三角面片相交
            //对相交次数计数
            if (PointInTriangle2(f, Ver2Pf3(v)))
                ++ret;
        }
        //次数为奇数跳出
        if (ret % 2 != 0)continue;

        //5.寻找需要受力支撑的点（凸包点）
        //凸包点高度判断（精度值为0.05）TODO:参数化
        if (find_convex_hull(i) < 0.05)continue;

        //6.忘了
        ret = 0;
        for (int j = 0; j < v_face.num; ++j) {
            //得到法向量的角度值
            if (180 / PI * vector_angle_3(Vectorf3(0, 0, -1), Nor2Vt3(this->stl.facet_start[v_face.v_shared_face[j]].normal)) < 90)
                ++ret;
        }
        if (ret <= 2) continue;

        nadir[num] = i;
        ++num;
    }
    return nadir;
}

//删除重复点
inline void IntsDelRepetition(std::vector<int>& ps)
{
    for (auto p1 = ps.begin(); p1 != ps.end(); ++p1) {
        for (auto p2 = p1 + 1; p2 != ps.end(); ++p2) {
            if (*p1 == *p2) {
                --p2;
                ps.erase(p2 + 1);
            }
        }
    }
}

//删除多余的部分,bole-twig
inline void IntsDelTwig(std::vector<int>& bole, const std::vector<int>& twig)
{
    int count = bole.size();
    for (int i = 0; i < count; ++i) {
        for each (int j in twig) {
            if (*(bole.begin() + i) == j) {
                bole.erase(bole.begin() + i);
                --count;
                --i;
                break;
            }
        }
    }
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
double TriangleMesh::find_convex_hull(int point)//寻找凸包点
{
    //初始化最小的包
    std::vector<int> ps;//储存多余的点
    std::vector<int> cur_ps;//储存同一层级种子点
    std::vector<double> cur_z_ps;//储存同一层级拐点的Z轴的坐标值
    bool once = false, done = false;

    ps.emplace_back(point);//将当前点存入多余点中
    for (int i = 0; i < this->stl.v_shared_faces[point].num; ++i) {
        for (int j = 0; j < 3; ++j) {
            int temp = this->stl.v_indices[this->stl.v_shared_faces[point].v_shared_face[i]].vertex[j];
            ps.emplace_back(temp);//将第一层的环绕点存入多余点中
            cur_ps.emplace_back(temp);//将第一层的环绕点存入种子点中
        }
    }
    IntsDelRepetition(ps);//存储多余点，并删除重复点
    IntsDelRepetition(cur_ps);//存储当前层的种子点，并删除重复点

    int layer = 0;//记录环绕层的层数
    double min = 0;
    //不断地在下一层环绕点中筛选拐点
    while (1) {
        ++layer;
        std::vector<int> ps2;
        for each (const int& p1 in cur_ps) {
            //-----------------得到当前环绕点的下一层的环绕点-------------------
            std::vector<int> ps1;
            for (int i = 0; i < this->stl.v_shared_faces[p1].num; ++i) {
                for (int j = 0; j < 3; ++j)
                    ps1.emplace_back(this->stl.v_indices[this->stl.v_shared_faces[p1].v_shared_face[i]].vertex[j]);
            }
            IntsDelRepetition(ps1);//删除重复点
            IntsDelTwig(ps1, ps);//删除多余点
            //------------------------------------------------------------------

            //判断当前点是否为拐点
            for each (const int& p2 in ps1) {
                ps2.emplace_back(p2);

                //有环绕点小于局部最低点
                if (this->stl.v_shared[p2].z <= this->stl.v_shared[point].z)
                    done = true;

                //存入拐点的值TODO:参数化
                if ((this->stl.v_shared[p1].z - this->stl.v_shared[p2].z) > 0.025 && !once)
                    cur_z_ps.emplace_back(this->stl.v_shared[p1].z);
            }
        }

        IntsDelRepetition(ps2);
        //存储下一层环绕点
        ps.insert(ps.end(), ps2.begin(), ps2.end());

        if (cur_z_ps.size() > 0 && !once) {
            //找到最低的拐点（最多只会寻找一次）
            double t_min = -1;
            for each (const double& p3 in cur_z_ps) {
                if (t_min == -1) {
                    t_min = p3;
                    continue;
                }
                if (p3 < t_min)
                    t_min = p3;
            }
            min = t_min - this->stl.v_shared[point].z;
            once = true;
        }
        //第一层有拐点后，不会继续到下一层环绕点
        //第一种判断方法
        if (done) return min;

        //第二种判断方法（有限层数为12）,10为大于凸包高度
        if (layer > 12) return 10;

        cur_ps = ps2;
    }
}

std::vector<v_face_struct> TriangleMesh::generate_feature_faces(char extra, float min_area)
{
    std::vector<v_face_struct> feature_faces;
    //悬吊面
    feature_faces = gather_feature_faces(extra);

    for (auto faces = feature_faces.begin(); faces != feature_faces.end(); ++faces) {
        float area = 0.0;//rect包围矩形的面积，eare特征面区域的总面积

        for (int i = 0; i < faces->num; ++i)
            area += TriArea(this->stl.facet_start[faces->v_shared_face[i]]);

        if (area < min_area) {
            delete faces->v_shared_face;
            --faces;
            feature_faces.erase(faces + 1);
        }
    }
    return feature_faces;
}

std::vector<v_face_struct> TriangleMesh::gather_feature_faces(char extra)
{
    std::vector<v_face_struct> feature_faces;
    //悬吊面
    for (int i = 0; i < this->stl.stats.number_of_facets; ++i) {
        //根据标记，提取特征面
        if (this->stl.facet_start[i].extra[1] == 1) //判断是否已判断
            continue;

        if (this->stl.facet_start[i].extra[0] != extra
            && this->stl.facet_start[i].extra[0] != extra - 1)
            continue;

        std::vector<int> faces;
        faces.emplace_back(i);
        seed_face(i, faces, extra);//种子面发散
        IntsDelRepetition(faces);
        if (faces.empty())
            continue;

        //----特征面区域存入特征面集合---
        v_face_struct faces1;
        faces1.num = faces.size();
        faces1.v_shared_face = new int[faces1.num];

        for (int i = 0; i < faces1.num; ++i)
            faces1.v_shared_face[i] = *(faces.begin() + i);

        feature_faces.emplace_back(faces1);
    }
    return feature_faces;
}

void TriangleMesh::seed_face(int f, std::vector<int>& faces, char extra)//种子面发散
{
    //控制递归的次数，减少堆栈内存的使用。 递归的次数会在10000这个值上下浮动
    if (faces.size() > 10000)
        return;

    for (int i = 0; i < 3; ++i) {
        //查找与当前面的共边面
        int f1 = find_common_edge_face(f, this->stl.v_indices[f].vertex[i], this->stl.v_indices[f].vertex[(i + 1) % 3]);
        if (f1 < 0)
            continue;

        //判断共边面是否为特征面，是，则存入面集合再继续以共边面为种子面发散
        if (this->stl.facet_start[f1].extra[1] == 1)
            continue;

        if (this->stl.facet_start[f1].extra[0] == extra
            || this->stl.facet_start[f1].extra[0] == extra - 1)
        {
            faces.emplace_back(f1);
            this->stl.facet_start[f1].extra[1] = 1;//设置为已判断
            seed_face(f1, faces, extra);
        }
    }
}

inline void look_for_polygon(std::vector<stl_edge>& edges, std::list<stl_vertex>& vertexs)
{
    if (vertexs.size() < 2)
        return;

    for (auto e = edges.begin(); e != edges.end(); ++e) {
        if (e->facet_number1 == -1 && e->facet_number2 == -1)
            continue;
        if (equal_vertex(vertexs.front(), e->p1)) {
            vertexs.push_front(e->p2);
            e->facet_number1 = -1;
            e->facet_number2 = -1;
            look_for_polygon(edges, vertexs);
            break;
        }
        else if (equal_vertex(vertexs.front(), e->p2)) {
            vertexs.push_front((*e).p1);
            e->facet_number1 = -1;
            e->facet_number2 = -1;
            look_for_polygon(edges, vertexs);
            break;
        }

        if (equal_vertex(vertexs.back(), e->p1)) {
            vertexs.push_back(e->p2);
            e->facet_number1 = -1;
            e->facet_number2 = -1;
            look_for_polygon(edges, vertexs);
            break;
        }
        else if (equal_vertex(vertexs.back(), e->p2)) {
            vertexs.push_back((*e).p1);
            e->facet_number1 = -1;
            e->facet_number2 = -1;
            look_for_polygon(edges, vertexs);
            break;
        }
    }
}

inline stl_vertexs balance_sampling_point(float space, const BoundingBoxf& box)//提取均匀采样点
{
    stl_vertexs points;
    std::vector<float> line_x, line_y;
    float midline_x = (box.max.x - box.min.x) / 2 + box.min.x;
    float midline_y = (box.max.y - box.min.y) / 2 + box.min.y;
    int num_x = (box.max.x - midline_x) / space;
    int num_y = (box.max.y - midline_y) / space;

    line_x.emplace_back(midline_x);
    line_y.emplace_back(midline_y);
    for (int i = 0; i < num_x; ++i) {
        line_x.emplace_back(midline_x + space * (i + 1));
        line_x.emplace_back(midline_x - space * (i + 1));
    }
    for (int j = 0; j < num_y; ++j) {
        line_y.emplace_back(midline_y + space * (j + 1));
        line_y.emplace_back(midline_y - space * (j + 1));
    }

    for each (const float& x in line_x) {
        for each (const float& y in line_y)
            points.emplace_back(stl_vertex{ x,y,0 });
    }
    return points;
}

BoundingBoxf TriangleMesh::face_to_2D_projection(const v_face_struct& faces)//特征面到二维投影
{
    BoundingBoxf box;
    for (int i = 0; i < faces.num; ++i) {
        stl_facet f = this->stl.facet_start[faces.v_shared_face[i]];

        if (i == 0) {
            box.min.x = f.vertex[0].x;
            box.min.y = f.vertex[0].y;
            box.max.x = f.vertex[0].x;
            box.max.y = f.vertex[0].y;
        }

        for (int j = 0; j < 3; ++j) {
            box.min.x = f.vertex[j].x < box.min.x ? f.vertex[j].x : box.min.x;
            box.min.y = f.vertex[j].y < box.min.y ? f.vertex[j].y : box.min.y;
            box.max.x = f.vertex[j].x > box.max.x ? f.vertex[j].x : box.max.x;
            box.max.y = f.vertex[j].y > box.max.y ? f.vertex[j].y : box.max.y;
        }
    }
    return box;
}

void TriangleMesh::feature_face_to_point(std::vector<v_face_struct>& feature_faces, float space, stl_vertexs& feature_faces_point, char extra)//特征面生成特征点
{
    if (extra == 90)
    {
        for each (const v_face_struct & faces in feature_faces)
        {
            if (faces.num <= 0)
                continue;

            //1.筛选出所有多边形的边
            std::vector<stl_edge> edges;
            for (int i = 0; i < faces.num; ++i) {
                int f = faces.v_shared_face[i];
                for (int j = 0; j < 3; ++j) {
                    //1.1依次选出每条边
                    stl_edge edge;
                    edge.p1 = this->stl.facet_start[f].vertex[j % 3];
                    edge.p2 = this->stl.facet_start[f].vertex[(j + 1) % 3];
                    edge.facet_number1 = f;
                    edge.facet_number2 = -1;

                    bool ret = false;
                    for (auto e = edges.begin(); e != edges.end(); ++e)
                    {
                        if (!equal_edge(edge, *e))
                            continue;

                         e->facet_number2 = f;
                         ret = true;
                         break;
                    }
                    if (!ret)
                        edges.emplace_back(edge);
                }
            }

            //2.筛选洞与边界的边
            std::vector<stl_edge> edges1;
            for each (const stl_edge & e in edges) {
                if (e.facet_number1 == -1 || e.facet_number2 == -1)
                    edges1.emplace_back(e);
            }

            //3.不知道
            bool ret = true;
            Polygons polygons;
            std::vector<stl_vertexs> polygons_v;//用来检测contours是否悬吊的数据
            while (ret) {
                std::list<stl_vertex> vertexs;
                for (auto e1 = edges1.begin(); e1 != edges1.end(); ++e1) {
                    if (e1->facet_number1 != -1 || e1->facet_number2 != -1) {
                        vertexs.emplace_back(e1->p1);
                        vertexs.emplace_back(e1->p2);
                        e1->facet_number1 = -1;
                        e1->facet_number2 = -1;
                        look_for_polygon(edges1, vertexs);
                        break;
                    }
                    //退出
                    if (e1 == edges1.end() - 1)
                        ret = false;
                }

                //检查是否闭合，生成polygon
                if (vertexs.size() < 3)
                    continue;

                if (equal_vertex(vertexs.back(), vertexs.front())) {
                    Points ps;
                    stl_vertexs polygon_v;
                    for each (const stl_vertex & v in vertexs) {
                        ps.emplace_back(Point::new_scale(v.x, v.y));
                        polygon_v.emplace_back(v);
                    }
                    polygons.emplace_back(Polygon(ps));
                    polygons_v.emplace_back(polygon_v);
                }
            }

            size_t dis = 0;
            for (auto p = polygons.begin(); p != polygons.end(); ++p) {
                for (auto p1 = polygons.begin(); p1 != polygons.end(); ++p1) {
                    if (p == p1)
                        continue;

                    if (p->contains(p1->points.front())) {
                        dis = std::distance(polygons.begin(), p);
                        break;
                    }
                }
            }

            Polygon contous = *(polygons.begin() + dis);
            Polygons polygons1, polygons2;
            polygons1.emplace_back(contous);
            polygons2.emplace_back(contous);

            stl_vertexs polygon_v = *(polygons_v.begin() + dis);

            //4.判断contous是否悬吊
            size_t cools = 0;
            for each (const stl_vertex & v in polygon_v) {
                for (int i = 0; i < this->stl.stats.shared_vertices; ++i) {
                    if (equal_vertex(this->stl.v_shared[i], v)) {
                        //得到包围当前点的环绕面
                        v_face_struct v_face = this->stl.v_shared_faces[i];
                        bool cool = true;

                        //3.筛选凸包点
                        for (int k = 0; k < v_face.num; ++k) {
                            bool ret2 = true;
                            for (int i = 0; i < faces.num; ++i) {
                                if (faces.v_shared_face[i] == v_face.v_shared_face[k]) {
                                    ret2 = false;
                                    break;
                                }
                            }
                            if (!ret2) continue;

                            //取出一个环绕面的点id
                            for (int j = 0; j < 3; ++j) {
                                //根据高度值的比较，判断筛选凸包。
                                if (this->stl.v_shared[this->stl.v_indices[v_face.v_shared_face[k]].vertex[j]].z < v.z) {
                                    cool = false;
                                    break;
                                }
                            }
                            if (!cool)break;
                        }

                        if (cool)
                            ++cools;
                    }
                }
            }

            float ratio;
            if (cools > polygon_v.size() / 2)ratio = 0.5;
            else ratio = space / 2 + 1;

            //向圆心偏移
            ret = true;
            size_t aaa = 0;
            Pointfs points;
            //在路径上插入支撑点
            while (ret) {
                if (aaa == 0)
                    polygons1 = offset(polygons1, -scale_(ratio));
                else 
                    polygons1 = offset(polygons1, -scale_(space));
                for(auto p=polygons1.begin();p!=polygons1.end();++p)
                ++aaa;

                points.clear();
                if (!polygons1.empty()) {
                    contous = *polygons1.begin();//只有一条多边形
                    Pointf beginP(Pointf(unscale(contous.points.front().x), unscale(contous.points.front().y)));
                    Pointf endP;
                    points.emplace_back(beginP);

                    float space1 = space;
                    contous.points.emplace_back(contous.points.front());
                    for (auto c = contous.points.begin() + 1; c != contous.points.end(); ++c) {
                        endP.x = unscale((*c).x);
                        endP.y = unscale((*c).y);
                        bool ret3 = true;
                        while (ret3) {
                            float lenght = DisPoint2(beginP, endP);
                            if (lenght >= space1) {
                                beginP = InterpolationPoint(beginP, endP, space1);
                                points.emplace_back(beginP);
                                space1 = space;
                            }
                            else {
                                beginP = endP;
                                space1 = space1 - lenght;
                                ret3 = false;
                            }
                        }
                    }
                }
                else
                {
                    Point p = contous.centroid();
                    points.emplace_back(Pointf(unscale(p.x), unscale(p.y)));
                    ret = false;
                }

                //删除接近的点
                for (auto p1 = points.begin(); p1 != points.end(); ++p1) {
                    for (auto p2 = p1 + 1; p2 != points.end(); ++p2) {
                        if (DisPoint2(*p1, *p2) < space / 2.) {
                            --p2;
                            points.erase(p2 + 1);
                        }
                    }
                }

                size_t count = 0;
                //判断采样点是否在特征面区域上
                for each (const Pointf & p in points) {
                    for (int i = 0; i < faces.num; ++i) {
                        stl_facet f = this->stl.facet_start[faces.v_shared_face[i]];

                        if ((p.x > f.vertex[0].x&& p.x > f.vertex[1].x&& p.x > f.vertex[2].x) ||
                            (p.x < f.vertex[0].x && p.x < f.vertex[1].x && p.x < f.vertex[2].x) ||
                            (p.y > f.vertex[0].y&& p.y > f.vertex[1].y&& p.y > f.vertex[2].y) ||
                            (p.y < f.vertex[0].y && p.y < f.vertex[1].y && p.y < f.vertex[2].y))
                            continue;
                        else
                        {
                            //xoy平面上的采样点向上延伸求交
                            Pointf3 _p(p.x, p.y, 0);
                            if (!PointInTriangle2(f, _p))
                                continue;
                            feature_faces_point.emplace_back(Pf32Ver(CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0]), Vectorf3(0, 0, 1), _p)));//存储采样点
                            ++count;
                            break;
                        }
                    }
                }

                if (count == 0 && aaa == 1) {
                    if (ratio > 0.1) {
                        ratio *= 0.8;
                        --aaa;
                    }
                    else
                        ratio = 0.0;
                    polygons1 = polygons2;
                }
            }
        }
    }
    else if (extra == 45)
    {
        for each (const v_face_struct & faces in feature_faces) {
            if (faces.num <= 0)
                continue;

            stl_vertexs points = balance_sampling_point(space, face_to_2D_projection(faces));//提取均匀采样点
            //判断采样点是否在特征面区域上
            for each (const stl_vertex & p in points) {
                for (int i = 0; i < faces.num; ++i) {
                    stl_facet f = this->stl.facet_start[faces.v_shared_face[i]];

                    if ((p.x > f.vertex[0].x&& p.x > f.vertex[1].x&& p.x > f.vertex[2].x) ||
                        (p.x < f.vertex[0].x && p.x < f.vertex[1].x && p.x < f.vertex[2].x) ||
                        (p.y > f.vertex[0].y&& p.y > f.vertex[1].y&& p.y > f.vertex[2].y) ||
                        (p.y < f.vertex[0].y && p.y < f.vertex[1].y && p.y < f.vertex[2].y))
                        continue;
                    else
                    {
                        //xoy平面上的采样点向上延伸求交
                        Pointf3 _p(p.x, p.y, 0);
                        if (!PointInTriangle2(f, _p))
                            continue;
                        feature_faces_point.emplace_back(Pf32Ver(CalPlaneLineIntersectPoint(Nor2Vt3(f.normal), Ver2Pf3(f.vertex[0]), Vectorf3(0, 0, 1), _p)));//存储采样点
                    }
                }
            }
        }
    }
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
        if ((p.x > f.vertex[0].x&& p.x > f.vertex[1].x&& p.x > f.vertex[2].x) ||
            (p.x < f.vertex[0].x && p.x < f.vertex[1].x && p.x < f.vertex[2].x) ||
            (p.y > f.vertex[0].y&& p.y > f.vertex[1].y&& p.y > f.vertex[2].y) ||
            (p.y < f.vertex[0].y && p.y < f.vertex[1].y && p.y < f.vertex[2].y) ||
            (p.z < f.vertex[0].z && p.z < f.vertex[1].z && p.z < f.vertex[2].z))
            continue;

        if (Ray2TrianglePoint(f, Vectorf3(0, 0, -1), p, c)) {
            if (p.z - c.z > 0.001)//参数化
                ps.emplace_back(c.z);
        }
    }

    if (ps.empty())
        return Pointf3(p.x, p.y, 0);

    bottom = *(ps.begin());
    for (auto v2 = ps.begin() + 1; v2 != ps.end(); ++v2) {
        if (bottom < *v2)
            bottom = *v2;
    }
    return Pointf3(p.x, p.y, bottom);
}

int TriangleMesh::find_common_edge_face(int face, int a, int b)//查找以ab为边另一个三角面片
{
    for (int i = 0; i < this->stl.v_shared_faces[a].num; ++i) {
        int f = this->stl.v_shared_faces[a].v_shared_face[i];
        if (f == face)
            continue;
        for (int j = 0; j < this->stl.v_shared_faces[b].num; ++j) {
            if (f == this->stl.v_shared_faces[b].v_shared_face[j])
                return f;
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/