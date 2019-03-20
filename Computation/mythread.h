#ifndef MYTHREAD_H
#define MYTHREAD_H
#include<QThread>
#include <stdint.h>
#include <Windows.h>


class MyThread : public QThread
{
    Q_OBJECT
    QObject * parent;
    FILE *f;
    uint32_t Len;
    void run() override;
    HWAVEOUT hWave; /* device handle */
    bool canGoOn;
public:
    MyThread(QObject * parent = nullptr);
    void setFile( FILE * _f ) { f = _f;}
    void setLength( uint32_t l) { Len = l;}
    void setDeviceHandle(HWAVEOUT dev ){hWave = dev; }
    bool openAudio(WAVEFORMATEX & wfx);
    void setEnabled(bool can = true) {canGoOn = can;}
signals:
    void doneWork();

};

#endif // MYTHREAD_H
