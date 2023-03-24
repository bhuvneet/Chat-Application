/*
	Date:
	Project:
	By:
	Description:
*/

#include "../../common/inc/common.h"
#include "../inc/chat-server.h"
#include <stdio.h>
#include <sys/socket.h>	// for socket programming
#include <netdb.h>		// for gethostbyname
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// to close socket

static int numClients = 0;	// to keep track of total clients connected

int main()
{	
	int server_socket, connected_clients[MAX_CLIENTS];
	struct sockaddr_in server, client;
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";	// for testing
	ConnectedClients activeClient[MAX_CLIENTS];
	messageFromClient message;
	
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
  	if (listen (server_socket, MAX_CLIENTS) < 0) 
  	{
		printf("ERROR: %s\n", hstrerror(errno));
		return ERROR;
  	}
  	
  	while(1)
  	{
  		// accept incoming connections - upto 10 clients
  		if (numClients >= MAX_CLIENTS)
  		{
  			break;
  		}
  		
	  	int client_len = sizeof(struct sockaddr_in);
	  	int new_connection = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&client_len);
	  	if(new_connection < 0)
	  	{
	  		printf("ERROR: %s\n", hstrerror(errno));
	  		close(server_socket);
	  	}
	  	else
	  	{
	  		activeClient->clientIP[numClients] = new_connection;	// add connected client to struct array
	  		numClients++;											// increment the number of clients connected
	  		
	  		// first message sent by connected client contains message like --> "-userID|IPAddress"
	  		// separate the information received and save in the struct array to keep track of connected clients
		  	int readMsg = read(new_connection, buffer, 1024);
		  	printf("received from client: %s\n", buffer);
		  	
		  	char* returnVal = strstr(buffer, "r");
		  	returnVal++;
		  	int i = 0;
		  	while(*returnVal != '|')
		  	{
		  		message.userID[i] = *returnVal;
		  		returnVal++;
		  		i++;
		  	}
		  	message.userID[i] = '\0';	// null terminate the string
		  	
		  	printf("userID: %s\n", message.userID);
	  	}
  	}

  	// client will send their IP address and userID as the first message, once the connection is established
  	
  	
  	
  	
	
	// close socket
	close(server_socket);
	
	return 0;
}
