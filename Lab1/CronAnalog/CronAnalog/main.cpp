#include <iostream>
#include <regex>
#include "task_schedular.cpp"

int main(){
    std::setlocale(LC_ALL, "Ru");

    TaskSchedular schedular;
    std::vector<ScheduleTask> tasks;
    std::cout << "Enter filepath to schedule file or use default: " << std::endl;
    std::string default_path = "D:\\OSiSP\\Lab1\\CronAnalog\\CronAnalog\\Schedule.txt";
    std::string file_path = "";
    std::getline(std::cin, file_path);

    if (file_path.size() == 0) {
        std::cout << "Using default schedule path" << std::endl;
        std::cout << default_path << std::endl;
        file_path = default_path;
    }
        
    while (true) {
        schedular.load_schedule_from_file("D:\\OSiSP\\Lab1\\CronAnalog\\CronAnalog\\Schedule.txt", tasks);
        schedular.run_schedule_tasks(tasks);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}