#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();
	
	std::string handle_command(const std::string& command);
private:
	std::string execute_command(const std::string& command, const std::vector<std::string>& args);
	std::pair<std::string, std::vector<std::string>> parse_command(const std::string& command);
};

