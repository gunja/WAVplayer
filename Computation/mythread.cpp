#include "mythread.h"
#if defined (_WIN32 ) || defined ( _WIN64 )
#include <mmsystem.h>
#endif
#include <iostream>

/* some good values for block size and count */
#define SAMPLES_PER_BLOCK 200
int g_blockAlign;
#define BLOCK_COUNT 20

MyThread::MyThread(QObject * _parent): QThread(_parent), parent(_parent)
  , f(nullptr), Len(0)
#if defined (_WIN32 ) || defined ( _WIN64 )
, hWave(nullptr)
#endif
, canGoOn(false)
, stream( nullptr)
{
    // TODO compelete is something else is needed
}

/* module level variables */
#if defined (_WIN32 ) || defined ( _WIN64 )
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR* waveBlocks;
static volatile int waveFreeBlockCount;
static int waveCurrentBlock;
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
void freeBlocks(WAVEHDR* blockArray);
#endif

#if defined (_WIN32 ) || defined ( _WIN64 )
void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
    WAVEHDR* current;
    int remain;
    current = &waveBlocks[waveCurrentBlock];
    while (size > 0) {
        /*
        * first make sure the header we're going to use is unprepared
        */
        if (current->dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
        if (size < (int)((SAMPLES_PER_BLOCK * g_blockAlign) - current->dwUser)) {
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }
        remain = (SAMPLES_PER_BLOCK * g_blockAlign) - current->dwUser;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = (SAMPLES_PER_BLOCK * g_blockAlign);
        waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
        EnterCriticalSection(&waveCriticalSection);
        waveFreeBlockCount--;
        LeaveCriticalSection(&waveCriticalSection);
        /*
        * wait for a block to become free
        */
        while (!waveFreeBlockCount)
            Sleep(10);
        /*
        * point to the next block
        */
        waveCurrentBlock++;
        waveCurrentBlock %= BLOCK_COUNT;
        current = &waveBlocks[waveCurrentBlock];
        current->dwUser = 0;
    }
}
#endif

#if defined (_WIN32 ) || defined ( _WIN64 )
WAVEHDR* allocateBlocks(int size, int count)
{
	unsigned char* buffer;
	int i;
	WAVEHDR* blocks;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;
	/*
	* allocate memory for the entire set in one go
	*/
	if ((buffer = (unsigned char*)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		totalBufferSize
	)) == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		ExitProcess(1);
	}
	/*
	* and set up the pointers to each bit
	*/
	blocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * count;
	for (i = 0; i < count; i++) {
		blocks[i].dwBufferLength = size;
		blocks[i].lpData = (LPSTR)buffer;
		buffer += size;
	}
	return blocks;
}
#endif

#if defined (_WIN32 ) || defined ( _WIN64 )
bool MyThread::openAudio(WAVEFORMATEX & wfx)
{
    g_blockAlign = wfx.nBlockAlign;
    return (waveOutOpen( &hWave, WAVE_MAPPER,
            &wfx, (DWORD_PTR)waveOutProc,
            (DWORD_PTR)&waveFreeBlockCount, CALLBACK_FUNCTION
        ) != MMSYSERR_NOERROR);
}
#endif

void MyThread::run()
{
#if defined (_WIN32 ) || defined ( _WIN64 )
    waveBlocks = allocateBlocks(SAMPLES_PER_BLOCK * g_blockAlign, BLOCK_COUNT);
    waveFreeBlockCount = BLOCK_COUNT;
    waveCurrentBlock = 0;
    InitializeCriticalSection(&waveCriticalSection);
#endif
    char buffer[1024];
    std::cout<<"Entered run with canGoOn=="<<canGoOn<<std::endl;
    while(canGoOn ) {
        size_t readBytes;
        if( f!= nullptr)
            readBytes = fread(buffer, 1, 1024, f);
        else if (stream != nullptr) {
            readBytes = stream->fread(buffer, 1, 1024);
        }
        if (readBytes == 0)
            break;
        if (readBytes < sizeof(buffer)) {
            printf("at end of buffer\n");
            memset(buffer + readBytes, 0, sizeof(buffer) - readBytes);
            printf("after memcpy\n");
        }
#if defined (_WIN32 ) || defined ( _WIN64 )
        writeAudio(hWave, buffer, sizeof(buffer));
#endif
    }
#if defined (_WIN32 ) || defined ( _WIN64 )
    while (waveFreeBlockCount < BLOCK_COUNT)
            Sleep(10);
    /* unprepare any blocks that are still prepared */
    for (int i = 0; i < waveFreeBlockCount; i++)
            if (waveBlocks[i].dwFlags & WHDR_PREPARED)
                    waveOutUnprepareHeader(hWave, &waveBlocks[i], sizeof(WAVEHDR));
    DeleteCriticalSection(&waveCriticalSection);
    freeBlocks(waveBlocks);
    waveOutClose(hWave);
    hWave = nullptr;
#endif
    if( f != nullptr) {
        fclose(f);
        f = nullptr;
    }
    if( stream != nullptr) {
        delete stream;
        stream = nullptr;
    }
	f = nullptr;
    std::cout<<"Right before quiting RUN method"<<std::endl;
    emit doneWork();
}

#if defined (_WIN32 ) || defined ( _WIN64 )
void freeBlocks(WAVEHDR* blockArray)
{
    /*
    * and this is why allocateBlocks works the way it does
    */
    HeapFree(GetProcessHeap(), 0, blockArray);
}

static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg
    , DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
    /* pointer to free block counter */
    int* freeBlockCounter = (int*)dwInstance;
    /* ignore calls that occur due to openining and closing the device. */
    if (uMsg != WOM_DONE)
        return;
    EnterCriticalSection(&waveCriticalSection);
        (*freeBlockCounter)++;
    LeaveCriticalSection(&waveCriticalSection);
}
#endif

