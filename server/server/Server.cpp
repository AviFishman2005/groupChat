#include "Server.h"
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
bool ready = false;
std::mutex users;
std::mutex messages;
using std::cout;
using std::endl;
std::condition_variable newMessage;

Server::Server()
{

	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	_serverSocket = socket(AF_INET,  SOCK_STREAM,  IPPROTO_TCP); 

	if (_serverSocket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__ " - socket");
}

Server::~Server()
{
	try
	{
		// the only use of the destructor should be for freeing 
		// resources that was allocated in the constructor
		closesocket(_serverSocket);
	}
	catch (...) {}
}

void Server::serve(int port)
{
	
	struct sockaddr_in sa = { 0 };
	
	sa.sin_port = htons(port); // port that server will listen for
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

	// Connects between the socket and the configuration (port and etc..)
	if (bind(_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");
	
	// Start listening for incoming requests of clients
	if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	std::cout << "Listening on port " << port << std::endl;

	while (true)
	{
		// the main thread is only accepting clients 
		// and add then to the list of handlers
		std::cout << "Waiting for client connection request" << std::endl;
		acceptClient();
	}
}


void Server::acceptClient()
{

	// this accepts the client and create a specific socket from server to this client
	// the process will not continue until a client connects to the server
	SOCKET client_socket = accept(_serverSocket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__);

	
	// the function that handle the conversation with the client
	std::thread t(&Server::clientHandler,this,client_socket);
	t.detach();
}

/// <summary>
/// This function recieves messages from user and adds to file
/// </summary>
/// <param name="clientSocket"></param>
void Server::clientHandler(SOCKET clientSocket)
{
	string userName;
	string allUsernames;

	try
	{
		//Add to user list
		std::cout << "Client accepted. Server and client can speak" << std::endl;
		Helper::getMessageTypeCode(clientSocket);
		userName = Helper::getUsername(clientSocket);
		cout << userName << " Has joint the chat" << endl;
		//Add to user list
		this->_users.insert({ userName,clientSocket });
		allUsernames = this->getAllUsernames();
		Helper::send_update_message_to_client(clientSocket, "", "", allUsernames);

		while(true)
		{
			//Helper::send_update_message_to_client(clientSocket, "", "", allUsernames);

			string secondUsername = "";
			allUsernames = this->getAllUsernames();

			if (Helper::getMessageTypeCode(clientSocket) == 204)
			{
				
				secondUsername = Helper::getUsername(clientSocket);
				int lenNewMessage = Helper::getIntPartFromSocket(clientSocket, 5);
				string messageIn = Helper::getStringPartFromSocket(clientSocket, lenNewMessage);
				if (secondUsername.size() > 0 && messageIn.size() > 0)
				{
					std::lock_guard<std::mutex> lck(messages);
					message m;
					m.userName = userName;
					m.secondUsername = secondUsername;
					m.content = messageIn;
					this->_messageQueue.push(m);
					ready = true;
					newMessage.notify_all();
				}
				
				Helper::send_update_message_to_client(clientSocket, this->getFile(userName, secondUsername), secondUsername, allUsernames);

				

			}
			

		}
	}
	catch (const std::exception& e)
	{
		this->_users.erase(userName);
		closesocket(clientSocket);
	}


}
/// <summary>
/// Retruns a string with all username separated with &
/// </summary>
/// <returns></returns>
std::string Server::getAllUsernames()
{
	string names;
	for (std::map<string, SOCKET>::iterator it = this->_users.begin(); it != this->_users.end(); ++it) {
		
		names += (it->first);
		names += "&";
	}
	names = names.substr(0, names.size() - 1);
	return names;
	
	
}
/// <summary>
/// redorect messages to where they need to go
/// </summary>
void Server::messageManeger()
{
	//std::this_thread::sleep_for(std::chrono::seconds(60));

	while (true) {
		std::unique_lock<std::mutex> lck(messages);
		while (!ready)
		{
			newMessage.wait(lck);
		}
		if (!this->_messageQueue.empty())
		{
			SOCKET socket;
			std::ofstream chat;
			string usersChatting;
			message m;
			m = this->_messageQueue.front();
			this->_messageQueue.pop();
			string author = m.userName;
			string other = m.secondUsername;
			string messageData = m.content;
			string content;
			usersChatting = this->fileName(author, other) + ".txt";

			socket = this->_users[author];

			chat.open(usersChatting, std::ios::out | std::ios::app);
			chat << "&MAGSH_MESSAGE&&Author&" << author << "&DATA&" << messageData;
			chat.close();
			

				
		}
	}
}
/// <summary>
/// returns file name in alphabetical order
/// </summary>
/// <param name="userName"></param>
/// <param name="secondUsername"></param>
/// <returns></returns>
string Server::fileName(string userName, string secondUsername)
{
	int compare = userName.compare(secondUsername);
	if (compare > 0)
	{
		return string(secondUsername + '&' + userName);
	}
	return string(userName + '&' + secondUsername);
	
}
/// <summary>
/// returns string of file content
/// </summary>
/// <param name="userName"></param>
/// <param name="secondUsername"></param>
/// <returns></returns>
string Server::getFile(string userName, string secondUsername)
{
	string fileName = this->fileName(userName, secondUsername);
	fileName = fileName + ".txt";
	std::ifstream readChat(fileName, std::ios::in);
	string line;
	string content;
	while (std::getline(readChat, line))
	{
		content += line;
	}
	readChat.close();
	return content;
}






