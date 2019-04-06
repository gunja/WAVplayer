#ifndef MYTHREAD_H
#define MYTHREAD_H
#include<QThread>
#include <stdint.h>
#if defined (_WIN32 ) || defined (_WIN64)
#include <Windows.h>
#endif

#include "streamBuffer.h"

class MyThread : public QThread
{
    Q_OBJECT
    QObject * parent;
    FILE *f;
    uint32_t Len;
    void run() override;
#if defined (_WIN32 ) || defined (_WIN64)
    HWAVEOUT hWave; /* device handle */
#endif
    bool canGoOn;
    bufferedStream * stream;
public:
    MyThread(QObject * parent = nullptr);
    void setFile( FILE * _f ) { f = _f;}
    void setLength( uint32_t l) { Len = l;}
#if defined (_WIN32 ) || defined (_WIN64)
    void setDeviceHandle(HWAVEOUT dev ){hWave = dev; }
    bool openAudio(WAVEFORMATEX & wfx);
#endif
    void setBuffer(bufferedStream*s) {stream = s;}
    void setEnabled(bool can = true) {canGoOn = can;}
signals:
    void doneWork();

};

#endif // MYTHREAD_H
