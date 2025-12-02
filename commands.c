//commands.c
#include "commands.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "my_system_call.h"
#include <signal.h>


static char oldpwd[CMD_LENGTH_MAX] = {0};
static Job jobs[JOBS_NUM_MAX];

static int jobs_count = 0;
//static int next_job_id = 1;

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
		if (argc != 2) //why written 2 and not 1?
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
//
		strcpy(oldpwd, current);
		return;
	}
	else if (strcmp(argv[0], "jobs") == 0)
	{
		if (argc != 1) {
			perrorSmash("jobs", "expected 0 arguments");
			return;
		}
		printJobs();
		return;
	}
	else if (strcmp(argv[0], "kill") == 0)
	{
		runKill(argc, argv);
		return;
	}
	else if (strcmp(argv[0], "fg") == 0)
	{
		runFg(argc, argv);
		return;
	}
	else
	{
		int isBackground = 0;
		if(argc > 0 && strcmp(argv[argc-1], "&") == 0){
		isBackground = 1;
		argv[argc-1] = NULL;
		argc--;
		}

		pid_t pid = my_system_call(SYS_FORK);     //pid_t pid = fork();
		if(pid < 0 ){
			perrorSmash(argv[0], "fork failed");
			return;
		}

		if (pid == 0){
			my_system_call(SYS_EXECVP, argv[0], argv); //execvp(argv[0], argv);
			perrorSmash(argv[0], "execvp failed");
			_exit(1);
		}
		if(isBackground){
            /* build command string from argv (without trailing &) and add job */
            char cmd[CMD_LENGTH_MAX] = {0};
            for (int ai = 0; ai < argc; ai++) {
                if (argv[ai] == NULL) break;
                if (ai > 0) {
                    strncat(cmd, " ", CMD_LENGTH_MAX - strlen(cmd) - 1);
                }
                strncat(cmd, argv[ai], CMD_LENGTH_MAX - strlen(cmd) - 1);
            }
            addJob(pid, cmd, 0);
            printf("smash: process %d started in background\n", pid);
            return;
         }
		else
		{
			my_system_call(SYS_WAITPID, pid, NULL, 0); //waitpid(pid, NULL, 0);
			return;
		}
		//maybe need return
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




int findSmallestJobId(){
	for(int id = 0; id < JOBS_NUM_MAX; id++)
	{
		if(jobs[id].job_id == 0)
		{
			return id+1;
		}
	}
	return JOBS_NUM_MAX+1;
}


void addJob(pid_t pid, const char* cmd, int stopped)
{
	if(jobs_count >= JOBS_NUM_MAX){
		perrorSmash("jobs", "jobs list is full");
		return;
	}
	Job* job = &jobs[findSmallestJobId()-1];
	job->job_id = findSmallestJobId();
	job->pid = pid;
	job->stopped = stopped;
	strcpy(job->cmd, cmd /* CMD_LENGTH_MAX-1*/);
	//job->cmd[CMD_LENGTH_MAX-1] = '\0';
	jobs_count++;
}



void printJobs(){
	for(int id = 0; id < JOBS_NUM_MAX; id++){
		if(jobs[id].job_id != 0)
		{
			printf("[%d] %s : %d %s\n",
					jobs[id].job_id,
					jobs[id].cmd,
					jobs[id].pid,	
					jobs[id].stopped ? "Stopped": "Running");

		}
	}
}


void runKill(int argc, char* argv[])
{
	if (argc != 3) {
		perrorSmash("kill", "invalid arguments");
		return;
	}

	int signum = atoi(argv[1]);
	int job_id = atoi(argv[2]);

	// Validate that both arguments are valid numbers
	if ((signum == 0 && strcmp(argv[1], "0") != 0) || 
		(job_id == 0 && strcmp(argv[2], "0") != 0)) {
		perrorSmash("kill", "invalid arguments");
		return;
	}

	// Find the job with the given job_id
	Job* target_job = NULL;
	for (int i = 0; i < JOBS_NUM_MAX; i++) {
		if (jobs[i].job_id == job_id) {
			target_job = &jobs[i];
			break;
		}
	}

	if (target_job == NULL) {
		fprintf(stderr, "smash error: kill: job id %d does not exist\n", job_id);
		return;
	}

	// Send signal to the process
	if (my_system_call(SYS_KILL, target_job->pid, signum) < 0) {
		perrorSmash("kill", "failed to send signal"); // not sure if needed
		return;
	}

	printf("signal %d was sent to pid %d\n", signum, target_job->pid);
}
void runFg(int argc, char* argv[])
{
	if (argc != 2) {
		perrorSmash("fg", "invalid arguments");
		return;
	}

	int job_id = atoi(argv[1]);
	
	if (job_id == 0 && strcmp(argv[1], "0") != 0) {
		perrorSmash("fg", "invalid arguments");
		return;
	}

	Job* target_job = NULL;
	for (int i = 0; i < JOBS_NUM_MAX; i++) {
		if (jobs[i].job_id == job_id) {
			target_job = &jobs[i];
			break;
		}
	}

	if (target_job == NULL) {
		fprintf(stderr, "smash error: fg: job id %d does not exist\n", job_id);
		return;
	}

	if (my_system_call(SYS_KILL, target_job->pid, SIGCONT) < 0) {
		perrorSmash("fg", "failed to send signal");
		return;
	}

	target_job->stopped = 0;
	my_system_call(SYS_WAITPID, target_job->pid, NULL, 0);
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

