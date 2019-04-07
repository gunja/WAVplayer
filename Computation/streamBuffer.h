#ifndef STREAM_BUUFERED_H
#define STREAM_BUUFERED_H
/**
    \file streamBuffer.h

    Contains declaration of buffered stream object

*/

#define BUFF_DEF_SIZE  50000

#include <QObject>
#include <cstddef>
#include "interleave.h"
#include <QMutex>
#include <QThread>

class buffUpdateThread : public QThread {
    Q_OBJECT
    std::list<struct __sourceDefine> lSrcs;
    qint64 Offset;
    virtual void run() override;
    public:
        buffUpdateThread( QObject * parent= nullptr) : QThread(parent) {}
    void setSources(const std::list<struct __sourceDefine>&sources)
        {lSrcs = sources;}
    void setOffset( qint64 off) { Offset = off;}
signals:
    void exchangeData(char*, qint64);
};

class bufferedStream: public QObject {
  Q_OBJECT
    std::size_t currentSize;
    std::size_t lastReadPosition;
    char * buffer;
    std::list<struct __sourceDefine> lSrcs;
    buffUpdateThread * fetchThread;
    QMutex mx;
    qint64 lastUsedOffset;
    bool resourceEndReached;
  public:
    bufferedStream( QObject * par= nullptr);
    ~bufferedStream();
    void setSources(const std::list<struct __sourceDefine>&sources)
        {lSrcs = sources;}
    size_t fread(void *, int siz, int C);
public slots:
    void appendDataToBuffer( char * buffer, qint64);
signals:
    void bufferState(int);
};

#endif
