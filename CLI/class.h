#pragma once
#include <QtCore>
#include <QStringList>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <iostream>
#include "Computation/playWave.hpp"

using namespace std;

class CLIClass : public QObject
{
	int argc;
    char **argv;
    int next;
	Q_OBJECT
public:
    CLIClass(int, char **b, QObject*par=nullptr);
    ~CLIClass();

public slots:
	void run();
    void onPlayerReport(int);
signals:
	void finished();

public:
	WavePlayer *kernel;
	string filePath;

};
