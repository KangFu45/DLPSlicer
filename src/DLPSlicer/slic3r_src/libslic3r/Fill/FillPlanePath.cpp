#include "../ClipperUtils.hpp"
#include "../PolylineCollection.hpp"
#include "../Surface.hpp"

#include "FillPlanePath.hpp"

namespace Slic3r {

void FillPlanePath::_fill_surface_single(
    unsigned int                     thickness_layers,
    const direction_t               &direction, 
    ExPolygon                       &expolygon, 
    Polylines*                      polylines_out)
{
    expolygon.rotate(-direction.first);

    const coord_t distance_between_lines = scale_(this->min_spacing) / this->density;
    
    // align infill across layers using the object's bounding box (if available)
    BoundingBox bounding_box = this->bounding_box.defined
        ? this->bounding_box
        : expolygon.contour.bounding_box();
    bounding_box = bounding_box.rotated(-direction.first);
    
    const Point shift = this->_centered()
        ? bounding_box.center()
        : bounding_box.min;
    expolygon.translate(-shift.x, -shift.y);
    bounding_box.translate(-shift.x, -shift.y);

    const Pointfs pts = this->_generate(
        coord_t(ceil(coordf_t(bounding_box.min.x) / distance_between_lines)),
        coord_t(ceil(coordf_t(bounding_box.min.y) / distance_between_lines)),
        coord_t(ceil(coordf_t(bounding_box.max.x) / distance_between_lines)),
        coord_t(ceil(coordf_t(bounding_box.max.y) / distance_between_lines))
    );

    Polylines polylines;
    if (pts.size() >= 2) {
        // Convert points to a polyline, upscale.
        polylines.push_back(Polyline());
        Polyline &polyline = polylines.back();
        polyline.points.reserve(pts.size());
        for (Pointfs::const_iterator it = pts.begin(); it != pts.end(); ++ it) {
            polyline.points.push_back(Point(
                coord_t(floor(it->x * distance_between_lines + 0.5)), 
                coord_t(floor(it->y * distance_between_lines + 0.5))
            ));
        }
//      polylines = intersection_pl(polylines_src, offset((Polygons)expolygon, scale_(0.02)));
        polylines = intersection_pl(polylines, (Polygons)expolygon);
        
        // Extend paths in order to ensure overlap with perimeters
        for (Polyline &p : polylines) {
            p.extend_start(this->endpoints_overlap);
            p.extend_end(this->endpoints_overlap);
        }
        
/*        
        if (1) {
            require "Slic3r/SVG.pm";
            print "Writing fill.svg\n";
            Slic3r::SVG::output("fill.svg",
                no_arrows       => 1,
                polygons        => \@$expolygon,
                green_polygons  => [ $bounding_box->polygon ],
                polylines       => [ $polyline ],
                red_polylines   => \@paths,
            );
        }
*/
        
        // paths must be repositioned and rotated back
        for (Polylines::iterator it = polylines.begin(); it != polylines.end(); ++ it) {
            it->translate(shift.x, shift.y);
            it->rotate(direction.first);
        }
    }

    // Move the polylines to the output, avoid a deep copy.
    size_t j = polylines_out->size();
    polylines_out->resize(j + polylines.size(), Polyline());
    for (size_t i = 0; i < polylines.size(); ++ i)
        std::swap((*polylines_out)[j++], polylines[i]);
}

// Follow an Archimedean spiral, in polar coordinates: r=a+b\theta
Pointfs FillArchimedeanChords::_generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y)
{
    // Radius to achieve.
    const coordf_t rmax = std::sqrt(coordf_t(max_x)*coordf_t(max_x)+coordf_t(max_y)*coordf_t(max_y)) * std::sqrt(2.) + 1.5;
    // Now unwind the spiral.
    const coordf_t a = 1.;
    const coordf_t b = 1./(2.*PI);
    coordf_t theta = 0.;
    coordf_t r = 1;
    Pointfs out;
    //FIXME Vojtech: If used as a solid infill, there is a gap left at the center.
    out.push_back(Pointf(0, 0));
    out.push_back(Pointf(1, 0));
    while (r < rmax) {
        theta += 1. / r;
        r = a + b * theta;
        out.push_back(Pointf(r * cos(theta), r * sin(theta)));
    }
    return out;
}

