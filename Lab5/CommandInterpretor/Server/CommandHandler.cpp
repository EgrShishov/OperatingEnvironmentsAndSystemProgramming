#include "CommandHandler.h"

/* basic acceptable commands
� ls ���������� ���������� �������� (������ �������� ������);
� cd ������ ������� (������� � ������);
� mkdir ������ ����� ����� (�������);
� touch ������ ����� ����;
� rm ������� ����;
� cat ���������� ���������� �����;
� pwd ���������� ������� ������� (������ ���� � ����� ��������);
� cp �������� ����/�����;
� mv ���������� ����/�����;
� grep ���� ���������� ����� � �����;
� find ���� ������ � �����;
� vi \ nano ��������� ��������� ���������;
� history ���������� 50 ��������� �������������� ������;
� clear ������� ���� ���������.
*/

CommandHandler::CommandHandler() {

}

CommandHandler::~CommandHandler() {

}

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
		return execute_pwd(args);
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
		return execute_history(args);
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
	// TODO
}

std::string execute_cd(const std::vector<std::string>& args) {

}

std::string execute_mkdir(const std::vector<std::string>& args) {

}

std::string execute_touch(const std::vector<std::string>& args) {

}

std::string execute_rm(const std::vector<std::string>& args) {

}

std::string execute_cat(const std::vector<std::string>& args) {

}

std::string execute_pwd(const std::vector<std::string>& args) {

}

std::string execute_cp(const std::vector<std::string>& args) {

}

std::string execute_mv(const std::vector<std::string>& args) {

}

std::string execute_grep(const std::vector<std::string>& args) {

}

std::string execute_find(const std::vector<std::string>& args) {

}

std::string execute_vi(const std::vector<std::string>& args) {

}

std::string execute_history(const std::vector<std::string>& args) {

}

std::string execute_clear() {

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
