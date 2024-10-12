#include "Server.h"

SOCKET listen_socket = INVALID_SOCKET;
CommandHandler command_handler = CommandHandler();
int server_port = DEFAULT_PORT;

bool start() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error in binding listening socket: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }

    iResult = listen(listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error in listening through socket: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }

    std::cout << "Server is listening on port " << server_port << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listen_socket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            closesocket(listen_socket);
            WSACleanup();
            return false;
        }

        std::cout << "Client connected!" << std::endl;

        HANDLE hThread = CreateThread(
            NULL,
            0,
            handle_client,
            new SOCKET(clientSocket),
            0,
            NULL);

        if (hThread == NULL) {
            std::cerr << "CreateThread failed: " << GetLastError() << std::endl;
            closesocket(clientSocket);
            continue;
        }

        CloseHandle(hThread);
    }

    return true;
}

std::string receive_command(SOCKET socket) {
    std::string result;
    char buf[DEFAULT_BUF_LEN]; // default: 512
    int iResult;

    iResult = recv(socket, buf, sizeof(buf), 0);
    if (iResult > 0) {
        //std::cout << iResult << " bytes received from client" << std::endl;
        result.append(buf, iResult);
    } else if (iResult == 0) {
        //std::cout << "Connection closed by client." << std::endl;
    } else {
        std::cerr << "Error in receiving data: " << WSAGetLastError() << std::endl;
    }

    return result;
}

DWORD WINAPI handle_client(LPVOID clientSocketPtr) {
    SOCKET socket = *reinterpret_cast<SOCKET*>(clientSocketPtr);
    delete reinterpret_cast<SOCKET*>(clientSocketPtr);

    while (true) {
        std::string command = receive_command(socket);
        if (command.empty()) {
            break;
        }

       // std::cout << "Received command from client: " << command << std::endl;

        std::string result = execute_command(command);
       // std::cout << "Command executed: " << result << std::endl;

        if (!send_result(socket, result)) {
            std::cerr << "Error in sending result to client." << std::endl;
            break;
        }
    }

    closesocket(socket);
    return 0;
}

std::string execute_command(const std::string& command) {
    return command_handler.handle_command(command);
}

bool send_result(SOCKET socket, const std::string& result) {
    int iResult = send(socket, result.c_str(), result.length(), 0);
    if (iResult != SOCKET_ERROR) {
       // std::cout << iResult << " bytes sent to client" << std::endl;
        return true;
    }
    else {
        std::cerr << "Error in sending result " << WSAGetLastError() << std::endl;
        return false;
    }
}