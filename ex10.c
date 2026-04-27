#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#define MAX_QUEUE 100
#define MAX_LINE 200
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
char queue[MAX_QUEUE][MAX_LINE];
int front = 0, rear = 0, count = 0;
omp_lock_t lock;
void produce(char line[], int tid)
{
    omp_set_lock(&lock);
    if (count < MAX_QUEUE)
    {
        strcpy(queue[rear], line);
        printf("Producer Thread %d produced: %s", tid, line);
        rear = (rear + 1) % MAX_QUEUE;
        count++;
    }
    omp_unset_lock(&lock);
}
void consume(int tid)
{
    char line[MAX_LINE];
    omp_set_lock(&lock);
    if (count > 0)
    {
        /* Remove line from queue */
        strcpy(line, queue[front]);
        front = (front + 1) % MAX_QUEUE;
        count--;
        omp_unset_lock(&lock);
        /* Print only tokenized words */
        printf("\nConsumer Thread %d Tokens:\n", tid);
        char *token = strtok(line, " \n");
        while (token != NULL)
        {
            printf("%s\n", token);   /* each token in new line */
            token = strtok(NULL, " \n");
        }
        printf("\n");
    }
    else
    {
        omp_unset_lock(&lock);
    }
}
int main()
{
    FILE *fp1, *fp2;
    /* Initialize lock */
    omp_init_lock(&lock);
    /* Open files */
    fp1 = fopen("file1.txt", "r");
    fp2 = fopen("file2.txt", "r");
    if (fp1 == NULL || fp2 == NULL)
    {
        printf("Error opening files\n");
        return 1;
    }
    printf("\n=== Producer Consumer using OpenMP ===\n\n");
    #pragma omp parallel num_threads(NUM_PRODUCERS + NUM_CONSUMERS)
    {
        int tid = omp_get_thread_num();
        /* Producer Threads */
        if (tid < NUM_PRODUCERS)
        {
            FILE *fp;
            char local_line[MAX_LINE];
            if (tid == 0)
                fp = fp1;
            else
                fp = fp2;
            while (fgets(local_line, sizeof(local_line), fp))
            {
                produce(local_line, tid);
            }
        }
        /* Wait until producers finish */
        #pragma omp barrier
        /* Consumer Threads */
        if (tid >= NUM_PRODUCERS)
        {
            while (1)
            {
                omp_set_lock(&lock);

                if (count == 0)
                {
                    omp_unset_lock(&lock);
                    break;
                }
                omp_unset_lock(&lock);

                consume(tid);
            }
        }
    }
    fclose(fp1);
    fclose(fp2);
    omp_destroy_lock(&lock);
    return 0;
}