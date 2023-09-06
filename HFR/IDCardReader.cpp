#include "IDCardReader.h"
#include <QDebug>
#include <windows.h>
#include "RegKey.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

IDCardReader::IDCardReader(QObject* parent /*= NULL*/)
{
	m_hNotifyEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\\HumanFaceRecognitionNotify"));
	ResetEvent(m_hNotifyEvent);

	start();
}

IDCardReader::~IDCardReader()
{
	requestInterruption();
	wait();
	CloseHandle(m_hNotifyEvent);
}

QString IDCardReader::cardNumber() const
{
	return m_strCardNumber;
}

QString IDCardReader::name() const
{
	return m_strName;
}

QString IDCardReader::sex() const
{
	return m_strSex;
}

QString IDCardReader::nation() const
{
	return m_strNation;
}

QString IDCardReader::birthday() const
{
	return m_strBirthday;
}

QString IDCardReader::photoPath() const
{
	return m_strPhotoPath;
}

QString IDCardReader::address() const
{
	return m_strAddress;
}

void IDCardReader::run()
{
	while (!isInterruptionRequested())
	{
		if (WaitForSingleObject(m_hNotifyEvent, 100) == WAIT_OBJECT_0) {
			ResetEvent(m_hNotifyEvent);

			RegKey reg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\CashHFR"));
			HRESULT hr;
			if ((hr = reg.Open(KEY_READ | KEY_WOW64_64KEY, false)) == S_OK) {
				std::wstring strValue;
				do 
				{
					hr = reg.GetSZValue(TEXT("name"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strName = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("nation"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strNation = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("birthday"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strBirthday = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("cardNum"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strCardNumber = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("photoPath"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strPhotoPath = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("sex"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strSex = QString::fromWCharArray(strValue.c_str());

					hr = reg.GetSZValue(TEXT("address"), strValue);
					if (hr != S_OK) {
						break;
					}
					m_strAddress = QString::fromWCharArray(strValue.c_str());

				} while (false);

				reg.Close();

				if (hr == S_OK) {
					emit newIdCard();
				}
			}
		}
	}
}
