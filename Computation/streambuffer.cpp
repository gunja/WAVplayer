#include "streamBuffer.h"

#define BUFF_DEF_SIZE  100000
//#define BUFF_DEF_SIZE  24000


bufferedStream::bufferedStream( QObject * par):
    QObject( par), currentSize(0), lastReadPosition(0),
    buffer(nullptr), fetchThread( nullptr)
    , lastUsedOffset(0), resourceEndReached(false)
{
}

size_t bufferedStream::fread(void *dst, int siz, int C)
{
    if( buffer != nullptr)
    {
      mx.lock();
        int rem = currentSize - lastReadPosition;
#if 0
        qDebug()<<"buffer fread siz="<<siz<<"  and count ="<<C
               <<"\tt while at curSize="<<currentSize<<"  and last read position"<<lastReadPosition
              <<"  remained "<<rem;
#endif
        if( rem <= 0) {
            mx.unlock();
            return 0;
        }
        if ( rem >= siz * C ) rem= siz * C;
        memcpy( dst, buffer + lastReadPosition, rem );
        lastReadPosition += rem;
        // how far are we from end of buffer?
        if( lastReadPosition > currentSize /2 ) {
            if( fetchThread == nullptr && ( ! resourceEndReached ) ) {
                fetchThread = new buffUpdateThread(this);
                fetchThread->setSources( lSrcs);
                fetchThread->setOffset( lastUsedOffset);
                connect( fetchThread, SIGNAL(exchangeData(char*, qint64)),
                    this, SLOT(appendDataToBuffer( char *, qint64)),
                Qt::DirectConnection );
                fetchThread->start();
                qDebug()<<"just startedd to fetch new portion";
            }
        }
      mx.unlock();
      //qDebug()<<" returning from bufferedStream::fread "<<(rem / siz);
        return rem / siz;
    }
    else {
        // we don't need to lock under mutex here since it's
        // first access to region
        buffer = new char[BUFF_DEF_SIZE];
        currentSize = BUFF_DEF_SIZE;
        multiPointReceiver mp( this);
        mp.setSources(lSrcs);
        mp.getDataToBufferFromOffset( buffer, BUFF_DEF_SIZE, 0);
        memcpy( dst, buffer, siz * C);
        lastReadPosition = siz * C;
        lastUsedOffset = BUFF_DEF_SIZE;
        return C;
    }
    // TODO implement this method
    // for now stubbed so that compilation is possible
    return 0;
}

bufferedStream::~bufferedStream()
{
    //mx.lock();
    if(buffer != nullptr)
        delete[] buffer;
    //mx.unlock();
}

void bufferedStream::appendDataToBuffer( char * buffIn, qint64 bufSize)
{
    qDebug()<<"exchage in appendDataToBuffer just started with size "<<bufSize;
    mx.lock();
        int newSize = currentSize - lastReadPosition + bufSize;
        char * buff2 = new char[newSize];
        memcpy( buff2, buffer + lastReadPosition, currentSize - lastReadPosition);
        memcpy( buff2 + currentSize - lastReadPosition, buffIn, bufSize);
        currentSize = newSize;
        delete[] buffer;
        buffer = buff2;
        lastUsedOffset +=bufSize;
        lastReadPosition = 0;
        resourceEndReached = (bufSize < BUFF_DEF_SIZE );
        qDebug()<<"resource completeness is "<<resourceEndReached;
        fetchThread = nullptr;
        qDebug()<<" Fetch thread just gave back data and new currSize"<<currentSize;
    mx.unlock();
}

void buffUpdateThread::run()
{
    qDebug()<<" Fetch thread started ";
    char *Lbuffer = new char[BUFF_DEF_SIZE];
    multiPointReceiver mp( this);
    mp.setSources(lSrcs);
    qint64 r = mp.getDataToBufferFromOffset( Lbuffer, BUFF_DEF_SIZE, Offset);
    emit exchangeData(Lbuffer, r);
    qDebug()<<"deleting buffer";
    delete[] Lbuffer;
    this->deleteLater();
    qDebug()<<"thread compelete existance";
    return;
}

