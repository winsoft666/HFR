#pragma once

#include <QObject>
#include <QImage>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

class OpenCVDetect : public QObject{
	Q_OBJECT
public:
	static OpenCVDetect* instance();

	virtual ~OpenCVDetect();


	bool drawFaceDetect(const cv::Mat & src, cv::Mat &dest);
	int faceImages(const cv::Mat & src, QVector<cv::Rect> *imgs);

	void setCascadeFrontal(const QString &str);

protected:
	QString m_strCascadeFrontal;

	cv::CascadeClassifier m_cvCascade;

	bool m_bReady;
private:
	OpenCVDetect(QObject* parent = NULL);
	static OpenCVDetect* m_pInstance;
};