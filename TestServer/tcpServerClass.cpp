#include "tcpServerClass.h"
#include <QTime>
#include <QFileDialog>
#include <iostream>

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
        qDebug() << QString::fromUtf8("We've gpt new connection!");
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
    QTextStream os(clientSocket);
    os.setAutoDetectUnicode(true);
    os << "HTTP/1.0 200 Ok\r\n"
          "Content-Type: text/html; charset=\"utf-8\"\r\n"
          "\r\n"
          "<h1>Nothing to see here</h1>\n"
          << QDateTime::currentDateTime().toString() << "\n";
    clientSocket->close();
    SClients.remove(idusersocs);
}
