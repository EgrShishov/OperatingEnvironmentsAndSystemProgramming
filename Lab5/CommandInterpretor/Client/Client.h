#pragma once
#include <string>
#include <WinSock2.h>

class Client
{
public:
	Client();
	~Client();

	bool connect(int port);
	std::string send_command(const std::string& command);
	std::string recieve_result();
};