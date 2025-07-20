#include<iostream>
#include<vector>
#include<list>


#include<memory>
#include<chrono>

#include"Settings.h"

//#include<boost/circular_buffer.hpp>

#include<opencv2/core.hpp>

//#define _MV_DEBUG_ 1


#ifdef _MV_DEBUG_
#include<opencv2/imgproc.hpp>
#endif /* _MV_DEBUG_ */


#include "ffmpeg_h.h"

#pragma once



#ifndef H264_MVD_H
#define H264_MVD_H

template <typename T>
class FixedList : public std::list<T> {
public:
    
    FixedList(int max)
    {
        MaxLen = max;
    }
    
    virtual ~FixedList() = default;
    
    void push_back(const T& value) {
        if (this->size() == MaxLen) {
           this->pop_front();
        }
        std::list<T>::push_back(value);
    }
    
     void push_back( T&& value) {
        if (this->size() == MaxLen) {
           this->pop_front();
        }
      std::list<T>::push_back( std::move(value) );
    }
    
    T& operator [ ]( int index)
    {
         if( index >= MaxLen  )
         {
             exit(0);
         }
                 
         auto it = std::next(this->begin(), index);
         return  *it;
    }
    
    int MaxLen;
};



//
//template <typename T, int MaxLen, typename Container=std::deque<T>>
//class FixedQueue : public std::queue<T, Container> {
//public:
//    void push(const T& value) {
//        if (this->size() == MaxLen) {
//           this->c.pop_front();
//        }
//        std::queue<T, Container>::push(value);
//    }
//};




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


struct st_mask
 {
     std::vector< float> xmask;
     std::vector< float> ymask;
 };
    

class MVFrameBase {
    friend class MVDetectorBase;
    friend class MVDetector;
public:
    ~MVFrameBase();
    MVFrameBase(
            int frame_index,
            int64_t pts,
            char pict_type,
            char origin,
            size_t grid_step,
            const std::pair<size_t, size_t>& shape, /* (cols, rows) (width, height) */
            // first = cols; second = rows
            const std::vector<AVMotionVector>& motion_vectors,
            std::vector< st_mask > &vt_mask);
    // draw arrows overlay for debug
    void interpolate_flow(const  std::unique_ptr< MVFrameBase>& prev, const  std::unique_ptr< MVFrameBase>& next);

    #ifdef _MV_DEBUG_
    void draw_arrows(cv::Mat& img);
    void draw_occupancy(cv::Mat& image);
    #endif /* _MV_DEBUG_ */


    bool empty() const {
        return _is_empty;
    }


protected:
    int _frame_index;
    int64_t _pts;
    char _pict_type;
    char _origin;
    size_t _grid_step;
    size_t _rows, _cols;
    //cv::Mat _dx, _dy;
    cv::Mat _mv;
    bool _is_empty;
     cv::Mat _occupancy;
public:
    int pnpoly(int npol, float *xp, float *yp, float x, float y);

    void set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors);
    void fill_in_mvs_grid8();
    
    std::vector< st_mask > &vt_mask;
};




class MVDetectorBase {
   // constexpr static int DEFAULT_WINDOW_SIZE = 3;
   // constexpr static float DEFAULT_OCCUPANCY_THRESHOLD = 2; // 3 %
   // constexpr static float DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD = 0.6;
   // constexpr static float DEFAULT_OCCUPANCY_AVG_THRESHOLD = 0.6;
public:

    MVDetectorBase(const std::pair<size_t, size_t>& frame_shape,
                 bool force_grid_8 );
    
    MVDetectorBase(
            const std::pair<size_t, size_t>& frame_shape,
            Settings::MDConfig &mdConfig,
            bool force_grid_8 = true) ;

    void denoise_occupancy_map(cv::Mat&);

   virtual bool process_frame(std::string &cam, int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors);

#ifdef _MV_DEBUG_
    void draw_occupancy(cv::Mat& img);
    void draw_arrows(cv::Mat& img);
    void draw_motion_vectors(cv::Mat& image);
#endif /* _MV_DEBUG_ */

protected:

    // width x height (e.g. 1280x720)
    const std::pair<size_t, size_t> _frame_shape;
    const size_t _grid_step;
    // width x height (e.g. 160x80)
    const std::pair<size_t, size_t> _grid_shape;
    cv::Mat _mv;
    const size_t _window_size;
    FixedList<std::unique_ptr<MVFrameBase>> _cb;
    //FixedList<int> _mcb;
    cv::Mat _occupancy;
    
 
    
    std::vector< st_mask > vt_mask;
};


#endif
