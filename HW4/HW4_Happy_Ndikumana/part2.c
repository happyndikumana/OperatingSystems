#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<semaphore.h>
#include<unistd.h>
//#include<bool.h>

#define BUFFER_SIZE 5

sem_t producerSem, consumerSem;

char buffer[BUFFER_SIZE];
pthread_mutex_t mutex, mutex2;
pthread_cond_t condition, condition2;
int i = 0, j = 0; 
int stop = 0;

// void * consumer(void * value)
// {
    
//     while(j < BUFFER_SIZE)
//     {
//         // if(j >= i)
//         // {
//         //     if(j == i)
//         //     {
//         //         printf("%c",buffer[j]);
//         //         sem_wait(&consumerSem);
//         //         j = 0;
                
//         //     }
//         //     else
//         //     {
//         //         sem_wait(&consumerSem);    
//         //     }

//         //     //pthread_cond_wait(&condition);
//         //     // printf("%c",buffer[j]);
//         //     // sem_post(&consumerSem);
//         //     // //sem_wait(&producerSem);
//         //     // j = 0;
//         // }
//         if(j == BUFFER_SIZE)
//         {
//             pthread_cond_signal(&condition);
//         }
//         pthread_mutex_lock(&mutex);
//         if(j >= i)
//         {
//             pthread_cond_wait(&condition2, &mutex);
//         }
//         pthread_mutex_unlock(&mutex);
//         printf("%c",buffer[j]);
//         j++;
//     }
//     // pthread_mutex_unlock(&mutex);
// }
// void * producer(void *value)
// {
//     FILE * filePointer = fopen("message.txt", "r");
//     if(filePointer  == NULL)
//     {
//         perror("could not open file");
//     }
    
//     while(!feof(filePointer))
//     {
//         pthread_mutex_lock(&mutex);
//         //sem_wait(&consumerSem);
//         if(i == BUFFER_SIZE)
//         {
//             //maxIndex = i;
//             pthread_cond_wait(&condition, &mutex);
//             i = 0;
//             j = 0;
//         }
//         pthread_mutex_unlock(&mutex);
//         //pthread_mutex_lock(&mutex);
//         buffer[i] = fgetc(filePointer);
//         printf("%c ",buffer[i]);
//         i++;
//         pthread_mutex_lock(&mutex);
//         {
//             if( i > j)
//             {
//                 pthread_cond_signal(&condition2);
//             }
//         }
//         pthread_mutex_unlock(&mutex);
//         // pthread_mutex_unlock(&mutex);
        
//     }
    
// }
void * producer(void *value)
{
    FILE * filePointer = fopen("message.txt", "r");
    if(filePointer  == NULL)
    {
        perror("could not open file");
    }
    while(!feof(filePointer))
    {
        pthread_mutex_lock(&mutex);
        buffer[i] = fgetc(filePointer);
        //printf("%c ",buffer[i]);
        i++;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condition);

        pthread_mutex_lock(&mutex2);
        while(j < i)
        {
            pthread_cond_wait(&condition2, &mutex2);
        }
        i = 0;
        j = i;
        pthread_mutex_unlock(&mutex2);
        
    }
    // pthread_mutex_lock(&mutex);
    // stop = 1;
    // pthread_mutex_unlock(&mutex);
}
void * consumer( void *value)
{
    sleep(1);
    while(j < BUFFER_SIZE)
    {
        pthread_mutex_lock(&mutex);
        while(j > i)
        {
            pthread_cond_wait(&condition, &mutex);   
        }
        printf("%c",buffer[j]);
        j++;
        pthread_mutex_lock(&mutex2);
        if(j)
        // if(stop)
        //     break;
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    // buffer[0] = '1';
    // buffer[1] = '2';
    // buffer[2] = '3';
    // buffer[3] = '4';
    // buffer[4] = '5';
    
    pthread_t consumerThread, producerThread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    // sem_init(&producerSem, NONSHARED, 0);
    // sem_init(&consumerSem, NONSHARED, 0);

    pthread_create(&producerThread, NULL, &producer, NULL);
    pthread_create(&consumerThread, NULL, &consumer, NULL);

    pthread_join(consumerThread, NULL);
    pthread_join(producerThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);


    return 0;
}