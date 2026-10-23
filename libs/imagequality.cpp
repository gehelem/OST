#include "imagequality.h"
#include <opencv2/opencv.hpp>

namespace OST
{

double laplacianVarianceSharpness(const uint8_t *buffer, int width, int height,
                                  int bytesPerPixel, double statMin, double statMax,
                                  double roiPercent)
{
    if (!buffer || width <= 0 || height <= 0)
        return 0;

    cv::Mat raw;
    if (bytesPerPixel == 2)
        raw = cv::Mat(height, width, CV_16UC1, const_cast<uint8_t *>(buffer));
    else
        raw = cv::Mat(height, width, CV_8UC1, const_cast<uint8_t *>(buffer));

    if (roiPercent < 100.0)
    {
        int rw = static_cast<int>(width * roiPercent / 100.0);
        int rh = static_cast<int>(height * roiPercent / 100.0);
        if (rw < 1) rw = 1;
        if (rh < 1) rh = 1;
        int rx = (width - rw) / 2;
        int ry = (height - rh) / 2;
        raw = raw(cv::Rect(rx, ry, rw, rh));
    }

    double hi = statMax;
    if (hi <= statMin)
        hi = statMin + 1;
    cv::Mat img8;
    raw.convertTo(img8, CV_8UC1, 255.0 / (hi - statMin), -255.0 * statMin / (hi - statMin));

    cv::Mat lap;
    cv::Laplacian(img8, lap, CV_64F);
    cv::Scalar mean, stddev;
    cv::meanStdDev(lap, mean, stddev);
    return stddev[0] * stddev[0];
}

}
