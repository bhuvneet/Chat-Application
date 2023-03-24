

#include "../../common/inc/common.h"
#define MAX_CLIENTS 10

typedef struct 
{
	char clientIP[MAX_IP];
} ConnectedClients;


typedef struct 
{
	char userID[USER_ID_LEN];
	ConnectedClients ip;
	char message[MAX_MSG];
} MessageFromClient;
