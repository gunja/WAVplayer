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
    uint32_t bytRate;
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
}

WavePlayer::~WavePlayer()
{
// TODO
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

void WavePlayer::playMusic(std::string filePath)
{
    // TODO
    struct wavHeaderStruct wv;
    struct _FMTsubChunkStruct fmt;
    struct _DatasubChunkHeaderStruct dataHdr;
    FILE * fin = fopen( filePath.c_str(),"rb");
    if( fin == nullptr) {
        emit playerReport( 5);
        return;
    }
    // if it's WAV file, first 12 bytes should be
    // of pre-defined form:
    size_t rv = fread(&wv, sizeof(struct wavHeaderStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return;
    }
    if(wv.riff != RIFF  || wv.wave != WAVE ) {
        fclose( fin);
        emit playerReport( 7);
        return;
    }
    rv = fread(&fmt, sizeof(struct _FMTsubChunkStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return;
    }
    if(fmt.id != FMT || fmt.size != 16) {
        fclose( fin);
        emit playerReport( 7);
        return;
    }
    rv = fread(&dataHdr, sizeof(struct _DatasubChunkHeaderStruct), 1, fin);
    if( rv != 1 ) {
        fclose( fin);
        emit playerReport( 6);
        return;
    }
    if(dataHdr.id != DATA ) {
        fclose( fin);
        emit playerReport( 7);
        return;
    }
    playerThread.setFile(fin);
    playerThread.setLength(dataHdr.len);
    playerThread.start();
        std::cout << "the music is playing ..." << std::endl;
    // if everything started well,
    isPlaying = true;
}


void WavePlayer::stopPlaying()
{
    if(isPaused) {
        // TODO needed for unpause
    }
    isPaused = false;
    if(isPlaying) {
        // TODO stop playing. clear objects
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
