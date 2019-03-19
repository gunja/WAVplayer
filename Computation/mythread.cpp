#include "mythread.h"
#include <mmsystem.h>

/* some good values for block size and count */
#define BLOCK_SIZE 8192
#define BLOCK_COUNT 20

MyThread::MyThread(QObject * _parent): QThread(_parent), parent(_parent)
  , f(nullptr), Len(0), hWave(nullptr), canGoOn(false)
{
    // TODO compelete is something else is needed
}

/* module level variables */
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR* waveBlocks;
static volatile int waveFreeBlockCount;
static int waveCurrentBlock;


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
        if (size < (int)(BLOCK_SIZE - current->dwUser)) {
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }
        remain = BLOCK_SIZE - current->dwUser;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = BLOCK_SIZE;
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

void MyThread::run()
{
    char buffer[1024];
    while(canGoOn ) {
        size_t readBytes = fread(buffer, 1, 1024, f);
        if (readBytes == 0)
            break;
        if (readBytes < sizeof(buffer)) {
            printf("at end of buffer\n");
            memset(buffer + readBytes, 0, sizeof(buffer) - readBytes);
            printf("after memcpy\n");
        }
        writeAudio(hWave, buffer, sizeof(buffer));
    }
}

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
        blocks[i].lpData = (LPSTR) buffer;
        buffer += size;
    }
    return blocks;
}

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
