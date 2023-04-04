/*
	Date:			April 4, 2023
	Project:		The Can We Talk System? - Assignment 4
	File:			chat-server.h
	By:				Bhuvneet Thakur, Maisa Wolff Resplande
	Description:	This file conatins the struct definitions and constants for the Server code.
*/

#include <pthread.h>
#define MAX_CLIENTS 		9	//numClient begins from 0; so total clients are 10
#define TIME_LEN			9
#define PACKET_SIZE 		40
#define MAX_IP				16
#define MAX_USERID			6
#define ERROR 				-1
#define PORT 				5000
#define TRUE 				1
#define FALSE 				0
#define MSG_SIZE			80
#define BUFF_SIZE			1024
#define	WHITE_SPACE			32
#define TOTAL_PACKETS		2

typedef struct
{
	int client_socket;
	char clientIP[MAX_IP];
	char userID[MAX_USERID];
	char message[MSG_SIZE];	
} ConnectedClients;

typedef struct 
{
	pthread_t client_threads;
} Threads;

typedef struct
{
	int clientSockets[MAX_CLIENTS];
	int server_socket;
} Sockets;
