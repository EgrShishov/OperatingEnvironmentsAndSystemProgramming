#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <cstring>
#include <utility>
#include <Windows.h>

struct Record {
    int id;
    char name[60];
    char surname[60];
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

    DirectAccessFileDb(const std::string& file_name, size_t initial) : file_name(file_name) {
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

        DWORD dwFileSize = GetFileSize(hFile, nullptr);

        if (dwFileSize == 0) {
            SetFilePointer(hFile, initial, NULL, FILE_BEGIN);
            SetEndOfFile(hFile);
        }

        if (dwFileSize == INVALID_FILE_SIZE) {
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
        UnmapViewOfFile(metadata);
        CloseHandle(hFile);
        CloseHandle(hMapping);
    }

    void write_record(int index, const Record& record) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            expand((index + 1) * RECORD_SIZE + METADATA_SIZE);
        }

        if (records[index].is_deleted) {
            records[index] = record;
        }

        records[metadata->free_list_head_index] = record;

        metadata->records_count++;
        metadata->free_list_head_index = index + 1;
    }

    Record* read_record(int index) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            //std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return nullptr;
        }

        Record* record = &records[index];
        if (record->is_deleted) {
            return nullptr;
        }

        return record;
    }

    bool delete_record(int index) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return false;
        }

        records[index].is_deleted = true;
        records[index].next_free_index = metadata->free_list_head_index;
        metadata->records_count--;
        metadata->free_list_head_index = index;
    }

    Record* async_read_record(int index, OVERLAPPED& overlapped) {
        if (index < 0 || index >= metadata->records_count) {
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
        if (index < 0 || index >= metadata->records_count) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return false;
        }
        
        WriteFile(hFile, &record, RECORD_SIZE, nullptr, &overlapped);
        WaitForSingleObject(overlapped.hEvent, INFINITE);

        return true;
    }

    void expand(size_t new_size) {
        if (!UnmapViewOfFile(metadata)) {
            std::cerr << "Ошибка отображения файла при расширении: " << GetLastError() << std::endl;
            return;
        }
        CloseHandle(hMapping);

        SetFilePointer(hFile, new_size, nullptr, FILE_BEGIN);
        SetEndOfFile(hFile);
        
        hMapping = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (!hMapping) {
            std::cerr << "Ошибка создания отображения файла при расширении: " << GetLastError() << std::endl;
            return;
        }

        void* map_view = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!map_view) {
            std::cerr << "Ошибка отображения файла в память при расширении: " << GetLastError() << std::endl;
            return;
        }

        metadata = static_cast<Metadata*>(map_view);
        records = reinterpret_cast<Record*>(static_cast<char*>(map_view) + METADATA_SIZE);

        if (!metadata) {
            std::cerr << "Ошибка получения метаданных при расширении: " << GetLastError() << std::endl;
            return;
        }
        if (!records) {
            std::cerr << "Ошибка получения указателя на записи после расширения: " << GetLastError() << std::endl;
            return;
        }
        metadata->file_size = new_size;
    }

    bool defragmentate_file() {
        HANDLE hTempFile = CreateFile(
            CastToLPCWSTR("temp.bin"),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (!hTempFile) {
            std::cerr << "Ошибка создания временного файла: " << GetLastError() << std::endl;
            return false;
        }

        DWORD bytes_read, bytes_written;
        int index = 0;
        Record record;

        SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
        while (ReadFile(hFile, &record, RECORD_SIZE, &bytes_read, nullptr)
            && bytes_read == RECORD_SIZE) {
            if (!record.is_deleted) {
                SetFilePointer(hTempFile, index * RECORD_SIZE, nullptr, FILE_BEGIN);
                WriteFile(hTempFile, &record, RECORD_SIZE, &bytes_written, nullptr);
                index++;
            }
        }

        CloseHandle(hTempFile);
        CloseHandle(hFile);

        DeleteFile(CastToLPCWSTR(file_name));
        MoveFile(CastToLPCWSTR("temp.bin"), CastToLPCWSTR(file_name));

        hFile = CreateFile(
            CastToLPCWSTR(file_name),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        return true;
    }

    size_t get_records_size() {
        return metadata->records_count;
    }

    bool ensure_deleted() {
        return DeleteFile(CastToLPCWSTR(file_name));
    }

private:
    LPCWSTR CastToLPCWSTR(const std::string& str) {
        int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        wchar_t* wideStr = new wchar_t[length];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);
        LPCWSTR lpcwstr = wideStr;
        return lpcwstr;
    }

    std::string file_name;
    HANDLE hFile, hMapping;
    Metadata* metadata;
    Record* records;
};


int main()
{
    setlocale(LC_ALL, "Ru");
    DirectAccessFileDb db = DirectAccessFileDb("database", 1024);

    //std::cout << sizeof(Record);
    //for (int i = 0; i < 200; i++) {
    //    db.write_record(i, Record{ i, "Test", "User", i, false });
    //}
    //db.delete_record(202);
    //db.delete_record(203);
    //db.delete_record(204);
    //db.delete_record(205);
    //db.write_record(1, Record{ 201, "Test", "User", 201, false });
    //std::cout << "records count: " << db.get_records_size() << std::endl;
    for (int i = 0; i < db.get_records_size(); i++) {
        //db.delete_record(i);
        Record* record = db.read_record(i);
        if (record != nullptr) {
            std::cout << record->id << std::endl;
        }
    }


   /* for (int i = 0; i < db.get_records_size(); i++) {
        OVERLAPPED overlapped = { 0 };
        overlapped.Offset = i * RECORD_SIZE + METADATA_SIZE;
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        auto record = db.async_read_record(i, overlapped);
        std::cout << record->age << std::endl;
    }

    OVERLAPPED overlapped = { 0 };
    overlapped.Offset = 8 * RECORD_SIZE + METADATA_SIZE;
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    db.async_write_record(8, Record{ 201, "Test", "User", 201, false }, overlapped);*/

    std::cout << "records count: " << db.get_records_size() << std::endl;
    return 0;
}
