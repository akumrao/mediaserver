#include<numeric>
#include<opencv2/core.hpp>
//#include<opencv2/videoio.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgcodecs.hpp>
#ifdef _MV_DEBUG_
#include<opencv2/highgui.hpp>
#endif
#include<opencv2/photo.hpp>

#include "MVDetector.h"
#define MV_DIST_THRESHOLD2 (0.00001)

#include "opencv2/highgui.hpp"

#include "MVDraw.h"

bool
MVDetector::process_frame(int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors) {
   
    std::unique_ptr< MVFrame> cur( new MVFrame(frame_index, pts, pict_type, 'v', _grid_step, _grid_shape, motion_vectors, xmask, ymask));

    if (cur->_grid_step == 8)
        cur->fill_in_mvs_grid8();

    if (_cb.size() > 1 && _cb.back()->empty() && !cur->empty()) {
        _cb.back()->interpolate_flow(_cb[_window_size - 2], cur);
    }

    if (_cb.size() > 1 && !_cb.back()->empty())
    {
        const int &rows = _grid_shape.second,
                &cols = _grid_shape.first;
        _mv = cv::Scalar(0.0); // reset to zero
        _occupancy = cv::Scalar(0.0);

        // add up individual vector components (avg.)
        for (const auto& fi : _cb) 
        {
            if (!fi->empty()) {
                //NOTE: _mv is not used for detecting motion anymore
                _mv += fi->_mv;
                _occupancy += fi->_occupancy;
            }
        }
        
       // int n =  _cb.size();

        // get avg of vector components
        _occupancy /= _cb.size();
        _mv /= _cb.size();

        //denoise_occupancy_map(_occupancy);  -->diennv

        int mv_cnt = cv::countNonZero(_occupancy > _occupancy_local_avg_threshold);
        _occupancy_pct = float(mv_cnt) / float((rows - 1) *(cols - 1))*100;
        int cur_movement = int(_occupancy_pct > _motion_occupancy_threshold);

        _mcb.push_back(cur_movement);
    }
    _cb.push_back(std::move(cur)); // delay one frame
    return float(std::accumulate(_mcb.begin(), _mcb.end(), 0)) / _mcb.size() > _occupancy_avg_threshold;
}

void
MVDetector::denoise_occupancy_map(cv::Mat& occupancy_map) {
    // https://docs.opencv.org/3.4/db/df6/tutorial_erosion_dilatation.html
    static int erosion_size = 1;
    static int dilation_size = 2;
    static cv::Mat element_erosion = cv::getStructuringElement(cv::MORPH_CROSS,
            cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
            cv::Point(erosion_size, erosion_size));
    static cv::Mat element_dilation = cv::getStructuringElement(cv::MORPH_CROSS,
            cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
            cv::Point(dilation_size, dilation_size));

    static cv::Mat dst;
    cv::erode(occupancy_map, dst, element_erosion);
    cv::dilate(dst, occupancy_map, element_dilation);
}

#ifdef _MV_DEBUG_

void
MVDetector::draw_occupancy(cv::Mat& img) {
    const int &rows = _grid_shape.second,
            &cols = _grid_shape.first;

    double midx = img.cols / cols / 2,
            midy = img.rows / rows / 2;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // OpenCV uses (col, row) coord. system
            cv::Point center(
                    double(j) / cols * img.cols + midx,
                    double(i) / rows * img.rows + midy
                    );
            if (_occupancy.at<float>(i, j) > _occupancy_avg_threshold) {
                cv::circle(img, center, _occupancy.at<float>(i, j), CV_RGB(204, 51, 255), 2);
            }
        }
    }
}

void
MVDetector::draw_motion_vectors(cv::Mat& img) {
    const int &rows = _grid_shape.second,
            &cols = _grid_shape.first;

    double midx = img.cols / cols / 2,
            midy = img.rows / rows / 2;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // OpenCV uses (col, row) coord. system
            cv::Point start(
                    double(j) / cols * img.cols + midx,
                    double(i) / rows * img.rows + midy
                    );
            float dx = _mv.at<float>(i, j, 0);
            float dy = _mv.at<float>(i, j, 1);

            if (dx != 0 || dy != 0) {
                cv::Point end(start.x + dx, start.y + dy);
                draw_single_arrow(img, start, end, CV_RGB(0, 255, 255));
            }
        }
    }
}
#endif /* _MV_DEBUG_ */

MVFrame::MVFrame(
        int frame_index,
        int64_t pts,
        char pict_type,
        char origin,
        size_t grid_step,
        const std::pair<size_t, size_t>& shape, /* (cols, rows) */
        const std::vector<AVMotionVector>& motion_vectors , std::vector< float> &xmask, std::vector< float>& ymask) :
_frame_index(frame_index), _pts(pts), _pict_type(pict_type), _origin(origin),
_grid_step(grid_step),
_rows(size_t(shape.second)),
_cols(size_t(shape.first)),
_occupancy(cv::Mat(_rows, _cols, CV_32F, cv::Scalar(0.0))),
_is_empty(motion_vectors.empty()),
xmask(xmask),
ymask(ymask)        
{
    const int sizes[] = {(int) _rows, (int) _cols, 2};
    _mv.create(3, sizes, CV_32F);
    _mv = cv::Scalar(0.0);

    set_motion_vectors(motion_vectors);
}


MVFrame::~MVFrame()
{
    //std::cout << "~MVFrame" << std::endl;
}
   

// place motion vector on Matrix for x & y dims

