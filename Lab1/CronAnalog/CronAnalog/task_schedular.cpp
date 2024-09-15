#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <Windows.h>

// maybe change to std::optional?
struct ScheduleTask {
	int minute = -1;
	int hour = -1;
	int day_of_month = -1;
	int month = -1;
	int day_of_week = -1;
	std::string executable;
	std::vector<std::string> arguments;

	bool operator==(const ScheduleTask& other) const {
		return minute == other.minute &&
			hour == other.hour &&
			day_of_month == other.day_of_month &&
			month == other.month &&
			day_of_week == other.day_of_week &&
			executable == other.executable &&
			arguments == other.arguments;
	}
};

class TaskSchedular {

public:

	TaskSchedular() = default;

	TaskSchedular(const TaskSchedular&) = delete;

	TaskSchedular& operator=(const TaskSchedular&) = delete;

	static std::vector<ScheduleTask> load_schedule_from_file(const std::string& file_path) {
		std::fstream input_file(file_path, std::ios::in | std::ios::out);
		if (!input_file.is_open()) {
			std::cerr << "Error in opening input file" << std::endl;
			return std::vector<ScheduleTask>();
		}

		std::vector<ScheduleTask> tasks;
		std::string line;

		while (std::getline(input_file, line)) {
			parse_schedule_string(line, tasks);
		}

		return tasks;
	}

	void run_schedule_tasks(const std::vector<ScheduleTask>& tasks) {
		SYSTEMTIME current_time;
		GetLocalTime(&current_time);

		/*for (int i = 0; i < tasks.size(); i++) {
			std::cout << tasks[i].minute << " " << tasks[i].hour << " " << tasks[i].executable << std::endl;
		}*/

		for (auto task : tasks) {

			if (should_run_task(task, current_time)) {
				HANDLE hTread = CreateThread(
					NULL,
					0,
					TaskThread,
					&task,
					0,
					NULL
				);

				if (hTread != NULL) {
					running_tasks.push_back(task);

					get_thread_info(hTread);

					WaitForSingleObject(hTread, INFINITE);
					CloseHandle(hTread);
				}

				//if(run_task(task.executable, task.arguments))
				//	running_tasks.push_back(task);
				//run_limited_tasks(tasks);
			}
		}
	}

private:
	std::vector<ScheduleTask> running_tasks;	

	void run_limited_tasks(const std::vector<ScheduleTask>& tasks) {
		SYSTEMTIME current_time;
		GetLocalTime(&current_time);

		for (ScheduleTask task : tasks) {
			if (running_threads.size() >= max_threads) {
				WaitForMultipleObjects(running_threads.size(), running_threads.data(), TRUE, INFINITE);
				running_threads.clear();
			}

			if (should_run_task(task, current_time)) {
				HANDLE hThread = CreateThread(
					NULL,
					0,
					TaskThread,
					&task,
					0,
					NULL);

				if (hThread != NULL) {
					running_tasks.push_back(task);
					running_threads.push_back(hThread);
				}
				else {
					CloseHandle(hThread);
				}
			}
		}
	}
	
	bool should_run_task(const ScheduleTask& task, const SYSTEMTIME& current_time) {
		auto it = std::find(running_tasks.begin(), running_tasks.end(), task);

		if (it != running_tasks.end())
		{
			auto el = *it;
			if (el.day_of_month == -1) return true;
			if (el.minute == -1) return true;
			if (el.hour == -1) return true;
			if (el.day_of_week == -1) return true;
			if (el.month == -1) return true;
			return false;
		}

		if (task.minute != -1 && task.minute != current_time.wMinute) return false;
		if (task.hour != current_time.wHour && task.hour != -1) return false;
		if (task.day_of_month != current_time.wDay && task.day_of_month != -1) return false;
		if (task.month != current_time.wMonth && task.month != -1) return false;
		if (task.day_of_week != current_time.wDayOfWeek && task.day_of_week != -1) return false;

		return true;
	}

	static void log_error(const std::string& message) {
		std::ofstream log_file("errors.log", std::ios::app | std::ios::out);
		if (log_file.is_open()) {
			log_file << message << std::endl;
			log_file.close();
		}
	}

