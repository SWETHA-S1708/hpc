#define _POSIX_C_SOURCE 199310L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#define ITERATIONS 100
double get_time_sec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
int main()
{
    int N;
    printf("Enter the size of the square matrix (N): ");
    scanf("%d", &N);
    if (N <= 0)
    {
        fprintf(stderr, "Matrix size must be a positive integer.\n");
        return 1;
    }
    int (*A)[N] = malloc(sizeof(int) * N * N);
    int (*B)[N] = malloc(sizeof(int) * N * N);
    if (A == NULL || B == NULL)
    {
        perror("malloc failed for A or B");
        return 1;
    }
    int shmid_add, shmid_sub;
    int (*Add)[N];
    int (*Sub)[N];
    shmid_add = shmget(IPC_PRIVATE, sizeof(int) * N * N, IPC_CREAT | 0666);
    if (shmid_add == -1)
    {
        perror("shmget for Add failed");
        free(A);
        free(B);
        return 1;
    }
    Add = shmat(shmid_add, NULL, 0);
    if (Add == (void *)-1)
    {
        perror("shmat for Add failed");
        shmctl(shmid_add, IPC_RMID, NULL);
        free(A);
        free(B);
        return 1;
    }
    shmid_sub = shmget(IPC_PRIVATE, sizeof(int) * N * N, IPC_CREAT | 0666);
    if (shmid_sub == -1)
    {
        perror("shmget for Sub failed");
        shmdt(Add);
        shmctl(shmid_add, IPC_RMID, NULL);
        free(A);
        free(B);
        return 1;
    }
    Sub = shmat(shmid_sub, NULL, 0);
    if (Sub == (void *)-1)
    {
        perror("shmat for Sub failed");
        shmdt(Add);
        shmctl(shmid_add, IPC_RMID, NULL);
        shmctl(shmid_sub, IPC_RMID, NULL);
        free(A);
        free(B);
        return 1;
    }
    srand(time(NULL));
    if (N <= 10)
    {
        printf("Matrix A:\n");
    }
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            A[i][j] = rand() % 10;
            if (N <= 10)
            {
                printf("%d ", A[i][j]);
            }
        }
        if (N <= 10)
        {
            printf("\n");
        }
    }
    if (N <= 10)
    {
        printf("\nMatrix B:\n");
    }
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            B[i][j] = rand() % 10;
            if (N <= 10)
            {
                printf("%d ", B[i][j]);
            }
        }
        if (N <= 10)
        {
            printf("\n");
        }
    }
    // ---------------- SERIAL EXECUTION ----------------
    double serial_add_start = get_time_sec();
    for (int r = 0; r < ITERATIONS; r++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                Add[i][j] = A[i][j] + B[i][j];
            }
        }
    }
    double serial_add_end = get_time_sec();
    double serial_add_time = serial_add_end - serial_add_start;
    double serial_sub_start = get_time_sec();
    for (int r = 0; r < ITERATIONS; r++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                Sub[i][j] = A[i][j] - B[i][j];
            }
        }
    }
    double serial_sub_end = get_time_sec();
    double serial_sub_time = serial_sub_end - serial_sub_start;
    double serial_total_time = serial_add_time + serial_sub_time;
    // ---------------- PARALLEL EXECUTION ----------------
    double parallel_add_time = 0;
    double parallel_sub_time = 0;
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe failed");
        shmdt(Add);
        shmctl(shmid_add, IPC_RMID, NULL);
        shmdt(Sub); shmctl(shmid_sub, IPC_RMID, NULL);
        free(A);
        free(B);
        return 1;
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        close(fd[0]);
        close(fd[1]);
        shmdt(Add);
        shmctl(shmid_add, IPC_RMID, NULL);
        shmdt(Sub);
        shmctl(shmid_sub, IPC_RMID, NULL);
        free(A);
        free(B);
        return 1;
    }
    else if (pid == 0)
    {
       close(fd[0]);
       double start = get_time_sec();
       for (int r = 0; r < ITERATIONS; r++)
       {
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    Sub[i][j] = A[i][j] - B[i][j];
                }
            }
        }
        double end = get_time_sec();
        parallel_sub_time = end - start;
        write(fd[1], &parallel_sub_time, sizeof(double));
        close(fd[1]);
        shmdt(Add);
        shmdt(Sub);
        free(A);
        free(B);
        exit(0);
    }
    else
    {
       close(fd[1]);
       double start = get_time_sec();
       for (int r = 0; r < ITERATIONS; r++)
       {
          for (int i = 0; i < N; i++)
          {
             for (int j = 0; j < N; j++)
             {
                Add[i][j] = A[i][j] + B[i][j];
             }
          }
       }
       double end = get_time_sec();
       parallel_add_time = end - start;
 //      wait(NULL);
       read(fd[0], &parallel_sub_time, sizeof(double));
       close(fd[0]);
       printf("\nParent (Addition) Execution Time:%.3f ms\n", parallel_add_time * 1000);
       printf("Child (Subtraction) Execution Time:%.3f ms\n", parallel_sub_time * 1000);
    }
    // ---------------- PARALLEL TOTAL TIME (MAX of the two parallel tasks) ----------------
    double parallel_total_time = (parallel_add_time > parallel_sub_time) ? parallel_add_time : parallel_sub_time;
    if (N <= 10)
    {
       printf("\nAddition Result Matrix:\n");
       for (int i = 0; i < N; i++)
       {
          for (int j = 0; j < N; j++)
             printf("%d ", Add[i][j]);
          printf("\n");
       }
       printf("\nSubtraction Result Matrix:\n");
       for (int i = 0; i < N; i++)
       {
          for (int j = 0; j < N; j++)
               printf("%d ", Sub[i][j]);
           printf("\n");
       }
    }
    else
    {
       printf("\nMatrices are too large to print. Displaying execution times only.\n");
    }
    // ---------------- TIME COMPARISON ----------------
    printf("\n------ TIME COMPARISON ------\n");
    printf("Serial Execution Time: %.3f ms\n", serial_total_time * 1000);
    printf("Parallel Execution Time: %.3f ms\n", parallel_total_time * 1000);
    printf("Time Difference: %.3f ms\n", (serial_total_time - parallel_total_time) * 1000);
    // Detach and remove shared memory segments
    shmdt(Add);
    shmctl(shmid_add, IPC_RMID, NULL);
    shmdt(Sub);
    shmctl(shmid_sub, IPC_RMID, NULL);
    // Free dynamically allocated memory
    free(A);
    free(B);
    return 0;
}