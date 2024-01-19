#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>

#define READ_BUF 1024

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr) {
	channel = new Channel(loop, sock->getFd());
	std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
	channel->setCallback(cb);
	channel->enableReading();
}

Connection::~Connection() {
	delete channel;
	delete sock;
}

void Connection::echo(int sockfd) {
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

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
	deleteConnectionCallback = _cb
}
