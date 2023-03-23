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



// move to common file
#define MSG_SIZE	80
#define ERROR		-1		
#define CMD_ERROR	-2
#define HOST_SEARCH_FAIL -3


int main (int argc, char *argv[])
{
	char userID[21];
	struct hostent *host;
	
	
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
	
	/*char hostIP[21];
	int i = 0;
	while(*r != '\0')
	{
		hostIP[i] = *r;
		r++;
		i++;
	}
	printf("%s\n", hostIP);*/
	
	// get host name passed as command line argument
	if((host = gethostbyname(hostID)) == NULL)
	{
		printf("ERROR host ID: %s\n", strerror(errno));
		return HOST_SEARCH_FAIL;
	}
	
	// create socket
	int server_socket;
	struct sockaddr_in server;	// create sockaddr_in variable to connect to server
	
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
		
		// create one thread to send outgoing message
		
		// create another thread to read from socket
	}
	
	
	
	// send message to server socket
	
	/*initscr();							// this function initializes the curses system and alloacte memory for window
	
	char message[MSG_SIZE];
	
	int count = 0;
	while (count < 80)					// take input while user doesn't enter ">>bye<<"
	{
		refresh();							/* Print it on to the real screen */
		//message[count] = getch();
		//count++;
		
		// if user enters "<<bye>>" shutdown client before sending one last message to server
	//}
	
	/*endwin();							// End curses mode - this will free memory taken by ncurses sub0system and its data structures and put terminal back in normal mode.
	
	printf("%d: \n", count);
	printf("%s: \n", message);*/
	
	
	// close client socket

	return 0;
	
}
