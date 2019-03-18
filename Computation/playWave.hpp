#pragma once
#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include <string>
#include <fstream>
#include <QObject>

using namespace std;

class WavePlayer
{
	Q_OBJECT
   std::string activeFileName;
   bool isPlaying;
public:
	WavePlayer();
	~WavePlayer();

	void playMusic(string filePath);
	void stopPlaying();
	// same call is used for pause/resume
	// set argument to false to resume playback
	void pausePlaying(bool = true);
	// method only as interface. Implemented as empty method;
	bool setVolumePercentage( int perc) { if( perc >=0 && perc <=100) return true; else return false;};
	// Player can be in the following states:
	// 0 - idle.
	// 1 - playing
	// 2 - paused (and can be resumed)
	int getPlayerState() const;
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
	void playerReport( int status);
};

