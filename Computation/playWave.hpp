#pragma once
#include <QtCore>
#include <iostream>
#include <string>
#include <fstream>
#include <QObject>
#include "mythread.h"
#include "interleave.h"

class WavePlayer: public QObject
{
	Q_OBJECT
   std::string activeFileName;
   bool isPlaying;
   bool isPaused;
   MyThread playerThread;
public:
    WavePlayer(QObject * par= nullptr);
	~WavePlayer();

    bool playMusic(const std::list<__sourceDefine>&);
    bool playMusic(const std::string&);
	void stopPlaying();
	// same call is used for pause/resume
	// set argument to false to resume playback
	void pausePlaying(bool = true);
	// method only as interface. Implemented as empty method;
    bool setVolumePercentage( int perc) { if( perc >=0 && perc <=100) return true; else return false;}
	// Player can be in the following states:
	// 0 - idle.
	// 1 - playing
	// 2 - paused (and can be resumed)
	int getPlayerState() const;
public slots:
    void onThreadFinish();
signals:
	// the code which uses this class for playing music
	// SHOULD connect to this signal. This signal is emitted
	// each time state machine of player changes.
	// Possible changes:
	//  1 - playing started
	//  2 - pause started
	//  3 - playing resumed
	//  4 - playing complete (file played till the end)
	//  5 - failed to start playing - file not found
	//  6 - failed to start playing - wrong file format
	//  7 - failed to start playing - file format is not supported
        //  15 - serious internal problems
	void playerReport( int status);
};

