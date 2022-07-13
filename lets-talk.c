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

struct talkArgs {
    List * talker_List;
    int talk_sockfd;
    struct addrinfo *talk_p;
};

pthread_mutex_t lock;
pthread_cond_t cond;
pthread_cond_t cond2;

int button = 1;

void *Keyboard_Input(void * talker_List) {
    char buffer[4800];
    while(button == 1) {
        if(fgets(buffer, sizeof(buffer), stdin)){
            pthread_mutex_lock(&lock);

            List_append(talker_List, (char *)buffer);

            //Notifies Send_Message function of a new message added to the List
            pthread_cond_signal(&cond);

            //Waits to check if !exit command is called or not and to save CPU usage slightly
            pthread_cond_wait(&cond2, &lock);
            pthread_mutex_unlock(&lock);
        }
    } 
    return 0;
}

void *Send_Message(void *args) {
    struct talkArgs *talker_args = args;
    List * talker_List = talker_args->talker_List;
    int talk_sockfd = talker_args->talk_sockfd;
    struct addrinfo *talk_p = talker_args->talk_p;

    char sendMe[4800];
    char checkMe[4800];
    char ch;

    int key = 7;

    while(button == 1){
        pthread_mutex_lock(&lock);

        // Waits for signal from Keyboard_Input function
        pthread_cond_wait(&cond, &lock);

        strcpy(sendMe, List_trim(talker_List));
        strcpy(checkMe, sendMe);
        
        //Encryption
        for(int i = 0; sendMe[i] != '\0'; i++){
            ch = sendMe[i];
            ch += key;
            ch = ch%256; //For char size
            sendMe[i] = ch;
        }

        //Sending through socket
        if ((sendto(talk_sockfd, sendMe, strlen(sendMe), 0, talk_p->ai_addr, talk_p->ai_addrlen)) == -1) {
            printf("ERROR: sending to socket failed");
            exit(1);
        }


        // if(!strcmp(checkMe, "!status")){
        //
        // }

        if(!strcmp(checkMe, "!exit\n")){
            button = 0;
        }

        //Notify Keyboard_Input function that it's done sending message
        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&lock);
    }
    return 0;
}

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

    

    // List * listener_List = List_create();
    List * talker_List = List_create();

    // Need struct to pass multiple arguments in pthread_create function
    struct talkArgs talker_args;
    talker_args.talker_List = talker_List;
    talker_args.talk_sockfd = talk_sockfd;
    talker_args.talk_p = talk_p;

    printf("Welcome to LetS-Talk! Please type your messages now.\n");

    pthread_t keyThread;
    pthread_t sendThread;
    // pthread_t recThread;
    // pthread_t printThread;

    pthread_create(&keyThread, NULL, Keyboard_Input, (void*)talker_List);
    pthread_create(&sendThread, NULL, Send_Message, &talker_args);
    // pthread_create(&recThread, NULL, Keyboard_Input, ???);
    // pthread_create(&printThread, NULL, Keyboard_Input, ???);


    while(button == 1){
        sleep(1);
    }

    pthread_join(keyThread, NULL);
    pthread_join(sendThread, NULL);

    freeaddrinfo(servinfo);
    freeaddrinfo(talk_servinfo);

    // List_free(listener_List, free);
    List_free(talker_List, NULL);


    return 0;
}