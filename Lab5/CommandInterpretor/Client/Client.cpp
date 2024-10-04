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

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//iResult = getaddrinfo(, DEFAULT_SERVER_PORT, &hints, nullptr);
	//if (iResult != 0) {
	//	std::cerr << "Getaddrinfo failed: " << iResult << std::endl;
	//	return false;
	//}

	iResult = connect(connect_socket, hints.ai_addr, (int)hints.ai_addrlen);

	//freeaddrinfo(hints);

	if (iResult == SOCKET_ERROR) {
		std::cerr << "Socket error: " << iResult << std::endl;
		closesocket(connect_socket);
		WSACleanup();
		return false;
	}
}

bool Client::send_command(const std::string& command) {
	int iResult = send(connect_socket, command.c_str(), command.length(), 0);
	if (iResult) {
		std::cout << iResult << " bytes sended to server" << std::endl;
		return true;
	}
	else {
		std::cerr << "Error in sendinh result " << WSAGetLastError() << std::endl;
		return false;
	}
}

std::string Client::recieve_result() {
	std::string result;
	char buf[DEFAULT_BUF_LEN];
	int iResult;

	do {
		iResult = recv(connect_socket, buf, sizeof(buf), 0);
		if (iResult) {
			std::cout << iResult << " bytes recieved from server" << std::endl;
			result.append(buf, iResult);
		}
		else if (iResult == 0) {
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