/**
implementace tridy server.h\n
@author Martin Caslava - xcasla03@stud.fit.vutbr.cz
@author Vojtech Prikryl - xprikr28@stud.fit.vutbr.cz
*
<b>popis tridy:</b>\n
trida predstavujici server - chovani podobne jako u klienta
**/

#include "server.h"

Server::Server(Game *game, QHostAddress addr, int port): cerr(stderr, QIODevice::WriteOnly)
{
    this->game=game;
    this->socket=NULL;
    // vytvoøíme TCP server
    this->server = new QTcpServer(this);

    // zajistíme zpracování požadavkù vlastním slotem
    connect(this->server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    // explicitnì zakážeme použití proxy
    this->server->setProxy(QNetworkProxy::NoProxy);
    // nastavíme max. poèet spojení
    this->server->setMaxPendingConnections(50);

    // nastavíme IP a port pro naslouchání
    bool listening = this->server->listen(addr, port);

    // pokud se TCP server nepodaøilo spustit na dané IP a portu
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
    // získáme socket
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
    // získáme socket (skrz objekt, který vyslal signál)
    this->socket = qobject_cast<QTcpSocket*>(this->sender());
    QByteArray rawdata = this->socket->readAll();

    this->game->gotMessage(QString(rawdata));
}


