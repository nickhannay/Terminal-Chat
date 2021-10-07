#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include <stdbool.h>
#include "list.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h> 
#include <pthread.h>

#define STATUS_RESPONSE_TIME 1
#define MAX_INPUT_BUFFER 5000
#define ENCRYPTION_KEY 50
typedef struct Receive_Handle Receive;
struct Receive_Handle;

// initialized to true and set to false once one thread exits
// allows other threads to safely exit
extern bool THREADS_ALIVE;

void decryptMessage(char* raw_received, int length);

// receiver threads
List* list_receive;
void* receiveMessage(void* args);
void* displayMessage(void* arg);

// sender threads
List* list_send;
void* getInput( void* arg);
void* sendMessage(void* args);

#endif 