#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

struct RegistrySnapshot {
    std::map<std::string, std::string> keys;
};

std::string binaryToHexString(const BYTE* data, DWORD dataSize) {
    std::ostringstream oss;
    for (DWORD i = 0; i < dataSize; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

void takeSnapshotRecursive(const HKEY hKey, const std::string& subKey, RegistrySnapshot& snapshot) {
    HKEY currentKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &currentKey) != ERROR_SUCCESS) {
        std::cerr << "Error opening registry key: " << subKey << std::endl;
        return;
    }

    DWORD index = 0;
    char valueName[256];
    BYTE data[256];
    DWORD valueNameSize, dataSize, type;

    while (RegEnumValueA(currentKey, index++, valueName, &(valueNameSize = sizeof(valueName)), nullptr,
        &type, data, &(dataSize = sizeof(data))) == ERROR_SUCCESS) {

        std::string key = subKey + "\\" + valueName;

        std::string value;
        switch (type) {
        case REG_SZ:
        case REG_EXPAND_SZ:
            value = std::string(reinterpret_cast<char*>(data), dataSize);
            break;
        case REG_BINARY:
            value = "Binary data: " + binaryToHexString(data, dataSize);
            break;
        case REG_DWORD:
            value = std::to_string(*reinterpret_cast<DWORD*>(data));
            break;
        case REG_QWORD:
            value = std::to_string(*reinterpret_cast<BYTE*>(data));
            break;
        case REG_MULTI_SZ:
            value = "Multi-string data: ";
            for (char* str = reinterpret_cast<char*>(data); *str; str += strlen(str) + 1) {
                value += std::string(str) + ", ";
            }
            value = value.substr(0, value.length() - 2);
            break;
        default:
            value = "Unsupported type";
            break;
        }

        snapshot.keys[key] = value;
    }

    index = 0;
    char subKeyName[256];
    DWORD subKeyNameSize;

    while (RegEnumKeyExA(currentKey, index++, subKeyName, &(subKeyNameSize = sizeof(subKeyName)), nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        takeSnapshotRecursive(currentKey, subKey + "\\" + subKeyName, snapshot);
    }

    RegCloseKey(currentKey);
}

RegistrySnapshot takeSnapshot(const HKEY rootKey, const std::string& subKey) {
    RegistrySnapshot snapshot;
    takeSnapshotRecursive(rootKey, subKey, snapshot);
    return snapshot;
}

void saveSnapshotToFile(const RegistrySnapshot& snapshot, const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& pair : snapshot.keys) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        file << key << "=" << value << "\n";
    }
    file.close();
}

RegistrySnapshot loadSnapshotFromFile(const std::string& filename) {
    RegistrySnapshot snapshot;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        auto delimiterPos = line.find("=");
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        snapshot.keys[key] = value;
    }
    file.close();
    return snapshot;
}


void listAvailableKeys(const RegistrySnapshot& snapshot) {
    std::cout << "Available keys:" << std::endl;
    for (const auto& pair : snapshot.keys) {
        std::cout << pair.first << std::endl;
    }
}

void compareSnapshots(const RegistrySnapshot& oldSnapshot, const RegistrySnapshot& newSnapshot) {
    for (const auto& pair : newSnapshot.keys) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        if (oldSnapshot.keys.find(key) == oldSnapshot.keys.end()) {
            std::cout << "\033[42m+ Added key:\033[0m " << key << " with value: " << value << std::endl;
        }
        else if (oldSnapshot.keys.at(key) != value) {
            std::cout << "~ Modified key: " << key << " old value: "
                << oldSnapshot.keys.at(key) << ", new value: " << value << std::endl;
        }
    }

    for (const auto& pair : oldSnapshot.keys) {
        const std::string& key = pair.first;
        if (newSnapshot.keys.find(key) == newSnapshot.keys.end()) {
            std::cout << "\033[41m- Deleted key:\033[0m " << key << std::endl;
        }
    }
}

void exportChangesToRegFile(const RegistrySnapshot& oldSnapshot, const RegistrySnapshot& newSnapshot, const std::string& filename) {
    std::ofstream regFile(filename);
    regFile << "Windows Registry Editor Version 5.00\n\n";
    for (const auto& pair : newSnapshot.keys) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        if (oldSnapshot.keys.find(key) == oldSnapshot.keys.end() || oldSnapshot.keys.at(key) != value) {
            regFile << "[" << key << "]\n" << "\"Value\"=\"" << value << "\"\n\n";
        }
    }
    regFile.close();
}

