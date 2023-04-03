

#include "../../common/inc/common.h"
#include <pthread.h>
#define MAX_CLIENTS 10
#define TIME_LEN	9

typedef struct
{
	int client_socket;
	char clientIP[MAX_IP];
	char userID[USER_ID_LEN];
	char message[MAX_MSG];	
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
