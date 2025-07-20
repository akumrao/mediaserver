

#include<memory>
#include<chrono>
#include "MVDetector.h"

#ifndef CONT_H264_MVD_H
#define CONT_H264_MVD_H





class MVFrameConture : public MVFrameBase{
    friend class MVDetectorContour;
public:
    ~MVFrameConture();
    
    MVFrameConture(
            int frame_index,
            int64_t pts,
            char pict_type,
            char origin,
            size_t grid_step,
            const std::pair<size_t, size_t>& shape, /* (cols, rows) (width, height) */
            // first = cols; second = rows
            const std::vector<AVMotionVector>& motion_vectors, std::vector< st_mask > &vt_mask);
    // draw arrows overlay for debug
    void interpolate_flow(const  std::unique_ptr< MVFrameConture>& prev, const  std::unique_ptr< MVFrameConture>& next);
    
    void set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors);
#ifdef _MV_DEBUG_
    void draw_arrows(cv::Mat& img);
    void draw_occupancy(cv::Mat& image);
#endif /* _MV_DEBUG_ */

    void fill_in_mvs_grid8();
    
    bool empty() const {
        return _is_empty;
    }


private:
   cv::Mat canvas;// = Mat::zeros(800,600,CV_8UC1);
  // std::vector<int> angle;

};


class MVDetectorContour : public MVDetectorBase {

public:

    MVDetectorContour(
            const std::pair<size_t, size_t>& frame_shape, Settings::MDConfig &mdConfig,
            uint16_t &cntArea, bool force_grid_8 = true) ;


    bool process_frame(std::string &cam, int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors);

    Settings::MDConfig &mdConfig;
    uint16_t &cntArea;
    int cont3MDFrame{0}; // if countinous 3 frame above Contour are are 

};


#endif
