#include "OpenCVCamera.h"
#include <QDebug>
#include <QImage>
#include <QUrl>
#include "GlobalValue.h"
#include "OpenCVCommon.h"
#include "OpenCVDetect.h"
#include "OpenCVRecognize.h"
#include "DlibRecognize.h"
#include <QDate>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

using namespace cv;

OpenCVCamera::OpenCVCamera(QObject *parent /*= 0*/) : 
	m_bEnabled(false),
	m_iGrapTotalTimes(0),
	m_iGrapTimes(0),
	m_bInRecongnizing(false),
	m_BeginTime(0),
	m_iPersonIndex(0)
{
	m_pGrapFaceTimer = new QTimer(this);
	m_pGrapFaceTimer->setInterval(100);
	connect(m_pGrapFaceTimer, SIGNAL(timeout()), this, SLOT(grapFaceFrameTimeProc()));

	m_pDlibRecognize = new DlibRecognize(parent);
	connect(m_pDlibRecognize, SIGNAL(RecoginizeResultNotify(bool)), this, SLOT(recognizeResultNotify(bool)));
}

OpenCVCamera::~OpenCVCamera()
{
	if (m_cvCap.isOpened()) {
		m_cvCap.release();
	}

	delete m_pDlibRecognize;
	m_pDlibRecognize = NULL;
}

bool OpenCVCamera::enabled() const
{
	return m_bEnabled;
}

void OpenCVCamera::setEnabled(bool b)
{
	m_bEnabled = b;

	if (m_bEnabled) {
		if (m_cvCap.isOpened())
			return;

		int deviceID = 0;             // 0 = open default camera
		int apiID = CV_CAP_ANY;       // 0 = autodetect default API
		m_cvCap.open(deviceID + apiID);

		if (!m_cvCap.isOpened()) {
			m_bEnabled = false;
			
			emit noVideoInputSignal();

			qDebug() << "ERROR camera open failed";
			return;
		}

		m_cvCap.set(CV_CAP_PROP_FRAME_WIDTH, 1024);
		m_cvCap.set(CV_CAP_PROP_FRAME_HEIGHT, 576);
	}
	else {
		if (m_cvCap.isOpened()) {
			m_cvCap.release();
		}
	}

	emit enabledChanged(b);
}

bool OpenCVCamera::readMat(cv::Mat & frame)
{
	cv::Mat src;
	static int iErrorTimes = 0;
	if (!m_cvCap.read(src)) {
		if (++iErrorTimes >= 3) {
			emit noVideoInputSignal();
		}
		return false;
	}

	if (src.empty()) {
		return false;
	}

	flip(src, frame, 1);

	return true;
}

void OpenCVCamera::grapImage(bool bFace, int iNum)
{
	if (bFace) {
		m_iGrapTimes = 0;
		m_iGrapTotalTimes = iNum;
		m_vGrapPerson.clear();
		m_pGrapFaceTimer->start();

		emit beginGrapFaceImage();
	}
	else {
		for (int i = 0; i < iNum; i++) {
			cv::Mat frame;
			if (readMat(frame)) {
				GrapImageData data;
				data.img = OpenCVCommon::cvMat2QImage(frame);

				emit newGrapImage(data);
			}
		}
	}

	return;
}

void OpenCVCamera::recognize(QString name,
	QString sex,
	QString nation,
	QString birthday,
	QString photoPath,
	QString address,
	QString cardNumber)
{
	if (m_bInRecongnizing) {
		return;
	}

	QImage imgPhoto(photoPath);
	cv::Mat faceMat = OpenCVCommon::QImage2cvMat(imgPhoto);
	if (faceMat.empty()) {
		return;
	}

	QVector<cv::Rect> faceRect;
	if (OpenCVDetect::instance()->faceImages(faceMat, &faceRect) < 0) {
		if (g_Root) {
			QMetaObject::invokeMethod(g_Root, "showMsg",
				Q_ARG(QVariant, QObject::tr("人脸检测模块加载失败")));
		}
		return;
	}
	if (faceRect.size() != 1) {
		if (g_Root) {
			QMetaObject::invokeMethod(g_Root, "showMsg",
				Q_ARG(QVariant, QObject::tr("身份证图片未含有人脸数据")));
		}
		return;
	}

	m_RecognizeFaceMat = cv::Mat(faceMat, faceRect[0]);

	m_IDCardInfo.imgPhoto = imgPhoto;
	m_IDCardInfo.strAddress = address;
	m_IDCardInfo.strBirthday = birthday;
	m_IDCardInfo.strCardNumber = cardNumber;
	m_IDCardInfo.strName = name;
	m_IDCardInfo.strNation = nation;
	m_IDCardInfo.strSex = sex;

	emit beginRecognize();
	
	m_bInRecongnizing = true;
	m_vGrapPerson.clear();

	grapImage(true, 1);
}

void OpenCVCamera::grapFaceFrameTimeProc()
{
	cv::Mat frame;
	if (!readMat(frame)) {
		return;
	}

	GrapImageData data;
	QVector<cv::Rect> faceRect;
	data.faceNum = OpenCVDetect::instance()->faceImages(frame, &faceRect);

	if (data.faceNum <= 0) {
		return;
	}

	for (size_t i = 0; i < faceRect.size(); i++) {
		FaceMat faceMat;
		faceMat.mat = frame.clone();
		faceMat.rect = faceRect[i];

		m_vGrapPerson.append(faceMat);

		cv::Mat mat = frame.clone();
		cv::rectangle(mat, faceRect[i], Scalar(0, 255, 0), 3);
		data.img = OpenCVCommon::cvMat2QImage(mat).copy();

		emit newGrapImage(data);
	}

	m_iGrapTimes++;

	if (m_iGrapTimes >= m_iGrapTotalTimes) {
		m_pGrapFaceTimer->stop();

		emit endGrapFaceImage();

		if (m_bInRecongnizing) {
			doRecognize();
		}
	}
}

void OpenCVCamera::recognizeResultNotify(bool bPass)
{
	qint64 endTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

	qDebug() << "DlibRecognize Used Time: " << endTime - m_BeginTime << "ms";

	m_iPersonIndex++;

	if (!bPass && m_iPersonIndex < m_vGrapPerson.size()) {
		
		cv::Mat faceMat(m_vGrapPerson[m_iPersonIndex].mat, m_vGrapPerson[m_iPersonIndex].rect);

		m_BeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

		m_pDlibRecognize->recognize(m_RecognizeFaceMat, faceMat);

		return;
	}

	m_bInRecongnizing = false;

	QString strResult;
	if (bPass) {
		strResult = tr("验证成功");
	}
	else {
		strResult = tr("验证失败");
	}

	if (g_Root) {
		QMetaObject::invokeMethod(g_Root, "showResult",
			Q_ARG(QVariant, strResult), Q_ARG(QVariant, bPass));
	}

	HFResult result;
	result.cardInfo = m_IDCardInfo;
	result.bRecognizeRet = bPass;
	result.imgCapture = OpenCVCommon::cvMat2QImage(m_vGrapPerson[0].mat);

	emit recognizeFinish(result);
}

void OpenCVCamera::doRecognize()
{
	if (m_RecognizeFaceMat.empty()) {
		return;
	}
	if (m_vGrapPerson.size() < 0) {
		return;
	}

	m_iPersonIndex = 0;

	cv::Mat faceMat(m_vGrapPerson[m_iPersonIndex].mat, m_vGrapPerson[m_iPersonIndex].rect);

	m_BeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

	m_pDlibRecognize->recognize(m_RecognizeFaceMat, faceMat);
}
