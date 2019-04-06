#include <QCoreApplication>
#include <QTextCodec>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include "Computation/interleave.h"

int main( int argc, char *argv[] )
{


    QCoreApplication a( argc, argv );
    QCoreApplication::setApplicationName("interleaviing-test");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::addLibraryPath("./plugins");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption fileOption(QStringList() << "f" << "file",
               QCoreApplication::translate("main", "read inputs from config"),
               QCoreApplication::translate("main", "fileName"));
    parser.addOption(fileOption);

    QCommandLineOption outfileOption(QStringList() << "o" << "output",
               QCoreApplication::translate("main", "write received data to <out>"),
               QCoreApplication::translate("main", "out"));
    parser.addOption(outfileOption);
    // Process the actual command line arguments given by the user
    parser.process(a);

     QString fileName = parser.value(fileOption);
     QString outFileName =parser.value(outfileOption); 

     qDebug()<<"After all we have input File"<<fileName<<"   and write output"<<outFileName;
     QFile qf(fileName );
     if(!qf.exists()) {
         qDebug()<<"File "<<fileName<<"  seems to not exists. Can not proceed";
     }
     if (!qf.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug()<<"Faild to open File "<<fileName<<". Can not proceed";
        return -1;
     }
     std::list<__sourceDefine> db;
     QTextStream in(&qf);
     while (!in.atEnd()) {
           __sourceDefine fr;
           int srcT;
            QString tmpPat;
           in>>srcT>>fr.path>>fr.port>>fr.remoteId;
           fr.srcType = ((srcT == 0)?( __sourceDefine::LOCAL) :__sourceDefine::REMOTE);
            if( fr.path != "")
                  db.push_back(fr);
            //qDebug()<<"path was"<<fr.path;
     }
     qf.close();

    multiPointReceiver m( &a);
    m.setSources(db);
    char buff[4000];
    qint64 rv;
    QFile outF(outFileName);
    if (!outF.open(QIODevice::WriteOnly )) {
        qDebug()<<"Faild to open File "<<outFileName<<". Can not proceed";
       return -1;
    }

    qDebug()<<"File is "<<m.getObjectSize()<<"  bytes long";

    qint64 total = 0;
    while((rv=m.getDataToBufferFromOffset(buff, 4000, total))> 0) {
        outF.write(buff, rv);
        total += rv;
    }
    outF.close();

    qDebug()<<"a.exec() exited";
}
