#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>

#define NUM_LISTEN 5
#define BAD_EXIT -1
#define GOOD_EXIT 0

Server::Server(int portnum){
    // make user database from file
    std::ifstream file;
    file.open("data_base.txt", std::ofstream::out);
    std::string line; 
    string_v user_pass;
    while(std::getline(file, line)){
        if (line.empty()) break;
        user_pass = parse_string(line);
        add_to_data(user_pass[0], user_pass[1]);
    }
    file.close();
	// make new socket fd
    welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (welcome_socket < 0){
        fprintf(stderr, "ERROR: could not open socket\n");
    }
    // make the address
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portnum);
    // bind the socket
    if (bind(welcome_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "ERROR: could not bind socket");
        exit(BAD_EXIT);
    }
    // set socket to listen
    listen(welcome_socket, NUM_LISTEN);
    // let admin know server is online
    std::cout << "Server is online." << std::endl;
    // start working
    work();
}


string_v Server::parse_string(std::string line){
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    size_t pos = 0;
    std::string token;
    string_v output; 
    while((pos = line.find(delimiter)) != std::string::npos){
        token = line.substr(0, pos);
        output.push_back(token);
        line.erase(0, pos + delimiter.length());
    }
    output.push_back(line);
    return output;
}


void Server::add_to_data(std::string username, std::string password){
    user_t new_user = {username, password};
    clients.push_back(new_user);
}


void Server::work(){
	// create local vars
	int new_client_socket;
	int run = 1;
	ssize_t n = 1;
	fd_set server_fd, read_fd;
	// zero the server list of fd and add IO and weolcome socket
	FD_ZERO(&server_fd);
    FD_SET(0, &server_fd);
    FD_SET(welcome_socket, &server_fd);
    // while server is open
    while(run){
    	// check who is ready to write
        FD_ZERO(&read_fd);
        read_fd = server_fd;
        select(30, &read_fd, 0, 0, 0);
        // test if it a connection
        if (FD_ISSET(welcome_socket, &read_fd)){
        	// make place holderfor user msg
        	std::string msg;
        	// try to connect to user
        	clilen = sizeof(client_addr);
        	new_client_socket = accept(welcome_socket, (struct sockaddr *) &client_addr, &clilen);
        	if (new_client_socket < 0){
        		fprintf(stderr, "ERROR: on accept");
        	}
        	// if connection is good recive message
        	bzero(buffer, 256);
        	n = read(new_client_socket, buffer, 256);
        	if (n < 0){
        		fprintf(stderr, "ERROR reading from socket");
        	}
        	msg = buffer;
            std::string to_client = "OK";
        	// do calculations
        	string_v parsed = parse_string(msg);
            if (parsed[1] == "login"){
                REPLY reply = log_user(parsed[2], parsed[3]);
                if (reply == BAD_PASSWORD){
                    to_client = "BAD PASSWORD";
                }
            } else if (parsed[1] == "sign"){
                REPLY reply = sign_user(parsed[2], parsed[3], true); 
                if (reply == USERNAME_IN){
                    to_client = "NAME CAUGHT";
                }
            } else if (parsed[1] == "change"){
                REPLY reply = sign_user(parsed[2], parsed[3], false); 
                if (reply == USERNAME_IN){
                    to_client = "NO USER";
                }
            }else{
                to_client = "BAD REQUEST";
            }
        	// send return msg
            std::cout << to_client << ": " << msg << std::endl;;
        	n = write(new_client_socket, to_client.c_str(), to_client.length());
        	if (n < 0){
        		fprintf(stderr, "ERROR writing to socket");
        	}
        	close(new_client_socket);
        } 	
    	// check is admin has input a msg
    	if (FD_ISSET(0, &read_fd)) {
            bzero(buffer, 256);
            n = read(0, buffer, 256);
            if (n < 0) {
            	fprintf(stderr, "ERROR reading from stdin");
            }
            if (strcmp(buffer, "EXIT\n") == 0) {
                printf("Shutting down server\n");
                break;
        	}
    	}
    }
    update_database();
    close(welcome_socket);
}


void Server::update_database(){
    std::ofstream file;
    file.open("data_base.txt", std::ofstream::in | std::ofstream::trunc);
    for (auto it = clients.begin(); it != clients.end(); it++){
        file << it->name << "-" << it->password << std::endl;;
    }
    file.close();
}


REPLY Server::log_user(std::string username, std::string password){
    for (auto it = clients.begin(); it != clients.end(); it++){
        if (it->name == username){
            if (it->password == password){
                return OK;
            }else{
                break;
            }
        }
    }
    return BAD_PASSWORD;
}


REPLY Server::sign_user(std::string username, std::string password, bool first_time=true){
    user_v::iterator it;
    for(it = clients.begin(); it != clients.end(); it++){
        if (it->name == username) break;
    }
    if (first_time && it == clients.end()){
        user_t user = {username, password};
        clients.push_back(user);
    }else if (!first_time && it != clients.end()){
        it->password = password;
    }else{
        return USERNAME_IN;
    }
    return OK;
}


int main(int argc, char* argv[]){
    if (argc != 2){
        fprintf(stderr, "USAGE: no port\n");
        exit(BAD_EXIT);
    }
    // make a server with a welcome socket
    Server server = Server(atoi(argv[1]));
}