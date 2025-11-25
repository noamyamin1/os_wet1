//commands.c
#include "commands.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>



static char oldpwd[CMD_LENGTH_MAX] = {0};


//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

void runShowPid()
{
	printf("smash pid is %d\n", getpid());
}

void runPwd()
{
	char cwd[CMD_LENGTH_MAX];

	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		perrorSmash("pwd", "getcwd failed");
		return;
	}

	printf("%s\n", cwd);

}

void executeCommandLine(char* line)
{
	if(!line|| strlen(line) == 0)
		return;

	char lineCopy[CMD_LENGTH_MAX];
	strcpy(lineCopy, line);
	
	char* argv[ARGS_NUM_MAX];
	int argc = splitCommand(lineCopy, argv, ARGS_NUM_MAX);

	if(argc == 0)
		return;

	if (strcmp(argv[0], "showpid") == 0)
	{
		if(argc != 1){
			perrorSmash("showpid" , "expected 0 arguments");
			return;
		}

		runShowPid();
		return;
	}
	else if (strcmp(argv[0], "pwd") == 0)
	{
		if(argc != 1){
			perrorSmash("pwd" , "expected 0 arguments");
			return;
		}
		runPwd();
		return;
	}
	else if (strcmp(argv[0], "cd") == 0)
	{
		if (argc != 2)
		{
    		perrorSmash("cd", "expected 1 arguments");
    		return;
		}
		char* target = argv[1];
		char current[CMD_LENGTH_MAX];


		if (strcmp(target, "-") == 0)
		{
			if (oldpwd[0] == '\0')
			{
				perrorSmash("cd", "old pwd not set");
				return;
			}

			printf("%s\n", oldpwd);

			if(getcwd(current,sizeof(current)) == NULL)
			{
				perrorSmash("cd", "getcwd failed");
				return;
			}

			if(chdir(oldpwd) != 0)
			{
				perrorSmash("cd", "target directory does not exist");
        		return;
			}

			strcpy(oldpwd,current);

			return;
		}

		if(strcmp(target, "..") == 0)
		{
			if(getcwd(current,sizeof(current)) == NULL)
			{
				perrorSmash("cd", "getcwd failed");
				return;
			}

			strcpy(oldpwd, current);

			if (chdir("..") != 0)
			{
        		return;
    		}

			return;
		}

		if (getcwd(current, sizeof(current)) == NULL)
		{
			perrorSmash("cd", "getcwd failed");
			return;
		}

		struct stat st;
		if (stat(target, &st) != 0)
		{
			perrorSmash("cd", "target directory does not exist");
			return;
		}

		if (!S_ISDIR(st.st_mode))
		{
			perrorSmash("cd", "not a directory");
			return;
		}

		if (chdir(target) != 0)
		{
			perrorSmash("cd", "failed to change directory");
			return;
		}

		strcpy(oldpwd, current);
		return;
	}
	else
	{
		pid_t pid =fork();

		if( pid < 0 ){
			perrorSmash(argv[0], "fork failed");
			return;
		}

		if (pid == 0){
			execvp(argv[0], argv);
			perrorSmash(argv[0], "execvp failed");
			_exit(1);
		}

    	waitpid(pid, NULL, 0);
		return;

	}

	printf("DEBUG: argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("DEBUG argv[%d] = %s\n", i, argv[i]);
    }
}

int splitCommand(char* line, char* argv[], int maxArgs)
{
	int argc = 0;

	char* token = strtok(line, " \t\n");

	while (token != NULL && argc < maxArgs - 1)
	{
		argv[argc] = token;
		argc++;

		token = strtok(NULL, " \t\n");
	}

	argv[argc] = NULL;
	return argc;
}



//example function for parsing commands
// int parseCmdExample(char* line)
// {
// 	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
// 	char* cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
// 	if(!cmd)
// 		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
// 	char* args[MAX_ARGS];
// 	int nargs = 0;
// 	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
// 	for(int i = 1; i < MAX_ARGS; i++)
// 	{
// 		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
// 		if(!args[i])
// 			break;
// 		nargs++;
// 	}
	/*
	At this point cmd contains the command string and the args array contains
	the arguments. You can return them via struct/class, for example in C:
		typedef struct {
			char* cmd;
			char* args[MAX_ARGS];
		} Command;
	Or maybe something more like this:
		typedef struct {
			bool bg;
			char** args;
			int nargs;
		} CmdArgs;
	*/
//}

