#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <list>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>

template <typename T>
class ThreadPool {
 public:
  ThreadPool(int thread_num = 4, int max_request = 10000);
  ~ThreadPool();
  bool Append(T *request);
 private:
  static void *worker(void *arg);
  void run();
 private:
  int _thread_number;
  int _max_request;
  pthread_t *_threads;
  std::list<T *> _work_queue;
  pthread_mutex_t _lock;
  sem_t _queuestat;
  bool _stop;
};

template <typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_request)
  :_thread_number(thread_num), _max_request(max_request)
  , _stop(false) {
  pthread_mutex_init(&_lock, NULL);
  sem_init(&_queuestat, 0, 0);
  _threads = new pthread_t[_thread_number];
  for (int i = 0; i < _thread_number; i++) {
    if (pthread_create(_threads + i, NULL, worker, this)!= 0) {
      delete []_threads;
    }
    if (pthread_detach(_threads[i])) {
      delete []_threads;
    }
  }

}

template <typename T>
ThreadPool<T>::~ThreadPool() {
  pthread_mutex_destroy(&_lock);
  sem_destroy(&_queuestat);
  delete []_threads;
  _stop = true;
}

template <typename T>
bool ThreadPool<T>::Append(T *request) {
  pthread_mutex_lock(&_lock);
  if (_work_queue.size() > _max_request) {
    pthread_mutex_unlock(&_lock);
    return false;
  }
  _work_queue.push_back(request);
  pthread_mutex_unlock(&_lock);
  sem_post(&_queuestat);
  return true;
}

template <typename T>
void *ThreadPool<T>::worker(void *arg) {
  ThreadPool *pool = (ThreadPool *)arg;
  pool->run();
  return pool;
}

template <typename T>
void ThreadPool<T>::run() {
  while (!_stop) {
    sem_wait(&_queuestat);
    pthread_mutex_lock(&_lock);
    if (_work_queue.empty()) {
      pthread_mutex_unlock(&_lock);
      continue;
    }
    T *request = _work_queue.front();
    _work_queue.pop_front();
    pthread_mutex_unlock(&_lock);
    if (!request) {
      continue;
    }
    request->process();
    delete request;
  }
}
#endif /* end of include guard: __THREAD_POOL_H__ */
