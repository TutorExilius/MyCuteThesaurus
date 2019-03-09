#include "mainwindow.h"
#include <QApplication>
#include <QDir>

#include "log.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // if logs dir not exists, create dir
    QString logDirStr{ QApplication::applicationDirPath() + "/logs" };

    QDir logDir;
    if( !logDir.exists( logDirStr ) )
    {
        logDir.mkdir( logDirStr );
    }
    // ---

    QString logPath{ logDirStr + "/log.txt" };

    ::init_log( logPath.toStdString().c_str() );

    MainWindow w;
    w.show();

    return a.exec();
}
