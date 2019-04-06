#ifndef STREAM_BUUFERED_H
#define STREAM_BUUFERED_H
/**
    \file streamBuffer.h

    Contains declaration of buffered stream object

*/



#include <QObject>
#include <cstddef>
#include "interleave.h"

class bufferedStream: public QObject {
  Q_OBJECT
    std::size_t currentSize;
    std::size_t lastReadPosition;
    char * buffer;
    std::list<struct __sourceDefine> lSrcs;
  public:
    bufferedStream( QObject * par): QObject( par), currentSize(0), lastReadPosition(0), buffer(nullptr) {}
    ~bufferedStream();
    char * getNextDataPointer( std::size_t requiredSize);
    void setSources(const std::list<struct __sourceDefine>&sources)
        {lSrcs = sources;}
    size_t fread(void *, int siz, int C);
public slots:
    int appendDataToBuffer( char * buffer, std::size_t bufSize);
signals:
    void bufferState(int);
};

#endif