// Adapted from 
// http://cpansearch.perl.org/src/KRYDE/Math-PlanePath-122/lib/Math/PlanePath/HilbertCurve.pm
//
// state=0    3--2   plain
//               |
//            0--1
//
// state=4    1--2  transpose
//            |  |
//            0  3
//
// state=8
//
// state=12   3  0  rot180 + transpose
//            |  |
//            2--1
//
static inline Point hilbert_n_to_xy(const size_t n)
{
    static const int next_state[16] = { 4,0,0,12, 0,4,4,8, 12,8,8,4, 8,12,12,0 };
    static const int digit_to_x[16] = { 0,1,1,0, 0,0,1,1, 1,0,0,1, 1,1,0,0 };
    static const int digit_to_y[16] = { 0,0,1,1, 0,1,1,0, 1,1,0,0, 1,0,0,1 };

    // Number of 2 bit digits.
    size_t ndigits = 0;
    {
        size_t nc = n;
        while(nc > 0) {
            nc >>= 2;
            ++ ndigits;
        }
    }
    int state    = (ndigits & 1) ? 4 : 0;
    int dirstate = (ndigits & 1) ? 0 : 4;
    coord_t x = 0;
    coord_t y = 0;
    for (int i = (int)ndigits - 1; i >= 0; -- i) {
        int digit = (n >> (i * 2)) & 3;
        state += digit;
        if (digit != 3)
            dirstate = state; // lowest non-3 digit
        x |= digit_to_x[state] << i;
        y |= digit_to_y[state] << i;
        state = next_state[state];
    }
    return Point(x, y);
}

Pointfs FillHilbertCurve::_generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y)
{
    // Minimum power of two square to fit the domain.
    size_t sz = 2;
    size_t pw = 1;
    {
        size_t sz0 = std::max(max_x + 1 - min_x, max_y + 1 - min_y);
        while (sz < sz0) {
            sz = sz << 1;
            ++pw;
        }
    }

    const size_t sz2 = sz * sz;
    Pointfs line;
    line.reserve(sz2);
    for (size_t i = 0; i < sz2; ++ i) {
        Point p = hilbert_n_to_xy(i);
        line.push_back(Pointf(p.x + min_x, p.y + min_y));
    }
    return line;
}

Pointfs FillOctagramSpiral::_generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y)
{
    // Radius to achieve.
    const coordf_t rmax = std::sqrt(coordf_t(max_x)*coordf_t(max_x)+coordf_t(max_y)*coordf_t(max_y)) * std::sqrt(2.) + 1.5;
    // Now unwind the spiral.
    coordf_t r = 0;
    const coordf_t r_inc = sqrt(2.);
    Pointfs out;
    out.push_back(Pointf(0, 0));
    while (r < rmax) {
        r += r_inc;
        coordf_t rx = r / sqrt(2.);
        coordf_t r2 = r + rx;
        out.push_back(Pointf( r,  0.));
        out.push_back(Pointf( r2, rx));
        out.push_back(Pointf( rx, rx));
        out.push_back(Pointf( rx, r2));
        out.push_back(Pointf(0.,  r));
        out.push_back(Pointf(-rx, r2));
        out.push_back(Pointf(-rx, rx));
        out.push_back(Pointf(-r2, rx));
        out.push_back(Pointf(-r,  0.));
        out.push_back(Pointf(-r2, -rx));
        out.push_back(Pointf(-rx, -rx));
        out.push_back(Pointf(-rx, -r2));
        out.push_back(Pointf(0., -r));
        out.push_back(Pointf( rx, -r2));
        out.push_back(Pointf( rx, -rx));
        out.push_back(Pointf( r2+r_inc, -rx));
    }
    return out;
}

} // namespace Slic3r
