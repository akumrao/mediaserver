#include<iostream>
#include<vector>
#include<list>

#include "MVDetectorBase.h"


#ifndef MVDECT_MVD_H
#define MVDECT_MVD_H



class MVDetector : public MVDetectorBase {
   // constexpr static int DEFAULT_WINDOW_SIZE = 3;
   // constexpr static float DEFAULT_OCCUPANCY_THRESHOLD = 2; // 3 %
   // constexpr static float DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD = 0.6;
   // constexpr static float DEFAULT_OCCUPANCY_AVG_THRESHOLD = 0.6;
public:

    MVDetector(
            const std::pair<size_t, size_t>& frame_shape,
            Settings::MDConfig &mdConfig,
            bool force_grid_8 = true) ;

    void denoise_occupancy_map(cv::Mat&);

    bool process_frame(std::string &cam, int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors); // do not invoke base function

#ifdef _MV_DEBUG_
    void draw_occupancy(cv::Mat& img);
    void draw_arrows(cv::Mat& img);
    void draw_motion_vectors(cv::Mat& image);
#endif /* _MV_DEBUG_ */

private:

//    // width x height (e.g. 1280x720)
//    const std::pair<size_t, size_t> _frame_shape;
//    const size_t _grid_step;
//    // width x height (e.g. 160x80)
//    const std::pair<size_t, size_t> _grid_shape;
//    cv::Mat _mv;
//    const size_t _window_size;
//    FixedList<std::unique_ptr<MVFrameBase>> _cb;
    FixedList<int> _mcb;
    const float _square_dist; // cache for squared diagonal distance 
    const float _motion_occupancy_threshold;
    float _occupancy_pct;
    float _occupancy_avg_threshold;
    float _occupancy_local_avg_threshold;
    float _avg_movement;
    cv::Mat _occupancy;
    
   // std::vector< float> xmask;
   // std::vector< float> ymask;
    
    
};


#endif
