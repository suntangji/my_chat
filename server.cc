/// \file server.cc
/// \brief my_chat server
/// \author suntangji, suntangj2016@gmail.com
/// \version 1.0
/// \date 2018-07-04

#include "my_chat.h"

Server::Server(std::string ip, std::string port)
  :_ip(ip), _port(port) {
}
int Server::Run() {
  // 1. create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    //perror("socket");
    std::cerr<< "socket error" << std::endl;
    return -1;
  } else {
    /*printf("socket create success!\n");*/
  }
  // 2. bind addr and port
  sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(_ip.c_str());
  server.sin_port = htons(atoi(_port.c_str()));

  int ret = bind(sockfd, (sockaddr *)&server, sizeof(server));
  if (ret < 0) {
    //perror("bind");
    std::cerr << "bind error" << std::endl;
    return -1;
  } else {
    std::cout << "bind success!" << std::endl;
  }
  // 3. listen
  if (listen(sockfd, 5) < 0) {
    //perror("listen");
    std::cerr<< "listen error" << std::endl;
    return -1;
  } else {
    std::cout << "listen success!" << std::endl;
  }
  // 4. wait connect
  while (1) {
    sockaddr_in client;
    socklen_t len = sizeof(client);
    // 5. accept
    int new_fd = accept(sockfd, (sockaddr *)&client, &len);
    if (new_fd < 0) {
      //perror("accept");
      std::cerr<< "accept error!" << std::endl;
      close(sockfd);
      break;
    } else {
      std::cout << "accept success!" << std::endl;
    }

    int64_t fd = new_fd;
    pthread_t thread_id;
    pthread_create(&thread_id, 0, ThreadRoute, reinterpret_cast<void *>(fd));
    pthread_detach(thread_id);
  }
  close(sockfd);
  return 0;
}
void *Server::ThreadRoute(void *arg) {
  int sockfd = reinterpret_cast<int64_t>(arg);
  MyChat my_chat(sockfd);
  while (1) {
    std::cout << "进入主循环" << std::endl;
    if (my_chat.ReadMessage() < 0) {
      printf("ReadMessage error!\n");
      sm.Erase(sockfd);
      break;
    } else {
      if (my_chat.SendMessage() < 0) {
        std::cerr << "send message error" << std::endl;
        sm.Erase(sockfd);
        break;
      }
    }
    printf("SendMessage ok!\n");
    //std::cout << ss.Size() << std::endl;
    //ss.Erase(sockfd);
    //std::cout << ss.Size() << std::endl;

  }
  close(sockfd);
  return NULL;
}
