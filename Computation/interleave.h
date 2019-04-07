#ifndef INTERLEAVE_H
#define INTERLEAVE_H
/**
    \file interleave.h
*/

#include <QObject>

#include <cstddef>
#include <list>
#include <map>
#include "streamBuffer.h"
#include <QTcpSocket>
#include <QThread>

//#include <QDebug>



struct __sourceDefine {
    enum __sourceType {
        LOCAL = 0,
        REMOTE } srcType;
    QString path;
    quint16 port;
    int remoteId;
    __sourceDefine( const __sourceDefine& s):
        srcType( s.srcType), port( s.port), remoteId( s.remoteId){
        path = s.path;}
    __sourceDefine(): srcType( LOCAL), path(""), port(65535), remoteId(-1) {}
};

class multiPointReceiver : QObject {
    Q_OBJECT
    std::list<__sourceDefine> sources;
    std::map<const struct __sourceDefine*, QTcpSocket* > connectionsMap;

  public:
    multiPointReceiver( QObject * par);
    virtual ~multiPointReceiver();

    qint64 getDataToBufferFromOffset(char *, size_t maxSize, qint64 offS= 0);
    qint64 getObjectSize();

    void setSources( const std::list<__sourceDefine>&s)
        {sources = s;
//for( auto it = begin(sources); it != end(sources); ++it) { qDebug()<<"path "<<it->path;};
        }
};

// Object: pointer where it should start storing data,
//         how many bytes it should store
//           + address, port, fileId
//    returns: how may has done
struct portionTask: __sourceDefine {
    char * ptr;
    quint32 BytesNeed;
    int bytesDone;
    quint32 offset;
    portionTask() : __sourceDefine() {}
    portionTask( const __sourceDefine& s): __sourceDefine(s),
        ptr(nullptr), BytesNeed(0), bytesDone(0) {}
};

class ThreadedSocketRead: public QThread {
    Q_OBJECT
    portionTask * task;
public:
    ThreadedSocketRead( portionTask *p, QObject * par = nullptr):
        QThread( nullptr), task( p) {}
    void run() override;
};

#define PROTO_RQ_SIZE (0x0A)
#define PROTO_RS_SIZE (0x0B)
#define PROTO_RQ_DATA (0xAA)


#endif
