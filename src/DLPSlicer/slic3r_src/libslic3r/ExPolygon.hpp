#pragma once
#ifndef slic3r_ExPolygon_hpp_
#define slic3r_ExPolygon_hpp_

#include "libslic3r.h"
#include "BoundingBox.hpp"
#include "Polygon.hpp"
#include "Polyline.hpp"
#include <ostream>
#include <vector>

namespace Slic3r {

class ExPolygon;
typedef std::vector<ExPolygon> ExPolygons;

class ExPolygon
{
    public:
    Polygon contour;
    Polygons holes;
    ExPolygon() {};
    explicit ExPolygon(const Polygon &_contour) : contour(_contour) {};
    operator Points() const;
    operator Polygons() const;
    void scale(double factor);
    void translate(double x, double y);
    void rotate(double angle);
    void rotate(double angle, const Point &center);
    double area() const;
    bool is_valid() const;
    bool contains(const Line &line) const;
    bool contains(const Polyline &polyline) const;
    bool contains(const Point &point) const;
    bool contains_b(const Point &point) const;
    bool has_boundary_point(const Point &point) const;
    BoundingBox bounding_box() const { return this->contour.bounding_box(); };
    void remove_vertical_collinear_points(coord_t tolerance);
    void simplify_p(double tolerance, Polygons* polygons) const;
    Polygons simplify_p(double tolerance) const;
    ExPolygons simplify(double tolerance) const;
    void simplify(double tolerance, ExPolygons* expolygons) const;
    void medial_axis(double max_width, double min_width, ThickPolylines* polylines) const;
    void medial_axis(double max_width, double min_width, Polylines* polylines) const;
    void get_trapezoids(Polygons* polygons) const;
    void get_trapezoids(Polygons* polygons, double angle) const;
    void get_trapezoids2(Polygons* polygons) const;
    void get_trapezoids2(Polygons* polygons, double angle) const;
    void triangulate(Polygons* polygons) const;
    void triangulate_pp(Polygons* polygons) const;
    void triangulate_p2t(Polygons* polygons) const;
    Lines lines() const;
    std::string dump_perl() const;
};

inline Polygons
to_polygons(const ExPolygons &expolygons)
{
    Polygons pp;
    for (ExPolygons::const_iterator ex = expolygons.begin(); ex != expolygons.end(); ++ex)
        append_to(pp, (Polygons)*ex);
    return pp;
}

inline ExPolygons
operator+(ExPolygons src1, const ExPolygons &src2) {
    append_to(src1, src2);
    return src1;
};

std::ostream& operator <<(std::ostream &s, const ExPolygons &expolygons);

}

// start Boost
#include <boost/polygon/polygon.hpp>
namespace boost { namespace polygon {
    template <>
        struct polygon_traits<ExPolygon> {
        typedef coord_t coordinate_type;
        typedef Points::const_iterator iterator_type;
        typedef Point point_type;

        // Get the begin iterator
        static inline iterator_type begin_points(const ExPolygon& t) {
            return t.contour.points.begin();
        }

        // Get the end iterator
        static inline iterator_type end_points(const ExPolygon& t) {
            return t.contour.points.end();
        }

        // Get the number of sides of the polygon
        static inline std::size_t size(const ExPolygon& t) {
            return t.contour.points.size();
        }

        // Get the winding direction of the polygon
        static inline winding_direction winding(const ExPolygon& t) {
            return unknown_winding;
        }
    };

    template <>
    struct polygon_mutable_traits<ExPolygon> {
        //expects stl style iterators
        template <typename iT>
        static inline ExPolygon& set_points(ExPolygon& expolygon, iT input_begin, iT input_end) {
            expolygon.contour.points.assign(input_begin, input_end);
            // skip last point since Boost will set last point = first point
            expolygon.contour.points.pop_back();
            return expolygon;
        }
    };
    
    
    template <>
    struct geometry_concept<ExPolygon> { typedef polygon_with_holes_concept type; };

    template <>
    struct polygon_with_holes_traits<ExPolygon> {
        typedef Polygons::const_iterator iterator_holes_type;
		typedef Slic3r::Polygon hole_type;
        static inline iterator_holes_type begin_holes(const ExPolygon& t) {
            return t.holes.begin();
        }
        static inline iterator_holes_type end_holes(const ExPolygon& t) {
            return t.holes.end();
        }
        static inline unsigned int size_holes(const ExPolygon& t) {
            return t.holes.size();
        }
    };

    template <>
    struct polygon_with_holes_mutable_traits<ExPolygon> {
         template <typename iT>
         static inline ExPolygon& set_holes(ExPolygon& t, iT inputBegin, iT inputEnd) {
              t.holes.assign(inputBegin, inputEnd);
              return t;
         }
    };
    
    //first we register CPolygonSet as a polygon set
    template <>
    struct geometry_concept<ExPolygons> { typedef polygon_set_concept type; };

    //next we map to the concept through traits
    template <>
    struct polygon_set_traits<ExPolygons> {
        typedef coord_t coordinate_type;
        typedef ExPolygons::const_iterator iterator_type;
        typedef ExPolygons operator_arg_type;

        static inline iterator_type begin(const ExPolygons& polygon_set) {
            return polygon_set.begin();
        }

        static inline iterator_type end(const ExPolygons& polygon_set) {
            return polygon_set.end();
        }

        //don't worry about these, just return false from them
        static inline bool clean(const ExPolygons& polygon_set) { return false; }
        static inline bool sorted(const ExPolygons& polygon_set) { return false; }
    };

    template <>
    struct polygon_set_mutable_traits<ExPolygons> {
        template <typename input_iterator_type>
        static inline void set(ExPolygons& expolygons, input_iterator_type input_begin, input_iterator_type input_end) {
            expolygons.assign(input_begin, input_end);
        }
    };
} }
// end Boost

#endif
