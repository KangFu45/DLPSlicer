#pragma once
#ifndef slic3r_SVG_hpp_
#define slic3r_SVG_hpp_

#include "libslic3r.h"
#include "clipper.hpp"
#include "ExPolygon.hpp"
#include "Line.hpp"
#include "TriangleMesh.hpp"
#include "Surface.hpp"

namespace Slic3r {

class SVG
{
    public:
    bool arrows;
    std::string fill, stroke;
    Point origin;
    bool flipY;

    SVG(const char* afilename) :
        arrows(false), fill("grey"), stroke("black"), filename(afilename), flipY(false)
        { open(filename); }
    SVG(const char* afilename, const BoundingBox &bbox, const coord_t bbox_offset = scale_(1.), bool aflipY = false) : 
        arrows(false), fill("grey"), stroke("black"), filename(afilename), origin(bbox.min - Point(bbox_offset, bbox_offset)), flipY(aflipY)
        { open(filename, bbox, bbox_offset, aflipY); }
    SVG(const std::string &filename) :
        arrows(false), fill("grey"), stroke("black"), filename(filename), flipY(false)
        { open(filename); }
    SVG(const std::string &filename, const BoundingBox &bbox, const coord_t bbox_offset = scale_(1.), bool aflipY = false) : 
        arrows(false), fill("grey"), stroke("black"), filename(filename), origin(bbox.min - Point(bbox_offset, bbox_offset)), flipY(aflipY)
        { open(filename, bbox, bbox_offset, aflipY); }
    ~SVG() { if (f != NULL) Close(); }

    bool open(const char* filename);
    bool open(const char* filename, const BoundingBox &bbox, const coord_t bbox_offset = scale_(1.), bool flipY = false);
    bool open(const std::string &filename) 
        { return open(filename.c_str()); }
    bool open(const std::string &filename, const BoundingBox &bbox, const coord_t bbox_offset = scale_(1.), bool flipY = false)
        { return open(filename.c_str(), bbox, bbox_offset, flipY); }

    void draw(const Line &line, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const ThickLine &line, const std::string &fill, const std::string &stroke, coordf_t stroke_width = 0);
    void draw(const Lines &lines, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const IntersectionLines &lines, std::string stroke = "black");

    void draw(const ExPolygon &expolygon, std::string fill = "grey", const float fill_opacity=1.f);
    void draw_outline(const ExPolygon &polygon, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);
    void draw(const ExPolygons &expolygons, std::string fill = "grey", const float fill_opacity=1.f);
    void draw_outline(const ExPolygons &polygons, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);

    void draw(const Surface &surface, std::string fill = "grey", const float fill_opacity=1.f);
    void draw_outline(const Surface &surface, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);
    void draw(const Surfaces &surfaces, std::string fill = "grey", const float fill_opacity=1.f);
    void draw_outline(const Surfaces &surfaces, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);
    void draw(const SurfacesPtr &surfaces, std::string fill = "grey", const float fill_opacity=1.f);
    void draw_outline(const SurfacesPtr &surfaces, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);
 
    void draw(const Polygon &polygon, std::string fill = "grey");
    void draw_outline(const Polygon &polygon, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const Polygons &polygons, std::string fill = "grey");
    void draw_outline(const Polygons &polygons, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const Polyline &polyline, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const Polylines &polylines, std::string stroke = "black", coordf_t stroke_width = 0);
    void draw(const ThickLines &thicklines, const std::string &fill = "lime", const std::string &stroke = "black", coordf_t stroke_width = 0);
    void draw(const ThickPolylines &polylines, const std::string &stroke = "black", coordf_t stroke_width = 0);
    void draw(const ThickPolylines &thickpolylines, const std::string &fill, const std::string &stroke, coordf_t stroke_width);
    void draw(const Point &point, std::string fill = "black", coord_t radius = 0);
    void draw(const Points &points, std::string fill = "black", coord_t radius = 0);

    // Support for rendering the ClipperLib paths
    void draw(const ClipperLib::Path  &polygon, double scale, std::string fill = "grey", coordf_t stroke_width = 0);
    void draw(const ClipperLib::Paths &polygons, double scale, std::string fill = "grey", coordf_t stroke_width = 0);

    void draw_text(const Point &pt, const char *text, const char *color);
    void draw_legend(const Point &pt, const char *text, const char *color);

    void Close();
    
    private:
    std::string filename;
    FILE* f;
    
    void path(const std::string &d, bool fill, coordf_t stroke_width, const float fill_opacity);
    std::string get_path_d(const MultiPoint &mp, bool closed = false) const;
    std::string get_path_d(const ClipperLib::Path &mp, double scale, bool closed = false) const;

public:
    static void export_expolygons(const char *path, const BoundingBox &bbox, const Slic3r::ExPolygons &expolygons, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0);
    static void export_expolygons(const std::string &path, const BoundingBox &bbox, const Slic3r::ExPolygons &expolygons, std::string stroke_outer = "black", std::string stroke_holes = "blue", coordf_t stroke_width = 0) 
        { export_expolygons(path.c_str(), bbox, expolygons, stroke_outer, stroke_holes, stroke_width); }
};

}

#endif
