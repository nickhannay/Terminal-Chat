#include "client_server.h"

// called when input thread is blocking on fgets()
static void thread_input_cleanup(void* arg) {      
	free((char *) arg);
}

void encryptMessage(char* raw_message, int length){
        for(int i=0; i < length; i++){
                *(raw_message+i) += ENCRYPTION_KEY;
        }

}

void * getInput( void * arg){
        // wait on input from stdin until another thread dies
        while(1) {
                char *input_buffer = malloc(sizeof(char) * MAX_INPUT_BUFFER);
                if (input_buffer == NULL){
                        fprintf(stderr, "Failed to allocate memory for inputted message\n");
                        break;
                }

                pthread_cleanup_push(thread_input_cleanup, input_buffer);
                fgets(input_buffer, MAX_INPUT_BUFFER ,stdin);
                *(input_buffer + strcspn(input_buffer, "\n")) = '\0';
                pthread_cleanup_pop(0);
                if(strcmp(input_buffer, "!status") == 0){
                        // send a status request to another user
                        List_prepend(list_send, input_buffer);
                        char* input_buffer_copy = strdup(input_buffer);
                        // allow display thread to wait for a response
                        List_prepend(list_receive, input_buffer_copy);
                }

                else{
                        // add inputted message to the end of list_send
                        if (List_append(list_send, input_buffer) == -1 ){
                                fprintf(stderr, "Failed adding <%s> to list_send\n", input_buffer);
                                break;  
                        }

                        if(strcmp(input_buffer, "!exit") == 0){
                                // exit thread;
                                break;
                        }
                }
                
        }
        THREADS_ALIVE = false;
        return(NULL);
}

/*
 * This function sends messages from list_send.
 * 
 * 
 */

void * sendMessage(void * args){
        bool not_exit = true;
        char * local_port = *((char **)args + 3);
        char * remote_ip = *((char **)args + 2);
        struct sockaddr_in send_addr;
        int send_socket;

        // zero out sockaddr structs 
        bzero(&send_addr, sizeof(send_addr));
        // create sender socket
        send_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (send_socket == -1){
                perror("Socket");
                THREADS_ALIVE = false;
                return(NULL);
        }
        // initialize send address
        send_addr.sin_family = AF_INET;
        send_addr.sin_port = htons(atoi(local_port));
        send_addr.sin_addr.s_addr = inet_addr(remote_ip);

        while (not_exit){
                // send messages in list_send
                while(List_count(list_send) > 0 ){
                        // move current list pointer to the start of the list (list_append() moves current to last)
                        char * send_buffer =  List_first(list_send);
                        send_buffer = List_remove(list_send);

                        if (strcmp(send_buffer, "internal_!exit") == 0){
                                // !exit sent by other user -> exit thread, but don't send message back.
                                not_exit = false;
                                free(send_buffer);
                                break;
                        }

                        // encrypt message 
                        encryptMessage(send_buffer, strlen(send_buffer));
                       
                        // send message 
                        if (sendto(send_socket, (const char *) send_buffer, strlen(send_buffer) + 1, 0, (const struct sockaddr*) &send_addr, sizeof(send_addr)) == -1){
                                decryptMessage(send_buffer, strlen(send_buffer));
                                fprintf(stderr, "failed to send message: %s\n", send_buffer);
                        }
                                
                        decryptMessage(send_buffer, strlen(send_buffer));
                        // !exit inputted by user -> exit thread
                        if(strcmp(send_buffer, "!exit") == 0){
                                not_exit = false;
                                free(send_buffer);
                                break;
                        }
                        free(send_buffer);
                }
                // don't sleep before exiting
                if(not_exit)
                        sleep(1);

        }
        close(send_socket);
        THREADS_ALIVE = false;
        return(NULL);
}