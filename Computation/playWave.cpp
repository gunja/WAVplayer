#include "playWave.hpp"
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)
struct wavHeaderStruct {
    uint32_t riff;
    uint32_t chunkSize;
    uint32_t wave;

};

struct _FMTsubChunkStruct {
    uint32_t id;
    uint32_t size;
    uint16_t format;
    uint16_t chanNum;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bpS;
};

struct _DatasubChunkHeaderStruct {
    uint32_t id;
    uint32_t len;
};
#pragma pack( pop)

#define RIFF 0x46464952
#define WAVE 0x45564157
#define FMT 0x20746d66
#define DATA 0x61746164

WavePlayer::WavePlayer(QObject * par) : QObject(par), activeFileName("")
    , isPlaying(false) , isPaused(false), playerThread(this)
{
// TODO
    connect(&playerThread, SIGNAL(doneWork()), this, SLOT(onThreadFinish()));
}

WavePlayer::~WavePlayer()
{
// TODO
    if(isPlaying) {
        playerThread.setEnabled(false);
    }
    int counter = 100;
    while(counter-- > 0 && isPlaying) {
        Sleep(10);
    }
    if(isPlaying) {
        // there's a problem somewhere
    }
    // DONE
}

int WavePlayer::getPlayerState() const
{
    if(isPlaying ) {
        if (isPaused)
            return 2;
        else
            return 1;
    }
    return 0;
}

bool WavePlayer::playMusic(const std::list<__sourceDefine>&sources)
{
    // TODO
    // Are we playing something already?
    // if so, first let's force file to exit
    if( isPlaying) {
        playerThread.setEnabled(false);
    }
    // now slot on finish should execute
    int counter = 100;
    while(counter-- > 0 && isPlaying) {
        Sleep(10);
    }
    if(isPlaying) {
        emit playerReport( 15);
        return false;
    }
    struct wavHeaderStruct wv;
    struct _FMTsubChunkStruct fmt;
    struct _DatasubChunkHeaderStruct dataHdr;
    FILE * fin = fopen( filePath.c_str(),"rb");
    if( fin == nullptr) {
        emit playerReport( 5);
        return false;
    }
    // if it's WAV file, first 12 bytes should be
    // of pre-defined form:
    size_t rv = fread(&wv, sizeof(struct wavHeaderStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return false;
    }
    if(wv.riff != RIFF  || wv.wave != WAVE ) {
        fclose( fin);
        emit playerReport( 7);
        return false;
    }
    rv = fread(&fmt, sizeof(struct _FMTsubChunkStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return false;
    }
    if(fmt.id != FMT || fmt.size != 16) {
        fclose( fin);
        emit playerReport( 7);
        return false;
    }
    rv = fread(&dataHdr, sizeof(struct _DatasubChunkHeaderStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return false;
    }
    if(dataHdr.id != DATA ) {
        fclose( fin);
        emit playerReport( 7);
        return false;
    }
    WAVEFORMATEX wfx;
    wfx.nSamplesPerSec =  fmt.sampleRate; /* sample rate */
    wfx.wBitsPerSample = fmt.bpS; /* sample size */
    wfx.nChannels = fmt.chanNum; /* channels*/
    wfx.cbSize = 0; /* size of _extra_ info */
    wfx.wFormatTag = fmt.format;
    wfx.nBlockAlign = fmt.blockAlign;
    wfx.nAvgBytesPerSec = fmt.byteRate;

    if( playerThread.openAudio(wfx)) {
        fclose( fin);
        emit playerReport( 7);
        return false;
    }
    playerThread.setFile(fin);
    playerThread.setEnabled();
    playerThread.setLength(dataHdr.len);

    // if everything started well,
    isPlaying = true;
    playerThread.start();
    std::cout << "the music is playing ..." << std::endl;
    return isPlaying;
    // close file when thread reports it's done
}

void WavePlayer::onThreadFinish()
{
    isPlaying = false;
    std::cout<<"play thread complete music file"<<std::endl;
    emit playerReport(4);
}

void WavePlayer::stopPlaying()
{
    if(isPaused) {
        // TODO needed for unpause
    }
    isPaused = false;
    if(isPlaying) {
        // TODO stop playing. clear objects
        playerThread.setEnabled(false);
    }
    std::cout << "the music is stop!" << std::endl;
}

void WavePlayer::pausePlaying( bool setPaused)
{
    if( ! isPlaying) return;

    if(setPaused) {
        // already paused
        if(isPaused) return;
        // TODO set to pause
        isPaused = true;
    } else {
        if( ! isPaused) return;
        // TODO do needed this to resume playing
        isPaused = false;
    }
}
