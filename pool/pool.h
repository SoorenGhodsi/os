#ifndef POOL_H
#define POOL_H

#include <string>
#include <pthread.h>
#include <deque>
#include <map>
#include <atomic>

using namespace std;

class Task {
public:
    Task();
    virtual ~Task();
    virtual void Run() = 0;  // implemented by subclass
};

struct TaskWrapper {
    string name;
    Task* task;
    TaskWrapper(const string &name, Task *task) : name(name), task(task) {}
};

class ThreadPool {
private:
    int num_threads;
    deque<TaskWrapper> taskQueue;
    map<string, bool> taskFinished;

    pthread_t* threads;
    pthread_mutex_t queueMutex;
    pthread_cond_t condVar;
    atomic<bool> stopFlag;

    static void* Worker(void* arg);

public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    void SubmitTask(const string &name, Task *task);
    void WaitForTask(const string &name);
    void Stop();
};

#endif
