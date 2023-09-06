#include "OpenCVDetect.h"
#include <QDebug>
#include <vector>
#include <QCoreApplication>
#include "OpenCVCommon.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTextCodec>
#include "CmnUtil.h"

using namespace cv;

OpenCVDetect* OpenCVDetect::m_pInstance = NULL;

OpenCVDetect * OpenCVDetect::instance()
{
	static QMutex mutex;
	if (!m_pInstance) {
		QMutexLocker lock(&mutex);
		if (!m_pInstance) {
			m_pInstance = new OpenCVDetect();
		}
	}
	return m_pInstance;
}

OpenCVDetect::OpenCVDetect(QObject* parent /* = NULL */) :
	QObject(QCoreApplication::instance()),
	m_bReady(false)
{
	setCascadeFrontal(QCoreApplication::applicationDirPath() + "/data/lbpcascades/lbpcascade_frontalface.xml");
}


OpenCVDetect::~OpenCVDetect()
{
}

bool OpenCVDetect::drawFaceDetect(const cv::Mat & src, cv::Mat &dest)
{
	if (!m_bReady) {
		return false;
	}

	dest = src.clone();

	std::vector<Rect> Faces;

	m_cvCascade.detectMultiScale(src, Faces, 1.15, 3, 0 | CASCADE_SCALE_IMAGE , Size(30, 30));
	
	size_t iFaceNum = Faces.size();

	for (size_t i = 0; i < iFaceNum; i++) {
		rectangle(dest, Faces[i], Scalar(0, 255, 0));
	}

	return true;
}

int OpenCVDetect::faceImages(const cv::Mat & src, QVector<cv::Rect>* imgs)
{
	if (!m_bReady) {
		return -1;
	}

	std::vector<Rect> Faces;

	m_cvCascade.detectMultiScale(src, Faces, 1.15, 3, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

	if (imgs) {
		*imgs = QVector<cv::Rect>::fromStdVector(Faces);
	}

	return Faces.size();
}

void OpenCVDetect::setCascadeFrontal(const QString & str)
{
	m_strCascadeFrontal = str;

	if (m_strCascadeFrontal.length() == 0) {
		return;
	}

	std::string strPath = cpp4j::Utf8ToAnsi(m_strCascadeFrontal.toStdString());

	if (!m_cvCascade.load(strPath)) {
		return;
	}

	m_bReady = true;
}
