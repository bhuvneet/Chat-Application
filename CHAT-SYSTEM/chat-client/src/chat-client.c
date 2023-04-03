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

// function prototypes
void *sendMessage(void* socket);
void *recvMessage(void* socket);

void blankWin(WINDOW *win);
void display_win(WINDOW *win, char *word, int whichRow, int shouldBlank);
void input_win(WINDOW *win, char *word);
WINDOW *create_newwin(int height, int width, int starty, int startx);

static int iQuit = 0;
WINDOW *chat_win, *msg_win;	// windows for ncurse

int main (int argc, char *argv[])
{
	// make constants for array lengths
	char userID[21];
	struct hostent *host;
	int endSession 		= 1;
	int msgRecd 		= 0;
	char message[81];	
	int count 			= 0;
	int server_socket;
	struct sockaddr_in server, addr;		// create sockaddr_in variable to connect to server
	socklen_t len = sizeof(addr);
	char myIPaddress[21];					// to send to server client's IP address as first message'
	pthread_t send_thread, recv_thread;		// threads to send and receive messages
	
	int chat_startx, chat_starty, chat_width, chat_height;
   int msg_startx, msg_starty, msg_width, msg_height, i;
	
	
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

	// initializes the curses system and alloacte memory for window
	initscr();
	cbreak();	// allowsto display the char entered by the user
	noecho();	// prevents from displaying message
	//keypad(stdscr, TRUE);		// allows the use of keys for the standard window
	refresh();

	// sets windows dimensions	   
	chat_height = 5;
	chat_width  = COLS - 2;
	chat_startx = 1;        
	chat_starty = LINES - chat_height;        
		  
	msg_height = LINES - chat_height - 1;
	msg_width  = COLS;
	msg_startx = 0;        
	msg_starty = 0;

	// create window to display messages sent/received
 	msg_win = create_newwin(msg_height, msg_width, msg_starty, msg_startx);
	scrollok(msg_win, TRUE);

	// create window to get user input
	chat_win = create_newwin(chat_height, chat_width, chat_starty, chat_startx);
	scrollok(chat_win, TRUE);
	
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
		//printf("sent: %s\n",message);
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
		
		// wait for threads to finish
		pthread_join(send_thread, NULL);
		pthread_join(recv_thread, NULL);					

		sleep(5);
		delwin(chat_win);
		delwin(msg_win);
		endwin();							// End curses mode - this will free memory taken by ncurses sub0system and its data structures and put terminal back in normal mode.

	}	
	
	return 0;
	
}

void *sendMessage(void* socket)
{
	char message[81] = {"\0"};		// use constant
	int server_socket = *((int*)socket);

	int ch;
	int maxrow, maxcol, row = 1, col = 0;	
	
	while(TRUE)
	{
		blankWin(chat_win);                          // cleans window
	   getmaxyx(chat_win, maxrow, maxcol);          /* get window size */
		bzero(message, 81);
		wmove(chat_win, 1, 1);                       /* position cusor at top */
		
		// read input from terminal
		// code based on ncurses-01.c
		for (int i = 0; (ch = wgetch(chat_win)) != '\n'; i++)
		{
			message[i] = ch;
			if (col++ < maxcol-2)               // if within window
			{
				wprintw(chat_win, "%c", message[i]);      // display the char recv'd 
			}
			else                                //* last char pos reached 
			{
				col = 1;
				if (row == maxrow-2)              //* last line in the window 
				{
				 	scroll(chat_win);                 //* go up one line 
				 	row = maxrow-2;                 //* stay at the last line 
				 	wmove(chat_win, row, col);           //* move cursor to the beginning 
				 	wclrtoeol(chat_win);                 //* clear from cursor to eol 
					box(chat_win, 0, 0);                 //* draw the box again 
				} 
				else
				{
				  row++;
				  wmove(chat_win, row, col);           //* move cursor to the beginning 
				  wrefresh(chat_win);
				  wprintw(chat_win, "%c", message[i]);    //display the char recv'd
				}
			}


		}	

		if(strcmp(message,">>bye<<") == 0) // check if the user wants to quit
		{
			// send final message to server
			send (server_socket, message, strlen (message), 0);
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
	}

	pthread_exit(NULL);
	exit(0);
}


void *recvMessage(void* socket)
{
	char message[79];
	char buffer[80];
	int readMsg;
	int server_socket = *((int*)socket);
	
	int maxrow, maxcol;

	int row = 1;
	int column = 0;
     
  	getmaxyx(msg_win, maxrow, maxcol);	// get window dimensions

	memset(message, 0, 79);	// reset buffer

	while(TRUE)
	{
		memset(message,0,79);
		bzero(message, 79);
		readMsg = read(server_socket, message, 79);
		//printf("length of msg received %d\n", readMsg);
		
		if(readMsg > 0)
		{
			wmove(msg_win, (row+1), 1);
			wprintw(msg_win, message);
			wrefresh(msg_win);
			row++;
		}
		else if(readMsg == 0)
		{			
			exit(0);		// this will exit the reading thread from executing
			break;
		}
	}
}



/*
	Function: create_newwin()
	Author: Sam Hsu (11/17/10)
	Description: Refreshes the window and recreates the box for messages
	Code available on: https://conestoga.desire2learn.com/d2l/le/content/687346/viewContent/14574861/View
*/
WINDOW *create_newwin(int height, int width, int starty, int startx)
{       
	WINDOW *local_win;
     
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);               // draw a box
	wmove(local_win, 1, 1);             // position cursor at top
	wrefresh(local_win);     
  
	return local_win;
}



/*
	Function: blankWin()
	Author: Sam Hsu (11/17/10)
	Description: Refreshes the window and recreates the box for messages
	Code available on: https://conestoga.desire2learn.com/d2l/le/content/687346/viewContent/14574861/View
*/     
void blankWin(WINDOW *win)
{
  int i;
  int maxrow, maxcol;
     
  getmaxyx(win, maxrow, maxcol);		// get windows dimensions

  for (i = 1; i < maxcol-2; i++)  
  {
    wmove(win, i, 1);
    refresh();
    wclrtoeol(win);
    wrefresh(win);
  }

  box(win, 0, 0);             // draw the box again
  wrefresh(win);					// refreshes the specified window
}
