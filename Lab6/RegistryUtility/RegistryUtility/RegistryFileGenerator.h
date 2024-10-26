#pragma once
#include <string>
#include <fstream>
#include <iomanip>
#include <Windows.h>
#include <iostream>

class RegistryFileGenerator {
public:
	void generateInverseRegFile(const std::string& originalFile, const std::string& inverseFile);
};

