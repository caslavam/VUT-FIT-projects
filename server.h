//predmet: Seminar C++
//autor: xcasla03@stud.fit.vbutbr.cz, Martin Caslava
//projekt: implementace herniho serveru pro deskovou hru dama (pouze dilci modul celeho programu)
//prog jazyk: C++
//hodnoceni: zapocteno

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTextStream>
#include <QList>
#include <QPair>
#include <QHostAddress>
#include <QHash>
#include <QSettings>
#include <QtNetwork>

#include "game.h"

class QTcpServer;
class QTcpSocket;
class Game;

class Server : public QObject
{
    Q_OBJECT
public:
    //QHostAddress adresa;
    //QHostAddress adresa = setAddress("83.240.113.37 ");
    Server(Game * game, QHostAddress addr = QHostAddress(QHostAddress::Any),
          int port = 2266);
    virtual ~Server();

private:
    QTcpServer* server;
    QTextStream cerr;
    QTcpSocket* socket;
    Game * game;
public slots:
    void socketWrite(QString data);
private slots:
    void handleNewConnection();
    void handleReply();

};

#endif // TCPSERVER_H
