/*
Name: Happy Ndikumana
ID: 1001641586
*/

#define _GNU_SOURCE

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<sys/wait.h>

#define WHITESPACE " \n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENT 10
#define SIZE 15
int token_count;

//char** tokenizing(char * input)
//input: command line input from the user.
//retruns array of null terminated strings
char** tokenizing(char * input)
{
    char ** token = (char**) malloc(sizeof(char*) * MAX_COMMAND_SIZE);
    char * argument_pointer;
    char * working_string = strdup(input);
    //saving the head of the string
    //strsep will move it.
    char * working_root = working_string;
    token_count = 0;

    while((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL &&
    (token_count < MAX_COMMAND_SIZE))
    {
        token[token_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
        /*
        by null terminating the tokens.
        No Null will need to be added 
        to the exec funtion as a parameter
        We will pass in an array of parameters instead
        */
        if(strlen(token[token_count]) == 0)
            token[token_count] = NULL;
        token_count++;
    }
    free(working_root);
    return token;
}
//void adding_history(char ** history, char * input)
//input: history - array of strings conatining the history
//       input - new history to add to array.
//returns nothing
void adding_history(char ** history, char * input)
{
    //null terminating all my inputs to make it easier to print.
    char *working_string = strdup(input);
    for(int i = 0; i <= SIZE; i++)
    {
        if(history[i] == NULL)
        {
            history[i] = strndup(working_string, MAX_COMMAND_SIZE);
            return;
        }
    }
    for(int i = 0; i < SIZE; i++)
    {
        history[i] = history[i+1];
    }
    history[SIZE] = strndup(working_string, MAX_COMMAND_SIZE);
}
//void adding_pids(char** pids, int pid)
//input: pids - char array of pids,
//       pid - new pid to add to the pid list
//returns nothing
void adding_pids(char** pids, int pid)
{
    char tempPid[50];
    sprintf(tempPid, "%d", pid);
    char *working_string = strdup(tempPid);
    for(int i = 0; i <= SIZE; i++)
    {
        if(pids[i] == NULL)
        {
            pids[i] = strndup(working_string, MAX_COMMAND_SIZE);
            return;
        }
    }
    for(int i = 0; i < SIZE; i++)
    {
        pids[i] = pids[i+1];
    }
    pids[SIZE] = strndup(working_string, MAX_COMMAND_SIZE);
}
/*
int current_size(char ** history)
input: - history array of strings
returns an int representing the current size of the history array.
*/
int current_size(char ** history)
{
    int i = 0;
    while(i <= SIZE)
    {
        if(history[i] == NULL)
            return i;
        i++;
    }
    return -1;
}

int main()
{
    char * cmd_str = (char *) malloc(sizeof(char) * MAX_COMMAND_SIZE);
    char ** history = (char**) malloc(sizeof(char*) * MAX_COMMAND_SIZE);
    char ** pids = (char**)malloc(sizeof(char*) * MAX_COMMAND_SIZE);

    while(1)
    {
        int pid;
        memset(cmd_str, '\0', MAX_COMMAND_SIZE);
        printf("msh> ");
        while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
        if(strcmp(cmd_str, "\n") != 0)
        {
            char **separated;
            if(cmd_str[0] != '!')
            {
                //We will only add to history if it's a command
                //anything else like "!4" will bot be added to history
                adding_history(history, cmd_str);
                separated = tokenizing(cmd_str);
            }
            else
            {
                char *temp = &cmd_str[1];
                //printf("index = %s\n",temp);
                int index = atoi(temp);
                if(index < 0 || index > current_size(history))
                {
                    printf("no such history.\n");
                    continue;
                }
                else
                {
                    separated = tokenizing(history[index]);
                }
            }
            if(strcmp(separated[0], "exit")== 0 || strcmp(separated[0], "quit")== 0) 
                exit(0);
            if(strcmp(separated[0], "history")== 0 
            || strcmp(separated[0], "listpids")== 0)
            {
                if(strcmp(separated[0], "history")== 0)
                {
                    for(int i = 0; history[i] != NULL; i++)
                    {
                        printf("[%d]: %s", i, history[i]);
                    }
                }
                if(strcmp(separated[0], "listpids")== 0)
                {
                    for(int i = 0; pids[i] != NULL; i++)
                    {
                        printf("%s\n", pids[i]);
                    }
                }
            }
            else
            {
                pid = fork();
                if( pid == 0)
                {
                    /*
                    since chdir() can only be called from parent,
                    we need to handle how the child will handle 
                    the command "cd"
                    */
                    if(strcmp(separated[0],"cd") == 0)
                        return pid;
                    int ret = execvp(separated[0], separated);
                    if(ret == -1)
                        printf("%s: Command not found.\n",separated[0]);
                }
                else
                {
                    wait(NULL);
                    if(strcmp(separated[0],"cd") == 0)
                    {
                        char currentDirectory[1000];
                        getcwd(currentDirectory, 1000);
                        strcat(currentDirectory,"/");
                        /*
                        in case just "cd" is entered, 
                        it needs to be handled, it should 
                        bring us back to thhe home path
                        */
                        if(separated[1] == NULL)
                        {
                            int ret = chdir(currentDirectory);
                            if(ret != 0)
                                perror("");
                        }
                        else
                        {
                            strcat(currentDirectory,separated[1]);
                            int ret = chdir(currentDirectory);
                            if(ret != 0)
                                perror("");
                        }
                    }
                    adding_pids(pids, pid);
                }
            }
        }
        memset(cmd_str, 0, MAX_COMMAND_SIZE);
    }
    return 0;
}