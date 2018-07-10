#ifndef __MY_CHAT_H__
#define __MY_CHAT_H__

#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "jsoncpp/json/json.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

class Server {
 public:
  Server(std::string ip, std::string port);
  int Run();
  static void *ThreadRoute(void *);
 private:
  std::string _ip;
  std::string _port;
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

  int _sockfd;
  std::string _cmd;
  std::string _name;
  std::string _message;
  std::string _to;
  char buf[1024];
};
class SafeMap {
 public:
  friend class MyChat;
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
