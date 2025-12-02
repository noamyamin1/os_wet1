#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100

typedef struct Job{     
    int job_id;
    pid_t pid;
    char cmd[CMD_LENGTH_MAX];
    int stopped;
} Job;



/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        fprintf(stderr, "%s: %d\n%s", __FILE__, __LINE__, msg); \
        exit(1); \
    } while(0);

static inline void* _validatedMalloc(size_t size)
{
    void* ptr = malloc(size);
    if(!ptr) ERROR_EXIT("malloc");
    return ptr;
}

// example usage:
// char* bufffer = MALLOC_VALIDATED(char, MAX_LINE_SIZE);
// which automatically includes error handling
#define MALLOC_VALIDATED(type, size) \
    ((type*)_validatedMalloc((size)))


/*=============================================================================
* error definitions
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 0,
	//feel free to add more values here or delete this
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;


/*=============================================================================
* global functions
=============================================================================*/
int parseCommandExample(char* line);
void executeCommandLine(char* line);
int splitCommand(char* line, char* argv[], int maxArgs);
char* getcwd(char* buf, size_t size);
void addJob(pid_t pid, const char* cmd, int stopped);
void printJobs(void);
void runKill(int argc, char* argv[]);
void runFg(int argc, char* argv[]);

#endif //COMMANDS_H