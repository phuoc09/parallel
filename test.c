#include <stdio.h>
#include <omp.h>
#include <limits.h>

#define SIZE 10

int main() {
    int arr[SIZE] = {12, 3, 7, 9, 2, 15, 6, 8, 4, 10};
    int min_values[SIZE],global_min;
    int num_threads;

    // Initialize min_values to a large value
    for (int i = 0; i < SIZE; i++) {
        min_values[i] = INT_MAX;
    }
    omp_set_num_threads(4);
    // Parallel region
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int local_min = INT_MAX;

        printf("Thread %d bắt đầu xử lý...\n", thread_id);

        // Each thread processes part of the array
        #pragma omp for
        for (int i = 0; i < SIZE; i++) {
            printf("Thread %d xử lý phần tử arr[%d] = %d\n", thread_id, i, arr[i]);
            if (arr[i] < local_min) {
                local_min = arr[i];
            }
        }

        printf("Thread %d tìm được local_min = %d\n", thread_id, local_min);

        // Store local minimum in min_values
        min_values[thread_id] = local_min;

        // Get the number of threads used
        #pragma omp single
        num_threads = omp_get_num_threads();
        #pragma omp atomic
        {
            if (local_min < global_min) {
                global_min = local_min;
            }
        }
    }

    // Find the overall minimum from min_values
  /*  int min = INT_MAX;
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d đóng góp giá trị local_min = %d\n", i, min_values[i]);
        if (min_values[i] < min) {
            min = min_values[i];
        }
    }*/

    printf("Giá trị nhỏ nhất trong mảng là: %d\n", global_min);

    return 0;
}
