// Sooren and Faruk 11/27/23

#include "life.h"
#include <pthread.h>
#include <vector>
#include <iostream>

using namespace std;

// To pass data to each thread
struct ThreadData {
    int start_row;
    int end_row;
    LifeBoard* state;
    LifeBoard* next_state;
    pthread_barrier_t* barrier;
    int num_threads;
    int thread_id;
    int steps;
};

// Function executed by each thread
void* thread_worker(void* arg) {
    ThreadData* data = (ThreadData*) arg;

    // Calculate valid start and end rows for this thread
    int rows_per_thread = (data->state->height()-1) / data->num_threads;
    int valid_start = data->thread_id * rows_per_thread + 1;
    int valid_end = valid_start + rows_per_thread - 1;
    if (data->thread_id == data->num_threads - 1)
        valid_end += (data->state->height()-1) % data->num_threads;
    
    // Modified from life-serial.cc:
    for (int step = 0; step < data->steps; ++step) {
        /* We use the range [1, width - 1) here instead of
         * [0, width) because we fix the edges to be all 0s.
         */
        for (int y = valid_start; y <= valid_end; ++y) {
            for (int x = 1; x < data->state->width() - 1; ++x) {
                int live_in_window = 0;
                /* For each cell, examine a 3x3 "window" of cells around it,
                 * and count the number of live (true) cells in the window. */
                for (int y_offset = -1; y_offset <= 1; ++y_offset) {
                    for (int x_offset = -1; x_offset <= 1; ++x_offset) {
                        int neighbor_x = x + x_offset;
                        int neighbor_y = y + y_offset;
                        // Check if within bounds
                        if (neighbor_x >= 0 && neighbor_x < data->state->width() &&
                            neighbor_y >= 0 && neighbor_y < data->state->height())
                            if (data->state->at(x + x_offset, y + y_offset))
                                ++live_in_window;
                    }
                }

                /* Cells with 3 live neighbors remain or become live.
                   Live cells with 2 live neighbors remain live. */
                data->next_state->at(x, y) = (
                    live_in_window == 3 /* dead cell with 3 neighbors or live cell with 2 */ ||
                    (live_in_window == 4 && data->state->at(x, y)) /* live cell with 3 neighbors */
                );
            }
        }

        // Sync threads using barrier and swap if last thread
        pthread_barrier_wait(data->barrier);
        
        if (data->thread_id == 0) swap(*data->state, *data->next_state);
        
        pthread_barrier_wait(data->barrier);
    }

    return nullptr;
}

// Actual parallel game-of-life function
void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, nullptr, threads); // barrier init

    vector<pthread_t> thread_ids(threads);
    vector<ThreadData> thread_data(threads);

    // Calculate rows
    int rows_per_thread = (state.height()-1) / threads;
    int remaining_rows = (state.height()-1) % threads;
    LifeBoard next_state(state.width(), state.height());

    // Create threads and give them their data
    for (int i = 0; i < threads; ++i) {
        int start_row = i * rows_per_thread;
        int end_row = start_row + rows_per_thread - 1;
        if (threads - 1 == i) end_row += remaining_rows; // only add the rest if last thread

        thread_data[i] = {start_row, end_row, &state, &next_state, &barrier, threads, i, steps};
        pthread_create(&thread_ids[i], nullptr, &thread_worker, &thread_data[i]); // thread create
    }

    // Wait for all threads to finish executing
    for (pthread_t& thread : thread_ids)
        pthread_join(thread, nullptr); // thread join

    // Clean up barrier after it's been used
    pthread_barrier_destroy(&barrier);
}
