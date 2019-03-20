#include "class.h"
#include <QTime>
#include <QFileDialog>
#include <iostream>

CLIClass::CLIClass(int a, char ** v, QObject * par) : QObject(par), argc(a), argv(v)
  , next(1)
{
	kernel = new WavePlayer( this);
    connect(kernel, SIGNAL(playerReport(int)), this, SLOT(onPlayerReport(int)));
}

CLIClass::~CLIClass()
{
}

void CLIClass::run()
{
	for(int i=1; i < argc; ++i){
        std::cout<<"Starting playing "<<argv[i]<<std::endl;
        kernel->playMusic(std::string(argv[i]));
        next = 0;
        while( ! next) Sleep(10);
	}
    std::cout<<"Playlist complete"<<std::endl;
    emit finished();
}

void CLIClass::onPlayerReport(int repVal)
{
    std::cout<<"Received from player thread value"<<repVal<<std::endl;
    next = 1;
}
