#include "class.h"
#include <QTime>
#include <QFileDialog>
#include <iostream>

CLIClass::CLIClass(int a, char ** v, QObject * par) : QObject(par), argc(a), argv(v)
  , next(1), i(1)
{
	kernel = new WavePlayer( this);
    connect(kernel, SIGNAL(playerReport(int)), this, SLOT(onPlayerReport(int)));
}

CLIClass::~CLIClass()
{
}

void CLIClass::run()
{
    if( i < argc)
    {
        if( std::string("-f") == argv[i]
                || std::string("--file") == argv[i] ) {
            ++i;
            std::cout<<"Starting playing "<<argv[i]<<std::endl;
            kernel->playMusic(std::string(argv[i]));
        }
        else if (std::string("-c") == argv[i]
                 || std::string("--config") == argv[i] ) {
             ++i;
            qDebug()<<"After all we have input File"<<argv[i];
            QFile qf(argv[i] );
            if(!qf.exists()) {
                qDebug()<<"File "<<argv[i]<<"  seems to not exists. Can not proceed";
            }
            if (!qf.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug()<<"Faild to open File "<<argv[i]<<". Can not proceed";
               emit finished();
                return;
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
            }
            qf.close();
            kernel->playMusic(db);
        }
        else {
            qDebug()<<"unknown option "<<argv[i]<<
               "on command line. Force exit";
            emit finished();
        }
	}
	else {
		std::cout << "Playlist is empty" << std::endl;
		emit finished();
	}
}

void CLIClass::onPlayerReport(int repVal)
{
    std::cout<<"Received from player thread value "<<repVal<<std::endl;
    i++;
    if(i < argc) {
        run();
        return;
    }
    std::cout<<"Playlist complete"<<std::endl;
    emit finished();
}
