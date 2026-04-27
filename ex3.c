#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
int main() {
    int rank, size;
    int search_no;
    int n = 10, i;
    int *arr = NULL;
    int *sub_arr;
    int local_n;
    int local_count = 0, total_count = 0;
    srand(time(NULL));
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    local_n = n / size;
    sub_arr = (int *)malloc(local_n * sizeof(int));
    if (rank == 0) {
        arr = (int *)malloc(n * sizeof(int));
        printf("Generated Array: ");
        for (i = 0; i < n; i++) {
            arr[i] = rand() % 10;
            printf("%d\t", arr[i]);
        }
        fflush(stdout);
        printf("\nEnter number: ");
        fflush(stdout);
        scanf("%d", &search_no);
    }
    MPI_Bcast(&search_no, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(arr, local_n, MPI_INT, sub_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    double start_time = MPI_Wtime();
    for (i = 0; i < local_n; i++) {
        if (sub_arr[i] == search_no) {
            local_count++;
        }
    }
    double end_time = MPI_Wtime();
    double exec_time = end_time - start_time;
    MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    printf("Process %d:\n", rank);
    printf("  Local Count of %d: %d\n", search_no, local_count);
    printf("  Execution Time (seconds): %f\n", exec_time);
    if (rank == 0) {
        printf("Total Frequency of %d: %d\n", search_no, total_count);
    }
    if (rank == 0) {
        free(arr);
    }
    free(sub_arr);
    MPI_Finalize();
    return 0;
}
