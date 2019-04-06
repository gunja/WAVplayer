#include "tcpServerClass.h"
#include <QTime>
#include <QFileDialog>
#include <iostream>
#include <QDataStream>
#include <QDebug>

TCPServerClass::TCPServerClass(int _port, const std::list<__FileRec> &fRs, QObject * par) :
    QObject(par), port(_port), dataBase(fRs)
{
    qDebug()<<"In tcp constructor: port = "<<port;
    qDebug()<<"and list of args is:";
    int c =1;
    for( auto it = fRs.begin(); it != fRs.end(); ++it, ++c)
    {
        qDebug()<<c<<":  "<<it->id<<"\t"<<it->filePath;
    }
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newuser()));
}

TCPServerClass::~TCPServerClass()
{
    delete tcpServer;
}

void TCPServerClass::run()
{
    if (!tcpServer->listen(QHostAddress::Any, port) && server_status==0) {
         qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
     } else {
         server_status=1;
         qDebug() << tcpServer->isListening() << "TCPSocket listen on port";
         qDebug() << QString::fromUtf8("Server is RUNNING on port:")<<port;
 }
    //emit finished();
}

void TCPServerClass::newuser()
{
    if(server_status==1){
        qDebug() << QString::fromUtf8("We've got new connection!");
        QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
        int idusersocs=clientSocket->socketDescriptor();
        SClients[idusersocs]=clientSocket;
        connect(SClients[idusersocs],SIGNAL(readyRead()),this, SLOT(slotReadClient()));
    }
}

void TCPServerClass::slotReadClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int idusersocs=clientSocket->socketDescriptor();
    QDataStream in(clientSocket);
    in.setByteOrder(QDataStream::BigEndian);
    quint8 rqType;
    quint32 fileId, offS, mSize;
    qint64 rv;
    char * buff= nullptr;
    in>>rqType>>fileId;
    qDebug()<<"on read handling got rqTpe="<<rqType
        <<"  and fileId="<<fileId;
    switch( rqType) {
        case PROTO_RQ_SIZE:
             clientSocket->readAll();
             rqType |= 1;
             in<<rqType;
             rv =getResourceSize(fileId);
             if( rv> 0) {
                 rqType = 0;
                 in<<rqType;
                 mSize = static_cast<quint32>(rv);
                 in<<mSize;
             }
            break;
        case PROTO_RQ_DATA:
            in>>offS>>mSize;
            clientSocket->readAll();
            qDebug()<<"Requested by PROTO_RQ_DATA for"<<mSize<<"  bytes starting offset"<<offS;
            buff = getResourceData(fileId, offS, mSize, rv);
            rqType |=1;
            in<<rqType;
            if( rv < 0) {
                rqType = static_cast<quint8>(-rv);
                in<<rqType;
            } else {
                rqType = 0;
                in<<rqType;
                mSize = static_cast<quint32>(rv);
                in<<mSize;
                in.writeRawData(buff, static_cast<int>(rv));
                qDebug()<<"after call to writeRawData with rv="<<rv;
            }
            break;
        default:
            clientSocket->readAll();
            in<<(rqType|1);
            rqType = 255;
            in<<rqType;
            break;
    }
    clientSocket->close();
    if(buff != nullptr) delete[] buff;
    SClients.remove(idusersocs);
}

qint64 TCPServerClass::getResourceSize(quint32 fId)
{
    for( auto it= begin(dataBase); it != end(dataBase); ++it)
    {
        if(it->id != fId ) continue;
        QFile fq( it->filePath);
        qDebug()<<"Corresponding to "<<fId<<
                  "  is "<<it->filePath;
        if( fq.exists()) {
            qDebug()<<"file exists and size is"<<fq.size();
            return fq.size();}
        else {
            qDebug()<<"system thinks that file does not exist. returning 0";
            return -1;
        }
    }
    return -1;
}

char * TCPServerClass::getResourceData(quint32 resId, quint32 offset,quint32 size, qint64 &rv)
{
    char * returnBuff = nullptr;
    rv = -1;
    for( auto it= begin(dataBase); it != end(dataBase); ++it)
    {
        if(it->id != resId ) continue;
        QFile fq( it->filePath);
        qDebug()<<"Corresponding to "<<resId<<
                  "  is "<<it->filePath;
        if( fq.exists() && fq.open(QIODevice::ReadOnly) ) {
            if(! fq.seek(offset)) {
                fq.close();
                rv = -2;
                break;
            } else {
                returnBuff = new char[size];
                rv = fq.read(returnBuff, size);
                fq.close();
            }
        }
        else {
            qDebug()<<"system thinks that file does not exist. returning 0";
            break;
        }
    }
    return returnBuff;
}
