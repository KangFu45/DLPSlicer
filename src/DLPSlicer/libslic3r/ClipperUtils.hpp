#pragma once

#include <libslic3r.h>
#include "clipper.hpp"
#include "ExPolygon.hpp"
#include "Polygon.hpp"
//#include "Surface.hpp"

// import these wherever we're included
using ClipperLib::jtMiter;
using ClipperLib::jtRound;
using ClipperLib::jtSquare;

namespace DLPSlicer {

// Factor to convert from coord_t (which is int32) to an int64 type used by the Clipper library.
//FIXME Vojtech: Better to use a power of 2 coefficient and to use bit shifts for scaling.
// How about 2^17=131072?
// By the way, is the scalling needed at all? Cura runs all the computation with a fixed point precision of 1um, while DLPSlicer scales to 1nm,
// further scaling by 10e5 brings us to 
const float CLIPPER_OFFSET_SCALE = 100000.0;
const auto MAX_COORD = ClipperLib::hiRange / CLIPPER_OFFSET_SCALE;

//-----------------------------------------------------------
// legacy code from Clipper documentation
void AddOuterPolyNodeToExPolygons(ClipperLib::PolyNode& polynode, DLPSlicer::ExPolygons& expolygons);
void PolyTreeToExPolygons(ClipperLib::PolyTree& polytree, DLPSlicer::ExPolygons& expolygons);
//-----------------------------------------------------------

ClipperLib::Path DLPSlicerMultiPoint_to_ClipperPath(const DLPSlicer::MultiPoint &input);
template <class T>
ClipperLib::Paths DLPSlicerMultiPoints_to_ClipperPaths(const T &input);
template <class T>
T ClipperPath_to_DLPSlicerMultiPoint(const ClipperLib::Path &input);
template <class T>
T ClipperPaths_to_DLPSlicerMultiPoints(const ClipperLib::Paths &input);
DLPSlicer::ExPolygons ClipperPaths_to_DLPSlicerExPolygons(const ClipperLib::Paths &input);

void scaleClipperPolygons(ClipperLib::Paths &polygons, const double scale);

// offset Polygons
ClipperLib::Paths _offset(const DLPSlicer::Polygons &polygons, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);
DLPSlicer::Polygons offset(const DLPSlicer::Polygons &polygons, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);

// offset Polylines
ClipperLib::Paths _offset(const DLPSlicer::Polylines &polylines, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtSquare, 
    double miterLimit = 3);
DLPSlicer::Polygons offset(const DLPSlicer::Polylines &polylines, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtSquare, 
    double miterLimit = 3);
//DLPSlicer::Surfaces offset(const DLPSlicer::Surface &surface, const float delta,
//    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtSquare, 
//    double miterLimit = 3);

DLPSlicer::ExPolygons offset_ex(const DLPSlicer::Polygons &polygons, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);
DLPSlicer::ExPolygons offset_ex(const DLPSlicer::ExPolygons &expolygons, const float delta,
    double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);

ClipperLib::Paths _offset2(const DLPSlicer::Polygons &polygons, const float delta1,
    const float delta2, double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);
DLPSlicer::Polygons offset2(const DLPSlicer::Polygons &polygons, const float delta1,
    const float delta2, double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);
DLPSlicer::ExPolygons offset2_ex(const DLPSlicer::Polygons &polygons, const float delta1,
    const float delta2, double scale = CLIPPER_OFFSET_SCALE, ClipperLib::JoinType joinType = ClipperLib::jtMiter, 
    double miterLimit = 3);

template <class T>
T _clipper_do(ClipperLib::ClipType clipType, const DLPSlicer::Polygons &subject, 
    const DLPSlicer::Polygons &clip, const ClipperLib::PolyFillType fillType, bool safety_offset_ = false);

ClipperLib::PolyTree _clipper_do(ClipperLib::ClipType clipType, const DLPSlicer::Polylines &subject, 
    const DLPSlicer::Polygons &clip, const ClipperLib::PolyFillType fillType, bool safety_offset_ = false);

DLPSlicer::Polygons _clipper(ClipperLib::ClipType clipType,
    const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false);
DLPSlicer::ExPolygons _clipper_ex(ClipperLib::ClipType clipType,
    const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false);
DLPSlicer::Polylines _clipper_pl(ClipperLib::ClipType clipType,
    const DLPSlicer::Polylines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false);
DLPSlicer::Polylines _clipper_pl(ClipperLib::ClipType clipType,
    const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false);
DLPSlicer::Lines _clipper_ln(ClipperLib::ClipType clipType,
    const DLPSlicer::Lines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false);

// diff
inline DLPSlicer::Polygons
diff(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctDifference, subject, clip, safety_offset_);
}

