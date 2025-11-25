#include "signals.h"
#include <stdio.h>
#include <signal.h>

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
    signal(SIGINT, ctrlCHandler);   // Ctrl+C
    signal(SIGTSTP, ctrlZHandler);
}
