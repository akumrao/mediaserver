

#ifndef MVDRAW_H
#define MVDRAW_H

#ifdef _MV_DEBUG_
//void draw_single_arrow(cv::Mat& img, const cv::Point& pStart, const cv::Point& pEnd, cv::Scalar startColor);


static void draw_single_arrow(cv::Mat& img, const cv::Point& pStart, const cv::Point& pEnd, cv::Scalar startColor) {
    static const double PI = acos(-1);
    static const int lineThickness = 1;
    static const int lineType = CV_AA;
    static const cv::Scalar lineColor = CV_RGB(255, 0, 0);
    static const cv::Scalar lineColor1 = CV_RGB(0, 255, 0);
    static const double alphaDegrees = 20.0;
    static const int arrowHeadLen = 4.0;

    double angle = atan2((double) (pStart.y - pEnd.y), (double) (pStart.x - pEnd.x));
    cv::line(img, pStart, pEnd, lineColor, lineThickness, lineType);
    img.at<cv::Vec3b>(pStart) = cv::Vec3b(startColor[0], startColor[1], startColor[2]);
    for (int k = 0; k < 2; k++) {
        int sign = k == 1 ? 1 : -1;
        cv::Point arrow(pEnd.x + arrowHeadLen * cos(angle + sign * PI * alphaDegrees / 180), pEnd.y + arrowHeadLen * sin(angle + sign * PI * alphaDegrees / 180));
        cv::line(img, pEnd, arrow, lineColor1, lineThickness, lineType);
    }
}


#endif /* _MV_DEBUG_ */


#endif
