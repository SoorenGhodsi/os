#include "pool.h"

Task::Task() {}

Task::~Task() {}

void* ThreadPool::Worker(void* p) {
    ThreadPool* pool = (ThreadPool*) p;

    while (true) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->task_queue.empty() && !pool->stop_flag)
            pthread_cond_wait(&pool->cond, &pool->mutex);

        if (pool->stop_flag && pool->task_queue.empty()) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        TaskBit taskBit = pool->task_queue.front();
        pool->task_queue.pop_front();
        pthread_mutex_unlock(&pool->mutex);

        taskBit.task->Run();

        pthread_mutex_lock(&pool->mutex);
        pool->task_finished[taskBit.name] = true;
        pthread_cond_broadcast(&pool->cond);
        pthread_mutex_unlock(&pool->mutex);

        delete taskBit.task;
    }
    return nullptr;
}

ThreadPool::ThreadPool(int num_threads) {
    this->num_threads = num_threads;

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    threads = new pthread_t[num_threads];
    for (int i = 0; i < num_threads; ++i)
        pthread_create(&threads[i], nullptr, Worker, this);
}

ThreadPool::~ThreadPool() {
    delete[] threads;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void ThreadPool::SubmitTask(const string &name, Task *task) {
    pthread_mutex_lock(&mutex);
    task_queue.push_back(TaskBit(name, task));
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void ThreadPool::WaitForTask(const string &name) {
    pthread_mutex_lock(&mutex);
    while (!task_finished[name])
        pthread_cond_wait(&cond, &mutex);

    task_finished.erase(name);
    pthread_mutex_unlock(&mutex);
}

void ThreadPool::Stop() {
    pthread_mutex_lock(&mutex);
    stop_flag = true;
    pthread_mutex_unlock(&mutex);

    pthread_cond_broadcast(&cond);

    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], nullptr);
}
