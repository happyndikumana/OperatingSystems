#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define WHITESPACE " \n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENT 10
int token_count;
char ** tokenizing(char * input)
{
    char ** token = (char **) malloc (sizeof(char*) * MAX_COMMAND_SIZE);
    // char *token[MAX_NUM_ARGUMENT];
    token_count = 0;
    char *argument_ptr;
    char *working_str = strdup(input);
    char *working_root = working_str;

    while((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL && 
    (token_count < MAX_NUM_ARGUMENT))
    {
        token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
        if(strlen(token[token_count]) == 0)
            token[token_count] = NULL;
        token_count++;
    }
    // for(int i = 0; i < token_count; i++)
    // {
    //     printf("token[%d] = %s\n", i, token[i]);
    // }
    free(working_root);
    return token;
}
void adding_history(char ** history, char * input)
{
    char *working_string = strdup(input);
    for(int i = 0; i < 16; i++)
    {
        if(history[i] == NULL)
        {
            history[i] = strndup(working_string, MAX_COMMAND_SIZE);
            return;
        }
    }
    for(int i = 0; i < 15; i++)
    {
        history[i] = history[i+1];
    }
    history[15] = strndup(working_string, MAX_COMMAND_SIZE);
}

int main()
{
    char * cmd_str = (char*)malloc(sizeof(MAX_COMMAND_SIZE));
    char ** history = (char**)malloc(sizeof(char*) * MAX_COMMAND_SIZE);
    while(1)
    {
        printf("msh> ");
        while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
        adding_history(history, cmd_str);
        for(int i = 0; history[i] != NULL; i++)
        {
            printf("history[%d] = %s",i,history[i]);
            // if(strcmp(history[i], "") == 0)
            //     break;
        }
        // char **back = tokenizing(cmd_str);
        // for(int i = 0; i < token_count; i++)
        // {
        //     printf("token[%d] = %s\n", i, back[i]);
        // }
        // int len = strlen(str);
        // str[len-1] = '\0';
        // if(strcmp(str, "exit") == 0 || strcmp(str, "quit") == 0)
        //     exit(0);
        // if(strcmp(str, "ls") == 0)
        // {
        //     pid_t pid = fork();
        //     if(pid == 0)
        //         execl("/bin/ls","ls",NULL);
        //     else
        //     wait();
        // }

    }
    return 0;
}