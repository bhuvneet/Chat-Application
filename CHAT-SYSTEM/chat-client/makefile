# This is the makefile for chat-client assignment 4.
# Date: 21/03/2023
# By:	Bhuvneet Thakur, Maisa Wolff Resplande


# final binary:

./bin/chat-client: ./obj/chat-client.o
		cc ./obj/chat-client.o -lncurses -o ./bin/chat-client -lpthread
	
	
# dependencies:	

./obj/chat-client.o: ./src/chat-client.c ./inc/chat-client.h
		cc -c ./src/chat-client.c -o ./obj/chat-client.o
	
# other targets:

all: ./bin/chat-client

clean: 
	rm ./bin/*
	rm ./obj/*