void exportInverseRegFile(const RegistrySnapshot& oldSnapshot, const RegistrySnapshot& newSnapshot, const std::string& filename) {
    std::ofstream regFile(filename);
    regFile << "Windows Registry Editor Version 5.00\n\n";
    for (const auto& pair : oldSnapshot.keys) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        if (newSnapshot.keys.find(key) == newSnapshot.keys.end()) {
            regFile << "[-" << key << "]\n\n"; // Удаление ключа
        }
        else if (newSnapshot.keys.at(key) != value) {
            regFile << "[" << key << "]\n\"Value\"=\"" << value << "\"\n\n";
        }
    }
    regFile.close();
}

bool findKeyInSnapshot(const RegistrySnapshot& snapshot, const std::string& keyToFind) {
    if (snapshot.keys.find(keyToFind) != snapshot.keys.end()) {
        std::cout << "Key found: " << keyToFind << " with value: " << snapshot.keys.at(keyToFind) << std::endl;
        return true;
    }
    else {
        std::cerr << "Error: Key not found." << std::endl;
        return false;
    }
}

void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

std::string selectRegistrySubKey(const HKEY rootKey) {
    HKEY hKey;
    if (RegOpenKeyExA(rootKey, "", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        std::cerr << "Error opening registry root key." << std::endl;
        return "";
    }

    std::vector<std::string> subKeys;
    char keyName[256];
    DWORD index = 0;
    DWORD keyNameSize;

    while (RegEnumKeyExA(hKey, index++, keyName, &(keyNameSize = sizeof(keyName)), nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        subKeys.push_back(keyName);
    }
    RegCloseKey(hKey);

    std::cout << "Available registry keys:\n";
    for (size_t i = 0; i < subKeys.size(); ++i) {
        std::cout << i + 1 << ". " << subKeys[i] << std::endl;
    }

    int choice;
    std::cout << "Select a registry key by entering its number: ";
    std::cin >> choice;
    std::cin.ignore();

    if (choice < 1 || choice > static_cast<int>(subKeys.size())) {
        std::cerr << "Invalid choice." << std::endl;
        return "";
    }
    return subKeys[choice - 1];
}

HKEY selectRootKey() {
    std::cout << "Select the root key:\n";
    std::cout << "1. HKEY_CLASSES_ROOT\n";
    std::cout << "2. HKEY_CURRENT_USER\n";
    std::cout << "3. HKEY_LOCAL_MACHINE\n";
    std::cout << "4. HKEY_USERS\n";
    std::cout << "5. HKEY_CURRENT_CONFIG\n";

    int choice;
    std::cout << "Enter your choice (1-5): ";
    std::cin >> choice;

    switch (choice) {
    case 1: return HKEY_CLASSES_ROOT;
    case 2: return HKEY_CURRENT_USER;
    case 3: return HKEY_LOCAL_MACHINE;
    case 4: return HKEY_USERS;
    case 5: return HKEY_CURRENT_CONFIG;
    default:
        std::cerr << "Invalid choice: default = HKEY_CURRENT_USER" << std::endl;
        return HKEY_CURRENT_USER;
    }
}

int main() {
    enableANSI();

    const std::string filename = "snapshot.reg";
    const std::string inverse_filename = "reverse.reg";

    HKEY rootKey = selectRootKey();

    std::string subKey = selectRegistrySubKey(rootKey);
    if (subKey.empty()) {
        return 1;
    }

    RegistrySnapshot initialSnapshot;

    std::ifstream fileCheck(filename);
    if (fileCheck.good()) {
        std::cout << "Loading previous snapshot from file..." << std::endl;
        initialSnapshot = loadSnapshotFromFile(filename);
    }
    else {
        std::cout << "No previous snapshot found. Creating initial snapshot..." << std::endl;
        initialSnapshot = takeSnapshot(rootKey, subKey);
        saveSnapshotToFile(initialSnapshot, filename);
        std::cout << "Initial snapshot saved to file. Run the program again to compare changes." << std::endl;
        return 0;
    }

    std::cout << "Creating current snapshot..." << std::endl;
    RegistrySnapshot currentShapshot = takeSnapshot(rootKey, subKey);

    compareSnapshots(initialSnapshot, currentShapshot);
    saveSnapshotToFile(currentShapshot, filename);
    std::cout << "Current snapshot saved to " << filename << " for future comparisons." << std::endl;
    exportInverseRegFile(initialSnapshot, currentShapshot, inverse_filename);

    return 0;
}