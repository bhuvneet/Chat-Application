/*
	Date:			April 4, 2023
	Project:		The Can We Talk System? - Assignment 4
	File:			chat-server.c
	By:				Bhuvneet Thakur, Maisa Wolff Resplande
	Description:	This file conatins the logic for the Server code. This file contains the functions related to the Server side of the assignment.
*/

#define _REENTRANT

#include "../inc/chat-server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>	// for socket programming
#include <netdb.h>		// for gethostbyname
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// to close socket
#include <pthread.h>
#include <time.h>		// to calculate current time
#include <signal.h>



static int numClients 	= 0;	// to keep track of total clients connected
static int keepRunning 	= 1;

ConnectedClients connected_client[MAX_CLIENTS];
Threads client_thread[MAX_CLIENTS];
Sockets sockets;

// function prototypes
void *client_handler(void* client_socket);
void broadcast_message(int sender, char* messageToSend);
int removeClientFromArray(int sender);
void formatMessage(char* message, int whichClient, int isSender);
void getCurrentTime(char* whatTime);
void shutdown_signal(int client_socket);
void kill_signal(int signal_number);

int main(void)
{	
	signal (SIGKILL, kill_signal);	// register the KILL command
	
	struct sockaddr_in server, client;
	
	// create socket
	sockets.server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(sockets.server_socket == ERROR)
	{
		printf("ERROR: %s\n", hstrerror(errno));
		return ERROR;
	}
	
	// initialize socket
	memset (&server, 0, sizeof (server));
  	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = htonl (INADDR_ANY);
  	server.sin_port = htons (PORT);
  	
  	// bind socket
  	if(bind(sockets.server_socket,(struct sockaddr *)&server, sizeof(server)) < 0)
  	{
  		printf("ERROR: %s\n", hstrerror(errno));
		return ERROR;
  	}
  	
  	// listen for connections
  	if (listen (sockets.server_socket, MAX_CLIENTS) < 0) 
  	{
		printf("ERROR: %s\n", hstrerror(errno));
		return ERROR;
  	}
    
  	while(keepRunning)
  	{	
  		if (numClients == MAX_CLIENTS)
  		{
  			break;
  		}
  		
	  	int client_len = sizeof(struct sockaddr_in);
	  	int new_connection = accept(sockets.server_socket, (struct sockaddr *)&client, (socklen_t*)&client_len);	// accept new client
	  	if(new_connection < 0)
	  	{
	  		printf("ERROR: %s\n", hstrerror(errno));
	  		close(sockets.server_socket);
	  		keepRunning = 0; // break loop when accept fails
	  	}
	  	else
	  	{
	  		sockets.clientSockets[numClients] = new_connection;	// add connected client to list

			numClients++;		// increment the number of clients connected	
			
			if (keepRunning != 0)
			{
				if (pthread_create(&client_thread[numClients].client_threads, NULL, client_handler, (void *)&new_connection))
				{
					printf("ERROR host ID: %s\n", strerror(errno));
					return ERROR;	
				}	
			}		
	  	}
  	}
	
	return 0;
}

/*
	Function:		kill_signal()
	Parameters:		int signal_number
	Output:			NONE
	Return value:	NONE	
	Description:	This function is invoked when the server is killed by the user. This function invoked using Signals for SIGKILL command.
					It ensures to close all client sockets, join all threads and close the server socket
*/
void kill_signal(int signal_number)
{

	for(int i = 0; i < numClients; i++)
	{
		close(sockets.clientSockets[i]); // close client connection
	}

	// join all threads
	int numOfThreads = sizeof(client_thread[numClients].client_threads) / sizeof(client_thread[0].client_threads);

	// wait for all clients to finish
	for(int i = 0; i < numOfThreads; i++)
	{
		pthread_join(client_thread[numClients].client_threads, NULL);
	}

	// close server socket
	close(sockets.server_socket);

	exit(-1);
}

/*
	Function:		shutdown_signal()
	Parameters:		int client_socket
	Output:			NONE
	Return value:	NONE	
	Description:	This function is invoked each time a client is disconnected and removed from the array.
					This function closes the client socket and if the server is to be shut down, it ensures to join all threads and closes the server socket.
					This function checks if the server needs to be shut down as there are no more clients connected.
					Hence, by implementing the logic in this function, a graceful shutdown of server
*/
void shutdown_signal(int client_socket)
{

	close(client_socket);	// close client connection
				
	// keepRunning is being set to 0 in removeClientFromArray function
	if((keepRunning == 0) && (numClients == 0))
	{
		// join all threads
		int numOfThreads = sizeof(client_thread[numClients].client_threads) / sizeof(client_thread[0].client_threads);
		
		// wait for all clients to finish
		for(int i = 0; i < numOfThreads; i++)
		{
			pthread_join(client_thread[numClients].client_threads, NULL);
		}

		// close server socket
		close(sockets.server_socket);
		
		exit(-1);
	}
}

