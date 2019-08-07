#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>

#include <iostream>
#include <sstream>

int
main()
{
  fd_set readFds;
  fd_set errFds;
  fd_set watchFds;
  FD_ZERO(&readFds);
  FD_ZERO(&errFds);
  FD_ZERO(&watchFds);

  // create a socket using TCP IP
  int listenSockfd = socket(AF_INET, SOCK_STREAM, 0);
  int maxSockfd = listenSockfd;

  // put the socket in the socket set
  FD_SET(listenSockfd, &watchFds);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(listenSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(40000);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  if (bind(listenSockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set the socket in listen status
  if (listen(listenSockfd, 10) == -1) {
    perror("listen");
    return 3;
  }

  // initialize timer (2s)
  struct timeval tv;
  while (true) {
    // set up watcher
    int nReadyFds = 0;
    readFds = watchFds;
    errFds = watchFds;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if ((nReadyFds = select(maxSockfd + 1, &readFds, NULL, &errFds, &tv)) == -1) {
      perror("select");
      return 4;
    }

    if (nReadyFds == 0) {
      std::cout << "no data is received for 3 seconds!" << std::endl;
    }
    else {
      for(int fd = 0; fd <= maxSockfd; fd++) {
        // get one socket for reading
        if (FD_ISSET(fd, &readFds)) {
          if (fd == listenSockfd) { // this is the listen socket
            struct sockaddr_in clientAddr;
            socklen_t clientAddrSize = sizeof(clientAddr);
            int clientSockfd = accept(fd, (struct sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSockfd == -1) {
              perror("accept");
              return 5;
            }

            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
            std::cout << "Accept a connection from: " << ipstr << ":" <<
              ntohs(clientAddr.sin_port) << " with Socket ID: " << clientSockfd << std::endl;

            // update maxSockfd
            if (maxSockfd < clientSockfd)
              maxSockfd = clientSockfd;

            // add the socket into the socket set
            FD_SET(clientSockfd, &watchFds);
          }
          else { // this is the normal socket
            char buf[20];
            int recvLen = 0;
            memset(buf, '\0', sizeof(buf));
            if ((recvLen = recv(fd, buf, 20, 0)) == -1) {
              perror("recv");
              return 6;
            }

            std::string output(buf);
            if (output == "50" || recvLen == 0) {
              close(fd);
              FD_CLR(fd, &watchFds);

            }
            else {
              std::cout << "Socket " << fd << " receives: " << buf << std::endl;
            }
          }
        }
      }
    }
  }

  return 0;
}
