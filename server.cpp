#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include "Socket.h"
#include "InetAddress.h"
#include "Epoll.h"
#include "util.h"
#include "Channel.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCKING);
}
void handleReadEvent(int);

int main() {
	Socket *serv_sock = new Socket();
	InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
	serv_sock->bind(serv_addr);
	serv_sock->listen();
	Epoll *ep = new Epoll();
	serv_sock->setnonblocking();
	Channel *servChannel = new Channel(ep, serv_sock->getFd());
	servChannel->enableReading();

	while (true) {
		std::vector<Channel*> activeChannels = ep->poll();
		int nfds = activeChannels.size();
		for (int i=0; i<nfds; ++i) {
			int chfd = activeChannels[i].getFd();
			if (chfd == serv_sock->getFd()) {
				InetAddress* clnt_addr = new InetAddress();
				Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
				printf("new client fd %d! IP %s Port %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
				clnt_sock->setnonblocking();
				Channel *clntChannel = new Channel(ep, clnt_sock->getFd());
				clntChannel->enableReading();
			} else if (activeChannels[i]->getRevents() & EPOLLIN) {
				handleReadEvent(activeChannels[i]->getFd());
			} else {
				printf("Something else happened\n");
			}
		}
	}
	delete clnt_addr;
	delete clnt_sock;
	delete serv_addr;
	delete serv_sock;
	return 0;
}

void handleReadEvent(int sockfd) {
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
