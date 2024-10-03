#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <cstring>
#include <utility>
#include <chrono>
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

        if (dwFileSize == 0 || dwFileSize == INVALID_FILE_SIZE) {
            SetFilePointer(hFile, initial, NULL, FILE_BEGIN);
            SetEndOfFile(hFile);
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
            metadata->file_size = dwFileSize;
        }
    }

    ~DirectAccessFileDb() {
        UnmapViewOfFile(metadata);
        CloseHandle(hFile);
        CloseHandle(hMapping);
    }

    void write_record(int index, const Record& record) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            if (!expand((index + 1) * RECORD_SIZE + METADATA_SIZE)) {
                return;
            }
        }

        if (records[index].is_deleted) {
            records[index].is_deleted = false;
        }

        records[index] = record;
        metadata->records_count++;
        metadata->free_list_head_index = index + 1;
    }

    Record* read_record(int index) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
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

    Record* async_read_record(int index, OVERLAPPED& overlapped) { // some problems
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            std::cerr << "Индекс вышел за пределы файла" << std::endl;
            return nullptr;
        }

        DWORD bytes_read;
        char* buffer = new char[RECORD_SIZE];
        bool succeed = ReadFile(hFile, buffer, RECORD_SIZE, &bytes_read, &overlapped);
        WaitForSingleObject(overlapped.hEvent, INFINITE);

        if (!GetOverlappedResult(hFile, &overlapped, &bytes_read, TRUE)) {
            std::cerr << "Ошибка получения результата чтения: " << GetLastError() << std::endl;
            CloseHandle(overlapped.hEvent);
            delete[] buffer;
            return nullptr;
        }

        return reinterpret_cast<Record*>(buffer);
    }

    bool async_write_record(int index, const Record& record, OVERLAPPED& overlapped) {
        if (index < 0 || (index + 1) * RECORD_SIZE + METADATA_SIZE > metadata->file_size) {
            if (!expand((index + 1) * RECORD_SIZE + METADATA_SIZE)) {
                return false;
            }
        }
        
        DWORD bytes_written;
        WriteFile(hFile, &record, RECORD_SIZE, &bytes_written, &overlapped);
        WaitForSingleObject(overlapped.hEvent, INFINITE);
        metadata->records_count++;
        return bytes_written;
    }

    bool expand(size_t new_size) {
        if (!UnmapViewOfFile(metadata)) {
            std::cerr << "Ошибка отображения файла при расширении: " << GetLastError() << std::endl;
            return false;
        }
        CloseHandle(hMapping);

        SetFilePointer(hFile, new_size, nullptr, FILE_BEGIN);
        SetEndOfFile(hFile);
        
        hMapping = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (!hMapping) {
            std::cerr << "Ошибка создания отображения файла при расширении: " << GetLastError() << std::endl;
            return false;
        }

        void* map_view = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!map_view) {
            std::cerr << "Ошибка отображения файла в память при расширении: " << GetLastError() << std::endl;
            return false;
        }

        metadata = static_cast<Metadata*>(map_view);
        records = reinterpret_cast<Record*>(static_cast<char*>(map_view) + METADATA_SIZE);

        if (!metadata) {
            std::cerr << "Ошибка получения метаданных при расширении: " << GetLastError() << std::endl;
            return false;
        }
        if (!records) {
            std::cerr << "Ошибка получения указателя на записи после расширения: " << GetLastError() << std::endl;
            return false;
        }
        metadata->file_size = new_size;
        return true;
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

    bool ensure_created() {

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

void show_menu() {
    std::cout << "============================" << std::endl;
    std::cout << "     Direct Access DB Menu   " << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "1. Add a record" << std::endl;
    std::cout << "2. Display all records" << std::endl;
    std::cout << "3. Delete a record" << std::endl;
    std::cout << "4. Performance test (Sync vs Async)" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "Choose an option: ";
}

void add_record(DirectAccessFileDb& db) {
    Record record;
    std::cout << "Enter ID: ";
    std::cin >> record.id;
    std::cin.ignore();
    std::cout << "Enter Name: ";
    std::cin.getline(record.name, 60);
    std::cout << "Enter Surname: ";
    std::cin.getline(record.surname, 60);
    std::cout << "Enter Age: ";
    std::cin >> record.age;
    std::cout << "Is Married? (1 for Yes, 0 for No): ";
    std::cin >> record.is_married;

    db.write_record(record.id, record);
    std::cout << "Record added successfully!" << std::endl;
}

void display_records(DirectAccessFileDb& db) {
    for (int i = 0; i < db.get_records_size(); i++) {
        Record* record = db.read_record(i);
        if (record != nullptr) {
            std::cout << "ID: " << record->id
                << ", Name: " << record->name
                << ", Surname: " << record->surname
                << ", Age: " << record->age
                << ", Married: " << (record->is_married ? "Yes" : "No")
                << std::endl;
        }
    }
}

void delete_record(DirectAccessFileDb& db) {
    int id;
    std::cout << "Enter ID of the record to delete: ";
    std::cin >> id;
    if (db.delete_record(id)) {
        std::cout << "Record deleted successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to delete record." << std::endl;
    }
}

void performance_test(DirectAccessFileDb& db, DirectAccessFileDb& db_async) {
    std::cout << "Running sync vs async performance tests..." << std::endl;

    auto sync_write_start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        db.write_record(i, Record{ i, "Test", "User", i, false });
    }
    auto sync_write_end_time = std::chrono::high_resolution_clock::now();
    auto sync_write_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(sync_write_end_time - sync_write_start_time).count();
    std::cout << "Sync write time (ms): " << sync_write_elapsed << std::endl;

    auto async_write_start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        OVERLAPPED overlapped = { 0 };
        overlapped.Offset = i * RECORD_SIZE + METADATA_SIZE;
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        db_async.async_write_record(i, Record{ i, "Test", "User", i, false }, overlapped);
        CloseHandle(overlapped.hEvent);
    }
    auto async_write_end_time = std::chrono::high_resolution_clock::now();
    auto async_write_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(async_write_end_time - async_write_start_time).count();
    std::cout << "Async write time (ms): " << async_write_elapsed << std::endl;
}

int main()
{
    setlocale(LC_ALL, "Ru");
    DirectAccessFileDb db = DirectAccessFileDb("database", 1024);
    DirectAccessFileDb db_async = DirectAccessFileDb("database_async", 1024);

    //for (int i = 0; i < 1024; ++i) {
    //    db.write_record(i, Record{ i, "Test", "TestSurname", 19, false });
    //}

    int choice = 0;
    do {
        show_menu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            add_record(db);
            break;
        case 2:
            display_records(db);
            break;
        case 3:
            delete_record(db);
            break;
        case 4:
        {
            DirectAccessFileDb db2 = DirectAccessFileDb("database2", 1024);
            DirectAccessFileDb db2_async = DirectAccessFileDb("databasу2_async", 1024);
            performance_test(db2, db2_async);
            break;
        }
        case 5:
            std::cout << "Exiting..." << std::endl;
            break;
        default:
            std::cout << "Invalid option. Try again." << std::endl;
            break;
        }
    } 
    while (choice != 5);
    return 0;
}
