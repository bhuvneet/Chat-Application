/*
	Date:
	Project:
	By:
	Description:
*/

#include "../../common/inc/common.h"
#include <stdio.h>
#include <string.h>
#include <ncurses.h>	// for NCURSES library
#include <sys/socket.h>	// for socket programming
#include <netdb.h>		// for gethostbyname
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>		// to close socket
#include <pthread.h>



// move to common file
#define MSG_SIZE	80
#define ERROR		-1		
#define CMD_ERROR	-2
#define HOST_SEARCH_FAIL -3

void *sendMessage(void* socket);
void *recvMessage(void* socket);


int main (int argc, char *argv[])
{
	char userID[21];
	struct hostent *host;
	int endSession 		= 1;
	int msgRecd 		= 0;
	char message[1024];	
	int count 			= 0;
	int server_socket;
	struct sockaddr_in server, addr;		// create sockaddr_in variable to connect to server
	socklen_t len = sizeof(addr);
	char myIPaddress[21];					// to send to server client's IP address as first message'
	pthread_t send_thread, recv_thread;		// threads to send and receive messages
	
	
	// check command line args
	if(argc != 3)
	{
		printf("USAGE: <chat-client> -user<userID> -server<serverName>\n");
		return CMD_ERROR;
	}
	
	strcpy(userID,argv[1]);	// user ID of the client connected
	
	printf("%s\n", argv[0]);
	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);
	
	char* returnVal = strstr(argv[2], "-server");	// point to the root folder of the project
	//printf("%d\n", returnVal);	// returns 0 
	
	char* hostID = strrchr(returnVal, 'r');
	
	// point to last occurence of r in "server" - this is where IP address or server's name is passed as cmd arg
	hostID++;
	printf("%s\n", hostID);
	
	// get host name passed as command line argument
	if((host = gethostbyname(hostID)) == NULL)
	{
		printf("ERROR host ID: %s\n", strerror(errno));
		return HOST_SEARCH_FAIL;
	}
	
	// create socket
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)	// initialize socket using Address Family (IP version 4), Socket Stream (TCP protocol), and Protocol 0 (IP protocol).
	{
		printf("ERROR: %s\n", strerror(errno));
	}
	
	// connect socket to server socket using IP address and port number
	memset (&server, 0, sizeof (server));
	server.sin_family = AF_INET;
	memcpy (&server.sin_addr, host->h_addr, host->h_length); // copy the host's internal IP addr into the server_addr struct
	server.sin_port = htons(PORT);
	
	// connect to remote host
	if(connect(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("ERROR: %s\n", strerror(errno));
	}
	else
	{
		printf("connected with server!\n");
		
		// send first message to server for userID and IP address
		getsockname(server_socket, (struct sockaddr *)&addr, &len);		// send server IP address of this client
		
		strcpy(myIPaddress, inet_ntoa(addr.sin_addr));	// get client's IP'
		
		strcpy(message, "FIRST|");
		strcat(message, userID);
		strcat(message, "|");
		strcat(message, myIPaddress);
		
		write (server_socket, message, strlen (message));
		memset(message, 0, 1024);

		// initializes the curses system and alloacte memory for window
		//initscr();
		
		void* arg;
		// launch sending thread to send messages to server
		if(pthread_create(&send_thread, NULL, sendMessage, (void *)&server_socket))
		{
			printf("ERROR host ID: %s\n", strerror(errno));
			return 0;
		}	
		
		if(pthread_create(&recv_thread, NULL, recvMessage, (void *)&server_socket))
		{
			printf("ERROR host ID: %s\n", strerror(errno));
			return 0;
		}		
		
		// wait for threads to finish
		pthread_join(send_thread, NULL);
		pthread_join(recv_thread, NULL);					

		
		//endwin();							// End curses mode - this will free memory taken by ncurses sub0system and its data structures and put terminal back in normal mode.
		
		// create one thread to send outgoing message
		
		// create another thread to read from socket
		
		// close client socket
		close(server_socket);
	}	
	
	return 0;
	
}

void *sendMessage(void* socket)
{
	char message[1024];		// use constant
	int endSession = 1;
	int server_socket = *((int*)socket);
	
	while(endSession)
	{
		fflush(stdout);
		
		//refresh();							// print it on to the real screen
		
		// read input from terminal
		if(fgets(message, 1024, stdin) != NULL)
		{
			if (message[strlen (message) - 1] == '\n') message[strlen (message) - 1] = '\0';	// remove new line character
			
			int len = strlen(message);
			printf("%d\n", len);

			if(len == 80)
			{
				// send message to server
				write (server_socket, message, strlen (message));
			}
			else if(strcmp(message,"<<bye>>") == 0) // check if the user wants to quit
			{
				// send final message to server
				write (server_socket, message, strlen (message));
				endSession = 0;
			}
			else
			{
				// send message to server
				send (server_socket, message, strlen (message), 0);
			}

			memset(message, 0, 1024);
		}			
		
	}
	
	pthread_exit(NULL);

}


void *recvMessage(void* socket)
{
	char message[1024];
	int readMsg;
	int server_socket = *((int*)socket);
	
	while(1)
	{
		readMsg = read(server_socket, message, 1024);
		
		if(readMsg > 0)
		{
			printf("Message RCD: %s\n", message);
		}
		else if(readMsg == 0)
		{
			printf("Server disconnected\n");
			break;
		}
	}
	
	pthread_exit(NULL);
}
