#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <algorithm>
#include <Windows.h>

class DataOutputer {

public:
	DataOutputer() {
		hSortPipe = CreateFile(
			convert_to_LPCWSTR(sort_pipe_name),
			GENERIC_READ,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);

		if (hSortPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Ошибка подключения к каналу сортировки: " << GetLastError() << std::endl;
			return;
		}
	}

	void print_data() {
		std::vector<int> sorted_data(100);
		DWORD bytes_read;
		bool succeed = ReadFile(hSortPipe, sorted_data.data(), sorted_data.size() * sizeof(int), &bytes_read, nullptr);

		if (bytes_read != 0 && succeed) {
			std::cerr << "Получено " <<  bytes_read << " байт" << std::endl;

			for (int el : sorted_data) {
				std::cout << el << std::endl;
			}
		}

		CloseHandle(hSortPipe);
	}

private:
	HANDLE hSortPipe = INVALID_HANDLE_VALUE;

	LPCWSTR convert_to_LPCWSTR(std::string str) {
		int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* wideStr = new wchar_t[length];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);

		return (LPCWSTR)wideStr;
	}

	std::string sort_pipe_name = R"(\\.\pipe\pipe_sort_to_out")";
};

int main() {
	setlocale(LC_ALL, "Ru");
	DataOutputer dout = DataOutputer();
	dout.print_data();
	return 0;
}