void
MVFrame::set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors) {
    // motion_vectors are on image dimension
     for (const auto &mv : motion_vectors) {
        int mvx =  (mv.motion_x >> 2)* mv.source,
                mvy = (mv.motion_y >> 2)*mv.source;

        // clipped
        size_t i = std::max(size_t(0), std::min(mv.dst_y / _grid_step, _rows - 1));
        size_t j = std::max(size_t(0), std::min(mv.dst_x / _grid_step, _cols - 1));
        
        if(xmask.size() && ymask.size())
        {
            int ret =   pnpoly (xmask.size(),  &xmask[0], &ymask[0],  j, i);
            if(ret)
             continue;
        }

        if( mvx* mvx +  mvy * mvy >  9 )
        {    
            _mv.at<float>(i, j, 0) = mvx;
            _mv.at<float>(i, j, 1) = mvy;
            _occupancy.at<float>(i, j) = float(mvx != 0 || mvy != 0);
        }

        
    }
}

void
MVFrame::interpolate_flow(const  std::unique_ptr< MVFrame>& prev, const  std::unique_ptr< MVFrame>& next) {
    cv::addWeighted(prev->_mv, 0.5, next->_mv, 0.5, 0.0, _mv);
    _origin = 'i'; // interpolated origin
}

void
MVFrame::fill_in_mvs_grid8() {
    for (int k = 0; k < 2; k++) {
        for (int i = 1; i < (int) _rows - 1; i++) {
            for (int j = 1; j < (int) _cols - 1; j++) {
                if (_occupancy.at<float>(i, j) == 0) 
                {
                    if (_occupancy.at<float>(i, j - 1) != 0 &&
                            _occupancy.at<float>(i, j + 1) != 0) {
                        _mv.at<float>(i, j, 0) = (_mv.at<float>(i, j - 1, 0) + _mv.at<float>(i, j + 1, 0)) / 2;
                        _mv.at<float>(i, j, 1) = (_mv.at<float>(i, j - 1, 1) + _mv.at<float>(i, j + 1, 1)) / 2;
                       
                        _occupancy.at<float>(i, j) = 1;
                        
                    } else if (_occupancy.at<float>(i - 1, j) != 0 &&
                            _occupancy.at<float>(i + 1, j) != 0) {
                        _mv.at<float>(i, j, 0) = (_mv.at<float>(i - 1, j, 0) + _mv.at<float>(i + 1, j, 0)) / 2;
                        _mv.at<float>(i, j, 1) = (_mv.at<float>(i - 1, j, 1) + _mv.at<float>(i + 1, j, 1)) / 2;
                        _occupancy.at<float>(i, j) = 1;
                        
                    }
                }
            }
        }
    }
}

// place motion vector on Matrix for x & y dims


//The following code is by Randolph Franklin, it returns 1 for interior points and 0 for exterior points.
// If there is only one connected component, then it is optional to repeat the first vertex at the end. It's also optional to surround the component with zero vertices.


int MVFrame::pnpoly(int npol, float *mask, float *ymask, float x, float y)
{
      int i, j, c = 0;
      for (i = 0, j = npol-1; i < npol; j = i++) {
        if ((((ymask[i] <= y) && (y < ymask[j])) ||
             ((ymask[j] <= y) && (y < ymask[i]))) &&
            (x < (mask[j] - mask[i]) * (y - ymask[i]) / (ymask[j] - ymask[i]) + mask[i]))
          c = !c;
      }
      return c;
}



 MVDetector::MVDetector(
            const std::pair<size_t, size_t>& frame_shape,
            size_t window_size ,
            float motion_occupancy_threshold ,
            float occupancy_local_avg_threshold ,
            float occupancy_avg_threshold ,
            bool force_grid_8 ) :
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
    _occupancy_avg_threshold(occupancy_avg_threshold) 
{
    const int sizes[] = {(int) _grid_shape.second, (int) _grid_shape.first, 2};
    _mv.create(3, sizes, CV_32F);

    
        //std::cout << "frame_shape=" << _frame_shape.first <<","<<_frame_shape.second << std::endl;
        //std::cout << "_grid_shape=" << _grid_shape.first <<"," << _grid_shape.second << std::endl;
 }


#ifdef _MV_DEBUG_



void
MVFrame::draw_occupancy(cv::Mat& img) {
    double midx = img.cols / _cols / 2,
            midy = img.rows / _rows / 2;

    for (int i = 0; i < _rows; i++) {
        for (int j = 0; j < _cols; j++) {
            // OpenCV uses (col, row) coord. system
            cv::Point center(
                    double(j) / _cols * img.cols + midx,
                    double(i) / _rows * img.rows + midy
                    );

            if (_occupancy.at<float>(i, j) > 0) {
                cv::circle(img, center, 1, CV_RGB(204, 51, 255), 2);
            }
        }
    }
}

void
MVFrame::draw_arrows(cv::Mat& img) {
    double midx = img.cols / _cols / 2,
            midy = img.rows / _rows / 2;

    for (int i = 0; i < _rows; i++) {
        for (int j = 0; j < _cols; j++) {
            // OpenCV uses (col, row) coord. system
            cv::Point start(
                    double(j) / _cols * img.cols + midx,
                    double(i) / _rows * img.rows + midy
                    );
            float dx = _mv.at<float>(i, j, 0);
            float dy = _mv.at<float>(i, j, 1);

            if (dx != 0 || dy != 0) {
                cv::Point end(start.x + dx, start.y + dy);
                cv::Scalar startColor = _occupancy.at<float>(i, j) > 0 ? CV_RGB(0, 255, 0) : CV_RGB(0, 255, 255);
                draw_single_arrow(img, start, end, startColor);
            }
        }
    }
}
#endif /* _MV_DEBUG_ */
