#include "my_chat.h"
MyChat::MyChat(int fd)
  :_sockfd(fd) {
}
int MyChat::ParseJson() {
  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(buf, root, false)) {
    return -1;
  } else {
    _name = root["name"].asString();
    _message = root["content"].asString();
    _cmd = root["cmd"].asString();
    _to = root["to"].asString();
    return 0;
  }
}
void MyChat::BuildJson(std::string &json) {
  Json::Value root;
  // 客户端无法解析，改用 toStyledString
  //Json::FastWriter writer;

  root["content"] = _message;
  root["cmd"] = _cmd;
  root["to"] = _to;
  root["num"] = static_cast<int>(sm.Size());
  // cmd 是 3 表示要单聊，需要单独构造 json
  if (_cmd == "3") {
    root["name"] = _name;
  } else {
    root["name"] = _name;
  }
  //json = writer.write(root);
  json = root.toStyledString();
  //std::cout << json << std::endl;
}
int MyChat::ReadMessage() {
  ssize_t read_size = read(_sockfd, buf, sizeof(buf) - 1);
  if (read_size < 0) {
    std::cerr << "read socket error" << std::endl;
    return -1;
  } else if (read_size == 0) {
    // client quit
    return -2;
  } else {
    buf[read_size] = '\0';
    std::cout << buf << std::endl;
    if (ParseJson() < 0) {
      std::cerr << "json 解析失败" << std::endl;
      return -3;
    }
    sm.Insert(_sockfd, _name);
    return 0;
  }
}
int MyChat::BroadCast() {
  // 广播
  for (auto i : sm.m) {
    if (i.first != _sockfd) {
      std::cout << buf << std::endl;
      ssize_t write_size = write(i.first, buf, strlen(buf));
      if (write_size < 0) {
        std::cerr << "write error" << std::endl;
        return -1;
      }
    }
  }
  return 0;
}
int MyChat::SendMessage() {
  std::string json;
  BuildJson(json);
  strcpy(buf, json.c_str());
  if (_cmd == "2") {
    sm.Erase(_sockfd);
    std::string json;
    BuildJson(json);
    strcpy(buf, json.c_str());
    BroadCast();
    return 1;
  } else if (_cmd == "3") {
    // 单聊
    for (auto i : sm.m) {
      if (i.second == _to) {
        ssize_t write_size = write(i.first, buf, strlen(buf));
        if (write_size < 0) {
          std::cerr << "write error" << std::endl;
          return -1;
        }
      }
    }
  } else {
    BroadCast();
  }
  //printf("%s\n", buf);
  return 0;
}
