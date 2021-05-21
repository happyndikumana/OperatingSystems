#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include<pthread.h>

#define MAX 5000000
#define NUM_THREAD 1

int total = 0;
int n1,n2; 
char *s1,*s2;
pthread_mutex_t mutex;

FILE *fp;

int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }
    
    s1=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }
    
    s2=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }
    
    /*read s1 s2 from the file*/
    
    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2)-1; /*length of s2*/
    
    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }
}

void * num_substring ( void *a)
{
    int index = *((int *) a);
    int i,j,k;
    int count;
    int start = index * (n1/NUM_THREAD);
    int end = (index + 1) * (n1/NUM_THREAD);

    /*
    making sure each thread is reading subtring length into 
    the next thread's workspace incase the main string is divided incorectly
    */
    if(index < NUM_THREAD - 1)
    {
        end += (n2 - 1);
    }

    //printf("index: %d, start: %d, end: %d\n", index, start, end);

    for (i = start; i < end; i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2)
            {
                pthread_mutex_lock(&mutex);
                total++; /*find a substring in this step*/
                pthread_mutex_unlock(&mutex);
            }
         }
    }
}
    
int main(int argc, char *argv[])
{

    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] );

    struct timeval start, end;
    float mtime; 
    int secs, usecs;

    pthread_mutex_init(&mutex, NULL);    

    gettimeofday(&start, NULL);

    pthread_t tid[NUM_THREAD];
    
    int i;
    int indicies[NUM_THREAD];
    for(i = 0; i < NUM_THREAD; i++)
    {
        /*
        to avoid using i because multiple threads will try to change it
        save it in an array. This way the value of "i" will not be overwritten
        */ 
        indicies[i] = i;
        pthread_create(&tid[i], NULL, num_substring, (void*)&indicies[i]);
    }


    // count = num_substring () ;

    for(i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(tid[i], NULL);
    }

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , total) ;
    printf ("Elapsed time is : %f milliseconds\n", mtime );

    if( s1 )
    {
      free( s1 );
    }

    if( s2 )
    {
      free( s2 );
    }

    pthread_mutex_destroy(&mutex);
    return 0 ; 
}