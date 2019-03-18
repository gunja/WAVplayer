#pragma once
#include <QtCore>
#include <QStringList>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <iostream>
#include "Computation/playWave.hpp"

using namespace std;

namespace Ui {
	class Screen;
};

class Screen : public QWidget
{
	Q_OBJECT
public:
	Screen();
	~Screen() { delete ui; };

private slots:
	void openWorkDir();
	void playIt();

public:
	WavePlayer *kernel;
	string filePath;

	Ui::Screen *ui;
};