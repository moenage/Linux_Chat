#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include "list.h"

// void *Keyboard_Input(void *threadid) {

// }

// void *Send_Message(void *threadid) {

// }

// void *Rec_Message(void *threadid) {

// }

// void *Print_Message(void *threadid) {

// }



int main(int argc, char ** argv) {

    // Not enough arguments checker
    if(argc != 4){
        printf("ERROR: Did not enter [my port number] [remote/local machine IP] [remote/local port number] correctly");
        return -1;
    }

    printf("loooooooooooooooool\n");
    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);
    printf("%s\n", argv[2]);
    printf("%s\n", argv[3]);


    // ***
    // Listener Socket
    // ***

    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        printf("ERROR: in getaddrinfo %s\n", gai_strerror(status));
        freeaddrinfo(servinfo);
        return -1;
    }

    for(p = servinfo;p != NULL; p = p->ai_next) {
        int check = -1;

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if(sockfd != -1){
            check = bind(sockfd, p->ai_addr, p->ai_addrlen);
            if(check != -1){
                break;
            }
        }
        close(sockfd);

    }

    if (p == NULL) {
        printf("ERROR: failed to bind socket\n");
        return -1;
    }

    // ***
    // Talker Socket
    // ***

    int talk_sockfd;
    struct addrinfo talk_hints, *talk_servinfo, *talk_p;
    int talk_status;

    talk_hints.ai_family = AF_INET; // set to AF_INET6 to use IPv6
    talk_hints.ai_socktype = SOCK_DGRAM;

    if ((talk_status = getaddrinfo(argv[2], argv[3], &talk_hints, &talk_servinfo)) != 0) {
        printf("ERROR: getaddrinfo %s\n", gai_strerror(talk_status));
        return -1;
    }

    for(talk_p = talk_servinfo;talk_p != NULL; talk_p = talk_p->ai_next) {

        talk_sockfd = socket(talk_p->ai_family, talk_p->ai_socktype, talk_p->ai_protocol);

        if(talk_sockfd != -1){
            break;
        }
        else{
            close(talk_sockfd);
        }

    }

    if (talk_p == NULL) {
        printf("ERROR: talker failed to create socket\n");
        return -1;
    }




    // ***
    //Threads
    // ***

    // pthread_t keyThread, sendThread, recThread, printThread;

    // pthread_create(&keyThread, NULL, Keyboard_Input, ???);
    // pthread_create(&sendThread, NULL, Keyboard_Input, ???);
    // pthread_create(&recThread, NULL, Keyboard_Input, ???);
    // pthread_create(&printThread, NULL, Keyboard_Input, ???);



    freeaddrinfo(servinfo);
    freeaddrinfo(talk_servinfo);


    return 0;
}