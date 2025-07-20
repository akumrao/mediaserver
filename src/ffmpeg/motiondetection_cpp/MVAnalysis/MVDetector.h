

#include<memory>
#include<chrono>


#include "ffmpeg_h.h"

#include "FixedList.h"

#include<opencv2/core.hpp>

#define _MV_DEBUG_ 1

#ifdef _MV_DEBUG_
#include<opencv2/imgproc.hpp>
#endif /* _MV_DEBUG_ */



#pragma once



#ifndef H264_MVD_H
#define H264_MVD_H





class MVFrame {
    friend class MVDetector;
public:
    ~MVFrame();
    
    MVFrame(
            int frame_index,
            int64_t pts,
            char pict_type,
            char origin,
            size_t grid_step,
            const std::pair<size_t, size_t>& shape, /* (cols, rows) (width, height) */
            // first = cols; second = rows
            const std::vector<AVMotionVector>& motion_vectors, std::vector< float> &xmask, std::vector< float> &ymask);
    // draw arrows overlay for debug
    void interpolate_flow(const  std::unique_ptr< MVFrame>& prev, const  std::unique_ptr< MVFrame>& next);
#ifdef _MV_DEBUG_
    void draw_arrows(cv::Mat& img);
    void draw_occupancy(cv::Mat& image);
#endif /* _MV_DEBUG_ */

    bool empty() const {
        return _is_empty;
    }


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
    int pnpoly(int npol, float *xp, float *yp, float x, float y);

    void set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors);
    void fill_in_mvs_grid8();
   
    std::vector< float>& xmask;
    std::vector< float>& ymask;
    
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
            bool force_grid_8 = true) ;

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
public:
    const size_t _grid_step;
private:
    // width x height (e.g. 160x80)
    const std::pair<size_t, size_t> _grid_shape;
    cv::Mat _mv;
    const size_t _window_size;
    FixedList<std::unique_ptr<MVFrame>> _cb;
    FixedList<int> _mcb;
    const float _square_dist; // cache for squared diagonal distance 
    const float _motion_occupancy_threshold;
    float _occupancy_pct;
    float _occupancy_avg_threshold;
    float _occupancy_local_avg_threshold;
    float _avg_movement;
    cv::Mat _occupancy;
    
  //  std::vector< float> xmask;
    //std::vector< float> ymask;

public:
    std::vector< float> xmask;// = { 79, 159, 159, 79, 79};
    std::vector< float> ymask;// = { 0, 0, 44, 44, 0};

   // std::vector< float> xmask = { 0, 159, 159, 0};
   // std::vector< float> ymask = { 0, 0, 89, 89};
    
};


#endif
