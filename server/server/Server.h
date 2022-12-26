#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <queue>
#include <exception>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>

#include<map>
#include "Helper.h"
#define LEN 500
using std::string;
struct message {
	string userName;
	string secondUsername;
	string content;
};

class Server
{
public:
	Server();
	~Server();
	void serve(int port);
	void messageManeger();
	string fileName(string userName,string secondUsername);
	string getFile(string userName,string secondUsername);
private:

	void acceptClient();
	void clientHandler(SOCKET clientSocket);
	std::string getAllUsernames();
	SOCKET _serverSocket;
	std::queue<message> _messageQueue;
	std::map<string,SOCKET> _users;

};

