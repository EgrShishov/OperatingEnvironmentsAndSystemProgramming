#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class RegistryComparer
{
public:
    struct RegistryEntry {
        std::string key;
        std::string name;
        DWORD type;
        std::vector<BYTE> data;
    };

    std::vector<RegistryEntry> getRegistryEntries(HKEY hKeyRoot, const std::string& subKey);
    void compareEntries(const std::vector<RegistryEntry>& oldEntries, const std::vector<RegistryEntry>& newEntries);
    void exportToRegFile(const std::string& filename, 
                         const std::vector<RegistryComparer::RegistryEntry>& changedEntries,
                         const std::vector<RegistryComparer::RegistryEntry>& addedEntries,
                         const std::vector<RegistryComparer::RegistryEntry>& deletedEntries);
    void generateInverseRegFile(const std::string& filename, 
                         const std::vector<RegistryComparer::RegistryEntry>& changedEntries,
                         const std::vector<RegistryComparer::RegistryEntry>& addedEntries,
                         const std::vector<RegistryComparer::RegistryEntry>& deletedEntries);
};
