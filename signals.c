#include "signals.h"
#include <stdio.h>
#include <signal.h>
#include "my_system_call.h"

void ctrlCHandler(int sig)
{
    printf("smash: caught CTRL-C\n");
}

void ctrlZHandler(int sig)
{
    printf("smash: caught CTRL-Z\n");
}

void setupSignalHandlers()
{
    my_system_call(SYS_SIGNAL, SIGINT, ctrlCHandler); //CTRL+C
    my_system_call(SYS_SIGNAL, SIGTSTP, ctrlZHandler); //CTRL+Z
}
