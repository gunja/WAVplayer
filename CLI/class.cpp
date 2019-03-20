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
        std::cout<<"Starting playing "<<argv[i]<<std::endl;
        kernel->playMusic(std::string(argv[i]));
        ++i;
	}
}

void CLIClass::onPlayerReport(int repVal)
{
    std::cout<<"Received from player thread value"<<repVal<<std::endl;
    if(i < argc) {
        run();
        return;
    }
    std::cout<<"Playlist complete"<<std::endl;
    emit finished();
}
