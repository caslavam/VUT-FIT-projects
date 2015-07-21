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
    // vytvo��me TCP server
    this->server = new QTcpServer(this);

    // zajist�me zpracov�n� po�adavk� vlastn�m slotem
    connect(this->server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    // explicitn� zak�eme pou�it� proxy
    this->server->setProxy(QNetworkProxy::NoProxy);
    // nastav�me max. po�et spojen�
    this->server->setMaxPendingConnections(50);

    // nastav�me IP a port pro naslouch�n�
    bool listening = this->server->listen(addr, port);

    // pokud se TCP server nepoda�ilo spustit na dan� IP a portu
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
    // z�sk�me socket
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
    // z�sk�me socket (skrz objekt, kter� vyslal sign�l)
    this->socket = qobject_cast<QTcpSocket*>(this->sender());
    QByteArray rawdata = this->socket->readAll();

    this->game->gotMessage(QString(rawdata));
}


