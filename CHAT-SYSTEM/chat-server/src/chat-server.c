/*
	Date:
	Project:
	By:
	Description:
*/

#define _REENTRANT

#include "../../common/inc/common.h"
#include "../inc/chat-server.h"
#include <stdio.h>
#include <sys/socket.h>	// for socket programming
#include <netdb.h>		// for gethostbyname
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// to close socket
#include <pthread.h>
#include <time.h>		// to calculate current time

static int numClients = 0;	// to keep track of total clients connected
ConnectedClients connected_client[MAX_CLIENTS];

// function prototypes
void *client_handler(void* client_socket);
void broadcast_message(int sender, char* messageToSend);
int removeClientFromArray(int sender);
void formatMessage(char* message, int whichClient, int isSender);
void getCurrentTime(char* whatTime);

int main()
{	
	int server_socket;
	struct sockaddr_in server, client;
	HowMany howMany;
	howMany.howManyClients = 0;
	
	int clientSockets[MAX_CLIENTS];	// data structure holds all sockets
	pthread_t client_threads[MAX_CLIENTS];
	//ClientThreads client_thread;
	
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
  	
  	while(numClients < MAX_CLIENTS)
  	{
  		// accept incoming connections - upto 10 clients
  		
	  	int client_len = sizeof(struct sockaddr_in);
	  	int new_connection = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&client_len);	// accept new client
	  	if(new_connection < 0)
	  	{
	  		printf("ERROR: %s\n", hstrerror(errno));
	  		close(server_socket);
	  	}
	  	else
	  	{
	  		clientSockets[numClients] = new_connection;	// add connected client to list
	  		printf("socket # %d:\n", clientSockets[numClients]);

			if (pthread_create(&client_threads[numClients], NULL, client_handler, (void *)&new_connection))
			{
				printf("ERROR host ID: %s\n", strerror(errno));
				return 0;
			}
			printf("Client #: %d\n", numClients);
			
			numClients++;		// increment the number of clients connected
	  	}

		// TODO stop server when the number of clients is zero
		// this part of the code isn't working
		if (numClients == 0)
		{
			break;	// stop listening to new clients
		}
  	}

	printf("Stop listening");
  	
  	int numOfThreads = sizeof(client_threads) / sizeof(client_threads[0]);

	// wait for all clients to finish
	for(int i = 0; i < numOfThreads; i++)
	{
		pthread_join(client_threads[i], NULL);
	}

	printf("Closing the server");
	
	
	// close socket
	close(server_socket);
	
	return 0;
}

// send and receive messages from clients
void *client_handler(void* client_socket)
{
	int client_sock = *(int *)client_socket;
	char buffer[1024] = {"\0"};
	char* returnVal;
	int readMsg;
	int i = 0;
	
	while ((readMsg = read(client_sock, buffer, 1024)) > 0)
	{
		printf("received from client: %s\n", buffer);
		
		// check if it's the first message sent -- "FIRST|-userID|IPAddress"
		// client will send their IP address and userID as the first message, once the connection is established
		returnVal = strstr(buffer, "FIRST|");
	  	if(returnVal)
	  	{
	  		connected_client[numClients - 1].client_socket = client_sock;	// add client socket to structure
	  		
	  		// first message is sent by client to register it's userID and IP address
	  		returnVal = strstr(buffer, "r");
		  	returnVal++;
		  	i = 0;
		  	while(*returnVal != '|')
		  	{
		  		connected_client[numClients - 1].userID[i] = *returnVal;
		  		returnVal++;
		  		i++;
		  	}
		  	connected_client[numClients - 1].userID[i] = '\0';	// null terminate the string
		  	
		  	returnVal = strrchr(buffer, '|');
		  	returnVal++;
		  	i = 0;
		  	while(*returnVal != '\0')	// while pointer hasn't reached end of string
		  	{
		  		connected_client[numClients - 1].clientIP[i] = *returnVal;
		  		returnVal++;
		  		i++;
		  	}
		  	connected_client[numClients - 1].clientIP[i] = '\0';	// null terminate the string
		  	
	  	}
	  	else if (strcmp(buffer, ">>bye<<") == 0){
	  		
	  		// remove client from array
			int client_removed = removeClientFromArray(client_sock);
	  		if (client_removed == ERROR)
			{
				printf("ERROR: Client not found.\n");
			}
			else
			{
				printf("Client disconnected.\n");
				break;
			}
		}
	  	else
	  	{
	  		// send a reponse to all clients excpet the sender
			// format the message first before invoking this function
			broadcast_message(client_sock, buffer);
	  	}
		
		memset(buffer, 0, 1024);
	}
	
	close(client_sock);	// close client connection
	pthread_exit(NULL);
}

