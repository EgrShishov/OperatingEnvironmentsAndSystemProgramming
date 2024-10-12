#pragma once
#include <string>
#include <iostream>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_SERVER_PORT 10000
#define DEFAULT_CLIENT_PORT 10001
#define DEFAULT_BUF_LEN 524288

#pragma comment(lib, "Ws2_32.lib")

class Client
{
public:
	Client();
	~Client();

	bool connect_to_server(int port);
	bool send_command(const std::string& command);
	std::string recieve_result();

private:
	int client_port;
	SOCKET connect_socket;
};