/*
	Function:		client_handler()
	Parameters:		void* client_socket
	Output:			NONE
	Return value:	void*	
	Description:	This function is invoked each time a client is connected to the server and a new thread is created for each client.
					This function reads the message from the clients, checks if its ">>bye<<" and broadcasts the message to other clients by calling 
					the broadcast_message().
*/
void *client_handler(void* client_socket)
{
	int client_sock = *(int *)client_socket;
	char buffer[BUFF_SIZE] = {"\0"};
	char* returnVal;
	int readMsg;
	int i = 0;
	
	while (numClients > 0)
	{
		while ((readMsg = read(client_sock, buffer, BUFF_SIZE)) > 0)
		{
			
			if (strcmp(buffer, ">>bye<<") == 0)
		  	{	
		  		// remove client from array
				int client_removed = removeClientFromArray(client_sock);
		  		if (client_removed == ERROR)
				{
					return (void*)ERROR;
				}
				
				shutdown_signal(client_sock);	// close socket, join threads and shutdown server
				
				break;
			}
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
		  	else
		  	{
		  		// send a reponse to all clients excpet the sender
				// format the message first before invoking this function
				broadcast_message(client_sock, buffer);
		  	}
			
			memset(buffer, 0, BUFF_SIZE);
		}
	}
}

/*
	Function:		broadcast_message()
	Parameters:		int sender, char* messageToSend
	Output:			NONE
	Return value:	void
	Description:	This function is invoked each time a client sends a message to the server. The message is formatted, by calling the formatMessage() function and
					broadcasted to all connected clients. If the message received is greater than 40 characters, this function breaks the message into packets
					before calling the formatMessage() function.
*/
void broadcast_message(int sender, char* messageToSend)
{
	char sendMsg[BUFF_SIZE];
	
	int senderIndx;
	
	int msgLength;
	
	int numPacket;
	
	char packet[TOTAL_PACKETS][BUFF_SIZE];
	char fstPacket[BUFF_SIZE];
	char sndPacket[BUFF_SIZE];
	
	strcpy(sendMsg, messageToSend);	// keep track of the original message sent by sender
	
	// loop to get sender's IP, userID, message to create the formatted message
	for(int i = 0; i < numClients; i++)
	{
		if(connected_client[i].client_socket == sender)
		{			
			senderIndx = i;	// get sender's index
		}
	}
	
	// check message size and break in chunks of 40 characters	
	msgLength = strlen(sendMsg);

	// if message is less than / equal to 40 characters
	if (msgLength <= PACKET_SIZE)
	{
		numPacket = 1;
		memset(packet[0], 0, BUFF_SIZE);
		memset(packet[1], 0, BUFF_SIZE);

		memcpy(packet[0], sendMsg, PACKET_SIZE);

		// send packets to sender
		formatMessage(packet[0], senderIndx, 1);
		write(connected_client[senderIndx].client_socket, packet[0] , strlen(packet[0]));

		memcpy(packet[0], sendMsg, PACKET_SIZE);		// packet[] has been overwritten, reset to the original message
		for (int j = 0; j < numPacket; j++)
		{
			formatMessage(packet[j], senderIndx, 0);

			// send formatted message to each client
			for(int i = 0; i < numClients; i++)
			{
				if(connected_client[i].client_socket != sender)
				{			
					// send a formatted reponse to all clients except the sender
					write(connected_client[i].client_socket, packet[j] , strlen(packet[j]));
				}
			}
		}
	}
	// else if message is greater than 40 characters
	else
	{
		numPacket = TOTAL_PACKETS;
		
		// clear the buffer
		memset(packet[0], 0, BUFF_SIZE);
		memset(packet[1], 0, BUFF_SIZE);

		if (sendMsg[PACKET_SIZE] == WHITE_SPACE)
		{
			// check if char at index 40 is an empty space
			//break message at index 40
			memcpy(packet[0], sendMsg, PACKET_SIZE); 			// first 40 characters
			memcpy(packet[1], sendMsg+41, PACKET_SIZE); 		// second part of the message
		}
		else
		{		
			//break next to the last empty space before 40 characters

			memcpy(packet[0], sendMsg, PACKET_SIZE); // first 40 characters			

			int i = 0;
			int index = 0;
			
			// loop the string until the end
			while(packet[0][i] != '\0')
		  	{
		  		if(packet[0][i] == WHITE_SPACE)  
				{
		  			index = i;		// get the index of the last empty space in the string
		 		}
		 		i++;
			}
			
			memset(packet[0], 0, BUFF_SIZE);				// clear the buffer
			memcpy(packet[0], sendMsg, index+1); 			// first part of the message


			memcpy(packet[1], sendMsg+index+1, 60); 		// second part of the message
			
		}

		// send packets to sender
		for (int j = 0; j < numPacket; j++)
		{
			strcpy(sendMsg, packet[j]);		// to prevent overwriting the broken up message
			formatMessage(sendMsg, senderIndx, 1);
			write(connected_client[senderIndx].client_socket, sendMsg, strlen(sendMsg));

			memset(sendMsg, 0, BUFF_SIZE);
		}
		
		// send packets to all other clients
		for (int j = 0; j < numPacket; j++)
		{
			strcpy(sendMsg, packet[j]);		// to prevent overwriting the broken up message
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
			
			memset(sendMsg, 0, BUFF_SIZE);
		}
	}
}

