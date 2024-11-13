#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <string>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "CommandHandler.h"

#define DEFAULT_PORT 10000
#define DEFAULT_BUF_LEN 524288

DWORD WINAPI handle_client(LPVOID clientSocket);
std::string receive_command(SOCKET socket);
std::string execute_command(const std::string& command);
bool send_result(SOCKET socket, const std::string& result);
bool start();

//class Server
//{
//public:
//	Server(int port);
//	~Server();
//
//	bool start();
//
//private:
//
//	void accept_connection();
//	std::string recieve_command(SOCKET socket);
//	std::string execute_command(const std::string& command);
//	bool send_result(SOCKET socket, const std::string& result);
//	DWORD WINAPI handle_client(LPVOID clientSocket);
//
//	SOCKET listen_socket;
//	CommandHandler command_handler;
//	int server_port;
//};

