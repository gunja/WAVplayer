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

/* some good values for block size and count */
//#define BLOCK_SIZE 32768
#define SAMPLES_PER_BLOCK 200
int g_blockAlign;
#define BLOCK_COUNT 20
/* function prototypes */
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static WAVEHDR* allocateBlocks(int size, int count);
static void freeBlocks(WAVEHDR* blockArray);
static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
/* module level variables */
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR* waveBlocks;
static volatile int waveFreeBlockCount;
static int waveCurrentBlock;
int main(int argc, char* argv[])
{
	HWAVEOUT hWaveOut; /* device handle */
	HANDLE hFile;/* file handle */
	WAVEFORMATEX wfx; /* look this up in your documentation */
	char buffer[1024]; /* intermediate buffer for reading */
	int i;
	wavHeaderStruct hdr;
	_FMTsubChunkStruct fmt;
	_DatasubChunkHeaderStruct dt;
	DWORD readBytes;
	/*
	* quick argument check
	*/
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		ExitProcess(1);
	}
	/*
	* initialise the module variables
	*/

	InitializeCriticalSection(&waveCriticalSection);
	/*
	* try and open the file
	*/
	if ( (hFile = CreateFile( argv[1], GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL) ) == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: unable to open file '%s'\n", argv[0], argv[1]);
		ExitProcess(1);
	}

	BOOL val = ReadFile(hFile, &hdr, sizeof(wavHeaderStruct), &readBytes, NULL);
	if (hdr.wave != WAVE || hdr.riff != RIFF) {
		fprintf(stderr, "Header mismatch\n");
		ExitProcess(1);
	} else{ fprintf(stderr, "Chunk size %d\n", hdr.chunkSize ); }

	val = ReadFile(hFile, &fmt, sizeof(_FMTsubChunkStruct), &readBytes, NULL);
	if (fmt.id != FMT) {
		fprintf(stderr, "FMT header mismatch\n");
		ExitProcess(1);
	}
	else { fprintf(stderr, "FMT size %d\n", fmt.size); }

	val = ReadFile(hFile, &dt, sizeof(_DatasubChunkHeaderStruct), &readBytes, NULL);
	if (dt.id != DATA) {
		fprintf(stderr, "FMT header mismatch\n");
		ExitProcess(1);
	}
	else { fprintf(stderr, "Data size %d\n", dt.len); }
	/*
	* set up the WAVEFORMATEX structure.
	*/
	//wfx.nSamplesPerSec = 44100; /* sample rate */
	wfx.nSamplesPerSec = fmt.sampleRate; /* sample rate */
	wfx.wBitsPerSample = fmt.bpS; /* sample size */
	wfx.nChannels = fmt.chanNum; /* channels*/
	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = fmt.format;
	wfx.nBlockAlign = fmt.blockAlign;
	wfx.nAvgBytesPerSec = fmt.byteRate;

	g_blockAlign = wfx.nBlockAlign;
	waveBlocks = allocateBlocks(SAMPLES_PER_BLOCK * g_blockAlign, BLOCK_COUNT);
	waveFreeBlockCount = BLOCK_COUNT;
	waveCurrentBlock = 0;
	/*
	* try to open the default wave device. WAVE_MAPPER is
	* a constant defined in mmsystem.h, it always points to the
	* default wave device on the system (some people have 2 or
	* more sound cards).
	*/
	if (waveOutOpen(
		&hWaveOut,
		WAVE_MAPPER,
		&wfx,
		(DWORD_PTR)waveOutProc,
		(DWORD_PTR)&waveFreeBlockCount,
		CALLBACK_FUNCTION
	) != MMSYSERR_NOERROR) {
		fprintf(stderr, "%s: unable to open wave mapper device\n", argv[0]);
		ExitProcess(1);
	}
	/*
	* playback loop
	*/
	while (1) {
		
		if (!ReadFile(hFile, buffer, sizeof(buffer), &readBytes, NULL))
			break;
		if (readBytes == 0)
			break;
		if (readBytes < sizeof(buffer)) {
			printf("at end of buffer\n");
			memset(buffer + readBytes, 0, sizeof(buffer) - readBytes);
			printf("after memcpy\n");
		}
		writeAudio(hWaveOut, buffer, sizeof(buffer));
	}
	/*
	* wait for all blocks to complete
	*/
	while (waveFreeBlockCount < BLOCK_COUNT)
		Sleep(10);
	/*
	* unprepare any blocks that are still prepared
	*/
	for (i = 0; i < waveFreeBlockCount; i++)
		if (waveBlocks[i].dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof(WAVEHDR));
	DeleteCriticalSection(&waveCriticalSection);
	freeBlocks(waveBlocks);
	waveOutClose(hWaveOut);
	CloseHandle(hFile);
	return 0;
}

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