/*
	Function:		formatMessage()
	Parameters:		char* message, int whichClient, int isSender
	Output:			NONE
	Return value:	void	
	Description:	This function is invoked by broadcast_message() function. This function created the formatted message to be broadcasted to all clients.
*/
void formatMessage(char* message, int whichClient, int isSender)
{
	int length = 0;
	char senderMessage[BUFF_SIZE];
	char whatTime[TIME_LEN];
	getCurrentTime(whatTime);
	strcpy(senderMessage, message);	// keep track of the message
	
	memset(message, 0, BUFF_SIZE);

	length = strlen(connected_client[whichClient].clientIP);		// get size of IP
	if(length <= (MAX_IP-1))
	{
		while(length < (MAX_IP-1))
		{
			strcat(connected_client[whichClient].clientIP, " ");
			length++;
		}
		strcpy(message, connected_client[whichClient].clientIP);	// get sender's IP
	}
	
	strcat(message, " [");	// bracket
	
	length = strlen(connected_client[whichClient].userID);			// get size of the userID
	if(length <= (MAX_USERID-1))
	{
		while(length < (MAX_USERID-1))
		{
			strcat(connected_client[whichClient].userID, " ");
			length++;
		}
		strcat(message, connected_client[whichClient].userID);		// get sender's ID
	}
	
	strcat(message, "] ");	// bracket
	
	if(isSender)
	{
		strcat(message, ">> ");
	}
	else
	{
		strcat(message, "<< ");
	}
	
	length = strlen(senderMessage);	// get size of the message
	
	if(length <= PACKET_SIZE)		// if message length is less than 40
	{
		while(length < PACKET_SIZE)
		{
			// append white space to message array
			strcat(senderMessage, " ");
			length++;
		}
		strcat(message, senderMessage);	
	}
	else
	{
		// no need to append blank space after the message
		strcat(message, senderMessage);	
	}
	
	// get time
	strcat(message, " (");
	strcat(message, whatTime);
	strcat(message, ")");
	strcat(message, "\0");	// null terminate string
}

/*
	Function:		getCurrentTime()
	Parameters:		char* whatTime
	Output:			NONE
	Return value:	NONE
	Description:	This function is invoked each time a message needs to be formatted for broadcasting to all clients. It calcuates the current time, 
					and fills the char array passed as the parameter.
*/
void getCurrentTime(char* whatTime)
{
	time_t currentTime;
	struct tm *timeIs;
	
	time(&currentTime);
	timeIs = localtime(&currentTime);
	
	strftime(whatTime, 9, "%H:%M:%S", timeIs);
}

/*
	Function:		removeClientFromArray()
	Parameters:		int sender
	Output:			NONE
	Return value:	int status
	Description:	This function is invoked each time a client sends a ">>bye<<" message to the server. The client is removed from the connected_client array and a status is returned.
					This function also checks if the numClients (static global variable) has reached zero, it then changed keepRunning (static global variable) to 0. 
					This helps in determining if the server needs to be shutdown.
*/
int removeClientFromArray(int sender)
{
	// flag to determine if the client was found
	int client_found = FALSE; // false	
		
	// get size of array connected_client
	int size = sizeof(connected_client) / sizeof(*connected_client);
	int position;
	int client_removed;
		
	// search sender in the array connected_client
	for (int i = 0; i < size; i++)
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
	if (numClients == 0)
	{
		keepRunning = 0;	// this will prevent while loop in main to accept more connections
	}

	return client_removed;
}

