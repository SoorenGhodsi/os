#include "pool.h"

Task::Task() {

}

Task::~Task() {

}

void* ThreadPool::Worker(void* arg) {
    ThreadPool* pool = (ThreadPool*) arg;

    while (true) {
        pthread_mutex_lock(&pool->queueMutex);
        while (pool->taskQueue.empty() && !pool->stopFlag.load())
            pthread_cond_wait(&pool->condVar, &pool->queueMutex);

        if (pool->stopFlag.load() && pool->taskQueue.empty()) {
            pthread_mutex_unlock(&pool->queueMutex);
            break;
        }

        TaskWrapper taskWrapper = pool->taskQueue.front();
        pool->taskQueue.pop_front();
        pthread_mutex_unlock(&pool->queueMutex);

        taskWrapper.task->Run();

        pthread_mutex_lock(&pool->queueMutex);
        pool->taskFinished[taskWrapper.name] = true;
        pthread_cond_broadcast(&pool->condVar);
        pthread_mutex_unlock(&pool->queueMutex);

        delete taskWrapper.task;
    }
    return nullptr;
}

ThreadPool::ThreadPool(int num_threads) : num_threads(num_threads), stopFlag(false) {
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&condVar, nullptr);

    threads = new pthread_t[num_threads];
    for (int i = 0; i < num_threads; ++i)
        pthread_create(&threads[i], nullptr, Worker, this);
}

ThreadPool::~ThreadPool() {
    delete[] threads;
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&condVar);
}

void ThreadPool::SubmitTask(const string &name, Task *task) {
    pthread_mutex_lock(&queueMutex);
    taskQueue.push_back(TaskWrapper(name, task));
    pthread_cond_signal(&condVar);
    pthread_mutex_unlock(&queueMutex);
}

void ThreadPool::WaitForTask(const string &name) {
    pthread_mutex_lock(&queueMutex);
    while (!taskFinished[name])
        pthread_cond_wait(&condVar, &queueMutex);

    taskFinished.erase(name);
    pthread_mutex_unlock(&queueMutex);
}

void ThreadPool::Stop() {
    stopFlag.store(true);
    pthread_cond_broadcast(&condVar);

    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], nullptr);
}
