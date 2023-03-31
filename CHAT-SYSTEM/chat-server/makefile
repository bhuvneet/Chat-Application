# This is my makefile for chat-client assignment 4.
# Date: 21/03/2023
# By:	Bhuvneet Thakur, Maisa Wolff Resplande


# final binary:

./bin/chat-server: ./obj/chat-server.o ../common/obj/common.o
		cc ./obj/chat-server.o ../common/obj/common.o -o ./bin/chat-server -lpthread
		
	
	
# dependencies:	

./obj/chat-server.o: ./src/chat-server.c ../common/inc/common.h
		cc -c ./src/chat-server.c -o ./obj/chat-server.o
	
# other targets:

all: ./bin/chat-server

clean: 
	rm ./bin/*
	rm ./obj/*
