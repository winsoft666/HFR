#pragma once

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QTimer>
#include "GrapImageData.h"
#include "HFResult.h"
#include "DlibRecognize.h"

struct FaceMat {
	cv::Mat mat;
	cv::Rect rect;
};

class OpenCVCamera : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
	explicit OpenCVCamera(QObject *parent = 0);
	virtual ~OpenCVCamera();

	bool enabled() const;
	virtual void setEnabled(bool b);


	bool readMat(cv::Mat &frame);
public slots:
	Q_INVOKABLE void grapImage(bool bFace, int iNum);
	Q_INVOKABLE void recognize(QString name,
		QString sex, 
		QString nation, 
		QString birthday, 
		QString photoPath, 
		QString address,
		QString cardNumber);
private slots:
	void grapFaceFrameTimeProc();
	void recognizeResultNotify(bool bPass);
signals:
	void enabledChanged(bool bEnable);

	void beginGrapFaceImage();
	void endGrapFaceImage();

	void newGrapImage(const GrapImageData& data);

	void beginRecognize();
	void endRecognize(bool result, double confidence);

	void noVideoInputSignal(); // ！！！ 如果在QML中定义OpenCVCamera对象，导致OpenCVCamera初始化过早，从而在QML中无法接受到信号 onEnableCameraFailed:{}
	void recognizeFinish(const HFResult &result);
protected:
	void doRecognize();
protected:
	bool m_bEnabled;
protected:
	QTimer* m_pGrapFaceTimer;
	cv::VideoCapture m_cvCap;

	bool m_bInRecongnizing;
	int m_iGrapTotalTimes;
	int m_iGrapTimes;
	QVector<FaceMat> m_vGrapPerson;
	int m_iPersonIndex;
	cv::Mat m_RecognizeFaceMat;

	IDCardInfo m_IDCardInfo;
	DlibRecognize *m_pDlibRecognize;
	qint64 m_BeginTime;
};