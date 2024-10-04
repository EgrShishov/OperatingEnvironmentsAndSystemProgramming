#include "CommandHandler.h"

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

std::string execute_ls(const std::vector<std::string>& args) {
	std::string dir = args.empty() ? "." : args[0];
    WIN32_FIND_DATA findFileData;
    HANDLE hFile = FindFirstFile(convert_string_to_LPWSTR(dir + "\\*"), &findFileData);

    if (hFile == INVALID_HANDLE_VALUE) {
        return "Directory not found";
    }

    std::string exec_result;
    do {
        exec_result += WCHAR_to_string(findFileData.cFileName) + "\n";
    } while (FindNextFile(hFile, &findFileData));
    return exec_result;
}

std::string execute_cd(const std::vector<std::string>& args) {
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

std::string execute_mkdir(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No directory name specified";
    }

    if (CreateDirectory(convert_string_to_LPWSTR(args[0]), nullptr)) {
        return "Directory created";
    } else {
        return "Failed to create directory" + GetLastError() + '\n';
    }
}

std::string execute_touch(const std::vector<std::string>& args) {
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

std::string execute_rm(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specified";
    }

    if (DeleteFile(convert_string_to_LPWSTR(args[0]))) {
        return "File deleted";
    } else {
        return "Failed to delete file: " + GetLastError();
    }
}

std::string execute_cat(const std::vector<std::string>& args) {
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
        return "Cannot open file: " + GetLastError();
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    char* buffer = new char[fileSize.QuadPart];
    DWORD bytes_readed;

    if (!ReadFile(hFile, buffer, sizeof(buffer), &bytes_readed, nullptr)) {
        CloseHandle(hFile);
        return "Error in reading file: " + GetLastError();
    }

    CloseHandle(hFile);
    return std::string(buffer);
}

std::string execute_pwd() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
        return std::string(buffer);
    } else {
        return "Error in getting current directory: " + GetLastError();
    }
}

std::string execute_cp(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No input or output file specifed";
    }
    std::string command = "cp " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string execute_mv(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No source or destination dir specifed";
    }
    std::string command = "mv " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string execute_grep(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No file or pattern specifed";
    }
    std::string command = "grep " + args[0] + " " + args[1];
    return execute_system_command(command);
}

std::string execute_find(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename to find specifed";
    }
    std::string command = "find " + args[0];
    return execute_system_command(command);
}

std::string execute_vi(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "No filename specifed";
    }
    std::string command = "vi " + args[0];
    return execute_system_command(command);
}

std::string execute_history() {
    std::string command = "history";
    return execute_system_command(command);
}

std::string execute_clear() {
    std::string command = "cls";
    return execute_system_command(command);
}

std::string execute_system_command(const std::string& command) {
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

LPWSTR convert_string_to_LPWSTR(const std::string& input) {
    int size = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> buffer(size);

    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, buffer.data(), size);
    return buffer.data();
}

std::string WCHAR_to_string(const wchar_t* wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}