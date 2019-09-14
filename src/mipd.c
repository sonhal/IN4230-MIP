#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "dbg.h"

int main(int argc, char *argv[]){

    pid_t pid = getpid();
    log_info("Lets go! pid: %d", pid);
}