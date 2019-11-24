
#ifndef _libslic3r_h_
#define _libslic3r_h_

// this needs to be included early for MSVC (listing it in Build.PL is not enough)
#include <ostream>
#include <iostream>
#include <math.h>
#include <queue>
#include <sstream>
#include <vector>
#include <boost/thread.hpp>

/* Implementation of CONFESS("foo"): */
void confess_at(const char *file, int line, const char *func, const char *pat, ...);
#ifdef _MSC_VER
	#define CONFESS(...) confess_at(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
	#define CONFESS(...) confess_at(__FILE__, __LINE__, __func__, __VA_ARGS__)
#endif
/* End implementation of CONFESS("foo"): */

// Which C++ version is supported?
// For example, could optimized functions with move semantics be used?
#if __cplusplus==201402L
	#define SLIC3R_CPPVER 14
	#define STDMOVE(WHAT) std::move(WHAT)
#elif __cplusplus==201103L
	#define SLIC3R_CPPVER 11
	#define STDMOVE(WHAT) std::move(WHAT)
#else
	#define SLIC3R_CPPVER 0
	#define STDMOVE(WHAT) (WHAT)
#endif

// dummy macro to mark strings for translation for gettext/poedit
#define __TRANS(s) s
namespace Slic3r {

const auto SLIC3R_VERSION = "1.3.0-dev";

//*********************
//���ڣ�2018.3.13
//���ܣ�һ��ģ�͵�ʵ������
//**********************
const int InstanceNum = 1000;

typedef long coord_t;
typedef double coordf_t;

// Scaling factor for a conversion from coord_t to coordf_t: 10e-6
// This scaling generates a following fixed point representation with for a 32bit integer:
// 0..4294mm with 1nm resolution
const double SCALING_FACTOR = 0.000001;
const inline coord_t  scale_(const coordf_t &val) { return val / SCALING_FACTOR; }
const inline coordf_t unscale(const coord_t &val) { return val * SCALING_FACTOR; }

//FIXME This epsilon value is used for many non-related purposes:
// For a threshold of a squared Euclidean distance,
// for a trheshold in a difference of radians,
// for a threshold of a cross product of two non-normalized vectors etc.
const auto EPSILON = 1e-4;
const auto SCALED_EPSILON = scale_(EPSILON);
// RESOLUTION, SCALED_RESOLUTION: Used as an error threshold for a Douglas-Peucker polyline simplification algorithm.
const auto RESOLUTION = 0.0125;
const auto SCALED_RESOLUTION = scale_(RESOLUTION);
const auto PI = 3.141592653589793238;
// When extruding a closed loop, the loop is interrupted and shortened a bit to reduce the seam.
const auto LOOP_CLIPPING_LENGTH_OVER_NOZZLE_DIAMETER = 0.15;
// Maximum perimeter length for the loop to apply the small perimeter speed. 
const coord_t SMALL_PERIMETER_LENGTH = scale_(6.5) * 2 * PI;
const coordf_t INSET_OVERLAP_TOLERANCE = 0.4;
const coordf_t EXTERNAL_INFILL_MARGIN = 3;
const coord_t SCALED_EXTERNAL_INFILL_MARGIN = scale_(EXTERNAL_INFILL_MARGIN);

enum Axis { X=0, Y, Z };

template <class T>
inline void append_to(std::vector<T> &dst, const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

//template <class T> void
//_parallelize_do(std::queue<T>* queue, boost::mutex* queue_mutex, boost::function<void(T)> func)
//{
//    //std::cout << "THREAD STARTED: " << boost::this_thread::get_id() << std::endl;
//    while (true) {
//        T i;
//        {
//            boost::lock_guard<boost::mutex> l(*queue_mutex);
//            if (queue->empty()) return;
//            i = queue->front();
//            queue->pop();
//        }
//        //std::cout << "  Thread " << boost::this_thread::get_id() << " processing item " << i << std::endl;
//        func(i);
//        boost::this_thread::interruption_point();
//    }
//}

template <class T> void
_parallelize_do(std::queue<T>* queue, boost::mutex* queue_mutex, boost::function<void(T)> func)
{
	//std::cout << "THREAD STARTED: " << boost::this_thread::get_id() << std::endl;
	while (true) {
		T i;
		{
			boost::lock_guard<boost::mutex> l(*queue_mutex);
			if (queue->empty()) return;
			i = queue->front();
			queue->pop();
		}
		//std::cout << "  Thread " << boost::this_thread::get_id() << " processing item " << i << std::endl;
		//�쳣����
	BAD:
		try {
			func(i);
		}
		catch (std::bad_alloc& ba) {
			std::cout << ba.what() << std::endl;
			goto BAD;
			// exit(EXIT_FAILURE);
		}

		boost::this_thread::interruption_point();
	}
}

template <class T> void
parallelize(std::queue<T> queue, boost::function<void(T)> func,
    int threads_count = boost::thread::hardware_concurrency())
{
    if (threads_count == 0) threads_count = 2;
    boost::mutex queue_mutex;
    boost::thread_group workers;
    for (int i = 0; i < std::min(threads_count, (int)queue.size()); i++)
        workers.add_thread(new boost::thread(&_parallelize_do<T>, &queue, &queue_mutex, func));
    workers.join_all();
}

template <class T> void
parallelize(T start, T end, boost::function<void(T)> func,
    int threads_count = boost::thread::hardware_concurrency())
{
    std::queue<T> queue;
    for (T i = start; i <= end; ++i) queue.push(i);
    parallelize(queue, func, threads_count);
}

} // namespace Slic3r

using namespace Slic3r;

#endif
