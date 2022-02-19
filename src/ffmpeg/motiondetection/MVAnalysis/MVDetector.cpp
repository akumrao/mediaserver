#include<numeric>
#include<opencv2/core.hpp>
//#include<opencv2/videoio.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgcodecs.hpp>
//#include<opencv2/highgui.hpp>
#include<opencv2/photo.hpp>

#include "MVDetector.h"
#define MV_DIST_THRESHOLD2 (0.00001)

MVFrame&
        MVFrame::operator=(MVFrame&& other) {
    if (this != &other) {
        _frame_index = other._frame_index;
        _pts = other._pts;
        _pict_type = other._pict_type;
        _origin = other._origin;
        _grid_step = other._grid_step;
        _rows = other._rows;
        _cols = other._cols;
        _mv = other._mv; // Mat is ref counted
        _is_empty = other._is_empty;
        _occupancy = other._occupancy;
        other._mv.release();
    }
    return *this;
}

bool
MVDetector::process_frame(int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors) {
    MVFrame cur(frame_index, pts, pict_type, 'v', _grid_step, _grid_shape, motion_vectors);

    if (cur._grid_step == 8)
        cur.fill_in_mvs_grid8();

    if (_cb.size() > 1 && _cb.back().empty() && !cur.empty()) {
        _cb.back().interpolate_flow(_cb[_window_size - 2], cur);
    }

    if (_cb.size() > 1 && !_cb.back().empty()) {
        const int &rows = _grid_shape.second,
                &cols = _grid_shape.first;
        _mv = cv::Scalar(0.0); // reset to zero
        _occupancy = cv::Scalar(0.0);

        // add up individual vector components (avg.)
        for (const MVFrame& fi : _cb) {
            if (!fi.empty()) {
                //NOTE: _mv is not used for detecting motion anymore
                _mv += fi._mv;
                _occupancy += fi._occupancy;
            }
        }

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
        const std::vector<AVMotionVector>& motion_vectors) :
_frame_index(frame_index), _pts(pts), _pict_type(pict_type), _origin(origin),
_grid_step(grid_step),
_rows(size_t(shape.second)),
_cols(size_t(shape.first)),
_occupancy(cv::Mat(_rows, _cols, CV_32F, cv::Scalar(0.0))),
_is_empty(motion_vectors.empty()) {
    const int sizes[] = {(int) _rows, (int) _cols, 2};
    _mv.create(3, sizes, CV_32F);
    _mv = cv::Scalar(0.0);

    set_motion_vectors(motion_vectors);
}

// place motion vector on Matrix for x & y dims

void
MVFrame::set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors) {
    // motion_vectors are on image dimension
    for (const auto &mv : motion_vectors) {
        int mvx = mv.dst_x - mv.src_x,
                mvy = mv.dst_y - mv.src_y;

        // clipped
        size_t i = std::max(size_t(0), std::min(mv.dst_y / _grid_step, _rows - 1));
        size_t j = std::max(size_t(0), std::min(mv.dst_x / _grid_step, _cols - 1));

        _mv.at<float>(i, j, 0) = float(mvx);
        _mv.at<float>(i, j, 1) = float(mvy);

        _occupancy.at<float>(i, j) = float(mvx != 0 || mvy != 0);
    }
}

void
MVFrame::interpolate_flow(const MVFrame& prev, const MVFrame& next) {
    cv::addWeighted(prev._mv, 0.5, next._mv, 0.5, 0.0, _mv);
    _origin = 'i'; // interpolated origin
}

void
MVFrame::fill_in_mvs_grid8() {
    for (int k = 0; k < 2; k++) {
        for (int i = 1; i < (int) _rows - 1; i++) {
            for (int j = 1; j < (int) _cols - 1; j++) {
                if (_occupancy.at<float>(i, j) == 0) {
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
