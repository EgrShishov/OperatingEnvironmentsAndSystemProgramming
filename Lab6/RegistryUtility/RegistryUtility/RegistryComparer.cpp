#include "RegistryComparer.h"  

std::vector<RegistryComparer::RegistryEntry> RegistryComparer::getRegistryEntries(HKEY hKeyRoot, const std::string& subKey)
{
    std::vector<RegistryComparer::RegistryEntry> entries;
    HKEY hKey;
    LONG result = RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key: " << subKey << ". Error code: " << result << std::endl;
        return entries;
    }

    DWORD valueCount;
    result = RegQueryInfoKeyA(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, NULL, NULL, NULL, NULL);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to query key info: " << result << std::endl;
        RegCloseKey(hKey);
        return entries;
    }

    std::cout << "Number of values in key: " << valueCount << std::endl;

    DWORD index = 0;
    DWORD valueNameSize = 255;
    DWORD valueDataSize = 255;
    char valueName[255];
    BYTE valueData[255];
    DWORD valueType;

    while (true) {
        valueNameSize = sizeof(valueName);
        valueDataSize = sizeof(valueData);

        result = RegEnumValueA(hKey,
            index,
            valueName,
            &valueNameSize,
            NULL,
            &valueType,
            valueData,
            &valueDataSize);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (result != ERROR_SUCCESS) {
            std::cerr << "Failed to enumerate registry values at index " << index << ". Error code: " << result << std::endl;
            break;
        }

        RegistryEntry entry;
        entry.key = subKey;
        entry.name = valueName;
        entry.type = valueType;
        entry.data.assign(valueData, valueData + valueDataSize);

        entries.push_back(entry);
        index++;
    }
    
    RegCloseKey(hKey);
    return entries;
}

void RegistryComparer::compareEntries(const std::vector<RegistryComparer::RegistryEntry>& oldEntries, 
    const std::vector<RegistryComparer::RegistryEntry>& newEntries) {

    std::vector<RegistryComparer::RegistryEntry> addedEntries;
    std::vector<RegistryComparer::RegistryEntry> modifiedEntries;
    std::vector<RegistryComparer::RegistryEntry> deletedEntries;

    for (const auto& entry : newEntries) {
        bool found = false;
        for (const auto& oldEntry : oldEntries) {
            if (entry.key == oldEntry.key && entry.name == oldEntry.name) {
                found = true;
                if (entry.data != oldEntry.data) {
                    modifiedEntries.push_back(entry);
                }
                break;
            }
        }
        if (!found) {
            addedEntries.push_back(entry);
        }
    }

    for (const auto& oldEntry : oldEntries) {
        bool found = false;
        for (const auto& newEntry : newEntries) {
            if (oldEntry.key == newEntry.key && oldEntry.name == newEntry.name) {
                found = true;
                break;
            }
        }

        if (!found) {
            deletedEntries.push_back(oldEntry);
        }
    }

    exportToRegFile("changes.reg", modifiedEntries, addedEntries, deletedEntries);
    generateInverseRegFile("inverse_changes.reg", modifiedEntries, addedEntries, deletedEntries);
}

