#include "client_server.h"

typedef struct Receive_Handle Receive;
struct Receive_Handle {
   int * socket;
   char * buf;
};

// handles cancel signal when receiver thread is blocking on recvfrom()
static void thread_receive_cleanup(void* arg) {      
	close(*((Receive*) arg) -> socket);
	free (((Receive*) arg) -> buf);
	free((Receive*) arg);
}

void decryptMessage(char* raw_received, int length){
	for (int i=0; i<length; i++){
		*(raw_received+i) -= ENCRYPTION_KEY;
	}

}


void * receiveMessage(void* args){
	Receive* r_handle = malloc(sizeof(Receive));
	char* local_port = *((char **)args +1);
	char* local_ip = *((char **)args +2);
	struct sockaddr_in serv_addr;
	int server_socket;
	r_handle->socket = &server_socket;

	bzero(&serv_addr, sizeof(serv_addr));
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (server_socket == -1){
		perror("Socket");
		return(NULL);
	}

	// setup socket to recive data
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(local_port));
	serv_addr.sin_addr.s_addr = inet_addr(local_ip); // replace with local ip
	socklen_t len = 0;

	// bind server to port
	int bind_check = bind(server_socket, (const struct sockaddr *) &serv_addr, sizeof(serv_addr) );
	if( bind_check == -1 ){
		perror("Bind");
                free(r_handle);
	        THREADS_ALIVE = false;
		close(server_socket);
		return(NULL);
	}

	pthread_cleanup_push(thread_receive_cleanup, r_handle);
	// receive message
	while (1){
		char * receive_buffer = malloc(sizeof(char)* MAX_INPUT_BUFFER);
		r_handle->buf = receive_buffer;

		// listen to socket for message
		recvfrom(server_socket, receive_buffer, MAX_INPUT_BUFFER , MSG_WAITALL, 0 , &len);
                decryptMessage(receive_buffer, strlen(receive_buffer));
                if(strcmp(receive_buffer, "Online!") == 0){
                        // if other user is responding to status request, pass along message to sleeping display thread
                        List_prepend(list_receive, receive_buffer);
                }
               
                else if( strcmp(receive_buffer, "!status") == 0){
                         // if other user is requesting status info, send "Online!" as next message
			char * tmp = "Online!";
                        char * status = strndup(tmp, strlen(tmp) + 1); 
                        List_prepend(list_send, status);
			free(receive_buffer);                                                              
                }
                else{
                        // add the received message to list_receive
                        if ( List_append(list_receive, receive_buffer) == -1 )
                                fprintf(stderr, "Failed to add <%s> to list_receive\n", receive_buffer);

                        if(strcmp(receive_buffer, "!exit") == 0){
				// notify the send thread to exit
				char * done = strndup("internal_!exit", 15);
				List_append(list_send, done);
                                break;
                        }
                }
	}

	close(server_socket);
	free(r_handle);
	THREADS_ALIVE = false;
	pthread_cleanup_pop(0);
	return NULL;
}


void * displayMessage(void * arg){
	bool not_exit = true;
	while ((not_exit && THREADS_ALIVE) || (List_count(list_receive) > 0) ){

		while(List_count(list_receive) > 0 ){
			// move current to start (append() moves current to last)
			char * display_buffer =  List_first(list_receive);
			display_buffer = List_remove(list_receive);

                        if(strcmp(display_buffer, "!status") == 0){
                               // wait STATUS_RESPONSE_TIME seconds for response
                                sleep(STATUS_RESPONSE_TIME);

                                if ( List_count(list_receive) > 0){
                                        char* status_buffer =  List_first(list_receive);
                                        status_buffer = List_remove(list_receive);

                                        if(strcmp(status_buffer, "Online!") != 0){
                                                // no response received, other messages still left to display.
						// other user has exited but, still more messages in list_receive
                                                printf("The other user is: OFFLINE\n");
                                                printf("received: %s\n", status_buffer);
                                        }
                                        else{
                                                printf("The other user is: ONLINE\n");
                                        }
                                        free(status_buffer);
                                }
                                else{
                                        // no response received, all messages have been displayed
                                        printf("The other user is: OFFLINE\n");
                                }
                        }
			else if (strcmp(display_buffer, "!exit") == 0){
                                printf("-- The other user has exited --\n");
				free(display_buffer);
                                not_exit = false;
                                break;
                        }
                        else {
                                printf("received: %s\n", display_buffer);
                                
                        }
			free(display_buffer);
                        
		}
		// don't sleep before exiting 
		if(not_exit)
			sleep(1);
	}
	THREADS_ALIVE = false;
	return(NULL);
}