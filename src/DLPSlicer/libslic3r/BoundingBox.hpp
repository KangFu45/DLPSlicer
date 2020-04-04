#pragma once

#include "libslic3r.h"
#include "Point.hpp"
#include "Polygon.hpp"

namespace DLPSlicer {

typedef Point   Size;
typedef Point3  Size3;
typedef Pointf  Sizef;
typedef Pointf3 Sizef3;

template <class PointClass>
class BoundingBoxBase
{
    public:
    PointClass min;
    PointClass max;
    bool defined;
    
    BoundingBoxBase() : defined(false) {};
   // BoundingBoxBase(const PointClass &pmin, const PointClass &pmax) : 
   //     min(pmin), max(pmax), defined(pmin.x < pmax.x && pmin.y < pmax.y) {};
    BoundingBoxBase(const std::vector<PointClass> &points);
    void merge(const PointClass &point);
    void merge(const std::vector<PointClass> &points);
    void merge(const BoundingBoxBase<PointClass> &bb);
    void scale(double factor);
    PointClass size() const;
    double radius() const;
    void translate(coordf_t x, coordf_t y);
    void offset(coordf_t delta);
    PointClass center() const;
    bool contains(const PointClass &point) const;
};

template <class PointClass>
class BoundingBox3Base : public BoundingBoxBase<PointClass>
{
    public:
    BoundingBox3Base() : BoundingBoxBase<PointClass>() {};
  //  BoundingBox3Base(const PointClass &pmin, const PointClass &pmax) : 
  //      BoundingBoxBase<PointClass>(pmin, pmax) 
  //      { if (pmin.z >= pmax.z) BoundingBoxBase<PointClass>::defined = false; }
    BoundingBox3Base(const std::vector<PointClass> &points);
    void merge(const PointClass &point);
    void merge(const std::vector<PointClass> &points);
    void merge(const BoundingBox3Base<PointClass> &bb);
    PointClass size() const;
    double radius() const;
    void translate(coordf_t x, coordf_t y, coordf_t z);
    void offset(coordf_t delta);
    PointClass center() const;
};

class BoundingBox : public BoundingBoxBase<Point>
{
    public:
    void polygon(Polygon* polygon) const;
    Polygon polygon() const;
    BoundingBox rotated(double angle) const;
    BoundingBox rotated(double angle, const Point &center) const;
    void rotate(double angle) {
        *this = this->rotated(angle);
    }
    void rotate(double angle, const Point &center) {
        *this = this->rotated(angle, center);
    }
    
    BoundingBox() : BoundingBoxBase<Point>() {};
   // BoundingBox(const Point &pmin, const Point &pmax) : BoundingBoxBase<Point>(pmin, pmax) {};
    BoundingBox(const Points &points) : BoundingBoxBase<Point>(points) {};
    BoundingBox(const Lines &lines);
};

/*
class BoundingBox3  : public BoundingBox3Base<Point3> {};
*/

class BoundingBoxf : public BoundingBoxBase<Pointf> {
    public:
    BoundingBoxf() : BoundingBoxBase<Pointf>() {};
  //  BoundingBoxf(const Pointf &pmin, const Pointf &pmax) : BoundingBoxBase<Pointf>(pmin, pmax) {};
    BoundingBoxf(const std::vector<Pointf> &points) : BoundingBoxBase<Pointf>(points) {};
};

class BoundingBoxf3 : public BoundingBox3Base<Pointf3> {
    public:
    BoundingBoxf3() : BoundingBox3Base<Pointf3>() {};
  //  BoundingBoxf3(const Pointf3 &pmin, const Pointf3 &pmax) : BoundingBox3Base<Pointf3>(pmin, pmax) {};
    BoundingBoxf3(const std::vector<Pointf3> &points) : BoundingBox3Base<Pointf3>(points) {};
};

template<typename VT>
inline bool operator==(const BoundingBoxBase<VT> &bb1, const BoundingBoxBase<VT> &bb2)
{
    return bb1.min == bb2.min && bb1.max == bb2.max;
}

template<typename VT>
inline bool operator!=(const BoundingBoxBase<VT> &bb1, const BoundingBoxBase<VT> &bb2)
{
    return !(bb1 == bb2);
}

}
