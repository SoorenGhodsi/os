#include "life.h"
#include <pthread.h>

pthread_barrier_t barrier;

// The worker function for each thread.
void* life_worker(void* args) {
    // Unpack arguments here (thread_id, state, steps, etc.)

    for(int s = 0; s < steps; s++) {
        // Process a subset of rows/columns based on thread_id
        
        // After processing, wait on the barrier
        pthread_barrier_wait(&barrier);
        
        // Swap boards if necessary
        if(thread_id == 0) {
            // If using two boards, swap them here
        }
        
        // Wait again for the swap to complete before moving to the next iteration
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
    pthread_t thread_ids[threads];
    pthread_barrier_init(&barrier, NULL, threads);

    // Launch the threads
    for(int i = 0; i < threads; i++) {
        // Pack arguments for each thread (e.g., thread_id, state, steps)
        pthread_create(&thread_ids[i], NULL, life_worker, /* args */);
    }

    // Join threads after work is done
    for(int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
}
