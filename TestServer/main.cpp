#include <QCoreApplication>
#include <QTextCodec>
#include "tcpServerClass.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>

int main( int argc, char *argv[] )
{


    QCoreApplication a( argc, argv );
    QCoreApplication::setApplicationName("my-tcp-server");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::addLibraryPath("./plugins");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    // An option with a value
    QCommandLineOption portNumberOption(QStringList() << "p" << "port",
               QCoreApplication::translate("main", "Listen to port <port>"),
                QCoreApplication::translate("main", "portNum"));
    parser.addOption(portNumberOption);

    QCommandLineOption fileOption(QStringList() << "f" << "file",
               QCoreApplication::translate("main", "read inputs from config"),
               QCoreApplication::translate("main", "fileName"));
    parser.addOption(fileOption);
    // Process the actual command line arguments given by the user
    parser.process(a);

     QString port = parser.value(portNumberOption);
     QString fileName = parser.value(fileOption);

     int portNum = port.toInt();

     qDebug()<<"After all we have port="<<portNum<<"   and fileName"<<fileName;
     QFile qf(fileName );
     if(!qf.exists()) {
         qDebug()<<"File "<<fileName<<"  seems to not exists. Can not proceed";
     }
     if (!qf.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug()<<"Faild to open File "<<fileName<<". Can not proceed";
        return -1;
     }
     std::list<__FileRec> db;
     QTextStream in(&qf);
     while (!in.atEnd()) {
           __FileRec fr;
           in>>fr.id>>fr.filePath;
           db.push_back(fr);
     }

    TCPServerClass m( portNum, db, &a);
	QObject::connect(&m, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &m, SLOT(run()));
    return a.exec();
    qDebug()<<"a.exec() exited";
}