inline DLPSlicer::ExPolygons
diff_ex(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctDifference, subject, clip, safety_offset_);
}

inline DLPSlicer::ExPolygons
diff_ex(const DLPSlicer::ExPolygons &subject, const DLPSlicer::ExPolygons &clip, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctDifference, to_polygons(subject), to_polygons(clip), safety_offset_);
}

inline DLPSlicer::Polygons
diff(const DLPSlicer::ExPolygons &subject, const DLPSlicer::ExPolygons &clip, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctDifference, to_polygons(subject), to_polygons(clip), safety_offset_);
}

inline DLPSlicer::Polylines
diff_pl(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_pl(ClipperLib::ctDifference, subject, clip, safety_offset_);
}

inline DLPSlicer::Polylines
diff_pl(const DLPSlicer::Polylines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_pl(ClipperLib::ctDifference, subject, clip, safety_offset_);
}

inline DLPSlicer::Lines
diff_ln(const DLPSlicer::Lines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_ln(ClipperLib::ctDifference, subject, clip, safety_offset_);
}

// intersection
inline DLPSlicer::Polygons
intersection(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctIntersection, subject, clip, safety_offset_);
}

inline DLPSlicer::ExPolygons
intersection_ex(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctIntersection, subject, clip, safety_offset_);
}

inline DLPSlicer::ExPolygons
intersection_ex(const DLPSlicer::ExPolygons &subject, const DLPSlicer::ExPolygons &clip, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctIntersection, to_polygons(subject), to_polygons(clip), safety_offset_);
}

inline DLPSlicer::Polygons
intersection(const DLPSlicer::ExPolygons &subject, const DLPSlicer::ExPolygons &clip, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctIntersection, to_polygons(subject), to_polygons(clip), safety_offset_);
}

inline DLPSlicer::Polylines
intersection_pl(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_pl(ClipperLib::ctIntersection, subject, clip, safety_offset_);
}

inline DLPSlicer::Polylines
intersection_pl(const DLPSlicer::Polylines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_pl(ClipperLib::ctIntersection, subject, clip, safety_offset_);
}

inline DLPSlicer::Lines
intersection_ln(const DLPSlicer::Lines &subject, const DLPSlicer::Polygons &clip, bool safety_offset_ = false)
{
    return _clipper_ln(ClipperLib::ctIntersection, subject, clip, safety_offset_);
}

// union
inline DLPSlicer::Polygons
union_(const DLPSlicer::Polygons &subject, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctUnion, subject, DLPSlicer::Polygons(), safety_offset_);
}

inline DLPSlicer::Polygons
union_(const DLPSlicer::Polygons &subject, const DLPSlicer::Polygons &subject2, bool safety_offset_ = false)
{
    return _clipper(ClipperLib::ctUnion, subject, subject2, safety_offset_);
}

inline DLPSlicer::ExPolygons
union_ex(const DLPSlicer::Polygons &subject, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctUnion, subject, DLPSlicer::Polygons(), safety_offset_);
}

inline DLPSlicer::ExPolygons
union_ex(const DLPSlicer::ExPolygons &subject, bool safety_offset_ = false)
{
    return _clipper_ex(ClipperLib::ctUnion, to_polygons(subject), DLPSlicer::Polygons(), safety_offset_);
}

//inline DLPSlicer::ExPolygons
//union_ex(const DLPSlicer::Surfaces &subject, bool safety_offset_ = false)
//{
//    return _clipper_ex(ClipperLib::ctUnion, to_polygons(subject), DLPSlicer::Polygons(), safety_offset_);
//}


ClipperLib::PolyTree union_pt(const DLPSlicer::Polygons &subject, bool safety_offset_ = false);
DLPSlicer::Polygons union_pt_chained(const DLPSlicer::Polygons &subject, bool safety_offset_ = false);
void traverse_pt(ClipperLib::PolyNodes &nodes, DLPSlicer::Polygons* retval);

/* OTHER */
DLPSlicer::Polygons simplify_polygons(const DLPSlicer::Polygons &subject, bool preserve_collinear = false);
DLPSlicer::ExPolygons simplify_polygons_ex(const DLPSlicer::Polygons &subject, bool preserve_collinear = false);

void safety_offset(ClipperLib::Paths* paths);

}
