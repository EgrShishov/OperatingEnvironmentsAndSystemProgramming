#include "Server.h"

Server::Server(int port = DEFAULT_PORT) {
	port = port;
	listen_socket = INVALID_SOCKET;
}

Server::~Server() {
	if (listen_socket != INVALID_SOCKET) {
		closesocket(listen_socket);
	}
	WSACleanup();
}

bool Server::start() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
		return false;
	}

	int iFamily = AF_INET;
	int iType = SOCK_STREAM;
	int iProtocol = IPPROTO_TCP;

	listen_socket = socket(iFamily, iType, iProtocol);
	if (listen_socket == INVALID_SOCKET) {
		std::cerr << "Error in creating listening socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	sockaddr_in server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_addr.sin_port = htons(server_port);

	iResult = bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (iResult == INVALID_SOCKET) {
		std::cerr << "Error in binding listening socket: " << WSAGetLastError() << std::endl;
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	iResult = listen(listen_socket, SOMAXCONN);
	if (iResult == INVALID_SOCKET) {
		std::cerr << "Error in listening through socket: " << WSAGetLastError() << std::endl;
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	std::cout << "Server is listening on port " << server_port << std::endl;

	accept_connection();
	return true;
}

void Server::accept_connection() {
	SOCKET client_socket = INVALID_SOCKET;

	client_socket = accept(listen_socket, nullptr, nullptr);
	if (client_socket == INVALID_SOCKET) {
		std::cerr << "" << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		return;
	}

	std::cout << "Client connected" << std::endl;

	std::string command = recieve_command(client_socket);
	std::cout << "Recieve command from client: " << command << std::endl;

	std::string result = execute_command(command);
	std::cout << "Command executed: " << result << std::endl;

	if (!send_result(client_socket, result)) {
		std::cerr << "Error in sending result to client: " << std::endl;
	}

	closesocket(client_socket);
	std::cout << "Client socker closed";
	return;
}

std::string Server::recieve_command(SOCKET socket) {
	std::string result;
	char buf[DEFAULT_BUF_LEN]; // default: 512
	int iResult;

	do {
		iResult = recv(socket, buf, sizeof(buf), 0);
		if (iResult) {
			std::cout << iResult << " bytes recieved from client" << std::endl;
			result.append(buf, iResult);
		} else if (iResult == 0) {
			std::cout << "Connection closed" << std::endl;
			break;
		}
		else {
			std::cerr << "Error in recieving result " << WSAGetLastError() << std::endl;
			break;
		}
	} while (iResult);

	return result;
}

std::string Server::execute_command(const std::string& command) {
	std::string result;
	// TODO
}

bool Server::send_result(SOCKET socket, const std::string& result) {
	int iResult = send(socket, result.c_str(), result.length(), 0);
	if (iResult) {
		std::cout << iResult << " bytes sended to client" << std::endl;
		closesocket(socket);
		return true;
	}
	else {
		std::cerr << "Error in sendinh result " << WSAGetLastError() << std::endl;
		closesocket(socket);
		return false;
	}
}