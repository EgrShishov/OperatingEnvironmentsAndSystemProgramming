#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <Windows.h>

struct ScheduleTask {
	int minute = -1;
	int hour = -1;
	int day_of_month = -1;
	int month = -1;
	int day_of_week = -1;
	std::string executable;
	std::vector<std::string> arguments;
};

bool parse_schedule_file(const std::string& file_path) {
	std::fstream input_file(file_path, std::ios::in | std::ios::out);
	if (input_file.is_open()) {
		std::cerr << "Error in opening input file" << std::endl;
		return;
	}

	std::string line;

	while (std::getline(input_file, line)) {
		ScheduleTask task;
		std::istringstream iss(line); // separate words by ' '
		std::string arg;

		int field = 0;

		while (iss >> arg) {

			if (field < 5) {
				if (arg == "*") {
					switch (field) {
					case 0: task.minute = -1; break;
					case 1: task.hour = -1; break;
					case 2: task.day_of_month = -1; break;
					case 3: task.month = -1; break;
					case 4: task.day_of_week = -1; break;
					default: break;
					}
				}
				else if (arg.find_first_of("/") != std::string::npos) {
					size_t pos = arg.find_first_of("/");
					int step = std::stoi(arg.substr(pos + 1));
					int value = arg.substr(0, pos) == "*" ? -1 : std::stoi(arg.substr(0, pos));


					switch (field) {
					case 0: task.minute = (value +
						std::chrono::system_clock::now().time_since_epoch().count() / std::chrono::minutes(1).count()) % step;
						break;
					case 1: task.hour = (value +
						std::chrono::steady_clock::now().time_since_epoch().count() / std::chrono::hours(24).count() / 7) % step;
						break;
					case 2: task.day_of_month = (std::chrono::system_clock::now().time_since_epoch().count() +
						value * std::chrono::hours(24).count()) % step;
						break;
					case 3: task.month = (value
						);
						break;
					case 4: task.day_of_week = (value
						);
						break;
					default: break;
					}
				}
				else {
					int value = std::stoi(arg);

					switch (field) {
					case 0:
						task.minute = value;
						break;
					case 1:
						task.hour = value;
						break;
					case 2:
						task.day_of_month = value;
						break;
					case 3:
						task.month = value;
						break;
					case 4:
						task.day_of_week = value;
						break;
					default: break;
					}
				}
			}
			// parametrs and args 
			else {
				if (field == 5) {
					task.executable = arg;
				}
				else {
					task.arguments.push_back(arg);
				}
			}
			field++;
		}
	}
	return true;
}

void run_schedule_tasks(ScheduleTask& task) {

}