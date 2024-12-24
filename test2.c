#include <stdio.h>
#include <omp.h>
#include <limits.h>

int main() {
    int arr[10] = {12, 3, 7, 9, 2, 15, 6, 8, 4, 10};
    int global_min = INT_MAX;

    #pragma omp parallel
    {
        int local_min = INT_MAX;

        #pragma omp for
        for (int i = 0; i < 10; i++) {
            if (arr[i] < local_min) {
                local_min = arr[i];
            }
        }

        #pragma omp atomic
        if (local_min < global_min) {
            global_min = local_min;
        }
    }

    printf("Minimum value: %d\n", global_min);
    return 0;
}