	static bool parse_schedule_string(const std::string& line, std::vector<ScheduleTask>& tasks) {
		std::istringstream iss(line); // separate words by ' '
		std::string arg;

		ScheduleTask task;

		std::vector<ScheduleTask> new_tasks; // if */5

		int field = 0;

		while (iss >> arg) {

			if (field < 5) {
				if (arg == "*") {
					switch (field) {
					case 0:
					{
						task.minute = -1;
						break;
					}
					case 1: {
						task.hour = -1;
						break;
					}
					case 2: {
						task.day_of_month = -1;
						break;
					}
					case 3: {
						task.month = -1;
						break;
					}
					case 4: {
						task.day_of_week = -1;
						break;
					}
					default: break;
					}
				}
				/*else if (arg.find_first_of(",") != std::string::npos) {
					size_t first_pos = arg.find_first_of(",");
					size_t last_pos = arg.find_last_of(",");

					std::cout << "arg" << arg;

					std::vector<int> args;
					size_t pos = 0;
					std::string arg_copy = arg;
					std::string token;

					while ((pos = arg_copy.find(',')) != std::string::npos) {
						token = arg_copy.substr(0, pos);
						args.push_back(std::stoi(token));  
						arg_copy.erase(0, pos + 1);
					}

					args.push_back(std::stoi(arg_copy));

					for (int i = 0; i < args.size(); i++) {
						std::cout << args[i] << " ";
					}

					switch (field) {
					case 0: {
						for (int i = 0; i < args.size(); i++) {
							ScheduleTask new_task = task;
							new_task.minute = args[i];
							new_tasks.push_back(new_task);
						}
						break;
					}
					case 1: {
						for (int i = 0; i < args.size(); i++) {
							ScheduleTask new_task = task;
							new_task.hour = args[i];
							new_tasks.push_back(new_task);
						}
						break;
					}
					case 2: {
						for (int i = 0; i < args.size(); i++) {
							ScheduleTask new_task = task;
							new_task.day_of_month = args[i];
							new_tasks.push_back(new_task);
						}
						break;
					}
					case 3: {
						for (int i = 0; i < args.size(); i++) {
							ScheduleTask new_task = task;
							new_task.month = args[i];
							new_tasks.push_back(new_task);
						}
						break;
					}
					case 4: {
						for (int i = 0; i < args.size(); i++) {
							ScheduleTask new_task = task;
							new_task.day_of_week = args[i];
							new_tasks.push_back(new_task);
						}
						break;
					}
					default:
						break;
					}
				}*/
				else if (arg.find_first_of("/") != std::string::npos) {
					size_t pos = arg.find_first_of("/");
					int step = std::stoi(arg.substr(pos + 1));
					int value = arg.substr(0, pos) == "*" ? 0 : std::stoi(arg.substr(0, pos));

					switch (field) {
						case 0: { // Minute
							for (int i = value; i < 60; i += step) {
								ScheduleTask new_task = task;
								new_task.minute = i;
								new_tasks.push_back(new_task);
							}
							break;
						}
						case 1: { // Hour
							for (int i = value; i < 24; i += step) {
								ScheduleTask new_task = task;
								new_task.hour = i;
								new_tasks.push_back(new_task);
							}
							break;
						}
						case 2: { // Day of month
							for (int i = value; i <= 31; i += step) {
								ScheduleTask new_task = task;
								new_task.day_of_month = i;
								new_tasks.push_back(new_task);
							}
							break;
						}
						case 3: { // Month
							for (int i = value; i <= 12; i += step) {
								ScheduleTask new_task = task;
								new_task.month = i;
								new_tasks.push_back(new_task);
							}
							break;
						}
						case 4: { // Day of week
							for (int i = value; i < 7; i += step) {
								ScheduleTask new_task = task;
								new_task.day_of_week = i + 1;
								new_tasks.push_back(new_task);
							}
							break;
						}
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
					if (!new_tasks.empty()) {
						for (ScheduleTask& task : new_tasks) {
							task.executable = arg;
						}
					} else
						task.executable = arg;
				}
				else {
					if (!new_tasks.empty()) {
						for (ScheduleTask& task : new_tasks) {
							task.arguments.push_back(arg);
						}
					} else
						task.arguments.push_back(arg);
				}
			}
			field++;
		}

		if (!new_tasks.empty()) {
			for (ScheduleTask& t : new_tasks) {
				tasks.push_back(t);
			}
		}
		else {
			tasks.push_back(task);
		}

		return true;
	}

	static bool run_task(const std::string& executable, const std::vector<std::string>& args) {
		std::string command = executable;
		for (std::string arg : args) {
			command += " " + arg;
		}


		STARTUPINFO startup_info = { sizeof(startup_info) };
		// информация о процессе
		PROCESS_INFORMATION process_info;

		auto parsed = stringToLPWSTR(command);

		if (!CreateProcess(NULL, parsed, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info)) {
			std::string error_message = "Ошибка запуска процесса: " + std::to_string(GetLastError());
			std::cerr << error_message << std::endl;
			log_error(error_message);
			return false;
		}
		else {
			std::cout << "Launching process " << std::endl;
			//set_process_priority(curr_proccess, HIGH_PRIORITY_CLASS);

			get_process_info(process_info.hProcess);

			// ожидание завершения задачи
			WaitForSingleObject(process_info.hProcess, INFINITE);

			DWORD exit_code;
			GetExitCodeProcess(process_info.hProcess, &exit_code);
			if (exit_code != 0) {
				std::string error_message = "Процесс завершился с ошибкой. Код: " + std::to_string(exit_code);
				std::cerr << error_message << std::endl;
				log_error(error_message);
			}

			// закрытие дескрипторов
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);

			return true;
		}
	}

