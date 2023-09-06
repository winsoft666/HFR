#pragma once

#include <QObject>
#include <QThread>
#include "HFResult.h"

class IDCardReader : public QThread {
	Q_OBJECT
	Q_PROPERTY(QString cardNumber READ cardNumber)
	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(QString sex READ sex)
	Q_PROPERTY(QString nation READ nation)
	Q_PROPERTY(QString birthday READ birthday)
	Q_PROPERTY(QString photoPath READ photoPath)
	Q_PROPERTY(QString address READ address)
public:
	IDCardReader(QObject* parent = NULL);
	virtual ~IDCardReader();

	QString cardNumber() const;
	QString name() const;
	QString sex() const;
	QString nation() const;
	QString birthday() const;
	QString photoPath() const;
	QString address() const;
protected:
	void run();
signals:
	void newIdCard();
protected:
	Qt::HANDLE m_hNotifyEvent;
	QString m_strPhotoPath;
	QString m_strCardNumber;
	QString m_strName;
	QString m_strSex;
	QString m_strNation;
	QString m_strBirthday;
	QString m_strAddress;
};