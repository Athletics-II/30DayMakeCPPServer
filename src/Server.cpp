#include <functional>
#include "Socket.h"
#include "Server.h"
#include "Channel.h"
#include "Acceptor.h"

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr) {
	acceptor = new Acceptor(loop);
	std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholder::_1);
	acceptor->setNewConnectionCallback(cb);
}

Server::~Server()
{
	delete acceptor;
}

void Server::newConnection(Socket *sock) {
	InetAddress* clnt_addr = new InetAddress();
	Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
	printf("new client fd %d! IP %s Port %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
	clnt_sock->setnonblocking();
	Channel *clntChannel = new Channel(loop, clnt_sock->getFd());
	std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
	clntChannel->setCallback(cb);
	clntChannel->enableReading();
}

void Server::deleteConnection(Socket *sock) {
	Connection *conn = connections[sock->getFd()];
	connections.erase(sock->getFd());
	delete conn;
}


	
