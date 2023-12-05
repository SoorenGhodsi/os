// Sooren Ghodsi (sg7ytn)
// Mehmet Faruk Yaylagul (urj5rb)

#include "pool.h"

// Task constructor/destructors
Task::Task() {}
Task::~Task() {}

// Main worker function in the threadpool
void* ThreadPool::Worker(void* p) {
    ThreadPool* pool = (ThreadPool*) p;

    while (true) {
        pthread_mutex_lock(&pool->mutex);

        // Wait for a task to be ready or for a stop signal
        while (pool->queue.empty() && !pool->stop_flag)
            pthread_cond_wait(&pool->cond, &pool->mutex);

        // If there's a stop signal and no tasks left, exit
        if (pool->stop_flag && pool->queue.empty()) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        // Take the next task from the front of the queue
        TaskBit task_bit = pool->queue.front();
        pool->queue.pop_front();

        pthread_mutex_unlock(&pool->mutex);

        task_bit.task->Run();

        pthread_mutex_lock(&pool->mutex);

        // Mark the task as finished
        pool->task_finished[task_bit.name] = true;
        pthread_cond_broadcast(&pool->cond);

        pthread_mutex_unlock(&pool->mutex);

        delete task_bit.task;
    }
    return nullptr;
}

// Constructor for threadpool: initializes threads, mutex, and condaddressr
ThreadPool::ThreadPool(int num_threads) {
    this->num_threads = num_threads;

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    threads = new pthread_t[num_threads];
    for (int i = 0; i < num_threads; ++i)
        pthread_create(&threads[i], nullptr, Worker, this);
}

// Destructor for threadpool: cleans up allocated memory
ThreadPool::~ThreadPool() {
    delete[] threads;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

// Queue a task with its name to be run
void ThreadPool::SubmitTask(const string &name, Task *task) {
    pthread_mutex_lock(&mutex);

    queue.push_back(TaskBit(name, task));

    // Signal a worker thread that a task is ready
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);
}

// Wait until the task with the name given is done running
void ThreadPool::WaitForTask(const string &name) {
    pthread_mutex_lock(&mutex);

    while (!task_finished[name])
        pthread_cond_wait(&cond, &mutex);

    // Remove the task from the map once it's finished
    task_finished.erase(name);

    pthread_mutex_unlock(&mutex);
}

// Signal all threads to stop after finishing their tasks
void ThreadPool::Stop() {
    pthread_mutex_lock(&mutex);

    stop_flag = true;

    pthread_mutex_unlock(&mutex);

    pthread_cond_broadcast(&cond);

    // Wait for all threads to finish their tasks
    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], nullptr);
}
