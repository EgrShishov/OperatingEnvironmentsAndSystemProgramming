#include "CommandHandler.h"
#include <iostream>

CommandHandler::CommandHandler() {

}

CommandHandler::~CommandHandler() {

}
/*
1. ls
    ������: ls [directory]
    ���������:
        directory (��������������): ���� � ��������, ���������� �������� ����� ����������. ���� �� ������, ������������ ������� �������.
    ������:
        ls � ���������� ���������� �������� ��������.
        ls C:\Users � ���������� ���������� �������� C:\Users.
    ����������: ���������� ������ ������ � ����� � ��������� ��������.

2. cd

    ������: cd <directory>
    ���������:
        directory: ���� � ��������, � ������� ����� �������.
    ������:
        cd C:\Users\John � ��������� � ������� C:\Users\John.
    ����������: �������� ������� ������� ������� �� ���������.

3. mkdir

    ������: mkdir <directory>
    ���������:
        directory: ��� ��� ���� ������ ��������, ������� ����� �������.
    ������:
        mkdir NewFolder � ������� ����� ������� � ������ NewFolder � ������� ��������.
        mkdir C:\Users\John\NewFolder � ������� ������� �� ���������� ����.
    ����������: ������� ����� �������.

4. touch

    ������: touch <filename>
    ���������:
        filename: ��� �����, ������� ����� ������� ��� ��������.
    ������:
        touch newfile.txt � ������� ����� ���� � ������ newfile.txt � ������� ��������.
    ����������: ������� ����� ���� ��� ��������� ����� ���������� ������� � ������������� �����.

5. rm

    ������: rm <filename>
    ���������:
        filename: ��� �����, ������� ����� �������.
    ������:
        rm oldfile.txt � ������� ���� oldfile.txt �� �������� ��������.
    ����������: ������� ��������� ����.

6. cat

    ������: cat <filename>
    ���������:
        filename: ��� �����, ���������� �������� ����� ����������.
    ������:
        cat myfile.txt � ���������� ���������� ����� myfile.txt.
    ����������: ������ � ���������� ���������� ���������� �����.

7. pwd

    ������: pwd
    ���������: ���.
    ������:
        pwd � ������� ������ ���� � �������� �������� ��������.
    ����������: �������� ������� ������� �������.

8. cp

    ������: cp <source> <destination>
    ���������:
        source: ���� � ����� ��� ��������, ������� ����� �����������.
        destination: ����, ���� ����� ����������� ���� ��� �������.
    ������:
        cp myfile.txt backup/myfile.txt � �������� ���� myfile.txt � ����� backup.
    ����������: �������� ���� ��� �������.

9. mv

    ������: mv <source> <destination>
    ���������:
        source: ���� � ����� ��� ��������, ������� ����� �����������.
        destination: ����, ���� ����� ����������� ���� ��� �������.
    ������:
        mv myfile.txt newfolder/myfile.txt � ���������� ���� myfile.txt � ����� newfolder.
    ����������: ���������� ���� ��� �������.

10. grep

    ������: grep <pattern> <filename>
    ���������:
        pattern: ������ (������), ������� ����� ����� � �����.
        filename: ��� �����, � ������� ����� ��������� �����.
    ������:
        grep "text" myfile.txt � ���� ������ "text" � ����� myfile.txt.
    ����������: ������� ������, ���������� ��������� ������ � �����.

11. find

    ������: find <name> [path]
    ���������:
        name: ��� ����� ��� �����, ������� ����� �����.
        path (��������������): ����, � ������� ����� ��������� �����. ���� �� ������, ����� ����������� � ������� ��������.
    ������:
        find myfile.txt � ���� ���� myfile.txt � ������� ��������.
        find myfile.txt C:\Users � ���� ���� myfile.txt � �������� C:\Users.
    ����������: ���� ����� ��� �������� �� ����� � ��������� ����.

12. vi / nano

    ������: vi <filename> ��� nano <filename>
    ���������:
        filename: ��� �����, ������� ����� ������� � ��������� ���������.
    ������:
        vi myfile.txt ��� nano myfile.txt � ��������� ���� myfile.txt � ��������� ���������.
    ����������: ��������� ��������� ���� � ��������� ��������� (���� ����������).

13. history

    ������: history
    ���������: ���.
    ������:
        history � ���������� ������ ��������� 50 ������.
    ����������: ���������� ������� ����������� ������.

14. clear

    ������: clear
    ���������: ���.
    ������:
        clear � ������� ��������.
    ����������: ������� ����� � ���������.*/

std::string CommandHandler::handle_command(const std::string& command) {
	std::string result;

    auto parsing_result = parse_command(command);
    if (parsing_result.first.empty()) {
        return "";
    }

    result = execute_command(parsing_result.first, parsing_result.second);

	return result;
}

std::string CommandHandler::execute_command(const std::string& command, const std::vector<std::string>& args) {
	std::string exec_result;

	if (command == "ls") {
		return execute_ls(args);
	}
	else if (command == "cd") {
		return execute_cd(args);
	}
	else if (command == "mkdir") {
		return execute_mkdir(args);
	}
	else if (command == "touch") {
		return execute_touch(args);
	}
	else if (command == "rm") {
		return execute_rm(args);
	}
	else if (command == "cat") {
		return execute_cat(args);
	}
	else if (command == "pwd") {
		return execute_pwd();
	}
	else if (command == "cp") {
		return execute_cp(args);
	}
	else if (command == "mv") {
		return execute_mv(args);
	}
	else if (command == "grep") {
		return execute_grep(args);
	}
	else if (command == "find") {
		return execute_find(args);
	}
	else if (command == "vi") {
		return execute_vi(args);
	}
	else if (command == "history") {
		return execute_history();
	}
	else if (command == "clear") {
		return execute_clear();
	}
	else {
		return "Unknown command\n";
	}

	return exec_result;
}

