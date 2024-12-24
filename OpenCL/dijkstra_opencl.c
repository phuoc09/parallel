#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#include <CL/cl.h>
#define INFNTY INT_MAX
#define TRUE 1
#define FALSE 0
typedef int boolean;

/* Generate random graph */
void generate_random_graph(int V, int *adjacency_matrix) {
    srand(time(NULL));
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (i != j) {
                adjacency_matrix[i * V + j] = rand() % 10;
                adjacency_matrix[j * V + i] = adjacency_matrix[i * V + j];
            } else {
                adjacency_matrix[i * V + j] = 0;
            }
        }
    }
}

/* Print adjacency matrix */
void print_adjacency_matrix(int V, int *adjacency_matrix) {
    printf("\nADJACENCY MATRIX:\n");
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            printf("%d ", adjacency_matrix[i * V + j]);
        }
        printf("\n");
    }
}

/* Dijkstra Algorithm on CPU */
void dijkstra_cpu(int V, int *adjacency_matrix, int *distances) {
    boolean *visited = (boolean *)malloc(V * sizeof(boolean));
    for (int i = 0; i < V; i++) {
        distances[i] = INFNTY;
        visited[i] = FALSE;
    }
    distances[0] = 0;

    for (int count = 0; count < V - 1; count++) {
        int min_dist = INFNTY, u = -1;
        for (int v = 0; v < V; v++) {
            if (!visited[v] && distances[v] <= min_dist) {
                min_dist = distances[v];
                u = v;
            }
        }
        visited[u] = TRUE;

        for (int v = 0; v < V; v++) {
            if (!visited[v] && adjacency_matrix[u * V + v] && distances[u] != INFNTY && distances[u] + adjacency_matrix[u * V + v] < distances[v]) {
                distances[v] = distances[u] + adjacency_matrix[u * V + v];
            }
        }
    }
    free(visited);
}

/* OpenCL Dijkstra Kernel */
const char *kernel_source = "\n" \
"__kernel void dijkstra_kernel(__global int *adj_matrix, __global int *dist, __global int *visited, int V, int src) {\n" \
"    int id = get_global_id(0);\n" \
"    if (id < V && !visited[id]) {\n" \
"        int min_dist = dist[src];\n" \
"        for (int i = 0; i < V; i++) {\n" \
"            if (!visited[i] && adj_matrix[id * V + i] && dist[src] + adj_matrix[id * V + i] < dist[i]) {\n" \
"                dist[i] = dist[src] + adj_matrix[id * V + i];\n" \
"            }\n" \
"        }\n" \
"    }\n" \
"}\n";

/* Dijkstra Algorithm on OpenCL */
void dijkstra_opencl(int V, int *adjacency_matrix, int *distances) {
    cl_int err;
    size_t global_size, local_size;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    /* Initialize OpenCL */
    err = clGetPlatformIDs(1, &platform, NULL);
    err |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(0, 1, &device, NULL, NULL, &err);
    queue = clCreateCommandQueue(context, device, 0, &err);

    /* Create buffers */
    cl_mem d_adj_matrix = clCreateBuffer(context, CL_MEM_READ_ONLY, V * V * sizeof(int), NULL, &err);
    cl_mem d_distances = clCreateBuffer(context, CL_MEM_READ_WRITE, V * sizeof(int), NULL, &err);
    cl_mem d_visited = clCreateBuffer(context, CL_MEM_READ_WRITE, V * sizeof(int), NULL, &err);

    /* Copy data to device */
    err |= clEnqueueWriteBuffer(queue, d_adj_matrix, CL_TRUE, 0, V * V * sizeof(int), adjacency_matrix, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_distances, CL_TRUE, 0, V * sizeof(int), distances, 0, NULL, NULL);

    /* Create program and kernel */
    program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    err |= clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "dijkstra_kernel", &err);

    /* Set kernel arguments */
    err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_adj_matrix);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_distances);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_visited);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &V);

    /* Execute kernel */
    global_size = V;
    local_size = 64;

    for (int src = 0; src < V; src++) {
        err |= clSetKernelArg(kernel, 4, sizeof(int), &src);
        err |= clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    }

    /* Read results */
    err |= clEnqueueReadBuffer(queue, d_distances, CL_TRUE, 0, V * sizeof(int), distances, 0, NULL, NULL);

    /* Cleanup */
    clReleaseMemObject(d_adj_matrix);
    clReleaseMemObject(d_distances);
    clReleaseMemObject(d_visited);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("USAGE: ./dijkstra_compare <number_of_vertices>\n");
        return 1;
    }

    int V = atoi(argv[1]);
    int *adjacency_matrix = (int *)malloc(V * V * sizeof(int));
    int *distances_cpu = (int *)malloc(V * sizeof(int));
    int *distances_opencl = (int *)malloc(V * sizeof(int));

    generate_random_graph(V, adjacency_matrix);

    clock_t start_cpu = clock();
    dijkstra_cpu(V, adjacency_matrix, distances_cpu);
    clock_t end_cpu = clock();

    printf("CPU Execution Time: %f seconds\n", (double)(end_cpu - start_cpu) / CLOCKS_PER_SEC);

    clock_t start_opencl = clock();
    dijkstra_opencl(V, adjacency_matrix, distances_opencl);
    clock_t end_opencl = clock();

    printf("OpenCL Execution Time: %f seconds\n", (double)(end_opencl - start_opencl) / CLOCKS_PER_SEC);

    

    free(adjacency_matrix);
    free(distances_cpu);
    free(distances_opencl);

    return 0;
}
