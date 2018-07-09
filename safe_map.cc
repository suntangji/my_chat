#include "my_chat.h"

SafeMap sm;

SafeMap::SafeMap() {
  pthread_mutex_init(&mutex, NULL);
}
SafeMap::~SafeMap() {
  pthread_mutex_destroy(&mutex);
}

size_t SafeMap::Size()const {
  return m.size();
}

void SafeMap::Insert(int fd, std::string &name) {
  pthread_mutex_lock(&mutex);
  m.insert(make_pair(fd, name));
  pthread_mutex_unlock(&mutex);
}
void SafeMap::Erase(int fd) {
  pthread_mutex_lock(&mutex);
  m.erase(fd);
  pthread_mutex_unlock(&mutex);
}
