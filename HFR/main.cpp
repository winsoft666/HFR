#include <QObject>
#include <QDebug>
#include <QSharedMemory>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDate>
#include <QtDebug>
#include <QSettings>
#include <qlogging.h>
#include <QTextCodec>
#include <windows.h>
#include "OpenCVCamera.h"
#include "OpenCVCameraPreview.h"
#include "VideoItem.h"
#include "GrapImageProvider.h"
#include "GrapImageListModel.h"
#include "IDCardReader.h"
#include "GlobalValue.h"
#include "CmnUtil.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif


#define _TIME_ qPrintable (QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))

void Log(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
	QString qstrText;
	switch (type)
	{
	case QtDebugMsg:
		qstrText = QString("%1: %2\n").arg(_TIME_, msg);
		break;
	case QtWarningMsg:
		qstrText = QString("%1: %2\n").arg(_TIME_, msg);
		break;
	case QtCriticalMsg:
		qstrText = QString("%1: %2\n").arg(_TIME_, msg);
		break;
	case QtFatalMsg:
		qstrText = QString("%1: %2\n").arg(_TIME_, msg);
		break;
	}

	OutputDebugString(qstrText.toStdWString().c_str());
	QFile out("log.txt");
	out.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream ts(&out);
	ts << qstrText << endl;
}

bool AssumeSingleInstance(const char* program)
{
	static QSharedMemory shm(program);
	if (!shm.create(12)) {
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	if (!AssumeSingleInstance("HFR.QT")) {
		return 0;
	}

	bool bUseOpenGLDesktop = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "opengl") == 0) {
			bUseOpenGLDesktop = true;
		}
		else {
			if (argv[i][0] == 't') {
				g_Threshold = atof(argv[i] + 1);
			}
		}
	}

	qmlRegisterType<OpenCVCamera>("HFR.OpenCV", 1, 0, "OpenCVCamera");
	qmlRegisterType<VideoItem>("HFR.VideoItem", 1, 0, "VideoItem");
	qmlRegisterType<GrapImageListModel>("HFR.Model", 1, 0, "GrapImageListModel");
	qmlRegisterType<IDCardReader>("HFR.IDCard", 1, 0, "IDCardReader");

	QGuiApplication::setApplicationName("HFR");
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	if (bUseOpenGLDesktop)
		QGuiApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	else
		QGuiApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

	QGuiApplication app(argc, argv);

	qInstallMessageHandler(Log);

	QQmlApplicationEngine engine;

	engine.rootContext()->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath());

	GrapImageListModel *pGrapImageModel = new GrapImageListModel();
	engine.rootContext()->setContextProperty("grapImageListModel", pGrapImageModel);
	engine.addImageProvider(QLatin1String("GrapImageProvider"), GrapImageProvider::instance());


	OpenCVCamera *pCamera = new OpenCVCamera();
	QObject::connect(pCamera, SIGNAL(newGrapImage(const GrapImageData&)), pGrapImageModel, SLOT(addGrapImage(const GrapImageData&)));

	OpenCVCameraPreview* pCameraPreview = new OpenCVCameraPreview();

	engine.rootContext()->setContextProperty("camera", pCamera);
	engine.rootContext()->setContextProperty("cameraPreview", pCameraPreview);

	engine.load(QUrl(QLatin1String("qrc:///HFR/Resources/main_high.qml")));

	QObject *root = NULL;
	QList<QObject*> rootObjects = engine.rootObjects();
	for (int i = 0; i < rootObjects.size(); i++) {
		if (rootObjects.at(i)->objectName() == "root") {
			root = rootObjects.at(i);
			break;
		}
	}

	g_Root = root;

	if (root) {
		QObject *pVideo = root->findChild<QObject*>("video");
		if (pVideo) {
			QObject::connect(pCameraPreview, SIGNAL(newVideoFrame(const QImage&)), pVideo, SLOT(updateFrame(const QImage&)));
		}
	}


	pCamera->setEnabled(true);
	pCameraPreview->setCamera(pCamera);
	pCameraPreview->setEnabled(true);

	int iAppReturn = app.exec();

	pCameraPreview->setEnabled(false);
	delete pCameraPreview;

	pCamera->setEnabled(false);
	delete pCamera;

	return iAppReturn;
}
