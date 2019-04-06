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

using namespace std;

struct __FileRec {
    int id;
    QString filePath;
};

class TCPServerClass : public QObject
{
    int port;
    std::list<__FileRec> dataBase;
    QTcpServer *tcpServer;
    int server_status;
    QMap<int,QTcpSocket *> SClients;
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
