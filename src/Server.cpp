#include <string.h>
#include <functional>
#include <unistd.h>
#include "Socket.h"
#include "InetAddress.h"
#include "Server.h"
#include "Channel.h"

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : loop(_loop) {
	Socket *serv_sock = new Socket();
	InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
	serv_sock->bind(serv_addr);
	serv_sock->listen();
	serv_sock->setnonblocking();
	Channel *servChannel = new Channel(loop, serv_sock->getFd());
	std::function<void()> cb = std::bind(&Server::newConnection, this, serv_lock);
	servChannel->setCallback(cb);
	servChannel->enableReading();
}

Server::~Server()
{

}

void Server::handleReadEvent(int sockfd) {
	std::vector<char> buf;
	buf.resize(BUFFER_SIZE);	
	while (true) {
		bzero(&buf, sizeof(buf));
		ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
		if (read_bytes > 0) {
			printf("message from client fd %d: %s\n", sockfd, buf);
			write(sockfd, buf, sizeof(buf));
			if (static_cast<size_t>(read_bytes) == buffer.size() {
				buf.resize(buf.size() * 2);
			}
		} else if (read_bytes == -1 && errno == EINTR) { // this condition means serv sock lost connection
			printf("continue reading");
			continue;
		} else if (read_bytes == -1 && ((errno==EAGAIN) || (errno==EWOULDBLOCK))) { // this condition means data are all read
			printf("finish reading once, errno: %d\n", errno);
			break;
		} else if (read_bytes == 0) {
			printf("EOF, client fd %d disconnected\n", sockfd);
			close(fd);
			break;
		}
	}
}

void Server::newConnection(Socket *serv_sock) {
	InetAddress* clnt_addr = new InetAddress();
	Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
	printf("new client fd %d! IP %s Port %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
	clnt_sock->setnonblocking();
	Channel *clntChannel = new Channel(loop, clnt_sock->getFd());
	std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
	clntChannel->setCallback(cb);
	clntChannel->enableReading();
}
	