	void terminate_process(HANDLE hProcess) {
		TerminateProcess(hProcess, 0);
		get_process_info(hProcess);
	}

	static DWORD WINAPI TaskThread(LPVOID lpParam) {
		ScheduleTask* task = static_cast<ScheduleTask*>(lpParam);

		if (!run_task(task->executable, task->arguments)) {
			std::string error_message = "Ошибка произошла с кодом: " + std::to_string(GetLastError());
			std::cerr << error_message << std::endl;
			log_error(error_message);
		}

		return 0;
	}

	static void set_process_priority(HANDLE hProcess, DWORD priority) {
		SetPriorityClass(hProcess, priority);
	}

	static void set_thread_priority(HANDLE hTread, DWORD priority) {
		SetThreadPriority(hTread, priority);
	}

	static void get_process_info(HANDLE hProcess) {
		FILETIME creationTime, exitTime, kernelTime, userTime;
		SYSTEMTIME systemTime;
		int process_id = GetProcessId(hProcess);
		int priority = GetPriorityClass(hProcess);

		if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
			FileTimeToSystemTime(&creationTime, &systemTime);
			std::cout << "-------------------------------------------" << std::endl;
			std::cout << "Process id: " << process_id << std::endl;
			std::cout << "Process creation time: " << systemTime.wHour << ":" << systemTime.wMinute << ":" << systemTime.wSecond << std::endl;

			/*FileTimeToSystemTime(&exitTime, &systemTime);
			std::cout << "Process exit time: " << systemTime.wHour << ":" << systemTime.wMinute << ":" << systemTime.wSecond << std::endl;*/

			std::cout << "Process priority: " << priority << std::endl;
			std::cout << "-------------------------------------------" << std::endl;
		}
	}

	static void get_thread_info(HANDLE hThread) {
		DWORD threadId = GetThreadId(hThread);
		FILETIME creationTime, exitTime, kernelTime, userTime;
		SYSTEMTIME systemTime;
		int priority = GetThreadPriority(hThread);

		if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime)) {
			FileTimeToSystemTime(&creationTime, &systemTime);
			std::cout << "-------------------------------------------" << std::endl;
			std::cout << "Thread ID: " << threadId << std::endl;
			std::cout << "Thread creation time: " << systemTime.wHour << ":" << systemTime.wMinute << ":" << systemTime.wSecond << std::endl;

			//FileTimeToSystemTime(&exitTime, &systemTime);
			//std::cout << "Thread exit time: " << systemTime.wHour << ":" << systemTime.wMinute << ":" << systemTime.wSecond << std::endl;

			std::cout << "Thread priority: " << priority << std::endl;
			std::cout << "-------------------------------------------" << std::endl;
		}
	}

	static LPWSTR stringToLPWSTR(const std::string& value)
	{
		int wchars_num = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, NULL, 0);
		LPWSTR buffer = new WCHAR[wchars_num];
		MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, buffer, wchars_num);
		return buffer;
	}

private:
	int max_threads = 7;
	std::vector<HANDLE> running_threads;
};