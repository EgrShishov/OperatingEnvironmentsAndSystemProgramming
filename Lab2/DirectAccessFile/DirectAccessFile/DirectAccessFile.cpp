#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <cstring>
#include <utility>
#include <Windows.h>

struct Record {
    int id;
    std::string name;
    std::string surname;
    int age;
    bool is_married;

    bool is_deleted;
    int next_free_index;
};

struct Metadata {
    int records_count;
    int free_list_head_index;
    size_t file_size;
};

const size_t RECORD_SIZE = sizeof(Record);
const size_t METADATA_SIZE = sizeof(Metadata);

class DirectAccessFileDb {

public:

    DirectAccessFileDb(const std::string& file_name, size_t initial) {
        hFile = CreateFile(
            CastToLPCWSTR(file_name),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Ошибка открытия файла: " << GetLastError() << std::endl;
            exit(1);
        }

        SetFilePointer(hFile, 1024, NULL, FILE_BEGIN);
        SetEndOfFile(hFile);

        DWORD dwFileSize = GetFileSize(hFile, nullptr);
        if (dwFileSize == INVALID_FILE_SIZE || dwFileSize == 0) {
            std::cerr << "Ошибка размера файла: " << GetLastError() << std::endl;
            exit(1);
        }

        hMapping = CreateFileMapping(
            hFile, 
            nullptr,
            PAGE_READWRITE,
            0,
            0,
            nullptr);

        if (!hMapping) {
            std::cerr << "Ошибка отображения файла: " << GetLastError() << std::endl;
            exit(1);
        }

        void* map_view = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!map_view) {
            std::cerr << "Ошибка отображения файла в память: " << GetLastError() << std::endl;
            exit(1);
        }

        metadata = static_cast<Metadata*>(map_view);
        records = reinterpret_cast<Record*>(static_cast<char*>(map_view) + METADATA_SIZE);

        if (metadata->file_size == 0) {
            metadata->records_count = 0;
            metadata->free_list_head_index = -1;
            metadata->file_size = initial;
        }
    }

    ~DirectAccessFileDb() {
        UnmapViewOfFile(hMapping);
        CloseHandle(hFile);
        CloseHandle(hMapping);
        delete metadata;
        delete records;
    }

    void write_record(int index, const Record& record) {
        if (index * RECORD_SIZE + METADATA_SIZE >= metadata->file_size) {
            std::cerr << "Недостаточно места для записи" << std::endl;
            return;
        }

        records[index] = record;
        metadata->records_count++;
        metadata->free_list_head_index = index + 1;
    }

    Record read_record(int index) {
        if (index * RECORD_SIZE + METADATA_SIZE >= metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
        }

        Record record;
        DWORD bytes_readed;
        record = records[index];
        return record;
    }

    bool delete_record(int index) {
        if (index * RECORD_SIZE + METADATA_SIZE >= metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return false;
        }

        Record record = records[index];
        record.is_deleted = true;
        record.next_free_index = metadata->free_list_head_index;
        metadata->records_count--;
        metadata->free_list_head_index = index;
    }

    Record* async_read_record(int index, OVERLAPPED& overlapped) {
        if (index * RECORD_SIZE + METADATA_SIZE >= metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return nullptr;
        }

        char buffer[RECORD_SIZE];
        ReadFile(hFile, buffer, RECORD_SIZE, nullptr, &overlapped);
        WaitForSingleObject(overlapped.hEvent, INFINITE);

        Record* record = reinterpret_cast<Record*>(buffer);
        return record;
    }

    bool async_write_record(int index, const Record& record, OVERLAPPED& overlapped) {
        if (index * RECORD_SIZE + METADATA_SIZE >= metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return false;
        }
        
        WriteFile(hFile, &record, RECORD_SIZE, nullptr, &overlapped);
        WaitForSingleObject(overlapped.hEvent, INFINITE);

        return true;
    }

    void expand(size_t new_size) {
        UnmapViewOfFile(metadata);
        CloseHandle(hMapping);

        SetFilePointer(hFile, new_size, nullptr, FILE_BEGIN);
        SetEndOfFile(hFile);
        
        hMapping = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        metadata = static_cast<Metadata*>(MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
        records = reinterpret_cast<Record*>(reinterpret_cast<char*>(metadata) + METADATA_SIZE);
        metadata->file_size = new_size;
    }

    bool defragmentate_file() {

    }

    bool compress_file() {

    }

private:

    LPCWSTR CastToLPCWSTR(const std::string& str) {
        int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        wchar_t* wideStr = new wchar_t[length];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);
        LPCWSTR lpcwstr = wideStr;
        return lpcwstr;
    }

    HANDLE hFile, hMapping;
    Metadata* metadata;
    Record* records;
};


int main()
{
    setlocale(LC_ALL, "Ru");
    DirectAccessFileDb db = DirectAccessFileDb("database", 1024);
    Record record = { 1, "egor", "shishov", 19, false};

    db.write_record(0, record);
    auto readed = db.read_record(0);
    std::cout << readed.age << " " << readed.surname << " " << readed.name << " " << readed.id;
    return 0;
}
