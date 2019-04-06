/**
    \file streamBuffer.h

    Contains declaration of buffered stream object

*/
#include <QObject>
#include <cstddef>


class bufferedStream {
  Q_OBJECT
    std::size_t currentSize;
    std::size_t lastReadPosition;
    char * buffer;
  public:
    bufferedStream( QObject * par): QObject( par), currentSize(0), lastReadPosition(0), buffer(nullptr) {};
    ~bufferedStream();
    char * getNextDataPointer( std::size_t requiredSize);

public slots:
    int appendDataToBuffer( char * buffer, std::size_t bufSize);
signals:
    void bufferState(int);
};