void RegistryComparer::exportToRegFile(const std::string& filename, 
    const std::vector<RegistryComparer::RegistryEntry>& changedEntries, 
    const std::vector<RegistryComparer::RegistryEntry>& addedEntries,
    const std::vector<RegistryComparer::RegistryEntry>& deletedEntries) {
   
    std::ofstream regFile(filename);
    regFile << "Windows Registry Editor Version 5.00\n\n";

    for (const auto& entry : addedEntries) {
        regFile << "; Added\n";
        regFile << "[" << entry.key << "]\n";
        if (entry.type == REG_SZ) {
            regFile << "\"" << entry.name << "\"=\"" << std::string(reinterpret_cast<const char*>(entry.data.data()), entry.data.size()) << "\"\n";
        } else if (entry.type == REG_DWORD) {
            DWORD value = *(DWORD*)(entry.data.data());
            regFile << "\"" << entry.name << "\"=dword:" << std::hex << value << "\n";
        } else if (entry.type == REG_BINARY) {
            regFile << "\"" << entry.name << "\"=hex:";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        } else if (entry.type == REG_MULTI_SZ) {
            regFile << "\"" << entry.name << "\"=hex(7):";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        }
        regFile << "\n";
    }

    for (const auto& entry : changedEntries) {
        regFile << "; Modified\n";
        regFile << "[" << entry.key << "]\n";
        if (entry.type == REG_SZ) {
            regFile << "\"" << entry.name << "\"=\"" << std::string(reinterpret_cast<const char*>(entry.data.data()), entry.data.size()) << "\"\n";
        } else if (entry.type == REG_DWORD) {
            DWORD value = *(DWORD*)(entry.data.data());
            regFile << "\"" << entry.name << "\"=dword:" << std::hex << value << "\n";
        } else if (entry.type == REG_BINARY) {
            regFile << "\"" << entry.name << "\"=hex:";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        } else if (entry.type == REG_MULTI_SZ) {
            regFile << "\"" << entry.name << "\"=hex(7):";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        }
        regFile << "\n";
    }

    for (const auto& entry : deletedEntries) {
        regFile << "; Deleted\n";
        regFile << "[-" << entry.key << "\\" << entry.name << "]\n\n";
    }

    regFile.close();
}

void RegistryComparer::generateInverseRegFile(const std::string& filename,
    const std::vector<RegistryComparer::RegistryEntry>& changedEntries,
    const std::vector<RegistryComparer::RegistryEntry>& addedEntries,
    const std::vector<RegistryComparer::RegistryEntry>& deletedEntries) {

    std::ofstream regFile(filename);
    regFile << "Windows Registry Editor Version 5.00\n\n";

    for (const auto& entry : changedEntries) {
        regFile << "; Undo Modifed\n";
        regFile << "[" << entry.key << "]\n";
        if (entry.type == REG_SZ) {
            regFile << "\"" << entry.name << "\"=\"" << std::string(reinterpret_cast<const char*>(entry.data.data()), entry.data.size()) << "\"\n";
        } else if (entry.type == REG_DWORD) {
            DWORD value = *(DWORD*)(entry.data.data());
            regFile << "\"" << entry.name << "\"=dword:" << std::hex << value << "\n";
        } else if (entry.type == REG_BINARY) {
            regFile << "\"" << entry.name << "\"=hex:";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        } else if (entry.type == REG_MULTI_SZ) {
            regFile << "\"" << entry.name << "\"=hex(7):";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        }
        regFile << "\n";
    }

    for (const auto& entry : addedEntries) {
        regFile << "; Undo Added\n";
        regFile << "[-" << entry.key << "\\" << entry.name << "]\n";
    }

    for (const auto& entry : deletedEntries) {
        regFile << "; Undo Deleted\n";
        regFile << "[" << entry.key << "]\n";
        if (entry.type == REG_SZ) {
            regFile << "\"" << entry.name << "\"=\"" << std::string(reinterpret_cast<const char*>(entry.data.data()), entry.data.size()) << "\"\n";
        } else if (entry.type == REG_DWORD) {
            DWORD value = *(DWORD*)(entry.data.data());
            regFile << "\"" << entry.name << "\"=dword:" << std::hex << value << "\n";
        } else if (entry.type == REG_BINARY) {
            regFile << "\"" << entry.name << "\"=hex:";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        } else if (entry.type == REG_MULTI_SZ) {
            regFile << "\"" << entry.name << "\"=hex(7):";
            for (size_t i = 0; i < entry.data.size(); ++i) {
                regFile << std::hex << (int)entry.data[i];
                if (i != entry.data.size() - 1) {
                    regFile << ",";
                }
            }
            regFile << "\n";
        }
        regFile << "\n";
    }

    regFile.close();
}