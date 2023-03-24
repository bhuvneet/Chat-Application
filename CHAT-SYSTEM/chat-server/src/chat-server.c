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
	char* returnVal;
	char* hello = "Hello from server";	// for testing
	int client_sockets[MAX_CLIENTS];
	MessageFromClient message;
	int i = 0;
	int readMsg;
	
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
	  	int new_connection = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&client_len);	// accept new client
	  	if(new_connection < 0)
	  	{
	  		printf("ERROR: %s\n", hstrerror(errno));
	  		close(server_socket);
	  	}
	  	else
	  	{
	  		client_sockets[numClients] = new_connection;	// add connected client to struct array	-- STORE SOCKETS
	  		printf("%d:\n", client_sockets[numClients]);
	  		numClients++;											// increment the number of clients connected

			while(numClients > 0)
			{
				// read message received from client(s)
			  	readMsg = read(new_connection, buffer, 1024);		// each client needs it own socket to able to send messages to server, this can be achieved using threads
			  	printf("received from client: %s\n", buffer);
			  	
			  	// check if it's the first message sent -- "FIRST|-userID|IPAddress"
			  	// client will send their IP address and userID as the first message, once the connection is established
			  	returnVal = strstr(buffer, "FIRST|");
			  	if(returnVal)
			  	{
			  		// first message is sent by client to register it's userID and IP address
			  		returnVal = strstr(buffer, "r");
				  	returnVal++;
				  	i = 0;
				  	while(*returnVal != '|')
				  	{
				  		message.userID[i] = *returnVal;
				  		returnVal++;
				  		i++;
				  	}
				  	message.userID[i] = '\0';	// null terminate the string
				  	
				  	printf("userID: %s\n", message.userID);
				  	
				  	returnVal = strrchr(buffer, '|');
				  	returnVal++;
				  	i = 0;
				  	while(*returnVal != '\0')	// while pointer hasn't reached end of string
				  	{
				  		message.ip.clientIP[i] = *returnVal;
				  		returnVal++;
				  		i++;
				  	}
				  	message.ip.clientIP[i] = '\0';	// null terminate the string
				  	printf("IP: %s\n", message.ip.clientIP);
			  	}
			  	
			  	// if message received is <<bye>>, remove client from array	  	
			  	
			  	memset(buffer,0,1024);
			  	
			  	// send message to clients
			  	write (new_connection, buffer, strlen(buffer));
			}
	  		
		  	
		  	while(numClients > 0)
	  		{
	  			// as long as there are at least 1 client connected
	  			// receive messages from connected clients
  				// send response to all clients except the one sending the message
	  		}
	  	}
  	}

  	
  	
  	
	// remove all client sockets
	
	// close socket
	close(server_socket);
	
	return 0;
}
