#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <algorithm>
#include <Windows.h>

class DataSorter {

public:
	DataSorter() {
		hGenPipe = CreateFile(
			convert_to_LPCWSTR(gen_pipe_name),
			GENERIC_READ,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);

		if (hGenPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Ошибка в подключении канала к генератору данных: " << GetLastError() << std::endl;
			return;
		}
	}

	void get_and_sort_data() {
		std::vector<int> data (100);
		DWORD bytes_read;
		ReadFile(hGenPipe, data.data(), data.size() * sizeof(int), &bytes_read, nullptr);

		if (bytes_read != 0) {
			std::cout << "Получено " << bytes_read << " данных от генератора" << std::endl;
		}
		std::sort(data.begin(), data.end());

		CloseHandle(hGenPipe);

		hPipe = CreateNamedPipe(
			convert_to_LPCWSTR(pipe_name),
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_BYTE | PIPE_WAIT,
			1, 0, 0, 0, nullptr);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Ошибка создания канала сортировки: " << GetLastError() << std::endl;
			return;
		}

		if (!ConnectNamedPipe(hPipe, nullptr)) {
			std::cerr << "Ошибка подключения клиента: " << GetLastError() << std::endl;
			return;
		}

		DWORD bytes_written;
		WriteFile(hPipe, data.data(), data.size() * sizeof(int), &bytes_written, nullptr);
		if (bytes_written == 0) {
			std::cerr << "Ошибка записи в канал вывода: " << GetLastError() << std::endl;
			return;
		}

		std::cout << "Отправлено " << bytes_written << " байт на вывод" << std::endl;

		CloseHandle(hPipe);
	}

private:
	HANDLE hGenPipe = INVALID_HANDLE_VALUE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	LPCWSTR convert_to_LPCWSTR(std::string str) {
		int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* wideStr = new wchar_t[length];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);

		return (LPCWSTR)wideStr;
	}

	std::string gen_pipe_name = R"(\\.\pipe\pipe_gen_to_sort)"; // все именнованные каналы находятся в одной папке \.\pipe
	std::string pipe_name = R"(\\.\pipe\pipe_sort_to_out")";
};

int main() {
	setlocale(LC_ALL, "Ru");
	DataSorter ds = DataSorter();
	ds.get_and_sort_data();
	return 0;
}