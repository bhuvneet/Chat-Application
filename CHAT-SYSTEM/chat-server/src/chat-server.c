/*
	Date:
	Project:
	By:
	Description:
*/

#include <stdio.h>
#include <sys/socket.h>	// for socket programming
#include <netdb.h>		// for gethostbyname
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// to close socket

// move to common file
#define MSG_SIZE	80
#define ERROR		-1		
#define PORT 		5000

int main()
{
	printf("Hello from chat-server.c\n");
	
	int server_socket;
	struct sockaddr_in server, client;
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";	// for testing
	
	// create socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket == -1)
	{
		printf("ERROR: %s\n", hstrerror(errno));
	}
	
	// initialize socket
	memset (&server, 0, sizeof (server));
  	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = htonl (INADDR_ANY);
  	server.sin_port = htons (PORT);
  	
  	// bind socket
  	if(bind(server_socket,(struct sockaddr *)&server, sizeof(server)) < 0)
  	{
  		printf("ERROR: %s\n", hstrerror(errno));
  	}
  	
  	// listen for connections
  	if (listen (server_socket, 5) < 0) 
  	{
		printf("ERROR: %s\n", hstrerror(errno));
		return ERROR;
  	}
  	
  	// accept incoming connections - upto 10 clients
  	int client_len = sizeof(struct sockaddr_in);
  	int new_connection = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&client_len);
  	if(new_connection < 0)
  	{
  		printf("ERROR: %s\n", hstrerror(errno));
  		close(server_socket);
  	}
  	
  	// client will send their IP address and userID as the first message, once the connection is established
  	
  	// read message sent by client
  	int readMsg = read(new_connection, buffer, 1024);
  	printf("received from client: %s\n", buffer);
  	
  	// send a test message to client
  	write(new_connection, hello, strlen(hello));
	
	// close socket
	close(server_socket);
	
	return 0;
}
