//predmet: Seminar C++
//autor: xcasla03@stud.fit.vbutbr.cz, Martin Caslava
//projekt: implementace herniho serveru pro deskovou hru dama (pouze dilci modul celeho programu)
//prog jazyk: C++
//hodnoceni: zapocteno

#include "server.h"

Server::Server(Game *game, QHostAddress addr, int port): cerr(stderr, QIODevice::WriteOnly)
{
    this->game=game;
    this->socket=NULL;
    // vytvorime TCP server
    this->server = new QTcpServer(this);

    // zajistime zpracovani pozadavku vlastnim slotem
    connect(this->server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    // explicitne zakazeme pouziti proxy 
    this->server->setProxy(QNetworkProxy::NoProxy);
    // nastavime max. poèet spojení
    this->server->setMaxPendingConnections(50);

    // nastavíme IP a port pro naslouchani
    bool listening = this->server->listen(addr, port);

    // pokud se TCP server nepodarilo spustit na dané IP a portu
    if(not listening)
    {
        this->game->printInfo(ERROR,QString("Server error"),QString("Server couldn't have been started"));
    }
    else{
        this->game->printInfo(INFO,QString("Server created"),QString("Server successfully created! Now waiting for a client to connect."));
    }

}

Server::~Server(){
    delete this->server;
    delete this->socket;
}

/** <b>popis metody</b>\n
  metoda na zpracovani spojeni pd druheho klienta
  @return void
*/
void Server::handleNewConnection()
{
    // ziskamesocket
    this->socket = this->server->nextPendingConnection();
    if(!this->socket)
        return;

    connect(this->socket, SIGNAL(readyRead()), SLOT(handleReply()));

    //spojeni
    this->game->setNetworkRole(SERVER);
    this->game->newGame(NETWORK);
    this->game->printInfo(INFO, QString("Connection successful"),QString("Someone connected to you!"));
}

/** <b>popis metody</b>\n
  metoda na zapis dat do socketu
  @param QString data
  @return void
*/
void Server::socketWrite(QString data)
{
    this->socket->write(data.toStdString().c_str());
}

/** <b>popis metody</b>\n
  metoda na vyrizeni pozadavku od klienta
  @return void
*/
void Server::handleReply()
{
    // ziskame socket (skrz objekt, ktery vyslal signal)
    this->socket = qobject_cast<QTcpSocket*>(this->sender());
    QByteArray rawdata = this->socket->readAll();

    this->game->gotMessage(QString(rawdata));
}


