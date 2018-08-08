#ifndef __MY_CHAT_H__
#define __MY_CHAT_H__

#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "jsoncpp/json/json.h"
#include "thread_pool.hpp"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

class Process;
class Server {
 public:
  Server(std::string ip, std::string port);
  ~Server();
  int Run();
  static void *ThreadRoute(void *);
 private:
  void SetNonBlock(int sockfd);
  void ProcessRequest(int connect_fd, int epoll_fd);
  void ProcessConnect(int listen_fd, int epoll_fd);
 private:
  std::string _ip;
  std::string _port;
  ThreadPool<Process> *pool;
};
class MyChat {
 public:
  MyChat(int fd);
  int ReadMessage();
  int SendMessage();
  int ParseJson();
  void BuildJson(std::string &json);
 private:
  int BroadCast();
  ssize_t NonBlockRead(int fd, char *buf, int size);

  int _sockfd;
  int _cmd;
  int _room;
  std::string _name;
  std::string _message;
  std::string _to;
  char buf[1024];
};
class SafeMap {
 public:
  friend class MyChat;
  friend class Server;
  SafeMap();
  ~SafeMap();
  void Insert(int fd, std::string &name);
  void Erase(int fd);
  size_t Size()const;
 private:
  std::map<int, std::string> m;
  pthread_mutex_t mutex;
};
extern SafeMap sm;
#endif /* end of include guard: __MY_CHAT_H__ */
