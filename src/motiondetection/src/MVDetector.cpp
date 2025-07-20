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



bool
MVDetector::process_frame(std::string &cam, int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors) {
   
    std::unique_ptr< MVFrameBase> cur( new MVFrameBase(frame_index, pts, pict_type, 'v', _grid_step, _grid_shape, motion_vectors,   vt_mask  ));

    if (cur->_grid_step == 8)
        cur->fill_in_mvs_grid8();

    if (_cb.size() > 1 && _cb.back()->empty() && !cur->empty()) {
        _cb.back()->interpolate_flow(_cb[_window_size - 2], cur);
    }

    if (_cb.size() > 1 && !_cb.back()->empty()) {
        const int &rows = _grid_shape.second,
                &cols = _grid_shape.first;
        _mv = cv::Scalar(0.0); // reset to zero
        _occupancy = cv::Scalar(0.0);

        // add up individual vector components (avg.)
        for (const auto& fi : _cb) {
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




MVDetector::MVDetector(
            const std::pair<size_t, size_t>& frame_shape,
            Settings::MDConfig &mdConfig,
            bool force_grid_8 ) : MVDetectorBase( frame_shape, mdConfig ,force_grid_8), 
//    _frame_shape(frame_shape),
//    _grid_step(force_grid_8 ? 8 : 16),
//    _grid_shape(std::pair<size_t, size_t>(_frame_shape.first / _grid_step, _frame_shape.second / _grid_step)),
//    _window_size(mdConfig.DEFAULT_WINDOW_SIZE),
//    _cb(_window_size),
    _mcb(_window_size * 2), // TODO
    _square_dist(float(_frame_shape.first*_frame_shape.first + _frame_shape.second*_frame_shape.second)),
    _motion_occupancy_threshold(mdConfig.DEFAULT_OCCUPANCY_THRESHOLD),
    _occupancy_pct(std::min(1.0, _motion_occupancy_threshold + 0.1)),
    _avg_movement(false),
    _occupancy(cv::Mat((int) _grid_shape.second, (int) _grid_shape.first, CV_32F, cv::Scalar(0.0))),
    _occupancy_local_avg_threshold(mdConfig.DEFAULT_LOCAL_OCCUPANCY_AVG_THRESHOLD),
    _occupancy_avg_threshold(mdConfig.DEFAULT_OCCUPANCY_AVG_THRESHOLD)
{

    
        //std::cout << "frame_shape=" << _frame_shape.first <<","<<_frame_shape.second << std::endl;
        //std::cout << "_grid_shape=" << _grid_shape.first <<"," << _grid_shape.second << std::endl;
 }


#ifdef _MV_DEBUG_

void draw_single_arrow(cv::Mat& img, const cv::Point& pStart, const cv::Point& pEnd, cv::Scalar startColor) {
    static const double PI = acos(-1);
    static const int lineThickness = 1;
    static const int lineType = CV_AA;
    static const cv::Scalar lineColor = CV_RGB(255, 0, 0);
    static const double alphaDegrees = 20.0;
    static const int arrowHeadLen = 2.0;

    double angle = atan2((double) (pStart.y - pEnd.y), (double) (pStart.x - pEnd.x));
    cv::line(img, pStart, pEnd, lineColor, lineThickness, lineType);
    img.at<cv::Vec3b>(pStart) = cv::Vec3b(startColor[0], startColor[1], startColor[2]);
    for (int k = 0; k < 2; k++) {
        int sign = k == 1 ? 1 : -1;
        cv::Point arrow(pEnd.x + arrowHeadLen * cos(angle + sign * PI * alphaDegrees / 180), pEnd.y + arrowHeadLen * sin(angle + sign * PI * alphaDegrees / 180));
        cv::line(img, pEnd, arrow, lineColor, lineThickness, lineType);
    }
}

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
