#pragma comment (lib, "ws2_32.lib")

#include "WSAInitializer.h"
#include "Server.h"
#include <iostream>
#include <exception>

int main()
{
	bool first = true;
	while (true)
	{
		try
		{
			WSAInitializer wsaInit;
			Server myServer;
			if (first)
			{
				std::thread manage(&Server::messageManeger, &myServer);
				manage.detach();
				first = false;
			}
			myServer.serve(8826);
			

		}
		catch (std::exception& e)
		{
			std::cout << "Error occured: " << e.what() << std::endl;
		}
		system("PAUSE");
	}

	return 0;
}