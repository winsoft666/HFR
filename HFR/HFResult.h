#pragma once
#include <QImage>

struct IDCardInfo {
	QString strCardNumber;
	QString strName;
	QString strSex;
	QString strNation;
	QString strBirthday;
	QString strAddress;
	QImage imgPhoto;
};

struct HFResult {
	bool bRecognizeRet;
	IDCardInfo cardInfo;
	QImage imgCapture;

	HFResult() {
		bRecognizeRet = false;
	}
};