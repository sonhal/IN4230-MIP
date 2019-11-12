#include <stdio.h>  
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "../../../commons/src/dbg.h"

#include "miptp_config.h"


MIPTPConfig *MIPTPConfig_parse_arguments(int argc, char *argv[]){
    unsigned int debug_active = 0;
    char *mipd_socket = NULL;
    char *app_socket = NULL;
    unsigned long timeout = 0;

    int opt = 0;

    // put ':' in the starting of the 
    // string so that program can  
    //distinguish between '?' and ':'  
    while((opt = getopt(argc, argv, "hd")) != -1)  
    {  
        switch(opt)  
        {  
            case 'h':
                printf(MIPTPD_HELP_MESSAGE);
                goto error;
            case 'd':
                debug_active = 1;
                break; 
            case '?':  
                printf(MIPTPD_HELP_MESSAGE);
                goto error;
        }  
    }  
      
    // optind is for the extra arguments 
    // which are not parsed 
    for(; optind < argc; optind++){   
        if(mipd_socket == NULL){
            mipd_socket = argv[optind];
            continue;
        }
        if(app_socket == NULL){
            app_socket = argv[optind];
            continue;            
        }
        sscanf(argv[optind], "%ld", &timeout);
    }
                

    check(mipd_socket != NULL, "Error occurred during parsing mipd_socket is NULL");
    check(app_socket != NULL, "Error occurred during parsing app_socket is NULL");
    MIPTPConfig *config = MIPTPConfig_create(mipd_socket, app_socket, debug_active, timeout);
    check(config != NULL, "Failed to create MIPTPConfig");

    return config;

    error:
        return NULL; 
}


MIPTPConfig *MIPTPConfig_create(char *mipd_socket, char *app_socket, unsigned int debug_active, unsigned long timeout){
    check(strnlen(mipd_socket, 257) < 257, "Invalid argument mipd_socket is to long");
    check(strnlen(app_socket, 257) < 257, "Invalid argument app_socket is to long");
    
    MIPTPConfig *config = calloc(1, sizeof(MIPTPConfig));
    check_mem(config);
    config->mipd_socket = calloc(1, strnlen(mipd_socket, 257));
    check_mem(config->mipd_socket);
    config->app_socket = calloc(1, strnlen(app_socket, 257));
    check_mem(config->app_socket);

    memcpy(config->mipd_socket, mipd_socket, strnlen(mipd_socket, 256));
    memcpy(config->app_socket, app_socket, strnlen(mipd_socket, 256));

    config->debug_active = debug_active;
    config->timeout = timeout;

    return config;
    
    error:
        return NULL;
}

void MIPTPConfig_destroy(MIPTPConfig *config){
    if(config){
        if(config->mipd_socket) free(config->mipd_socket);
        if(config->app_socket) free(config->app_socket);
        free(config);
    }
}