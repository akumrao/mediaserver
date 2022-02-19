#include<iostream>
#include<vector>
#include<memory>
#include<chrono>

#include<boost/circular_buffer.hpp>

#include<opencv2/core.hpp>

#ifdef _MV_DEBUG_
#include<opencv2/imgproc.hpp>
#endif /* _MV_DEBUG_ */

extern "C" {
#include<libavutil/motion_vector.h>
}

#pragma once



#ifndef H264_MVD_H
#define H264_MVD_H


#ifdef _MV_DEBUG_
void draw_single_arrow(cv::Mat& img, const cv::Point& pStart, const cv::Point& pEnd, cv::Scalar startColor);
#endif /* _MV_DEBUG_ */

struct Measure {

    explicit Measure(const std::string& t) : _t(t) {
        reset();
    }

    void reset() {
        _st = std::chrono::steady_clock::now();
    }

    double elapsed() {
        std::chrono::steady_clock::duration et_ = std::chrono::steady_clock::now() - _st;
        return ::std::chrono::duration_cast< std::chrono::duration< double > >(et_).count();
    }
    std::string _t;
    std::chrono::steady_clock::time_point _st;
};

class MVFrame {
    friend class MVDetector;
public:
    MVFrame(
            int frame_index,
            int64_t pts,
            char pict_type,
            char origin,
            size_t grid_step,
            const std::pair<size_t, size_t>& shape, /* (cols, rows) (width, height) */
            // first = cols; second = rows
            const std::vector<AVMotionVector>& motion_vectors);
    // draw arrows overlay for debug
    void interpolate_flow(const MVFrame& prev, const MVFrame& next);
#ifdef _MV_DEBUG_
    void draw_arrows(cv::Mat& img);
    void draw_occupancy(cv::Mat& image);
#endif /* _MV_DEBUG_ */

    bool empty() const {
        return _is_empty;
    }

    // move ctor

    MVFrame(MVFrame&& other)
    : _frame_index(other._frame_index),
    _pts(other._pts),
    _pict_type(other._pict_type),
    _origin(other._origin),
    _grid_step(other._grid_step),
    _cols(other._cols), _rows(other._rows),
    // cp ctor, Mat is ref counted
    //_dx(other._dx), _dy(other._dy),
    _mv(other._mv),
    _is_empty(other._is_empty),
    _occupancy(other._occupancy) {
        //other._dx.release();
        //other._dy.release();
        other._mv.release();
    }
    MVFrame& operator=(MVFrame&&);

private:
    int _frame_index;
    int64_t _pts;
    char _pict_type;
    char _origin;
    size_t _grid_step;
    size_t _rows, _cols;
    //cv::Mat _dx, _dy;
    cv::Mat _mv;
    bool _is_empty;
    // occupancy type (0: empty, 1: filled, 2: interpolated)
    cv::Mat _occupancy;

    void set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors);
    void fill_in_mvs_grid8();
};

class MVDetector {
    constexpr static int DEFAULT_WINDOW_SIZE = 3;
    constexpr static float DEFAULT_OCCUPANCY_THRESHOLD = 2; // 3 %
    constexpr static float DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD = 0.6;
    constexpr static float DEFAULT_OCCUPANCY_AVG_THRESHOLD = 0.6;
public:

    MVDetector(
            const std::pair<size_t, size_t>& frame_shape,
            size_t window_size = MVDetector::DEFAULT_WINDOW_SIZE,
            float motion_occupancy_threshold = MVDetector::DEFAULT_OCCUPANCY_THRESHOLD,
            float occupancy_local_avg_threshold = MVDetector::DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD,
            float occupancy_avg_threshold = MVDetector::DEFAULT_OCCUPANCY_AVG_THRESHOLD,
            bool force_grid_8 = true) :
    _frame_shape(frame_shape),
    _grid_step(force_grid_8 ? 8 : 16),
    _grid_shape(std::pair<size_t, size_t>(_frame_shape.first / _grid_step, _frame_shape.second / _grid_step)),
    _window_size(window_size),
    _cb(_window_size),
    _mcb(_window_size * 2), // TODO
    _square_dist(float(_frame_shape.first*_frame_shape.first + _frame_shape.second*_frame_shape.second)),
    _motion_occupancy_threshold(motion_occupancy_threshold),
    _occupancy_pct(std::min(1.0, _motion_occupancy_threshold + 0.1)),
    _avg_movement(false),
    _occupancy(cv::Mat((int) _grid_shape.second, (int) _grid_shape.first, CV_32F, cv::Scalar(0.0))),
    _occupancy_local_avg_threshold(occupancy_local_avg_threshold),
    _occupancy_avg_threshold(occupancy_avg_threshold) {
        const int sizes[] = {(int) _grid_shape.second, (int) _grid_shape.first, 2};
        _mv.create(3, sizes, CV_32F);

        //std::cout << "frame_shape=" << _frame_shape.first <<","<<_frame_shape.second << std::endl;
        //std::cout << "_grid_shape=" << _grid_shape.first <<"," << _grid_shape.second << std::endl;
    }

    void denoise_occupancy_map(cv::Mat&);

    bool process_frame(int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors);

#ifdef _MV_DEBUG_
    void draw_occupancy(cv::Mat& img);
    void draw_arrows(cv::Mat& img);
    void draw_motion_vectors(cv::Mat& image);
#endif /* _MV_DEBUG_ */

private:

    // width x height (e.g. 1280x720)
    const std::pair<size_t, size_t> _frame_shape;
    const size_t _grid_step;
    // width x height (e.g. 160x80)
    const std::pair<size_t, size_t> _grid_shape;
    cv::Mat _mv;
    const size_t _window_size;
    boost::circular_buffer<MVFrame> _cb;
    boost::circular_buffer<int> _mcb;
    const float _square_dist; // cache for squared diagonal distance 
    const float _motion_occupancy_threshold;
    float _occupancy_pct;
    float _occupancy_avg_threshold;
    float _occupancy_local_avg_threshold;
    float _avg_movement;
    cv::Mat _occupancy;
};


#endif