/**
    \file interleave.h
*/

#include <QObject>

#include <cstddef>
#include <list>
#include "interleave.h"
#include <QFile>
#include <QtEndian>
#include <QDataStream>


multiPointReceiver::multiPointReceiver( QObject * par): QObject(par)
{
}

multiPointReceiver::~multiPointReceiver()
{
    for(auto it = begin(connectionsMap); it != end(connectionsMap); ++it )
    {
        it->second->abort();
        delete it->second;
    }
}

/**
    @return How many bytes where copied to buffer
        -1 if file was not found, not readable or other problems
*/
qint64 multiPointReceiver::getDataToBufferFromOffset(char * dest, size_t maxSize, qint64 offS)
{
    // similar to getObjectSize : if there's a local file - read it directly
    for( auto it = sources.begin(); it != sources.end(); ++it)
    {
        if( it->srcType == __sourceDefine::LOCAL) {
            QFile fq(it->path);
            if( fq.exists() && fq.open(QIODevice::ReadOnly) ) {
                if(! fq.seek(offS)) {
                    fq.close();
                    return -2;
                } else {
                    qint64 rv = fq.read(dest, maxSize);
                    fq.close();
                    return rv;
                }
            }
        }
    }
    std::list< portionTask *> tsks;
    for( auto it = sources.begin(); it != sources.end(); ++it)
    {
        if( it->srcType == __sourceDefine::REMOTE)
        {
            portionTask * n = new portionTask(*it);
            tsks.push_back( n);
        }
    }
    int remotesCount = tsks.size();
    if( remotesCount < 1) {
        return -1;
    }
    int parConnectionSize = maxSize / remotesCount;
    int c=0;
    for( auto it: tsks) {
        it->ptr = dest + c * parConnectionSize;
        it->BytesNeed = parConnectionSize;
        it->offset = offS + c * parConnectionSize;
        ++c;
    }
    // last connection will take more then other threads
    // to get remaining bytes (not even divition for example)
    auto ita = tsks.end();
    --ita;
    (*ita)->BytesNeed += (maxSize - parConnectionSize*remotesCount);
    std::list <ThreadedSocketRead*> threadsList;
    for( auto itt: tsks) {
        auto n = new ThreadedSocketRead( itt, this);
        threadsList.push_back( n);
        n->start();
    }
    // let's assume that each thread will return result
    for( auto it : threadsList) {
        while( !it->isFinished() ) {
            QThread::msleep( 10);
        }
        delete it;
    }
    qint64 rv =0;
    for( auto it : tsks) {
        rv += it->bytesDone;
        delete it;
    }
    return rv;
}

/**
 * @brief multiPointReceiver::getObjectSize
 *
 * Attention: blocking operation.
 *
 * @return size of object received from first replied source
 */
qint64 multiPointReceiver::getObjectSize()
{
    for( auto it = sources.begin(); it != sources.end(); ++it)
    {
        if( it->srcType == __sourceDefine::LOCAL) {
            QFile fi(it->path);
            return fi.size();
        }
    }
    // if there's no local source found, first replyed
    // remote will work
    for( auto it = sources.begin(); it != sources.end(); ++it)
    {
        QTcpSocket *c = new QTcpSocket(this);
        c->connectToHost(it->path, it->port);
        const int Timeout = 5 * 1000;
        if (!c->waitForConnected(Timeout)) {
            c->abort();
            delete c;
            continue;
        }
        QByteArray ba(20, 0);
        QDataStream ds(&ba, QIODevice::ReadWrite);
        ds.setByteOrder(QDataStream::BigEndian);
        quint8 v= PROTO_RQ_SIZE;
        ds<<v;
        ds<<it->remoteId;
        qDebug()<<"sending QByteArray "<<ba;
        ba.resize(5);
        qint64 actWr = c->write(ba);
        c->flush();
        qDebug()<<"actually written"<<actWr;
        //c->waitForReadyRead();
        //char tmpBuff[200];
        //QByteArray rv = c->read();
        //actWr = c->readData( tmpBuff, 200);
        //qDebug()<<"Readd Data returned "<<actWr;
        //QByteArray rv( tmpBuff);
        qint8 type, status;
        quint32 fileS;
        QDataStream dO(c);
        dO.setVersion(QDataStream::Qt_4_0);
        dO.setByteOrder(QDataStream::BigEndian);
        do {
            if (!c->waitForReadyRead(Timeout)) {
                return -1;
            }
            dO.startTransaction();
            dO >>type>>status;
        } while( !dO.commitTransaction());
        //dO>>type>>status;
        if( type == PROTO_RS_SIZE && status == 0) 
        {
            dO>>fileS;
            c->abort();
            delete c;
            return fileS;
        }
    }
    return -1;
}

void ThreadedSocketRead::run()
{
    QTcpSocket *c = new QTcpSocket(this);
    c->connectToHost(task->path, task->port);
    const int Timeout = 5 * 1000;
        if (!c->waitForConnected(Timeout)) {
            c->abort();
            delete c;
            return;
        }
        QByteArray ba(20, 0);
        QDataStream ds(&ba, QIODevice::ReadWrite);
        ds.setByteOrder(QDataStream::BigEndian);
        quint8 v= PROTO_RQ_DATA;
        ds<<v;
        ds<<task->remoteId;
        ds<<task->offset;
        ds<<task->BytesNeed;
        qDebug()<<"sending QByteArray "<<ba;
        ba.resize(13);
        qint64 actWr = c->write(ba);
        c->flush();
        qDebug()<<"actually written"<<actWr;
        quint8 type, status;
        quint32 dataLen;
        QDataStream dO(c);
        dO.setVersion(QDataStream::Qt_4_0);
        dO.setByteOrder(QDataStream::BigEndian);
        do {
            if (!c->waitForReadyRead(Timeout)) {
                return;
            }
            dO.startTransaction();
            dO >>type>>status;
        } while( !dO.commitTransaction());
        qDebug()<<"Received type="<<type<<"  and status ="<<status;
        if( type == (PROTO_RQ_DATA +1 ) && status == 0) 
        {
            dO>>dataLen;
            qDebug()<<"Reading "<<dataLen<<"  from socket";
            qint64 v = c->read( task->ptr, dataLen);
            c->abort();
            delete c;
            if( v > 0) {
                task->bytesDone = static_cast<int>( v);
            }
            return;
        }
}

