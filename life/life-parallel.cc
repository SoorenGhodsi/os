#include "life.h"
#include <pthread.h>
#include <vector>

using namespace std;

struct ThreadData {
    int start_row;
    int end_row;
    LifeBoard* state;
    LifeBoard* next_state;
    pthread_barrier_t* barrier;
};

void* thread_worker(void* arg) {
    ThreadData* data = (ThreadData*) arg;

    for (int y = data->start_row; y < data->end_row; ++y) {
        for (int x = 1; x < data->state->width() - 1; ++x) {
            int live_in_window = 0;
            for (int y_offset = -1; y_offset <= 1; ++y_offset)
                for (int x_offset = -1; x_offset <= 1; ++x_offset)
                    if (data->state->at(x + x_offset, y + y_offset))
                        ++live_in_window;

            data->next_state->at(x, y) = (
                live_in_window == 3 ||
                (live_in_window == 4 && data->state->at(x, y))
            );
        }
    }
    
    pthread_barrier_wait(data->barrier);
    return nullptr;
}

void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, nullptr, threads);
    vector<pthread_t> thread_ids(threads);
    vector<ThreadData> thread_data(threads);
    int rows_per_thread = state.height() / threads;
    
    for (int step = 0; step < steps; ++step) {
        LifeBoard next_state(state.width(), state.height());

        for (int i = 0; i < threads; ++i) {
            thread_data[i] = {
                i * rows_per_thread, 
                (i == threads - 1) ? state.height() - 1 : (i + 1) * rows_per_thread, 
                &state, 
                &next_state, 
                &barrier 
            };
            pthread_create(&thread_ids[i], nullptr, &thread_worker, &thread_data[i]);
        }

        for (int i = 0; i < threads; ++i)
            pthread_join(thread_ids[i], nullptr);
        
        swap(state, next_state);
    }

    pthread_barrier_destroy(&barrier);
}



// #include "life.h"
// #include <pthread.h>
// #include <vector>

// using namespace std;

// struct ThreadData {
//     int thread_id;
//     int start_row;
//     int end_row;
//     LifeBoard* state;
//     LifeBoard* next_state;
//     pthread_barrier_t* barrier;
// };

// void* thread_worker(void* arg) {
//     ThreadData* data = (ThreadData*) arg;

//     for (int y = data->start_row; y < data->end_row; ++y) {
//         for (int x = 1; x < data->state->width() - 1; ++x) {

//             int live_in_window = 0;
//             for (int y_offset = -1; y_offset <= 1; ++y_offset)
//                 for (int x_offset = -1; x_offset <= 1; ++x_offset)
//                     if (data->state->at(x + x_offset, y + y_offset))
//                         ++live_in_window;

//             data->next_state->at(x, y) = (
//                 live_in_window == 3 ||
//                 (live_in_window == 4 && data->state->at(x, y))
//             );
//         }
//     }
    
//     pthread_barrier_wait(data->barrier);

//     if(data->thread_id == 0)
//         swap(*(data->state), *(data->next_state));

//     pthread_barrier_wait(data->barrier);

//     return nullptr;
// }

// void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
//     pthread_barrier_t barrier;
//     pthread_barrier_init(&barrier, nullptr, threads+1);
//     vector<pthread_t> thread_ids(threads);
//     vector<ThreadData> thread_data(threads);
//     int rows_per_thread = state.height() / threads;
    
//     for (int step = 0; step < steps; ++step) {
//         LifeBoard next_state(state.width(), state.height());

//         for (int i = 0; i < threads; ++i) {
//             thread_data[i] = {
//                 i,
//                 i * rows_per_thread, 
//                 (i == threads - 1) ? state.height() - 1 : (i + 1) * rows_per_thread, 
//                 &state, 
//                 &next_state, 
//                 &barrier 
//             };
//             pthread_create(&thread_ids[i], nullptr, &thread_worker, &thread_data[i]);
//         }

//         // Main thread waits on both barriers too
//         pthread_barrier_wait(&barrier);
//         pthread_barrier_wait(&barrier);

//         for (int i = 0; i < threads; ++i)
//             pthread_join(thread_ids[i], nullptr);
//     }

//     pthread_barrier_destroy(&barrier);
// }
