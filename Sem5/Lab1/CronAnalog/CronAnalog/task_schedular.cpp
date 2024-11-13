#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <regex>
#include <functional>
#include <Windows.h>

// maybe change to std::optional?
struct ScheduleTask {
	int minute = 0;
	int hour = 0;
	int day_of_month = 0;
	int month = 0;
	std::string executable;
	std::vector<std::string> arguments;

	bool operator==(const ScheduleTask& other) const {
		return minute == other.minute &&
			hour == other.hour &&
			day_of_month == other.day_of_month &&
			month == other.month &&
			executable == other.executable &&
			arguments == other.arguments;
	}
};

class TaskSchedular {

public:

	TaskSchedular() {

	}

	void load_schedule_from_file(const std::string& file_path, std::vector<ScheduleTask>& tasks) {
		tasks.clear();
		std::fstream input_file(file_path, std::ios::in | std::ios::out);

		if (!input_file.is_open()) {
			std::cerr << "Error in opening input file" << std::endl;
		}
		std::string line;

		while (std::getline(input_file, line)) {
			parse_cron(line, tasks);
		}
	}

	void parse_cron(const std::string& line, std::vector<ScheduleTask>& tasks) {
		const std::regex re(R"(((\d\d?-\d\d?)|(\d\d?)|(\*))(\/\d\d?)?)");
		
		std::smatch sm;

		std::istringstream iss(line);

		std::string minute;
		std::string hour;
		std::string day_of_month;
		std::string month;

		int minute_initial = 0;
		int hour_initial = 0;
		int month_initial = 0;
		int day_of_month_initial = 0;

		int minute_upper = 60;
		int hour_upper = 24;
		int month_upper = 12;
		int day_of_month_upper = 31;

		int minute_step = 1;
		int hour_step = 1;
		int month_step = 1;
		int day_of_month_step = 1;

		iss >> minute >> hour >> day_of_month >> month ;

		parse_cron_field(minute, minute_initial, minute_upper, minute_step, re);
		parse_cron_field(hour, hour_initial, hour_upper, hour_step, re);
		parse_cron_field(day_of_month, day_of_month_initial, day_of_month_upper, day_of_month_step, re);
		parse_cron_field(month, month_initial, month_upper, month_step, re);

		std::string executable;
		std::string args;
		iss >> executable >> args;

		for (int dom = day_of_month_initial; dom <= day_of_month_upper; dom += day_of_month_step) {
			for (int mon = month_initial; mon <= month_upper; mon += month_step) {
				for (int h = hour_initial; h <= hour_upper; h += hour_step) {
					for (int m = minute_initial; m <= minute_upper; m += minute_step) {
						ScheduleTask task;
						task.minute = m;
						task.hour = h;
						task.day_of_month = dom;
						task.month = mon;
						task.executable = executable;
						task.arguments.push_back(args);

						tasks.push_back(task);
						/*std::cout << task.minute << " " << task.hour << " " << task.day_of_month << " "
							<< task.month << " " << std::endl;*/
					}
				}
			}
		}
	}

	void parse_cron_field(const std::string& field, int& initial, int& upper, int& step, const std::regex& re) {
		std::smatch sm;
		std::regex_search(field, sm, re);

		if (sm.size() != 0) {
			if (sm[2].matched) {
				std::string parsed = sm[2].str();
				initial = std::stoi(parsed.substr(0, parsed.find_first_of('-')));
				upper = std::stoi(parsed.substr(parsed.find_first_of('-') + 1));
			}
			if (sm[3].matched && sm[5].matched) {
				std::string parsed1 = sm[3].str();
				std::string parsed2 = sm[5].str();

				initial = std::stoi(parsed1);
				step = std::stoi(parsed2.substr(parsed2.find_first_of('/') + 1));
			}
			else if (sm[3].matched) {
				std::string parsed = sm[3].str();
				initial = upper = std::stoi(parsed);
			}
			else if (sm[5].matched) {
				std::string parsed = sm[5].str();
				step = std::stoi(parsed.substr(parsed.find_first_of('/') + 1));
			}
			
		}
		//std::cout << "start: " <<  field << " " << initial << " " << upper << " " << step << " ";
	}

	void run_schedule_tasks(const std::vector<ScheduleTask>& tasks) {
		SYSTEMTIME current_time;
		GetLocalTime(&current_time);

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
			return false;

		if (task.minute != current_time.wMinute) return false;
		if (task.hour != current_time.wHour) return false;
		if (task.day_of_month != current_time.wDay) return false;
		if (task.month != current_time.wMonth) return false;

		return true;
	}

	static void log_error(const std::string& message) {
		std::ofstream log_file("errors.log", std::ios::app | std::ios::out);
		if (log_file.is_open()) {
			log_file << message << std::endl;
			log_file.close();
		}
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