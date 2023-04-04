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
#include <unistd.h>		// to close socket
#include <pthread.h>


// move to common file
#define MSG_SIZE	80
#define ERROR		-1		
#define CMD_ERROR	-2
#define HOST_SEARCH_FAIL -3
#define h_addr h_addr_list[0] /* for backward compatibility */

// for displaying messages
WINDOW *display_window;
WINDOW *input_window;
int startingLine = 1;
int getInput = 1;
	
// function prototypes
void *sendMessage(void* socket);
void *recvMessage(void* socket);
void destroy_win(WINDOW *win);
void blankWin(WINDOW *win);

static int keepRunning = 1;

int main (int argc, char *argv[])
{
	// make constants for array lengths
	char userID[21];
	struct hostent *host;
	int endSession 		= 1;
	int msgRecd 		= 0;
	char message[1024];	
	int count 			= 0;
	int server_socket;
	struct sockaddr_in server, addr;		// create sockaddr_in variable to connect to server
	socklen_t len = sizeof(addr);
	char myIPaddress[21];					// to send to server client's IP address as first message'
	pthread_t send_thread, recv_thread;		// threads to send and receive messages
	
	
	
	// screen dimensions
	int x, y;
	
	// check command line args
	if(argc != 3)
	{
		printf("USAGE: <chat-client> -user<userID> -server<serverName>\n");
		return CMD_ERROR;
	}
	
	// validate arg lengths
	strcpy(userID, argv[1]);	// user ID of the client connected
	if (strlen(userID) > 10)	// exit program if userID is greater than 5 char
	{
		printf("User greater than 5 char\n");
		return ERROR;
	}
	
	printf("%s\n", argv[0]);
	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);
	
	char* returnVal = strstr(argv[2], "-server");	// point to the root folder of the project
	//printf("%d\n", returnVal);	// returns 0 
	
	char* hostID = strrchr(returnVal, 'r');
	
	// point to last occurence of r in "server" - this is where IP address or server's name is passed as cmd arg
	hostID++;
	printf("%s\n", hostID);
	
	// get host name passed as command line argument
	if((host = gethostbyname(hostID)) == NULL)
	{
		printf("ERROR host ID: %s\n", strerror(errno));
		return HOST_SEARCH_FAIL;
	}
	
	// create socket
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
		// initializes the curses system and alloacte memory for window
		initscr();
		getmaxyx(stdscr, x, y);	// set up window dimensions
		
		// set up dimensions of screens
		display_window 		= newwin(y/2, 85, 0, 0);
    	input_window 		= newwin(y/2, 85, x/2, 0);
    	scrollok(display_window,TRUE);
    	scrollok(input_window,TRUE);
    	box(display_window, 0, 0);
		box(input_window, 0, 0);

		wsetscrreg(display_window,1,y/2-2);
		wsetscrreg(input_window,1,y/2-2);
		
		//printf("connected with server!\n");
		
		// send first message to server for userID and IP address
		getsockname(server_socket, (struct sockaddr *)&addr, &len);		// send server IP address of this client
		
		strcpy(myIPaddress, inet_ntoa(addr.sin_addr));	// get client's IP'
		
		memset(message, 0, 1024);
		strcpy(message, "FIRST|");
		strcat(message, userID);
		strcat(message, "|");
		strcat(message, myIPaddress);
		strcat(message, "\0");	// null terminate the string
		
		write (server_socket, message, strlen (message));
		printf("sent: %s\n",message);
		memset(message, 0, 1024);

		
		
		void* arg;
		// launch sending thread to send messages to server
		if(pthread_create(&send_thread, NULL, sendMessage, (void *)&server_socket))
		{
			printf("ERROR host ID: %s\n", strerror(errno));
			return 0;
		}	
		
		if(pthread_create(&recv_thread, NULL, recvMessage, (void *)&server_socket))
		{
			printf("ERROR host ID: %s\n", strerror(errno));
			return 0;
		}		
		
		while (keepRunning);
		
		// wait for threads to finish
		pthread_join(send_thread, NULL);
		pthread_join(recv_thread, NULL);					

		
		//endwin();							// End curses mode - this will free memory taken by ncurses sub0system and its data structures and put terminal back in normal mode.
		
		// create one thread to send outgoing message
		
		// create another thread to read from socket
		
		
	}	
	
	return 0;
	
}

WINDOW* createOutputWin(int height, int width, int x, int y)
{
	
}

void blankWin(WINDOW *win)
{
  int i;
  int maxrow, maxcol;
     
  getmaxyx(win, maxrow, maxcol);
  for (i = 1; i < 85-2; i++)  
  {
    wmove(win, i, 1);
    refresh();
    wclrtoeol(win);
    wrefresh(win);
  }
  box(win, 0, 0);             /* draw the box again */
  wrefresh(win);
}  /* blankWin */

void destroy_win(WINDOW *win)
{
  delwin(win);
}  /* destory_win */

void *sendMessage(void* socket)
{
	char message[81] = {"\0"};		// use constant
	int server_socket = *((int*)socket);
	
	//blankWin(display_window);		// clear the input screen for new input
	
	while(keepRunning)
	{
		blankWin(input_window);		// clear the input screen for new input
        bzero(message, 81);
        wrefresh(display_window);
        wrefresh(input_window);
        
        // get input message in bottom window
        mvwgetnstr(input_window, getInput, 2, message, 80);	// limit to 80 characters
        
        if(strcmp(message,">>bye<<") == 0) // check if the user wants to quit
		{
			keepRunning = 0;
			
			// display message in top window
        	mvwprintw(display_window, startingLine, 2, message);
        
			// send final message to server
			send (server_socket, message, strlen (message), 0);
			
			// destroy windows
			destroy_win(display_window);
			destroy_win(input_window);
			
			// end window
			endwin();
			
			pthread_exit(NULL);
			// close client socket
			close(server_socket);
			break;
		}
		else
		{
			// send message to server
			send (server_socket, message, strlen (message), 0);
			memset(message, 0, 81);
		}
        
        // display message in top window
        mvwprintw(display_window, startingLine, 2, message);
	}
}


void *recvMessage(void* socket)
{
	char buffer[79];
	int readMsg;
	int server_socket = *((int*)socket);
	int row = 1;
	
	while(keepRunning)
	{
		bzero(buffer, 79);
        wrefresh(display_window);
        wrefresh(input_window);
        
		readMsg = read(server_socket, buffer, 78);
		
		//Print on own terminal
        mvwprintw(display_window, startingLine, 3, buffer);
        startingLine++;
		//printf("length of msg received %d\n", readMsg);
		
		if(readMsg > 0)
		{
			if (readMsg == 78)	// TODO when message length is 80 characters IP address in printed after time
			{

			}
		}
		else if(readMsg == 0)
		{			
			printf("Server disconnected\n");
			//exit(0);		// this will exit the reading thread from executing
			break;
		}
	}
}
