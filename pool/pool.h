#ifndef POOL_H_
#include <string>
#include <pthread.h>

#include <map>
#include <deque>

using namespace std;

class Task {
public:
    Task();
    virtual ~Task();
    
    virtual void Run() = 0;  // implemented by subclass
};

struct TaskBit {
    string name;
    Task* task;

    TaskBit(const string &name, Task *task) {
        this->name = name;
        this->task = task;
    }
};

class ThreadPool {
private:
    int num_threads;
    deque<TaskBit> task_queue;
    map<string, bool> task_finished;

    pthread_t* threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop_flag = false;

    static void* Worker(void* p);

public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    // Submit a task with a particular name.
    void SubmitTask(const string &name, Task *task);

    // Wait for a task by name, if it hasn't been waited for yet. Only returns after the task is completed.
    void WaitForTask(const string &name);

    // Stop all threads. All tasks must have been waited for before calling this.
    // You may assume that SubmitTask() is not caled after this is called.
    void Stop();
};

#endif
