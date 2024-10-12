#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Client.h"

Client::Client() {
	connect_socket = INVALID_SOCKET;
	client_port = DEFAULT_CLIENT_PORT;
}

Client::~Client() {
	if (connect_socket != INVALID_SOCKET) {
		closesocket(connect_socket);
	}
	WSACleanup();
}

bool Client::connect_to_server(int port) {
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
		return false;
	}

	int iFamily = AF_INET;
	int iType = SOCK_STREAM;
	int iProtocol = IPPROTO_TCP;

	connect_socket = socket(iFamily, iType, iProtocol);
	if (connect_socket == INVALID_SOCKET) {
		std::cerr << "Connecting to the socket error: " << iResult << std::endl;
		WSACleanup();
		return false;
	}

	sockaddr_in server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(DEFAULT_SERVER_PORT); // change to getaddr

	int retryAttempt = 10;
	do {
		iResult = connect(connect_socket, (sockaddr*)&server_addr, sizeof(server_addr));
		Sleep(500);
		--retryAttempt;
	} while (iResult == -1);


	if (iResult == SOCKET_ERROR) {
		std::cerr << "Socket error: " << iResult << std::endl;
		closesocket(connect_socket);
		WSACleanup();
		return false;
	}

	const std::string greeting = R"(
	______                                       __             ______   __                  __  __ 
	/      \                                     /  |           /      \ /  |                /  |/  |
/$$$$$$  |  ______    ______   _______    ____$$ |  ______  /$$$$$$  |$$ |____    ______  $$ |$$ |
$$ |__$$ | /      \  /      \ /       \  /    $$ | /      \ $$ \__$$/ $$      \  /      \ $$ |$$ |
$$    $$ |/$$$$$$  |/$$$$$$  |$$$$$$$  |/$$$$$$$ | $$$$$$  |$$      \ $$$$$$$  |/$$$$$$  |$$ |$$ |
$$$$$$$$ |$$ |  $$ |$$    $$ |$$ |  $$ |$$ |  $$ | /    $$ | $$$$$$  |$$ |  $$ |$$    $$ |$$ |$$ |
$$ |  $$ |$$ \__$$ |$$$$$$$$/ $$ |  $$ |$$ \__$$ |/$$$$$$$ |/  \__$$ |$$ |  $$ |$$$$$$$$/ $$ |$$ |
$$ |  $$ |$$    $$ |$$       |$$ |  $$ |$$    $$ |$$    $$ |$$    $$/ $$ |  $$ |$$       |$$ |$$ |
$$/   $$/  $$$$$$$ | $$$$$$$/ $$/   $$/  $$$$$$$/  $$$$$$$/  $$$$$$/  $$/   $$/  $$$$$$$/ $$/ $$/ 
			/  \__$$ |                                                                              
			$$    $$/                                                                               
			$$$$$$/                                                                                
Welcome to AgendaShell
)";
	std::cout << greeting;
	while (true) {
		std::string command;
		std::getline(std::cin, command);
		if (send_command(command)) {
			std::cout << recieve_result();
		} else {
			std::cerr << "Unknown server error\n";
			break;
		}
	}

	closesocket(connect_socket);
	return true;
}

bool Client::send_command(const std::string& command) {
	int iResult = send(connect_socket, command.c_str(), command.length(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "Error in sending command: " << WSAGetLastError() << std::endl;
		return false;
	} else if (iResult) {
		return true;
	}
	else {
		std::cerr << "Error in sending result " << WSAGetLastError() << std::endl;
		return false;
	}
}

std::string Client::recieve_result() {
	std::string result;
	char buf[DEFAULT_BUF_LEN];
	int iResult;

	iResult = recv(connect_socket, buf, sizeof(buf), 0);
	if (iResult > 0) {
		result.append(buf, iResult);
	} else if (iResult == 0) {
		std::cout << "Connection closed by server." << std::endl;
	} else {
		std::cerr << "Error in receiving result: " << WSAGetLastError() << std::endl;
	}

	return result;
}