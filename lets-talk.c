#include "client_server.h"

bool THREADS_ALIVE = true;

    
int main(int argc, char* argv[]){
	list_send = List_create();
	list_receive = List_create();
	if(argc < 4 ){
		fprintf(stderr, "usage: <local port> <receive ip address> <receive port>\n");
		return(1);
	}
	// allow user to use localhost as ip
	if(strcmp(argv[2], "localhost") == 0){
		argv[2] = "127.0.0.1";
	}

	// thread to receive input from keyboard
	pthread_t thread_input;
	pthread_create(&thread_input, NULL, getInput, argv);

	// thread to send messages
	pthread_t thread_send;
	pthread_create(&thread_send, NULL, sendMessage, argv);

	// thread to receive messages from socket
	pthread_t thread_receive;
	pthread_create(&thread_receive, NULL,receiveMessage, argv);

	// thread to display received messages 
	pthread_t thread_display;
	pthread_create(&thread_display, NULL, displayMessage, argv);

	// wait for threads to finish
	pthread_join(thread_send,NULL);
	pthread_join(thread_display,NULL);
	pthread_cancel(thread_input);
	pthread_join(thread_input,NULL);
	pthread_cancel(thread_receive);
	pthread_join(thread_receive,NULL);

	List_free(list_receive, NULL);
	List_free(list_send, NULL);

	printf("-- connection terminated --\n");
	return 0;

}
