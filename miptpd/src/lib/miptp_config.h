#ifndef _MIPTP_CLI_H
#define _MIPTP_CLI_H

#define MIPTPD_HELP_MESSAGE "miptpd [-h] [-d] <mipd app socket> <app socket> <timeout>\n"

typedef struct MIPTPConfig {
    char *mipd_socket; 
    char *app_socket; 
    unsigned short int debug_active; 
    unsigned long timeout;
} MIPTPConfig;

MIPTPConfig *MIPTPConfig_parse_arguments(int argc, char *argv[]);

MIPTPConfig *MIPTPConfig_create(char *mipd_socket, char *app_socket, unsigned int debug_active, unsigned long timeout);

void MIPTPConfig_destroy(MIPTPConfig *config);

#endif