std::string CommandHandler::execute_ls(const std::vector<std::string>& args) {
    std::string dir;

    if (args.empty() || args[0] == ".") {
        char buffer[MAX_PATH];
        if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
            dir = buffer;
        }
        else {
            return "Unable to get current directory";
        }
    }
    else {
        dir = args[0];
    }

    dir += "\\*";

    WIN32_FIND_DATA findFileData;
    LPWSTR root = convert_string_to_LPWSTR(dir);
    HANDLE hFile = FindFirstFile(root, &findFileData);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << root;
        return "Directory not found";
    }

    std::string exec_result;
    
    do {
        std::string fileName = WCHAR_to_string(findFileData.cFileName);
        if (fileName != "." && fileName != "..") {
            exec_result += fileName + "\n";
        }
    } while (FindNextFile(hFile, &findFileData));
    
    CloseHandle(hFile);

    return exec_result;
}

std::string CommandHandler::execute_cd(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No directory specified";
    }

    if (SetCurrentDirectory(convert_string_to_LPWSTR(args[0]))) {
        char cur_dir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, cur_dir);
        return std::string(cur_dir) + "> ";
    } else {
        return "Failed to change directory: " + GetLastError();
    }
}

std::string CommandHandler::execute_mkdir(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No directory name specified";
    }

    if (CreateDirectory(convert_string_to_LPWSTR(args[0]), nullptr)) {
        return "Directory created";
    } else {
        return "Failed to create directory" + GetLastError() + '\n';
    }
}

std::string CommandHandler::execute_touch(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specified";
    }

    HANDLE hFile = CreateFile(
        convert_string_to_LPWSTR(args[0]),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        return "File created\n";
    } else {
        return "Failed to create file or file already exists";
    }
}

std::string CommandHandler::execute_rm(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specified";
    }

    if (DeleteFile(convert_string_to_LPWSTR(args[0]))) {
        return "File deleted";
    } else {
        return "Failed to delete file: " + GetLastError();
    }
}

std::string get_error_message(DWORD error_code) {
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        0, NULL);

    std::wstring message_wstr = (LPWSTR)lpMsgBuf;
    std::string message(message_wstr.begin(), message_wstr.end());
    LocalFree(lpMsgBuf);

    return message;
}

std::string CommandHandler::execute_cat(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specified";
    }

    HANDLE hFile = CreateFile(
        convert_string_to_LPWSTR(args[0]),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error_code = GetLastError();
        return "Cannot open file: " + get_error_message(error_code);
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        DWORD error_code = GetLastError();
        CloseHandle(hFile);
        return "Error getting file size: " + get_error_message(error_code);
    }
    if (fileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return "File is empty";
    }

    char* buffer = new char[fileSize.QuadPart + 1];
    DWORD bytes_readed = 0;

    if (!ReadFile(hFile, buffer, fileSize.QuadPart, &bytes_readed, nullptr)) {
        DWORD error_code = GetLastError();
        CloseHandle(hFile);
        delete[] buffer;
        return "Error reading file: " + get_error_message(error_code);
    }

    CloseHandle(hFile);
    buffer[bytes_readed] = '\0';
    std::string file_content(buffer);
    delete[] buffer;

    return file_content;
}

std::string CommandHandler::execute_pwd() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
        return std::string(buffer);
    } else {
        return "Error in getting current directory: " + GetLastError();
    }
}

std::string CommandHandler::execute_cp(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No input or output file specifed";
    }
    std::string command = "cp " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string CommandHandler::execute_mv(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No source or destination dir specifed";
    }
    std::string command = "mv " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string CommandHandler::execute_grep(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No file or pattern specifed";
    }
    std::string command = "grep " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string CommandHandler::execute_find(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename to find specifed";
    }
    std::string command = "find " + args[0];
    return execute_system_command(command);
}

std::string CommandHandler::execute_vi(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specifed";
    }
    std::string command = "vi " + args[0];
    return execute_system_command(command);
}

std::string CommandHandler::execute_history() {
    std::string command = "history";
    return execute_system_command(command);
}

std::string CommandHandler::execute_clear() {
    std::string command = "cls";
    return execute_system_command(command);
}

std::string CommandHandler::execute_system_command(const std::string& command) {
	std::string result = command + " > temp_output.txt";
	system(result.c_str());

	std::ifstream file("temp_output.txt");
	std::string output((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	remove("temp_output.txt");
	return output;
}

std::pair<std::string, std::vector<std::string>> CommandHandler::parse_command(const std::string& input) {
	std::istringstream iss(input);

	std::string command;
	iss >> command;

	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg) {
		args.push_back(arg);
	}

	return { command, args };
}


LPWSTR CommandHandler::convert_string_to_LPWSTR(const std::string& input) {
    int length = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL, 0);
    wchar_t* wideStr = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, wideStr, length);
    LPWSTR lpwstr = wideStr;
    return lpwstr;
}

std::string CommandHandler::WCHAR_to_string(const wchar_t* wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);

    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, nullptr, nullptr);
    return result;
}