#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "list.h"

struct talkArgs {
    List * talker_List;
    int talk_sockfd;
    struct addrinfo *talk_p;
};

struct receiverArgs {
    int rec_sockfd;
	// char* myPortNumber;
	// char* remoteMachineName;
	// char* remotePortNumber;
    struct addrinfo *rec_p;
	List* receiver_List;
    struct sockaddr_storage their_addr;
	// pthread_mutex_t* bufferMutexPtr;
};

pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_cond_t cond;
pthread_cond_t cond2;
pthread_cond_t cond3;

int button = 1;

void *Keyboard_Input(void * talker_List) {
    char buffer[4800];
    while(button == 1) {
        if(fgets(buffer, sizeof(buffer), stdin)){
            pthread_mutex_lock(&lock);
            List_append(talker_List, (char *)buffer);
            fflush(stdin);
            
            //Notifies Send_Message function of a new message added to the List
            pthread_cond_signal(&cond);

            //Waits to check if !exit command is called or not and to save CPU usage slightly
            pthread_cond_wait(&cond2, &lock);
            pthread_mutex_unlock(&lock);
        }
    } 
    // printf("test key exit\n");
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
            //5,6,7
            //12,13,14
        }

        //Sending through socket
        if ((sendto(talk_sockfd, sendMe, strlen(sendMe), 0, talk_p->ai_addr, talk_p->ai_addrlen)) == -1) {
            printf("ERROR: sending to socket failed");
            exit(1);
        }

        if(!strcmp(checkMe, "!exit\n")){
            button = 0;
        }

        //Notify Keyboard_Input function that it's done sending message
        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&lock);
    }
    // printf("test send exit\n");
    return 0;
}

void *Rec_Message(void *args) {
    //initializations/assignments
    struct receiverArgs* receiver_args = args;
    List * receiver_List = receiver_args->receiver_List;
    int rec_socketfd = receiver_args->rec_sockfd;
    struct sockaddr_storage their_addr = receiver_args->their_addr;
    socklen_t address_len;
    int len = 0;
    char receiveMe[4800];
    char sendMe[4800];
    char ch;
    char temp;
    int key = 7;
    
    while(button == 1){
        pthread_mutex_lock(&lock2);
        address_len = sizeof(their_addr);

        if((len = recvfrom(rec_socketfd, receiveMe, sizeof(receiveMe), 0, (struct sockaddr *)&their_addr, &address_len)) == -1){
            printf("ERROR: receiving from socket failed");
            exit(1);
        }
        receiveMe[len] = '\0';
        for(int i = 0; receiveMe[i] != '\0'; i++){
            temp = receiveMe[i];
            ch = 256 - ((temp - key) * -1);
            if(ch > 256){
                ch = ch % 256;
            }
            ch = ch%256; //For char size
            receiveMe[i] = ch;
        }

        if(!strcmp(receiveMe, "!exit\n")){
            button = 0;
        }

        strcpy(sendMe, receiveMe);
        List_append(receiver_List, (char *)sendMe);
        pthread_cond_signal(&cond3);
        pthread_mutex_unlock(&lock2);
    }

    // printf("test rec exit\n");
    return 0;
}

void *Print_Message(void *print_list) {
    char printMe[4800];
    while(button == 1){
        pthread_mutex_lock(&lock3);
        pthread_cond_wait(&cond3, &lock3);

        while(List_count(print_list) > 0){
            strcpy(printMe, List_trim(print_list));
            printf("%s", printMe);
        }

        pthread_mutex_unlock(&lock3);
    }
    // printf("test print exit\n");
    return 0;
}


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
    struct sockaddr_storage their_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
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
            close(sockfd);
        }

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
    

    memset(&talk_hints, 0, sizeof talk_hints);
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

    
    List * talker_List = List_create();
    List * receiver_List = List_create();

    // Need struct to pass multiple arguments in pthread_create function
    struct talkArgs talker_args;
    talker_args.talker_List = talker_List;
    talker_args.talk_sockfd = talk_sockfd;
    talker_args.talk_p = talk_p;

    struct receiverArgs receiver_args;
    receiver_args.receiver_List = receiver_List;
    receiver_args.rec_sockfd = sockfd;
    receiver_args.rec_p = p;
    receiver_args.their_addr = their_addr;

    printf("Welcome to LetS-Talk! Please type your messages now.\n");

    pthread_t keyThread;
    pthread_t sendThread;
    pthread_t recThread;
    pthread_t printThread;

    pthread_create(&keyThread, NULL, Keyboard_Input, (void*)talker_List);
    pthread_create(&sendThread, NULL, Send_Message, &talker_args);
    pthread_create(&recThread, NULL, Rec_Message, &receiver_args);
    pthread_create(&printThread, NULL, Print_Message, (void*)receiver_List);

    pthread_join(keyThread, NULL);
    pthread_join(sendThread, NULL);
    pthread_join(recThread, NULL);
    pthread_join(printThread, NULL);

    freeaddrinfo(servinfo);
    freeaddrinfo(talk_servinfo);

    List_free(receiver_List, NULL);
    List_free(talker_List, NULL);


    return 0;
}