#pragma once

#include <QImage>
#include <QQuickItem>
#include <QMutex>

class VideoItem : public QQuickItem {
	Q_OBJECT
public:
	explicit VideoItem(QQuickItem* parent = NULL);
	virtual ~VideoItem();

signals:
	void fpsChanged(int fps);
public slots:
	void updateFrame(const QImage &img);
protected:
	QSGNode* updatePaintNode(QSGNode * old, UpdatePaintNodeData *);

	QImage m_Img;
	QMutex m_imgMutex;

	QVector<qint64> _times;
};