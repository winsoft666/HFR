#pragma once


#include <QTimer>
#include <opencv2/opencv.hpp>

class OpenCVCamera;

class OpenCVCameraPreview : public QObject {
	Q_OBJECT
	Q_PROPERTY(QObject* camera READ camera WRITE setCamera NOTIFY cameraChanged)
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(bool liveFaceDetect READ liveFaceDetect WRITE setLiveFaceDetect NOTIFY liveFaceDetectChanged)
public:
	explicit OpenCVCameraPreview(QObject* parent = NULL);
	virtual ~OpenCVCameraPreview();

	bool enabled() const;
	virtual void setEnabled(bool b);

	bool liveFaceDetect() const;
	virtual void setLiveFaceDetect(bool b);

	QObject* camera() const;
	void setCamera(QObject* pCamera);

signals:
	void cameraChanged(QObject*);
	void enabledChanged(bool);
	void liveFaceDetectChanged(bool);
	void newVideoFrame(const QImage &img);

public slots:
	void updateFrame();
protected:
	bool m_bEnabled;
	bool m_bLiveFaceDetect;
	QTimer *m_pTimer;
	QObject *m_pCamera;
	OpenCVCamera* m_pOpenCVCamera;
};