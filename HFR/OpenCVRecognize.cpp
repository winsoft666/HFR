#include "OpenCVRecognize.h"
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QMutexLocker>
#include <QCoreApplication>

using namespace cv;


OpenCVRecognize* OpenCVRecognize::m_pInstance = NULL;

OpenCVRecognize::OpenCVRecognize(QObject* parent /* = NULL*/) : 
	QObject(QCoreApplication::instance()),
	m_iLabel(1)
{
	m_pFaceRecognizer = createLBPHFaceRecognizer(1, 8, 8, 8, 1800);
}

bool OpenCVRecognize::readTrainingSet(const QString &listPath, QVector<cv::Mat> &images)
{
	QFile file(listPath);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "ERROR: open training set failed " << listPath;
		return false;
	}

	QString strDir = QCoreApplication::applicationDirPath();

	while (!file.atEnd()) {
		QString imgPath = strDir + "/" + QString(file.readLine()).trimmed();
		cv::Mat mat = imread(imgPath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
		if (!mat.empty())
			images.push_back(mat);
	}

	return true;
}

OpenCVRecognize* OpenCVRecognize::instance()
{
	static QMutex mutex;
	if (!m_pInstance) {
		QMutexLocker lock(&mutex);
		if (!m_pInstance) {
			m_pInstance = new OpenCVRecognize();
		}
	}
	return m_pInstance;
}

OpenCVRecognize::~OpenCVRecognize()
{
}

bool OpenCVRecognize::recognize(const QVector<cv::Mat> &trainSet, const cv::Mat &face, int faceLabel, double &confidence)
{
	if (trainSet.size() == 0) {
		return false;
	}

	//cv::Size faceSize = Size(trainSet[0].size().width, trainSet[0].size().height);
	cv::Size faceSize = Size(face.size().width, face.size().height);

	QVector<cv::Mat> trainSetGray;
	try {
		int i = 0;
		for (QVector<cv::Mat>::const_iterator it = trainSet.begin(); it != trainSet.end(); it++) {

			cv::Mat grayMat;
			cv::resize(*it, grayMat, faceSize);
			cvtColor(grayMat, grayMat, COLOR_BGR2GRAY);

			imshow(QString::number(i++).toStdString(), grayMat);
			trainSetGray.append(grayMat);
		}
	}
	catch (cv::Exception &e) {
		qDebug() << "EXCEPTION: " << e.what();
		return false;
	}

	faceLabel = m_iLabel++;

	QVector<int> labels(trainSetGray.size());
	for (QVector<int>::iterator it = labels.begin(); it != labels.end(); *(it++) = faceLabel) ;

	try {
		m_pFaceRecognizer->train(trainSetGray.toStdVector(), labels.toStdVector());
	}
	catch (cv::Exception &e) {
		qDebug() << "EXCEPTION: FaceRecognizer train: " << e.what();
		return false;
	}

	Mat faceGray;
	int label = 0;

	try {
		cvtColor(face, faceGray, CV_BGR2GRAY);
		//resize(faceGray, faceGray, faceSize);
		imshow("Face", faceGray);
	}
	catch (cv::Exception &e) {
		qDebug() << "EXCEPTION: " << e.what();
		return false;
	}

	try {
		m_pFaceRecognizer->predict(faceGray, label, confidence);
	}
	catch (cv::Exception &e) {
		qDebug() << "EXCEPTION: FaceRecognizer predict: " << e.what();
		return false;
	}

	return label == faceLabel ? true : false;
}

