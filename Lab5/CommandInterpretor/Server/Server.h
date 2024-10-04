#pragma once
#include <string>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#define DEFAULT_PORT 10000
#define DEFAULT_BUF_LEN 512

//Установку TCP - соединения на определённый порт.
//Приём команд от клиента.
//Выполнение команд в системе.
//Возвращение результата клиенту

class Server
{
public:
	Server(int port = DEFAULT_PORT);
	~Server();

	bool start();

private:

	void accept_connection();
	std::string recieve_command(SOCKET socket);
	std::string execute_command(const std::string& command);
	bool send_result(SOCKET socket, const std::string& result);

	SOCKET listen_socket;
	int server_port;
};

