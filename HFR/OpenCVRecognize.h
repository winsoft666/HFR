#pragma once

#include <QObject>
#include "opencv2\opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"

class OpenCVRecognize : public QObject {
	Q_OBJECT
public:
	static OpenCVRecognize* instance();
	virtual ~OpenCVRecognize();

	bool recognize(const QVector<cv::Mat> &trainSet, const cv::Mat &face, int faceLabel, double &confidence);
protected:

private:
	OpenCVRecognize(QObject* parent = NULL);
	bool readTrainingSet(const QString &listPath, QVector<cv::Mat> &images);
	static OpenCVRecognize* m_pInstance;
private:
	cv::Ptr<cv::FaceRecognizer> m_pFaceRecognizer;
	cv::Size m_FaceSize;
	int m_iLabel;
};