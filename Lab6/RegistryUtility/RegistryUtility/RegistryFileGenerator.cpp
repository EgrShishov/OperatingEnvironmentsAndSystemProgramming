#include "RegistryFileGenerator.h"


void RegistryFileGenerator::generateInverseRegFile(const std::string& originalFile, const std::string& inverseFile) {
	std::ifstream inputFile(originalFile);
	std::ofstream outputFile(inverseFile);

	if (!inputFile.is_open() || !outputFile.is_open()) {
		std::cerr << "Ошибка: не удалось открыть файлы." << std::endl;
		return;
	}

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.find("dword:") != std::string::npos) {
            std::string originalValue = line.substr(line.find(":") + 1);
            DWORD value = std::stoul(originalValue, nullptr, 16);

            if (value == 1) {
                outputFile << line.replace(line.find("dword:"), 6, "dword:00000000") << "\n";
            } else {
                outputFile << line.replace(line.find("dword:"), 6, "dword:00000001") << "\n";
            }
        } else {
            outputFile << line << "\n";
        }
    }

    inputFile.close();
    outputFile.close();
}