#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>

class RegistryShanpshot {
public:
	void saveRegistrySnapshot(HKEY rootKey, const std::string& subKey, const std::string& fName);

private:
	void saveKey(HKEY hKey, const std::string& subKey, std::ofstream& file);
};

