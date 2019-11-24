#ifndef slic3r_FillPlanePath_hpp_
#define slic3r_FillPlanePath_hpp_

#include <map>

#include "../libslic3r.h"

#include "Fill.hpp"

namespace Slic3r {

// The original Perl code used path generators from Math::PlanePath library:
// http://user42.tuxfamily.org/math-planepath/
// http://user42.tuxfamily.org/math-planepath/gallery.html

class FillPlanePath : public Fill
{
public:
    virtual ~FillPlanePath() {}

protected:
    virtual void _fill_surface_single(
        unsigned int                     thickness_layers,
        const direction_t               &direction, 
        ExPolygon                       &expolygon, 
        Polylines*                      polylines_out);

    virtual float _layer_angle(size_t idx) const { return 0.f; }
    virtual bool  _centered() const = 0;
    virtual Pointfs _generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y) = 0;
};

class FillArchimedeanChords : public FillPlanePath
{
public:
    virtual Fill* clone() const { return new FillArchimedeanChords(*this); };
    virtual ~FillArchimedeanChords() {}
    virtual bool can_solid() const { return true; };

protected:
    virtual bool  _centered() const { return true; }
    virtual Pointfs _generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y);
};

class FillHilbertCurve : public FillPlanePath
{
public:
    virtual Fill* clone() const { return new FillHilbertCurve(*this); };
    virtual ~FillHilbertCurve() {}
    virtual bool can_solid() const { return true; };

protected:
    virtual bool  _centered() const { return false; }
    virtual Pointfs _generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y);
};

class FillOctagramSpiral : public FillPlanePath
{
public:
    virtual Fill* clone() const { return new FillOctagramSpiral(*this); };
    virtual ~FillOctagramSpiral() {}
    virtual bool can_solid() const { return true; };

protected:
    virtual bool  _centered() const { return true; }
    virtual Pointfs _generate(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y);
};

} // namespace Slic3r

#endif // slic3r_FillPlanePath_hpp_
