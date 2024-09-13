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

class TaskSchedular {

public:
	TaskSchedular() {

	}

	~TaskSchedular() {

	}

	std::vector<ScheduleTask> load_schedule_from_file(const std::string& file_path) {
		std::fstream input_file(file_path, std::ios::in | std::ios::out);
		if (input_file.is_open()) {
			std::cerr << "Error in opening input file" << std::endl;
			return;
		}

		std::vector<ScheduleTask> tasks;
		std::string line;

		while (std::getline(input_file, line)) {
			ScheduleTask task;
			if (parse_schedule_string(line, task))
				tasks.push_back(task);
		}

		return tasks;
	}

	void run_schedule_tasks(const std::vector<ScheduleTask>& tasks) {
		SYSTEMTIME current_time;
		GetLocalTime(&current_time);

		for (auto task : tasks) {

			if (should_run_task(task, current_time)) {
				std::cout << "launching task" << " " << task.executable << std::endl;
				run_task(task.executable, task.arguments);
			}
		}
	}

private:

	bool should_run_task(const ScheduleTask& task, const SYSTEMTIME& current_time) {
		if (task.minute != current_time.wMinute && task.minute != -1) return false;
		if (task.hour != current_time.wHour && task.hour != -1) return false;
		if (task.day_of_month != current_time.wDay && task.day_of_month != -1) return false;
		if (task.month != current_time.wMonth && task.month != -1) return false;
		if (task.day_of_week != current_time.wDayOfWeek && task.day_of_week != -1) return false;
	}

	bool parse_schedule_string(const std::string& line, ScheduleTask& task) {
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
		return true;
	}

	bool run_task(const std::string& executable, const std::vector<std::string>& args) {
		std::string command = executable;
		for (std::string arg : args) {
			command += " " + arg;
		}

		STARTUPINFO startup_info = { sizeof(startup_info) };
		PPROCESS_INFORMATION process_info;

		if (!CreateProcess(NULL, stringToLPWSTR(command), NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info, )) {
			std::cerr << "Ошибка запуска процесса: " << GetLastError() << std::endl;
		}
		else {
			WaitForSingleObject(process_info->hProcess, INFINITE);
			CloseHandle(process_info->hProcess);
			CloseHandle(process_info->hThread);
		}
	}

	//from ethernet
	std::wstring stringToWideString(const std::string& str)
	{
		int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
		std::vector<wchar_t> buffer(length);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), &buffer[0], length);
		return std::wstring(buffer.begin(), buffer.end());
	}

	LPWSTR stringToLPWSTR(const std::string& str)
	{
		std::wstring wideStr = stringToWideString(str);
		wchar_t* lpwstr = const_cast<wchar_t*>(wideStr.c_str());
		return lpwstr;
	}
};