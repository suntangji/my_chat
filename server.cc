/// \file server.cc
/// \brief my_chat server
/// \author suntangji, suntangj2016@gmail.com
/// \version 1.0
/// \date 2018-07-04

#include "my_chat.h"

class Process {
 public:
  Process(int sockfd, int epoll_fd):_sockfd(sockfd),_epollfd(epoll_fd) {
  }
  void process() {
    //int sockfd = reinterpret_cast<int64_t>(arg);
    MyChat my_chat(_sockfd);
    std::cout << "进入主循环" << std::endl;
    if (my_chat.ReadMessage() < 0) {
      printf("ReadMessage error!\n");
      epoll_ctl(_epollfd, EPOLL_CTL_DEL, _sockfd, NULL);
      sm.Erase(_sockfd);
      close(_sockfd);
    } else {
      int ret = my_chat.SendMessage();
      if (ret < 0) {
        std::cerr << "send message error" << std::endl;
        sm.Erase(_sockfd);
      } else if (ret > 0) {
        // client quit
      }
      printf("SendMessage ok!\n");
    }
  }
 private:
  int _sockfd;
  int _epollfd;

};

Server::Server(std::string ip, std::string port)
  :_ip(ip), _port(port) {
  pool = new ThreadPool<Process>();
}
Server::~Server() {
  delete pool;
}

void Server::SetNonBlock(int fd) {
  int f1 = fcntl(fd, F_GETFL);
  if (f1 < 0) {
    perror("fcntl");
    return;
  }
  fcntl(fd, F_SETFL, f1 | O_NONBLOCK);
}

void Server::ProcessConnect(int listen_fd, int epoll_fd) {
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);
  int connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
  if (connect_fd < 0) {
    perror("accept");
    return;
  }
  SetNonBlock(connect_fd);
  struct epoll_event ev;
  ev.data.fd = connect_fd;
  ev.events = EPOLLIN | EPOLLET;
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev);
  if (ret < 0) {
    perror("epoll_ctl");
    return;
  }
}

void Server::ProcessRequest(int connect_fd, int epoll_fd) {
  Process *process = new Process(connect_fd, epoll_fd);
  pool->Append(process);
}
int Server::Run() {
  // 1. create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    //perror("socket");
    std::cerr<< "socket error" << std::endl;
    return -1;
  }
  // 端口复用
  int opt = 1;
  setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
  // 2. bind addr and port
  sockaddr_in server;
  bzero(&server, sizeof(server));
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
  int epoll_fd = epoll_create(10);
  if (epoll_fd < 0) {
    perror("epoll_create");
    return -1;
  }
  SetNonBlock(sockfd);
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = sockfd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
    perror("epoll_create");
    return -1;
  }
  while (1) {
    struct epoll_event events[1024];
    int size = epoll_wait(epoll_fd, events, sizeof(events)/sizeof(events[0]), -1);
    if (size < 0) {
      perror("epoll_wait");
      continue;
    } else if (size == 0) {
      continue;
    } else {
      for (int i = 0; i < size; i++) {
        if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          sm.Erase(events[i].data.fd);
          close(events[i].data.fd);
          std::cout << "EPOLLHUP" << std::endl;
        } else if (events[i].data.fd == sockfd) {
          ProcessConnect(sockfd, epoll_fd);
        } else if (events[i].events & EPOLLIN) {
          std::cout << "EPOLLIN" << std::endl;
          ProcessRequest(events[i].data.fd, epoll_fd);
        } else if (events[i].events & EPOLLOUT) {
          std::cout << "EPOLLOUT" << std::endl;
        }
      }
    }
  }
  close(sockfd);
  close(epoll_fd);
  return 0;
}
