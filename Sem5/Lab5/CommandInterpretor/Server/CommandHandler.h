#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <locale>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <codecvt>
#include <array>

class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();
	
	std::string handle_command(const std::string& command);
private:
	std::string execute_command(const std::string& command, const std::vector<std::string>& args);
	std::pair<std::string, std::vector<std::string>> parse_command(const std::string& command);
	std::string execute_ls(const std::vector<std::string>& args);
	std::string execute_mkdir(const std::vector<std::string>& args);
	std::string execute_touch(const std::vector<std::string>& args);
	std::string execute_cat(const std::vector<std::string>& args);
	std::string execute_grep(const std::vector<std::string>& args);
	std::string execute_rm(const std::vector<std::string>& args);
	std::string execute_cd(const std::vector<std::string>& args);
	std::string execute_pwd();
	std::string execute_cp(const std::vector<std::string>& args);
	std::string execute_mv(const std::vector<std::string>& args);
	std::string execute_find(const std::vector<std::string>& args);
	std::string execute_vi(const std::vector<std::string>& args);
	std::string execute_system_command(const std::string& command);
	std::string execute_history();
	std::string execute_clear();
	std::string WCHAR_to_string(const wchar_t* wstr);
	LPWSTR convert_string_to_LPWSTR(const std::string& input);

	std::vector<std::string> command_history;
};

