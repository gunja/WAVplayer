#include "uiClass.h"
#include <QTime>
#include <QFileDialog>
#include "ui_GUI.h"

Screen::Screen() : ui(new Ui::Screen)
{
	ui->setupUi(this);
	setWindowTitle(tr("Template"));
	//setWindowState(Qt::WindowMaximized);

	//! connects
	connect(ui->pushButton_select, SIGNAL(clicked()), this, SLOT(openWorkDir()));
	connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(playIt()));
}


void Screen::openWorkDir()
{
	QString workDir = QFileDialog::getExistingDirectory(0, "Select", "./");
	if (workDir == nullptr)
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Invalid directory!"), QMessageBox::Ok);
		return;
	}
	workDir.replace("\\", "/");
	//workDir = workDir + "/FMWorkDir";
	ui->lineEdit_workDir->setText(workDir);
	string workDir_str = string((const char *)workDir.toLocal8Bit());
	filePath = workDir_str + "/numb.wav";
	kernel = new WavePlayer();
}


void Screen::playIt()
{
	kernel->playMusic(filePath);
}