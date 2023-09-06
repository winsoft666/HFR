#include <VideoItem.h>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGSimpleTextureNode>
#include <QSGGeometryNode>
#include <QThread>
#include <QDate>
#include "OpenCVCommon.h"

VideoItem::VideoItem(QQuickItem* parent /*= NULL*/) : QQuickItem(parent)
{
	setFlag(QQuickItem::ItemHasContents);
}

VideoItem::~VideoItem()
{

}

void VideoItem::updateFrame(const QImage &img)
{
	do 
	{
		QMutexLocker locker(&m_imgMutex);
		m_Img = img.copy();
	} while (false);
	update();
}

QSGNode* VideoItem::updatePaintNode(QSGNode * old, UpdatePaintNodeData *)
{
	qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
	_times.push_back(currentTime);

	while (_times[0] < currentTime - 1000) {
		_times.pop_front();
	}
	emit fpsChanged(_times.length());

	QMutexLocker locker(&m_imgMutex);
	if (m_Img.isNull()) {
		return old;
	}

	QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode*>(old);
	if (node == NULL) {
		node = new QSGSimpleTextureNode();
	}

	QSGTexture *t = window()->createTextureFromImage(m_Img.scaled(boundingRect().size().toSize()));

	if (t) {
		QSGTexture *tt = node->texture();
		if (tt) {
			tt->deleteLater();
		}
		node->setRect(boundingRect());
		node->setTexture(t);
	}

	return node;
}

