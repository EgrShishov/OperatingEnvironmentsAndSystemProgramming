#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <Windows.h>

class DataGenerator {

public:

	DataGenerator() {
		srand(static_cast<unsigned int>(time(nullptr)));

		hPipe = CreateNamedPipe(
			convert_to_LPCWSTR(pipe_name),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_WAIT,
			1, 0, 0, 0, nullptr);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Ошибка в создании канала: " << GetLastError() << std::endl;
			return;
		}

		if (!ConnectNamedPipe(hPipe, nullptr)) {
			std::cerr << "Ошибка подключения: " << GetLastError() << std::endl;
			return;
		}
	}

	void generate_and_send_data() {
		std::vector<int> data(100);
		for (auto& val : data) {
			val = rand() % 1000;
		}

		DWORD bytes_written;
		WriteFile(hPipe, data.data(), data.size() * sizeof(int), &bytes_written, nullptr);
		std::cout << "Данные отправлены на сортировку" << std::endl;
	}

private:
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	LPCWSTR convert_to_LPCWSTR(std::string str) {
		int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* wideStr = new wchar_t[length];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);

		return (LPCWSTR)wideStr;
	}

	std::string pipe_name = R"(\\.\pipe\pipe_gen_to_sort)";
};

int main()
{
	setlocale(LC_ALL, "Ru");
	DataGenerator dg = DataGenerator();
	dg.generate_and_send_data();
	return 0;
}
