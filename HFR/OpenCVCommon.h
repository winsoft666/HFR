#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>


class OpenCVCommon {
public:
	static QImage cvMat2QImage(const cv::Mat& mat);
	static cv::Mat QImage2cvMat(const QImage &image);
};