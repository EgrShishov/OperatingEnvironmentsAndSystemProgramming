#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <Windows.h>

class DataGenerator {

public:

	DataGenerator() {
		srand(static_cast<unsigned int>(time(nullptr)));

		//hPipe = CreateNamedPipe(
		//	convert_to_LPCWSTR(pipe_name),
		//	PIPE_ACCESS_DUPLEX,
		//	PIPE_TYPE_BYTE | PIPE_WAIT,
		//	1, 0, 0, 0, nullptr);

		//if (hPipe == INVALID_HANDLE_VALUE) {
		//	std::cerr << "Ошибка в создании канала: " << GetLastError() << std::endl;
		//	return;
		//}

		//if (!ConnectNamedPipe(hPipe, nullptr)) {
		//	std::cerr << "Ошибка подключения: " << GetLastError() << std::endl;
		//	return;
		//}

		SECURITY_ATTRIBUTES sec_atr;

		sec_atr.lpSecurityDescriptor = nullptr;
		sec_atr.bInheritHandle = true;
		sec_atr.nLength = sizeof(SECURITY_ATTRIBUTES);

		if (!CreatePipe(
			&hReadPipe,
			&hWritePipe,
			&sec_atr,
			0)) {
			std::cerr << "Ошибка в создании канала: " << GetLastError() << std::endl;
			return;
		}

		if (!SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0)) {
			std::cerr << "Не удалось установить флаг для канала записи: " << GetLastError() << std::endl;
			return;
		}
	}

	void generate_and_send_data() {
		std::vector<int> data(100);
		for (auto& val : data) {
			val = rand() % 1000;
		}

		//DWORD bytes_written;
		//WriteFile(hPipe, data.data(), data.size() * sizeof(int), &bytes_written, nullptr);
		//std::cout << "Данные отправлены на сортировку" << std::endl;

		HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
		
		PROCESS_INFORMATION piProcInfo;
		STARTUPINFO siStartInfo;
		
		ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
		
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdInput = hReadPipe; // сюда передаём дескриптор чтения канала
		siStartInfo.hStdOutput = hConsoleOut;
		siStartInfo.hStdError = hConsoleErr;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
		
		if (!CreateProcess(
			NULL,
			(LPWSTR)convert_to_LPCWSTR("D:\\OSiSP\\Lab3\\ConveyorProcessingModeling\\x64\\Debug\\DataSorter.exe"),
			NULL,                    
			NULL,                    
			TRUE,                    
			0,                       
			NULL,                    
			NULL,                    
			&siStartInfo,
			&piProcInfo))
		{
			std::cerr << "Не удалось создать процесс: " << GetLastError() << std::endl;
			return;
		}
		
		DWORD bytes_written;
		WriteFile(hWritePipe, data.data(), data.size() * sizeof(int), &bytes_written, nullptr);
		for (int el : data) {
			std::cout << el << "\n";
		}
		std::cout << "Данные отправлены на сортировку: " << bytes_written << " байт" << std::endl;
		
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		
		WaitForSingleObject(piProcInfo.hProcess, INFINITE);
		
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

private:
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	HANDLE hReadPipe = INVALID_HANDLE_VALUE;
	HANDLE hWritePipe = INVALID_HANDLE_VALUE;

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
