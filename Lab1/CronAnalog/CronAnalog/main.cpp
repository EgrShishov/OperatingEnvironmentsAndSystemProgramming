#include <iostream>
#include "task_schedular.cpp"

int main(){
	
	TaskSchedular schedular = TaskSchedular();

	const std::string& file_path = "";
	schedular.load_schedule_from_file(file_path);

	while (true) {
		
	}
}