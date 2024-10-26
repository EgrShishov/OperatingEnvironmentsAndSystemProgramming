#include "RegistryShanpshot.h"

void RegistryShanpshot::saveRegistrySnapshot(HKEY rootKey, const std::string& subKey, const std::string& fName) {
	HKEY hKey;
	if (RegOpenKeyExA(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		std::ofstream file(fName);
		if (!file.is_open()) {
			std::cerr << "Error in opening file\n";
			return;
		}

		file << "Windows Registry Editor Version 5.00\n\n"; // заголовок с версией редактора
		saveKey(hKey, subKey, file);
		file.close();
		RegCloseKey(hKey);
	}
	else {
		std::cerr << "Ошибка: не удалось открыть ключ реестра." << std::endl;
	}
}

void RegistryShanpshot::saveKey(HKEY hKey, const std::string& subKey, std::ofstream& file) {
	char valueName[255];
	BYTE valueData[255];
	
	DWORD valueNameSize = 255;
	DWORD valueDataSize = 255;

	DWORD valueType;

	file << "[" << subKey << "]\n";
	for (DWORD i = 0; 
		RegEnumValueA(
			hKey, 
			i, 
			valueName,
			&valueNameSize, 
			NULL, 
			&valueType, 
			valueData, 
			&valueDataSize) == ERROR_SUCCESS; ++i) {
		file << "\"" << valueName << "\"=";
		if (valueType == REG_SZ) {
			file << "\"" << reinterpret_cast<char*>(valueData) << "\"\n";
		} else if (valueType == REG_DWORD) {
			file << "dword: " << std::hex << *reinterpret_cast<DWORD*>(valueData) << "\n";
		} else if (valueType == REG_BINARY) {
			file << "hex:";
			for (DWORD i = 0; i < valueDataSize; ++i) {
				file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(valueData[i]);
				if (i < valueDataSize - 1) {
					file << ",";
				}
			}
			file << "\n";
		} else if (valueType == REG_MULTI_SZ) {
			file << "hex(7):";
			bool first = true;
			for (DWORD i = 0; i < valueDataSize; ++i) {
				if (!first) {
					file << ",";
				}
				file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(valueData[i]);
				first = false;
			}
			file << "\n";
		}

		valueDataSize = 255;
		valueNameSize = 255;
	}
}