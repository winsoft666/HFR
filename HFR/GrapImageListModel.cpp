#include "GrapImageListModel.h"
#include <QVector>
#include <QDebug>
#include <QDateTime>
#include <GrapImageProvider.h>

enum Roles {
	ImgRole = Qt::UserRole + 1,
	TimeRole,
	FaceNumRole,
};

class GrapImageListModelPrivate {
public:
	GrapImageListModelPrivate() {
		m_roleNames.insert(ImgRole, "imgUri");
		m_roleNames.insert(TimeRole, "time");
		m_roleNames.insert(FaceNumRole, "faceNum");
	}

	~GrapImageListModelPrivate() {
		clear();
	}

	void add(const GrapImageData& data) {
		m_vGrapImages.append(data);
	}

	void clear() {
		m_vGrapImages.clear();
	}

	QHash<int, QByteArray> m_roleNames;
	QVector<GrapImageData> m_vGrapImages;
};

GrapImageListModel::GrapImageListModel(QObject* parent /*= NULL*/) :
	QAbstractListModel(parent),
	m_pPrivate(new GrapImageListModelPrivate()),
	m_iIndex(0)
{
}

GrapImageListModel::~GrapImageListModel()
{
	delete m_pPrivate;
	m_pPrivate = NULL;
}

int GrapImageListModel::rowCount(const QModelIndex & parent) const
{
	return m_pPrivate->m_vGrapImages.size();
}

QVariant GrapImageListModel::data(const QModelIndex & index, int role) const
{
	int row = index.row();
	if (row < 0 || row >= m_pPrivate->m_vGrapImages.size()) {
		return QVariant();
	}

	GrapImageData &data = m_pPrivate->m_vGrapImages[row];
	QDateTime now = QDateTime::currentDateTime();

	switch (role)
	{
	case ImgRole:
		m_iIndex++;
		GrapImageProvider::instance()->addImage(QString::number(m_iIndex), data.img);

		return "image://GrapImageProvider/" + QString::number(m_iIndex);
	case FaceNumRole:
		return data.faceNum;
	}

	return QVariant();
}

QHash<int, QByteArray> GrapImageListModel::roleNames() const
{
	return m_pPrivate->m_roleNames;
}


void GrapImageListModel::addGrapImage(const GrapImageData& data)
{
	int size = m_pPrivate->m_vGrapImages.size();

	beginInsertRows(QModelIndex(), size, size);

	m_pPrivate->add(data);

	endInsertRows();
}

void GrapImageListModel::clearGrapImage()
{
	beginResetModel();

	m_pPrivate->clear();

	endResetModel();
}
