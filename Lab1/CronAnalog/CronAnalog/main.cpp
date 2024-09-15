#include <iostream>
#include "task_schedular.cpp"

int main(){
    std::setlocale(LC_ALL, "Ru");
    TaskSchedular schedular;

    while (true) {
        auto tasks = schedular.load_schedule_from_file("D:\\OSiSP\\Lab1\\CronAnalog\\CronAnalog\\Schedule.txt");
        schedular.run_schedule_tasks(tasks);

        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}