#pragma once
#include "ClipperUtils.hpp"
#include "Polygon.hpp"
#include "Polyline.hpp"

namespace DLPSlicer {

Polygon::operator Polygons() const
{
    Polygons pp;
    pp.push_back(*this);
    return pp;
}

Polygon::operator Polyline() const
{
    return this->split_at_first_point();
}

Point&
Polygon::operator[](Points::size_type idx)
{
    return this->points[idx];
}

const Point&
Polygon::operator[](Points::size_type idx) const
{
    return this->points[idx];
}

Point
Polygon::last_point() const
{
    return this->points.front();  // last point == first point for polygons
}

Lines
Polygon::lines() const
{
    Lines lines;
    lines.reserve(this->points.size());
    for (Points::const_iterator it = this->points.begin(); it != this->points.end()-1; ++it) {
        lines.push_back(Line(*it, *(it + 1)));
    }
    lines.push_back(Line(this->points.back(), this->points.front()));
    return lines;
}

Polyline
Polygon::split_at_vertex(const Point &point) const
{
    // find index of point
    for (Points::const_iterator it = this->points.begin(); it != this->points.end(); ++it) {
        if (it->coincides_with(point)) {
            return this->split_at_index(it - this->points.begin());
        }
    }
   // CONFESS("Point not found");
    return Polyline();
}

// Split a closed polygon into an open polyline, with the split point duplicated at both ends.
Polyline
Polygon::split_at_index(int index) const
{
    Polyline polyline;
    polyline.points.reserve(this->points.size() + 1);
    for (Points::const_iterator it = this->points.begin() + index; it != this->points.end(); ++it)
        polyline.points.push_back(*it);
    for (Points::const_iterator it = this->points.begin(); it != this->points.begin() + index + 1; ++it)
        polyline.points.push_back(*it);
    return polyline;
}

// Split a closed polygon into an open polyline, with the split point duplicated at both ends.
Polyline
Polygon::split_at_first_point() const
{
    return this->split_at_index(0);
}

Points
Polygon::equally_spaced_points(double distance) const
{
    return this->split_at_first_point().equally_spaced_points(distance);
}

double
Polygon::area() const
{
    return ClipperLib::Area(DLPSlicerMultiPoint_to_ClipperPath(*this));
}

bool
Polygon::is_counter_clockwise() const
{
    return ClipperLib::Orientation(DLPSlicerMultiPoint_to_ClipperPath(*this));
}

bool
Polygon::is_clockwise() const
{
    return !this->is_counter_clockwise();
}

bool
Polygon::make_counter_clockwise()
{
    if (!this->is_counter_clockwise()) {
        this->reverse();
        return true;
    }
    return false;
}

bool
Polygon::make_clockwise()
{
    if (this->is_counter_clockwise()) {
        this->reverse();
        return true;
    }
    return false;
}

bool
Polygon::is_valid() const
{
    return this->points.size() >= 3;
}

// Does an unoriented polygon contain a point?
// Tested by counting intersections along a horizontal line.
bool
Polygon::contains(const Point &point) const
{
    // http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
    bool result = false;
    Points::const_iterator i = this->points.begin();
    Points::const_iterator j = this->points.end() - 1;
    for (; i != this->points.end(); j = i++) {
        //FIXME this test is not numerically robust. Particularly, it does not handle horizontal segments at y == point.y well.
        // Does the ray with y == point.y intersect this line segment?
        if ( ((i->y > point.y) != (j->y > point.y))
            && ((double)point.x < (double)(j->x - i->x) * (double)(point.y - i->y) / (double)(j->y - i->y) + (double)i->x) )
            result = !result;
    }
    return result;
}

void
Polygon::douglas_peucker(double tolerance)
{
    this->points.push_back(this->points.front());
    this->points = MultiPoint::_douglas_peucker(this->points, tolerance);
    this->points.pop_back();
}

void
Polygon::remove_vertical_collinear_points(coord_t tolerance)
{
    Points &pp = this->points;
    pp.push_back(pp.front());
    for (size_t i = 0; i < pp.size()-1; ++i) {
        while (i < pp.size()-1) {
            const Point &p = pp[i];
            const Point &next = pp[i+1];
            if (next.x == p.x && std::abs(next.y - p.y) <= tolerance) {
                pp.erase(pp.begin() + i);
            } else {
                break;
            }
        }
    }
    pp.pop_back();
}

// this only works on CCW polygons as CW will be ripped out by Clipper's simplify_polygons()
Polygons
Polygon::simplify(double tolerance) const
{
    // repeat first point at the end in order to apply Douglas-Peucker
    // on the whole polygon
    Points points = this->points;
    points.push_back(points.front());
    Polygon p(MultiPoint::_douglas_peucker(points, tolerance));
    p.points.pop_back();
    
    return simplify_polygons(p);
}

void
Polygon::simplify(double tolerance, Polygons &polygons) const
{
    Polygons pp = this->simplify(tolerance);
    polygons.reserve(polygons.size() + pp.size());
    polygons.insert(polygons.end(), pp.begin(), pp.end());
}

// Only call this on convex polygons or it will return invalid results
void
Polygon::triangulate_convex(Polygons* polygons) const
{
    for (Points::const_iterator it = this->points.begin() + 2; it != this->points.end(); ++it) {
        Polygon p;
        p.points.reserve(3);
        p.points.push_back(this->points.front());
        p.points.push_back(*(it-1));
        p.points.push_back(*it);
        
        // this should be replaced with a more efficient call to a merge_collinear_segments() method
        if (p.area() > 0) polygons->push_back(p);
    }
}

// center of mass
Point
Polygon::centroid() const
{
    double area_temp = this->area();
    double x_temp = 0;
    double y_temp = 0;
    
    Polyline polyline = this->split_at_first_point();
    for (Points::const_iterator point = polyline.points.begin(); point != polyline.points.end() - 1; ++point) {
        x_temp += (double)( point->x + (point+1)->x ) * ( (double)point->x*(point+1)->y - (double)(point+1)->x*point->y );
        y_temp += (double)( point->y + (point+1)->y ) * ( (double)point->x*(point+1)->y - (double)(point+1)->x*point->y );
    }
    
    return Point(x_temp/(6*area_temp), y_temp/(6*area_temp));
}

std::string
Polygon::wkt() const
{
    std::ostringstream wkt;
    wkt << "POLYGON((";
    for (Points::const_iterator p = this->points.begin(); p != this->points.end(); ++p) {
        wkt << p->x << " " << p->y;
        if (p != this->points.end()-1) wkt << ",";
    }
    wkt << "))";
    return wkt.str();
}

// find all concave vertices (i.e. having an internal angle greater than the supplied angle)
// (external = right side, thus we consider ccw orientation)
Points
Polygon::concave_points(double angle) const
{
    angle = 2*PI - angle + EPSILON;
    const Points &pp = this->points;
    Points concave;
    
    // check whether first point forms a concave angle
    if (pp.front().ccw_angle(pp.back(), *(pp.begin()+1)) <= angle)
        concave.push_back(pp.front());
    
    // check whether points 1..(n-1) form concave angles
    for (Points::const_iterator p = pp.begin()+1; p != pp.end()-1; ++p)
        if (p->ccw_angle(*(p-1), *(p+1)) <= angle)
            concave.push_back(*p);
    
    // check whether last point forms a concave angle
    if (pp.back().ccw_angle(*(pp.end()-2), pp.front()) <= angle)
        concave.push_back(pp.back());
    
    return concave;
}

// find all convex vertices (i.e. having an internal angle smaller than the supplied angle)
// (external = right side, thus we consider ccw orientation)
Points
Polygon::convex_points(double angle) const
{
    angle = 2*PI - angle - EPSILON;
    const Points &pp = this->points;
    Points convex;
    
    // check whether first point forms a convex angle
    if (pp.front().ccw_angle(pp.back(), *(pp.begin()+1)) >= angle)
        convex.push_back(pp.front());
    
    // check whether points 1..(n-1) form convex angles
    for (Points::const_iterator p = pp.begin()+1; p != pp.end()-1; ++p)
        if (p->ccw_angle(*(p-1), *(p+1)) >= angle)
            convex.push_back(*p);
    
    // check whether last point forms a convex angle
    if (pp.back().ccw_angle(*(pp.end()-2), pp.front()) >= angle)
        convex.push_back(pp.back());
    
    return convex;
}

}
