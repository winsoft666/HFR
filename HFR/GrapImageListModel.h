#pragma once

#include <QAbstractListModel>
#include <QImage>
#include "GrapImageData.h"


class GrapImageListModelPrivate;

class GrapImageListModel : public QAbstractListModel {
	Q_OBJECT
public:
	GrapImageListModel(QObject* parent = NULL);
	virtual ~GrapImageListModel();

	// virtual function.
	int rowCount(const QModelIndex &parent = QModelIndex() ) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
	QHash<int, QByteArray> roleNames() const;
public slots:
	void addGrapImage(const GrapImageData& data);
	void clearGrapImage();
signals:
	void newImage(const QImage &img) const; // 连接ImageDisplay::setImage槽
protected:
	GrapImageListModelPrivate* m_pPrivate;
private:
	mutable int m_iIndex;
};
