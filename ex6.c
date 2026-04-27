#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define BUFFER_SIZE 20
#define WORD_LEN 50
#define THREADS 3
char buffer[BUFFER_SIZE][WORD_LEN];
char result[BUFFER_SIZE][WORD_LEN];
int count = 0;
int result_count = 0;
int i;
pthread_mutex_t mutex;
pthread_cond_t cond;
char *dictionary[] = {"system", "jasmine", "rose", "good_day", "hello", "world", "operating"};
int dict_size = 7;
int check_dictionary(char *word)
{
    for (i = 0; i < dict_size; i++)
    {
        if (strcmp(dictionary[i], word) == 0)
            return 1;
    }
    return 0;
}
void* spell_check(void* arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (count == 0)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        if (count == -1)
        {
            pthread_mutex_unlock(&mutex);
            pthread_cond_broadcast(&cond);
            break;
        }
        char word[WORD_LEN];
        strcpy(word, buffer[0]);
        for (i = 0; i < count - 1; i++)
        {
            strcpy(buffer[i], buffer[i + 1]);
        }
        count--;
        pthread_mutex_unlock(&mutex);
        int status = check_dictionary(word);
        pthread_mutex_lock(&mutex);
        sprintf(result[result_count++], "%s -> %s", word, status ? "Correct" : "Incorrect");
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
    pthread_exit(NULL);
}
int main()
{
    pthread_t threads[THREADS];
    int n;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    printf("Enter number of words: ");
    scanf("%d", &n);
    if (n > BUFFER_SIZE)
    {
        printf("Error: Maximum %d words allowed!\n", BUFFER_SIZE);
        return 1;
    }
    printf("Enter %d words:\n", n);
    pthread_mutex_lock(&mutex);
    for (i = 0; i < n; i++)
    {
        scanf("%s", buffer[count]);
        count++;
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_broadcast(&cond);
    for (i = 0; i < THREADS; i++)
    {
        pthread_create(&threads[i], NULL, spell_check, NULL);
    }
    pthread_mutex_lock(&mutex);
    while (result_count < n)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    count = -1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    for (i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("\nSpell Check Results:\n");
    for (i = 0; i < result_count; i++)
    {
        printf("%s\n", result[i]);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}