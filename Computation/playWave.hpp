#pragma once
#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include <string>
#include <fstream>

using namespace std;

class WavePlayer
{
public:
	WavePlayer() {};
	~WavePlayer() {};

	void playMusic(string filePath);
	void stopPlaying();
};