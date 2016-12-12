#include "durl.h"
#include "dfmglobal.h"

#include "filemanagerapp.h"
#include "logutil.h"
#include "singleapplication.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "widgets/commandlinemanager.h"

#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "dialogs/openwithdialog.h"
#include "controllers/appcontroller.h"
#include "widgets/singleton.h"

#include "filedialogmanager_adaptor.h"
#include "filedialog/dbusfiledialogmanager.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QProcess>

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

#define fileManagerApp Singleton<FileManagerApp>::instance()

DWIDGET_USE_NAMESPACE

bool registerDialogDBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    if (!QDBusConnection::sessionBus().registerService("com.deepin.filemanager.filedialog")) {
        qWarning("Cannot register the \"com.deepin.filemanager.filedialog\" service.\n");
        return false;
    }

    DBusFileDialogManager *manager = new DBusFileDialogManager();
    Q_UNUSED(new FiledialogmanagerAdaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject("/com/deepin/filemanager/filedialogmanager", manager)) {
        qWarning("Cannot register to the D-Bus object.\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    SingleApplication::loadDXcbPlugin();
    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationName(QMAKE_TARGET);
    app.setApplicationVersion(QMAKE_VERSION);
    app.loadTranslator();

    DFMGlobal::installTranslator();
    DThemeManager::instance()->setTheme("light");

    QFont font;
    font.setPixelSize(14);
    app.setFont(font);

    LogUtil::registerLogger();

    CommandLineManager::instance()->process();

    // show file selection dialog
    if (CommandLineManager::instance()->isSet("f")) {
        if (!registerDialogDBus()) {
            qWarning() << "register dialog dbus failed.";

            return 1;
        }

        app.setQuitOnLastWindowClosed(false);

        return app.exec();
    }

    DUrlList commandlineUrlList;
    foreach (QString path, CommandLineManager::instance()->positionalArguments()) {
        commandlineUrlList << DUrl::fromUserInput(path);
    }
    if (commandlineUrlList.isEmpty()){
        commandlineUrlList << DUrl::fromLocalFile(QDir::homePath());
    }

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);
    bool isBackendRun = CommandLineManager::instance()->isSet("d");
    bool isShowPropertyRequest = CommandLineManager::instance()->isSet("p");

    qDebug() << isSingleInstance << commandlineUrlList;

    if (isSingleInstance){
        DFMGlobal::installTranslator();
        DThemeManager::instance()->setTheme("light");

        if (!isBackendRun && !isShowPropertyRequest){
            foreach (DUrl url, commandlineUrlList) {
                fileManagerApp->show(url);
            }
        }else{
            fileManagerApp;
        }

        if(isShowPropertyRequest){
            QStringList paths = CommandLineManager::instance()->positionalArguments();
            fileManagerApp->showPropertyDialog(paths);
        }

        FileUtils::setDefaultFileManager();
#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();
        quick_exit(request);
#else
        int ret = app.exec();
#ifdef AUTO_RESTART_DEAMON
        app.closeServer();
        QProcess::startDetached(QString("%1 -d").arg(QString(argv[0])));
#endif
        quick_exit(ret);
#endif
    }else{
        if (isBackendRun){
            return 0;
        }
        SingleApplication::newClientProcess(uniqueKey);
        QWidget w;
        w.setWindowFlags(Qt::FramelessWindowHint);
        w.setAttribute(Qt::WA_TranslucentBackground);
        w.resize(0, 0);
        w.show();
    }
}
