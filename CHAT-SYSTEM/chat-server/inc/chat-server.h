
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

// to hold last 10 messages with IP address, userID and messages in char array format
typedef struct
{
	char prevMsgs[1024];
}prevMsgs;
