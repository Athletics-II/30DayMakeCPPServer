#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "util.h"
#include <unistd.h>
#include <string.h>
#include <iostream>

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), inBuffer(new std::string()), readBuffer(nullptr) {
	channel = new Channel(loop, sock->getFd());
	std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
	channel->setCallback(cb);
	channel->enableReading();
	readBuffer = new Buffer();
}

Connection::~Connection() {
	delete channel;
	delete sock;
}

void Connection::echo(int sockfd) {
	char buf[1024];
	while (true) {
		bzero(&buf, sizeof(buf));
		ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
		if (read_bytes > 0) {
			readBuffer->append(buf, read_bytes);
		} else if (read_bytes == -1 && errno == EINTR) { // this condition means serv sock lost connection
			printf("continue reading");
			continue;
		} else if (read_bytes == -1 && ((errno==EAGAIN) || (errno==EWOULDBLOCK))) { // this condition means data are all read
			printf("finish reading once, errno: %d\n", errno);
			printf("message from client fd %d: %s\n", sockfd, buf);
			errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
			readBuffer->clear();
			break;
		} else if (read_bytes == 0) {
			printf("EOF, client fd %d disconnected\n", sockfd);
			// close(sockfd) // closing sock will automatically remove the fd
			deleteConnectionCallback(sock);
			break;
		}
	}
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
	deleteConnectionCallback = _cb;
}
