#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>

int
main()
{
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(40000);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  // send/receive data to/from connection
  int count = 0;
  srand(time(NULL));

  while (count <= 50) {
    std::string str = std::to_string(count);

    if (send(sockfd, str.c_str(), str.size(), 0) == -1) {
      perror("send");
      return 4;
    }

    int sleepTime = rand() % 2 + 1;
    sleep(sleepTime);

    count++;
  }

  sleep(5);

  close(sockfd);

  return 0;
}
