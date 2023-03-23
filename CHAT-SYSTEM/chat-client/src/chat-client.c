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
#include <arpa/inet.h>
#include <errno.h>



#define MSG_SIZE	80
#define ERROR		-1		// move to common file


int main()
{
	// create socket
	int server_socket;
	struct sockaddr_in server;	// create sockaddr_in variable to connect to server
	
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)	// initialize socket using Address Family (IP version 4), Socket Stream (TCP protocol), and Protocol 0 (IP protocol).
	{
		printf("ERROR: %s\n", strerror(errno));
	}
	
	// connect socket to server socket using IP address and port number
	
	
	memset (&server, 0, sizeof (server));
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
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
