#pragma once
#include <QtCore>
#include <QStringList>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <iostream>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#ifndef TCP_CUSTOM_SERVER_H
#define TCP_CUSTOM_SERVER_H

using namespace std;

struct __FileRec {
    quint32 id;
    QString filePath;
};

class TCPServerClass : public QObject
{
    int port;
    std::list<__FileRec> dataBase;
    QTcpServer *tcpServer;
    int server_status;
    QMap<int,QTcpSocket *> SClients;

    qint64 getResourceSize(quint32);
    char * getResourceData(quint32, quint32,quint32, qint64 &);
	Q_OBJECT
public:
    TCPServerClass(int,const std::list<__FileRec> &, QObject*par=nullptr);
    ~TCPServerClass();

public slots:
	void run();
    void newuser();
    void slotReadClient();
signals:
	void finished();

public:
	string filePath;

};

// should match those in interleave.h
#define PROTO_RQ_SIZE (0x0A)
#define PROTO_RS_SIZE (0x0B)
#define PROTO_RQ_DATA (0xAA)


#endif
