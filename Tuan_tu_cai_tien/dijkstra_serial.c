#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define INFNTY INT_MAX

// Hàm tạo đồ thị ngẫu nhiên
void generate_random_graph(int V, int *adjacency_matrix) {
    srand(time(NULL));
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (i != j) {
                adjacency_matrix[i * V + j] = rand() % 10 + 1; // Trọng số từ 1 đến 10
            } else {
                adjacency_matrix[i * V + j] = 0; // Đường chéo chính là 0
            }
        }
    }
}

// Hàm in ma trận kề
void print_adjacency_matrix(int V, int *adjacency_matrix) {
    printf("\nADJACENCY MATRIX:\n");
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (adjacency_matrix[i * V + j] == INFNTY) {
                printf("INF ");
            } else {
                printf("%d ", adjacency_matrix[i * V + j]);
            }
        }
        printf("\n");
    }
}

// Thuật toán Dijkstra chạy tuần tự
void dijkstra(int V, int *adjacency_matrix, int source, int *distances) {
    int *visited = (int *)malloc(V * sizeof(int));
    
    for (int i = 0; i < V; i++) {
        distances[i] = INFNTY;
        visited[i] = 0;
    }
    distances[source] = 0;

    for (int count = 0; count < V - 1; count++) {
        // Tìm đỉnh chưa thăm với khoảng cách nhỏ nhất
        int min_dist = INFNTY, u = -1;
        for (int v = 0; v < V; v++) {
            if (!visited[v] && distances[v] <= min_dist) {
                min_dist = distances[v];
                u = v;
            }
        }

        visited[u] = 1;

        // Cập nhật khoảng cách cho các đỉnh lân cận
        for (int v = 0; v < V; v++) {
            if (!visited[v] && adjacency_matrix[u * V + v] && distances[u] != INFNTY &&
                distances[u] + adjacency_matrix[u * V + v] < distances[v]) {
                distances[v] = distances[u] + adjacency_matrix[u * V + v];
            }
        }
    }

    free(visited);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("USAGE: ./dijkstra_cpu <number_of_vertices>\n");
        return 1;
    }

    int V = atoi(argv[1]);
    int *adjacency_matrix = (int *)malloc(V * V * sizeof(int));
    int *distances = (int *)malloc(V * sizeof(int));

    generate_random_graph(V, adjacency_matrix);


    clock_t start = clock();
    dijkstra(V, adjacency_matrix, 0, distances); // Tìm đường đi từ đỉnh nguồn 0
    clock_t end = clock();


    printf("CPU Execution Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(adjacency_matrix);
    free(distances);

    return 0;
}
