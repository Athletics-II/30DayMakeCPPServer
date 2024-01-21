server:
	g++ src/util.cpp src/Buffer.cpp src/Socket.cpp src/InetAddress.cpp client.cpp -o client && \
	g++ server.cpp \
	-pthread \
	src/util.cpp src/Epoll.cpp src/InetAddress.cpp src/Buffer.cpp src/ThreadPool.cpp \
	src/Socket.cpp src/Channel.cpp src/Eventloop.cpp src/Server.cpp src/Acceptor.cpp src/Connection.cpp \
	-o server

clean:
	rm server && rm client 
