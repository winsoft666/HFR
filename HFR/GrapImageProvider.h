#pragma once
#include <QImage>
#include <QMap>
#include <QQuickImageProvider>

class GrapImageProvider : public QQuickImageProvider, public QObject
{
public:
	static GrapImageProvider* instance();
	virtual ~GrapImageProvider();

	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
	QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
public:
	void addImage(const QString &id, const QImage &image);
protected:
	QMap<QString, QImage> m_mapImages;
private:
	GrapImageProvider();
	static GrapImageProvider *m_pInstance;
};

