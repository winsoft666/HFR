#include "OpenCVCameraPreview.h"
#include <QImage>
#include <QThread>
#include <QDate>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "OpenCVCamera.h"
#include "OpenCVDetect.h"
#include "OpenCVCommon.h"

using namespace cv;

OpenCVCameraPreview::OpenCVCameraPreview(QObject* parent /*= NULL*/) :
	QObject(parent),
	m_bEnabled(false),
	m_bLiveFaceDetect(false),
	m_pCamera(NULL),
	m_pOpenCVCamera(NULL)
{
	m_pTimer = new QTimer(this);
	m_pTimer->setInterval(100);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
}

OpenCVCameraPreview::~OpenCVCameraPreview()
{
}

bool OpenCVCameraPreview::enabled() const
{
	return m_bEnabled;
}

void OpenCVCameraPreview::setEnabled(bool b)
{
	m_bEnabled = b;

	if (m_bEnabled) {
		m_pTimer->start();
	}
	else {
		m_pTimer->stop();
	}

	emit enabledChanged(b);
}

bool OpenCVCameraPreview::liveFaceDetect() const
{
	return m_bLiveFaceDetect;
}

void OpenCVCameraPreview::setLiveFaceDetect(bool b)
{
	m_bLiveFaceDetect = b;

	emit liveFaceDetectChanged(b);
}

QObject * OpenCVCameraPreview::camera() const
{
	return m_pCamera;
}

void OpenCVCameraPreview::setCamera(QObject * pCamera)
{
	m_pCamera = pCamera;

	if (m_pCamera) {
		m_pOpenCVCamera = dynamic_cast<OpenCVCamera*>(m_pCamera);
	}

	if (m_pCamera && m_bEnabled) {
		m_pTimer->start();
	}
	else {
		m_pTimer->stop();
	}

	emit cameraChanged(pCamera);
}

void OpenCVCameraPreview::updateFrame()
{
	cv::Mat frame;
	QImage img;

	if (m_pOpenCVCamera && m_bEnabled && m_pOpenCVCamera->readMat(frame) && !frame.empty()) {
		if (m_bLiveFaceDetect) {
			if (!OpenCVDetect::instance()->drawFaceDetect(frame, frame)) {
				qDebug() << "ERROR drawFaceDetect failed";
			}
		}
		img = OpenCVCommon::cvMat2QImage(frame);
	}

	emit newVideoFrame(img);
}