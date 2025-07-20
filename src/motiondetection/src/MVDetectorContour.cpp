#include<numeric>
#include<opencv2/core.hpp>
//#include<opencv2/videoio.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgcodecs.hpp>
#ifdef _MV_DEBUG_
#include<opencv2/highgui.hpp>
#endif
#include<opencv2/photo.hpp>

#include "MVDetectorContour.h"

#include "base/logger.h"

using namespace base;


bool
MVDetectorContour::process_frame( std::string &cam, int64_t pts, int frame_index, char pict_type, const std::vector<AVMotionVector>& motion_vectors) {
   
    std::unique_ptr< MVFrameConture> cur( new MVFrameConture(frame_index, pts, pict_type, 'v', _grid_step, _grid_shape, motion_vectors, vt_mask));

    if (cur->_grid_step == 8)
        cur->fill_in_mvs_grid8();

//    if (_cb.size() > 1 && _cb.back()->empty() && !cur->empty()) {
//        _cb.back()->interpolate_flow(_cb[_window_size - 2], cur);
//    }

//    if (_cb.size() > 1 && !_cb.back()->empty())
//    {
//        const int &rows = _grid_shape.second,
//                &cols = _grid_shape.first;
//        _mv = cv::Scalar(0.0); // reset to zero
//        _occupancy = cv::Scalar(0.0);
//
//        // add up individual vector components (avg.)
//        for (const auto& fi : _cb) 
//        {
//            if (!fi->empty()) {
//                //NOTE: _mv is not used for detecting motion anymore
//                _mv += fi->_mv;
//                _occupancy += fi->_occupancy;
//            }
//        }
//        
//       // int n =  _cb.size();
//
//        // get avg of vector components
//        _occupancy /= _cb.size();
//        _mv /= _cb.size();
//
//        //denoise_occupancy_map(_occupancy);  -->diennv
//
//        int mv_cnt = cv::countNonZero(_occupancy > _occupancy_local_avg_threshold);
//        _occupancy_pct = float(mv_cnt) / float((rows - 1) *(cols - 1))*100;
//        int cur_movement = int(_occupancy_pct > _motion_occupancy_threshold);
//
//        _mcb.push_back(cur_movement);
//    }
  
          
    std::vector<std::vector<cv::Point> > cnts;


    // RETR_LIST
     
     //RETR_TREE
     
    // RETR_TREE  // slow
     // RETR_EXTERNAL // very fast        
    cv::findContours( cur->canvas, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int x = cnts.size();
    
    bool mdDetected = false;
    
    cv::Mat drawing = cv::Mat::zeros( cur->canvas.size(), CV_8UC3 );
    
    for( size_t i = 0; i< cnts.size(); i++ )
    {
        
         int sz = contourArea(cnts[i]);
         
         if( sz <  cntArea)
             continue;
         
         mdDetected = true;
      
        SDebug << "Motion detected for Cam:" << cam << " current contour sz:"  << sz   <<   " threshold sz(1/8):" <<  cntArea;
         
//        cv::Rect ret= cv::boundingRect(cnts[i] );
//         
//       // std::cout << sz << " " 
//        cv::Scalar color = cv::Scalar(0,0,255); 
// 
//       // Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//        cv::rectangle( drawing,  ret, cv::Scalar(0, 255, 0) );
//        drawContours(drawing, cnts, i, color, 2);
        
        break;
    }
        
        
        //polylines
   
     
        // cv::rectangle( drawing,  a, b, cv::Scalar(0, 0, 255) );
        

        //polylines example 1 
//        std::vector< cv::Point> contour;
//        
//        for( int i =0; i< cur->xmask.size(); ++i)
//        {
//           contour.push_back(cv::Point( cur->xmask[i], cur->ymask[i]));
//        }
        
        
//        const cv::Point *pts1 = (const cv::Point*) cv::Mat(contour).data;
    //    int npts = cv::Mat(contour).rows;

        //std::cout <<  "Number of polygon vertices: " <<  npts <<  std::endl;
        // draw the polygon 
       // cv::polylines(drawing, &pts1, &npts, 1, false, cv::Scalar(0, 0, 255));

         
//        std::cout <<  " " << std::endl;
////    
//        cv::imshow("black&white", cur->canvas);
//       
//        cv::imshow("contour", drawing);
//        
                 
//       vector<vector<Point> > contours;
//       vector<Vec4i> hierarchy;
//       findContours( cur->_occupancy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
////       Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
//       for( size_t i = 0; i< contours.size(); i++ )
//       {
//           
//           int x = 1;
//         //  Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
//           //drawContours( drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
//       }

    if(mdDetected && cont3MDFrame < _window_size )
        ++cont3MDFrame;
    else if(!mdDetected)
        cont3MDFrame = 0;
    
    return (cont3MDFrame > (_window_size -1) ? true : false);
}


#ifdef _MV_DEBUG_

void
MVDetectorContour::draw_occupancy(cv::Mat& img) {
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
MVDetectorContour::draw_motion_vectors(cv::Mat& img) {
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

MVFrameConture::MVFrameConture(
        int frame_index,
        int64_t pts,
        char pict_type,
        char origin,
        size_t grid_step,
        const std::pair<size_t, size_t>& shape, /* (cols, rows) */
        const std::vector<AVMotionVector>& motion_vectors , std::vector< st_mask > &vt_mask) :
        MVFrameBase( frame_index, pts , pict_type , origin,grid_step, shape , motion_vectors , vt_mask ),
        canvas(cv::Mat(_rows, _cols, CV_8UC1, cv::Scalar(0)))//         cv::Ma canvas;// = Mat::zeros(800,600,CV_8UC1);
//        angle(12,0)    
        
{
    const int sizes[] = {(int) _rows, (int) _cols, 2};
    _mv.create(3, sizes, CV_32F);
    _mv = cv::Scalar(0.0);

    set_motion_vectors(motion_vectors);
}


MVFrameConture::~MVFrameConture()
{
  //  std::cout << "~MVFrameConture" << std::endl;
}
   

// place motion vector on Matrix for x & y dims

void
MVFrameConture::set_motion_vectors(const std::vector<AVMotionVector>& motion_vectors) {
    // motion_vectors are on image dimension
    for (const auto &mv : motion_vectors) {
        int mvx =  (mv.motion_x >> 2)* mv.source,
                mvy = (mv.motion_y >> 2)*mv.source;
        
    
//        if( mvx != 0 )
//        {
//           double resultRadians = atan(mvy/mvx);
//           double resultDegrees = (resultRadians * 180) / M_PI;
//           
//           std::cout << "Deg:" <<  resultDegrees    <<   " mvy:" << mvy  << " mvx:" << mvx << std::endl << std::flush;
//        }

        // clipped
        size_t i = std::max(size_t(0), std::min(mv.dst_y / _grid_step, _rows - 1));
        size_t j = std::max(size_t(0), std::min(mv.dst_x / _grid_step, _cols - 1));
        
        
       
        int ret =0;
        for( auto &mask : vt_mask)
        {
            ret =   pnpoly (mask.xmask.size(),  &mask.xmask[0], &mask.ymask[0],  j, i);
            if(ret)
             break;
        }
        
        if(ret)
            continue;
        

        
        

        if( mvx* mvx +  mvy * mvy >  9 )
        {   
            _occupancy.at<float>(i, j) = float(mvx != 0 || mvy != 0);
            
            _mv.at<float>(i, j, 0) = mvx;
            _mv.at<float>(i, j, 1) = mvy;
            
            double degrees = std::atan2(mvy,  mvx) * 180 / M_PI;

            if (degrees < 0)
            {
                degrees = 360 + degrees;
            }
             
          //  angle[int (degrees/30)] += 1 ;

           // _angular.at<float>( i, j ) = degrees;
            
            canvas.at<uchar>( i, j ) = 255;
        }    


    
    }
    
    
  //  sort(angle.begin(), angle.end(), greater<int>());
    
//    std::cout << "Sorted \n";
//    for (auto x : angle)
//        std::cout << x << " ";
    

    
}

void
MVFrameConture::interpolate_flow(const  std::unique_ptr< MVFrameConture>& prev, const  std::unique_ptr< MVFrameConture>& next) {
    cv::addWeighted(prev->_mv, 0.5, next->_mv, 0.5, 0.0, _mv);
    _origin = 'i'; // interpolated origin
}

void MVFrameConture::fill_in_mvs_grid8() {
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
                        canvas.at<uchar>( i, j ) = 255;
                        
                    } else if (_occupancy.at<float>(i - 1, j) != 0 &&
                            _occupancy.at<float>(i + 1, j) != 0) {
                        _mv.at<float>(i, j, 0) = (_mv.at<float>(i - 1, j, 0) + _mv.at<float>(i + 1, j, 0)) / 2;
                        _mv.at<float>(i, j, 1) = (_mv.at<float>(i - 1, j, 1) + _mv.at<float>(i + 1, j, 1)) / 2;
                        _occupancy.at<float>(i, j) = 1;
                        canvas.at<uchar>( i, j ) = 255;
                        
                    }
                }
            }
        }
    }
}

// place motion vector on Matrix for x & y dims


//The following code is by Randolph Franklin, it returns 1 for interior points and 0 for exterior points.
// If there is only one connected component, then it is optional to repeat the first vertex at the end. It's also optional to surround the component with zero vertices.


MVDetectorContour::MVDetectorContour(
            const std::pair<size_t, size_t>& frame_shape, Settings::MDConfig &mdConfig,
            uint16_t &cntArea,
            bool force_grid_8 ): MVDetectorBase(frame_shape, mdConfig, force_grid_8 ), mdConfig(mdConfig), cntArea(cntArea)
{
    //const int sizes[] = {(int) _grid_shape.second, (int) _grid_shape.first, 2};
    //_mv.create(3, sizes, CV_32F);

    
        //std::cout << "frame_shape=" << _frame_shape.first <<","<<_frame_shape.second << std::endl;
        //std::cout << "_grid_shape=" << _grid_shape.first <<"," << _grid_shape.second << std::endl;
}


#ifdef _MV_DEBUG_


void MVFrameConture::draw_occupancy(cv::Mat& img) {
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

void MVFrameConture::draw_arrows(cv::Mat& img) {
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
