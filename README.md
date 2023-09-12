# Chat-Application
The project uses socket-based TCP/IP communication and is comprised of two applications: the server (chat-server) and the client (chat-client). Here's an overview of the system's capabilities and requirements:

<b>System Description and Project Features:</b>
- TCP/IP is the most commonly used communication protocol for inter-process communication. This project demonstrates the basics of socket-based TCP/IP communication.
- Use C programming techniques for socket-level programming and multi-threaded solutions.
- Utilize the NCURSES library for the client-side user interface.

<b>Server Architecture:</b>
- The server is a multi-threaded application.
- Each new user joining the conversation spawns a new thread responsible for accepting incoming messages and broadcasting them to all chat users.
- Information about all client connections, such as IP addresses and user IDs, should be stored in a shared data structure accessible by all communication threads.

<b>Command-line Arguments:</b>
- The client application must accept two command-line switches: --user<userID> and --server<server name>. This allows specifying the username and server's name or IP address.

<b>Client UI:</b>
- The client's user interface should be basic and use the NCURSES library.
- It should include a prompt area for message input and a dedicated area to display the conversation.
- Messages should be sent when the user presses the carriage return.

<b>Message Length:</b>
- Messages exceeding 80 characters should be broken at/near the 40-character boundary based on spaces between words.

<b>Incoming Message Window:<b/>
- Incoming messages display the sender's IP address, user ID, message direction (>> for outgoing, << for incoming), message content, and a timestamp.

<b>Real-time Messaging:</b>
- Messages are received by all clients as soon as they are sent.
- Receiving a message does not interrupt the user while they are composing a message.
- 
<b>Shutdown:</b>
- Clients can shut down properly by sending the message ">>bye<<".
- The server ends the thread connected to the client and cleans up client information.
- When the number of threads reaches zero in the server, it should shut down and clean up resources.

<b>Graceful Error Handling:</b> 
- The solution should gracefully handle errors and shutdowns.
