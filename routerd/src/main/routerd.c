#include <stdio.h>


#include "../lib/cli.h"

int main(int argc, char *argv[]){
    printf("Routing up!\n");

    //get mipd communication socket
    //get remote routerd communication socket
    RouterdConfig *config = parse_args(argc, argv);
    if(config == NULL) return 0;




    RouterdConfig_destroy(config);
    return 0;
}