#include "GrapImageProvider.h"
#include <QMutex>
#include <QCoreApplication>

GrapImageProvider* GrapImageProvider::m_pInstance = NULL;

GrapImageProvider::GrapImageProvider() :
	QQuickImageProvider(QQuickImageProvider::Image),
	QObject(QCoreApplication::instance())
{

}

GrapImageProvider * GrapImageProvider::instance()
{
	static QMutex mutex;
	if (!m_pInstance) {
		QMutexLocker lock(&mutex);
		if (!m_pInstance) {
			m_pInstance = new GrapImageProvider();
		}
	}
	return m_pInstance;
}

GrapImageProvider::~GrapImageProvider()
{

}

QImage GrapImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
	return m_mapImages[id];
}

QPixmap GrapImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
	return QPixmap::fromImage(m_mapImages[id]);
}

void GrapImageProvider::addImage(const QString &id, const QImage &image)
{
	m_mapImages[id] = image.copy();
}
