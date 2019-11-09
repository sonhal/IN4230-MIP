#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <ifaddrs.h>		/* getifaddrs */
#include <unistd.h>
#include <sys/un.h>		/* struct sockaddr_un */
#include <errno.h>

#include "../../../commons/src/dbg.h"
#include "../../../commons/src/time_commons.h"
#include "../../../commons/src/client_package.h"


#define TPCLIENT_HELP_MESSAGE "tpclient [-h] [-d] <miptpd socket> <mip address> <port> <file>"


int main(int argc, char *argv[]){
    printf("%s\n", TPCLIENT_HELP_MESSAGE);
    struct sockaddr_un so_name;
    int rc = 0;
    int so = 0;
    char *data = "After school, Kamal took the girls to the old house. It was very old and very dirty too. There was rubbish everywhere. The windows were broken and the walls were damp. It was scary. Amy didn’t like it. There were paintings of zombies and skeletons on the walls. “We’re going to take photos for the school art competition,” said Kamal. Amy didn’t like it but she didn’t say anything. “Where’s Grant?” asked Tara. “Er, he’s buying more paint.” Kamal looked away quickly. Tara thought he looked suspicious. “It’s getting dark, can we go now?” said Amy. She didn’t like zombies. Then, they heard a loud noise coming from a cupboard in the corner of the room. “What’s that?” Amy was frightened. “I didn’t hear anything,” said Kamal. Something was making strange noises. There was something inside the cupboard. “Oh no! What is it?” Amy was very frightened now. “What do you mean? There’s nothing there!” Kamal was trying not to smile. Suddenly the door opened with a bang and a zombie appeared, shouting and moving its arms. Amy screamed and covered her eyes. “Oh very funny, Grant!” said Tara looking bored. Kamal and Grant started laughing. “Ha ha, you were frightened!” said Grant. Do you like my zombie costume?” Amy took Tara’s arm. “I can’t breathe,” she said. Kamal looked worried now. “Is she OK? It was only a joke.” “No she’s not OK, you idiot. She’s having an asthma attack and she forgot her inhaler.” Tara took out her phone. “I’m calling her dad. Next evening was Halloween. The girls were at the school disco. “Are you better now?” asked Tara. “I’m fine,” said Amy. “I think it was the smell of paint that started the attack.” Tara looked around. “So, where are the zombies?” “I don’t know, I don’t want to see Kamal again,” said Amy. “Come on, let’s dance!”";
    ClientPackage *package = ClientPackage_create(10, 110, data, strlen(data));
    check(package != NULL, "Failed to create ClientPackage");
  
    /* Create socket */
    so = socket(AF_UNIX, SOCK_STREAM, 0);
    if (so == -1) {
    perror("socket");
    return 1;
    }
    // Set timeout on socket
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(so, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    /* Zero out name struct */
    memset(&so_name, 0, sizeof(struct sockaddr_un));

    /* Prepare UNIX socket name */
    so_name.sun_family = AF_UNIX;
    strncpy(so_name.sun_path, argv[1], sizeof(so_name.sun_path) - 1);

    rc = connect(so, (const struct sockaddr*)&so_name, sizeof(struct sockaddr_un));
    check(rc != -1, "[TPCLIENT] Failed to connect to domain socket: %s",so_name.sun_path);
    printf("[TPCLIENT] port: %d\tmip destination: %d \n", package->port, package->destination);

    BYTE s_package[2000];
    rc = ClientPackage_serialize(&s_package, package);
 
    // Write to mipd 
    long before = get_now_milli();
    rc = write(so, &s_package, rc);

    BYTE response[2000];
    rc = recv(so, &response, 10, 0);
    long after = get_now_milli();

    if(rc == -1 && errno == EAGAIN){
        printf("[MIPTP CLIENT] Timeout\t time elapsed: %ld\n",after - before);
    }else {
        check(rc != -1, "Failed to read response from mipd");
        printf("[MIPTP CLIENT] time elapsed: %ld\n", after - before);
    }

    ClientPackage_destroy(package);
    if(so)close(so);
    return 0;
    
    error:
        log_err("[MIPTP CLIENT] Failed unexpectedly");
        return 1;
}