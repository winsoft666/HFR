#include "DlibRecognize.h"
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QMutexLocker>
#include <QCoreApplication>
#include "GlobalValue.h"
#include "CmnUtil.h"


#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

DlibRecognize::DlibRecognize(QObject* parent /* = NULL*/)
{
	m_bInit = true;
	QString strDataPath = QCoreApplication::applicationDirPath() + "/data/dlib_face_recognition_resnet_model_v1.dat";

	try {
		deserialize(cpp4j::Utf8ToAnsi(strDataPath.toStdString())) >> net;
	}
	catch (serialization_error &e) {
		qDebug() << "DlibRecognize deserialize failed: " << strDataPath;
		m_bInit = false;
	}
}


DlibRecognize::~DlibRecognize()
{
}


void DlibRecognize::recognize(const cv::Mat &face1, const cv::Mat &face2)
{
	m_Face1 = face1.clone();
	m_Face2 = face2.clone();

	start();
}

void DlibRecognize::run()
{
	if (!m_bInit) {
		if (g_Root) {
			QMetaObject::invokeMethod(g_Root, "showMsg",
				Q_ARG(QVariant, QObject::tr("人脸识别模块初始化失败")));
		}
		return;
	}
	QString str = QCoreApplication::applicationDirPath() + "/HFR_FACE1.jpg";
	std::string strTmpFace1 = cpp4j::Utf8ToAnsi(str.toStdString());

	str = QCoreApplication::applicationDirPath() + "/HFR_FACE2.jpg";
	std::string strTmpFace2 = cpp4j::Utf8ToAnsi(str.toStdString());

	cv::Size faceSize = cv::Size(150, 150);
	cv::resize(m_Face1, m_Face1, faceSize);
	cv::resize(m_Face2, m_Face2, faceSize);

	cv::imwrite(strTmpFace1, m_Face1);
	cv::imwrite(strTmpFace2, m_Face2);

	matrix<rgb_pixel> img1;
	matrix<rgb_pixel> img2;
	load_image(img1, strTmpFace1);
	load_image(img2, strTmpFace2);

	std::vector<matrix<rgb_pixel>> faces;
	faces.push_back(img1);
	faces.push_back(img2);

	std::vector<matrix<float, 0, 1>> face_descriptors = net(faces);

	float f = length(face_descriptors[0] - face_descriptors[1]);
	qDebug() << "DlibRecognize VALUE: " << f;

	bool bRet = f <= g_Threshold;

	emit RecoginizeResultNotify(bRet);
}