// send message to all clients except sender
void broadcast_message(int sender, char* messageToSend)
{
	char sendMsg[1024];
	
	int senderIndx;
	
	strcpy(sendMsg, messageToSend);	// keep track of the original message sent by sender
	
	// loop to get sender's IP, userID, message to create the formatted message
	for(int i = 0; i < numClients; i++)
	{
		if(connected_client[i].client_socket == sender)
		{			
			formatMessage(messageToSend, i, 1);
			senderIndx = i;	// get sender's index
			
			// send a formatted reponse to sender
			write(connected_client[i].client_socket, messageToSend , strlen(messageToSend));	
		}
	}
	printf("sent: %s\n",messageToSend);
	
	// server has knowledge of all the client's connected to it (their userID, IP ect.)
	formatMessage(sendMsg, senderIndx, 0);
			
	// send formatted message to each client
	for(int i = 0; i < numClients; i++)
	{
		if(connected_client[i].client_socket != sender)
		{			
			// send a formatted reponse to all clients except the sender
			write(connected_client[i].client_socket, sendMsg , strlen(sendMsg));
		}
	}
	printf("sent: %s\n",sendMsg);
}


void formatMessage(char* message, int whichClient, int isSender)
{
	char whatTime[TIME_LEN];
	getCurrentTime(whatTime);
	
	char senderMessage[1024];
	strcpy(senderMessage, message);
	
	// IP, userID and message should sender's which will be sent to all connected clients	
	memset(message, 0, 1024);

	strcpy(message, connected_client[whichClient].clientIP);	// get sender's IP
	strcat(message, " [");	// whitespace
	strcat(message, connected_client[whichClient].userID);		// get sender's ID
	strcat(message, "] ");	// whitespace
	if(isSender)
	{
		strcat(message, ">> ");
	}
	else
	{
		strcat(message, "<< ");
	}
	strcat(message, senderMessage);
	strcat(message, " (");
	strcat(message, whatTime);
	strcat(message, ")");
}

void getCurrentTime(char* whatTime)
{
	time_t currentTime;
	struct tm *timeIs;
	
	time(&currentTime);
	timeIs = localtime(&currentTime);
	
	strftime(whatTime, 9, "%H:%M:%S", timeIs);
}

// remove sender from array and update the number of clients
int removeClientFromArray(int sender)
{
	// flag to determine if the client was found
	int client_found = FALSE; // false	
		
	// get size of array connected_client
	int size = sizeof(connected_client) / sizeof(*connected_client);
	int position;
	int client_removed;


	// search sender in the array connected_client
	for (int i; i < size; i++)
	{
		if (connected_client[i].client_socket == sender)
		{
			client_found = TRUE; // true
			position = i;			// get index of element
			break;
		}	
	}

	// if client is in the list, delete it
	// if not, return error
	if (client_found == TRUE)
	{
		for (int i = position; i < size; i++)
		{
			connected_client[position] = connected_client[position + 1];
		}
		
		numClients--;	// update number of clients

		client_removed = TRUE;	
	}
	else
	{
		client_removed = ERROR;	// sender not in the list
	}

	return client_removed;
}

