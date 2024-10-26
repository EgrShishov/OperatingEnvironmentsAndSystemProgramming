#include "framework.h"
#include "RegistryUtility.h"
#include <iostream>
#include <sstream>
#include <commctrl.h>
#include <string>
#include <commdlg.h>

#define MAX_LOADSTRING 100

// for buttons
#define ID_BUTTON_READ 1
#define ID_BUTTON_EXPORT 2
#define ID_BUTTON_COMPARE 3
#define ID_LISTVIEW 4
#define ID_TREEVIEW 5
#define ID_MENU_SELECT_PATH 6

// for table
#define CHANGE_ADDED 1
#define CHANGE_REMOVED 2
#define CHANGE_MODIFIED 3

#pragma comment(lib, "comctl32.lib")

// Глобальные переменные
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

RegistryComparer comparer = RegistryComparer();
RegistryShanpshot snapshoter = RegistryShanpshot();
RegistryFileGenerator fGenerator = RegistryFileGenerator();

std::vector<RegistryComparer::RegistryEntry> snapshotBefore, snapshotAfter;
std::vector<RegistryComparer::RegistryEntry> changes;
std::wstring savedPath;

static HWND hButtonRead, hButtonExport, hEditOutput, hButtonCompare, hEditKeyInput, hTreeView, hListView;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


HTREEITEM AddItemToTree(HWND hTreeView, HTREEITEM hParent, LPCWSTR itemName, LPCWSTR subKey, bool isRoot) {
    TVITEM item;
    TVINSERTSTRUCT insert;

    item.mask = TVIF_TEXT | TVIF_PARAM;
    item.pszText = (LPWSTR)itemName;
    item.lParam = (LPARAM)subKey;

    insert.hParent = hParent;
    insert.hInsertAfter = TVI_LAST;
    insert.item = item;

    HTREEITEM hNewItem = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&insert);
    return hNewItem;
}

void ShowErrorMessage(LONG errorCode, const std::wstring& subKey) {
    LPWSTR errorMessage = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorMessage, 0, NULL);

    std::wstringstream ss;
    ss << L"Failed to open key: " << subKey << L"\nError code: " << errorCode << L"\nMessage: " << errorMessage;

    MessageBoxW(NULL, ss.str().c_str(), L"Error", MB_OK | MB_ICONERROR);

    if (errorMessage) {
        LocalFree(errorMessage);
    }
}

HTREEITEM AddItemToTree(HWND hTreeView, HTREEITEM hParent, const std::wstring& text) {
    TVINSERTSTRUCT tvis = { 0 };
    tvis.hParent = hParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT;
    tvis.item.pszText = const_cast<LPWSTR>(text.c_str());
    return (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvis);
}

// Рекурсивная функция для обхода реестра и добавления ключей в TreeView
void PopulateRegistryTree(HWND hTreeView, HTREEITEM hParent, HKEY hKeyRoot, const std::wstring& subKey) {
    HKEY hKey;
    if (RegOpenKeyEx(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        wchar_t keyName[255];
        DWORD keyNameSize;

        while (true) {
            keyNameSize = sizeof(keyName) / sizeof(keyName[0]);
            LONG result = RegEnumKeyEx(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL);
            if (result == ERROR_NO_MORE_ITEMS) {
                break;
            }
            if (result == ERROR_SUCCESS) {
                std::wstring fullSubKey = subKey.empty() ? keyName : subKey + L"\\" + keyName;
                HTREEITEM hNewItem = AddItemToTree(hTreeView, hParent, keyName);
                PopulateRegistryTree(hTreeView, hNewItem, hKeyRoot, fullSubKey);
            }
            index++;
        }

        RegCloseKey(hKey);
    }
}

void InitTreeView(HWND hWnd) {
    hTreeView = CreateWindowEx(0, WC_TREEVIEW, TEXT("Registry Tree"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
        20, 70, 300, 400, hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);

    if (!hTreeView) {
        MessageBox(hWnd, L"TreeView creation failed!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    HTREEITEM hRootItem = AddItemToTree(hTreeView, NULL, L"Компьютер");

    //hRootItem = AddItemToTree(hTreeView, NULL, L"HKEY_CLASSES_ROOT");
    //PopulateRegistryTree(hTreeView, hRootItem, HKEY_CLASSES_ROOT, L"");

    hRootItem = AddItemToTree(hTreeView, NULL, L"HKEY_CURRENT_USER");
    PopulateRegistryTree(hTreeView, hRootItem, HKEY_CURRENT_USER, L"");

    //hRootItem = AddItemToTree(hTreeView, NULL, L"HKEY_LOCAL_MACHINE");
    //PopulateRegistryTree(hTreeView, hRootItem, HKEY_LOCAL_MACHINE, L"");

    //hRootItem = AddItemToTree(hTreeView, NULL, L"HKEY_USERS");
    //PopulateRegistryTree(hTreeView, hRootItem, HKEY_USERS, L"");

    //hRootItem = AddItemToTree(hTreeView, NULL, L"HKEY_CURRENT_CONFIG");
    //PopulateRegistryTree(hTreeView, hRootItem, HKEY_CURRENT_CONFIG, L"");
}


//void AddListViewColumns() {
//    LVCOLUMN column;
//    ZeroMemory(&column, sizeof(column));
//    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
//
//    column.pszText = LPWSTR(L"Key Name");
//    column.cx = 200;
//    ListView_InsertColumn(hListView, 0, &column);
//
//    column.pszText = LPWSTR(L"Value Name");
//    column.cx = 100;
//    ListView_InsertColumn(hListView, 1, &column);
//
//    column.pszText = LPWSTR(L"New Value");
//    column.cx = 100;
//    ListView_InsertColumn(hListView, 2, &column);
//}

void InsertListViewItem(LPCWSTR keyName, LPCWSTR valueName, LPCWSTR newValue, int changeType) {
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT | LVIF_PARAM;

    item.iItem = ListView_GetItemCount(hListView);
    item.iSubItem = 0;
    item.pszText = (LPWSTR)keyName;
    item.lParam = changeType;
    ListView_InsertItem(hListView, &item);

    ListView_SetItemText(hListView, item.iItem, 1, (LPWSTR)valueName); 
    ListView_SetItemText(hListView, item.iItem, 2, (LPWSTR)newValue);

    LPCWSTR changeText = (changeType == CHANGE_ADDED) ? L"Added" :
        (changeType == CHANGE_REMOVED) ? L"Removed" :
        (changeType == CHANGE_MODIFIED) ? L"Modified" : L"";
    ListView_SetItemText(hListView, item.iItem, 3, (LPWSTR)changeText);
}

//void InitListView(HWND hWnd) {
//    hListView = CreateWindowEx(
//        WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
//        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
//        20, 400, 440, 280,
//        hWnd, NULL, GetModuleHandle(NULL), NULL
//    );
//    AddListViewColumns();
//}

void OnNotifyListView(LPNMHDR nmhdr) {
    if (nmhdr->code == NM_CUSTOMDRAW) {
        LPNMLVCUSTOMDRAW lvcd = (LPNMLVCUSTOMDRAW)nmhdr;

        if (lvcd->nmcd.dwDrawStage == CDDS_PREPAINT) {
            return;
        }

        // Обработка перерисовки отдельного элемента
        if (lvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
            int itemIndex = lvcd->nmcd.dwItemSpec;

            LVITEM item;
            item.mask = LVIF_PARAM;
            item.iItem = itemIndex;
            ListView_GetItem(hListView, &item);

            if (item.lParam == CHANGE_ADDED) {
                SetTextColor(lvcd->nmcd.hdc, RGB(0, 128, 0));
            }
            else if (item.lParam == CHANGE_REMOVED) {
                SetTextColor(lvcd->nmcd.hdc, RGB(255, 0, 0));
            }
            else if (item.lParam == CHANGE_MODIFIED) {
                SetTextColor(lvcd->nmcd.hdc, RGB(0, 0, 255));
            }

            return;
        }
    }
}

void CreateMenu(HWND hWnd) {
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    AppendMenu(hSubMenu, MF_STRING, ID_MENU_SELECT_PATH, L"Select Reference Path");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"File");
    SetMenu(hWnd, hMenu);
}

void CreateControls(HWND hWnd) {
    CreateMenu(hWnd);

    CreateWindow(L"BUTTON", L"Read Registry", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_READ, NULL, NULL);

    CreateWindow(L"BUTTON", L"Export to .reg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        150, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_EXPORT, NULL, NULL);

    CreateWindow(L"BUTTON", L"Compare", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        280, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_COMPARE, NULL, NULL);

    InitTreeView(hWnd);
    //hTreeView = CreateWindowEx(0, WC_TREEVIEW, L"Registry Tree",
    //    WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
    //    20, 70, 300, 400, hWnd, (HMENU)ID_TREEVIEW, NULL, NULL);

    hListView = CreateWindowEx(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        350, 70, 400, 400,
        hWnd, (HMENU)ID_LISTVIEW, NULL, NULL
    );

    LVCOLUMN column;
    ZeroMemory(&column, sizeof(column));
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    column.pszText = (LPWSTR)L"Key Name"; 
    column.cx = 200;
    ListView_InsertColumn(hListView, 0, &column);

    column.pszText = LPWSTR(L"Value Name"); 
    column.cx = 100;
    ListView_InsertColumn(hListView, 1, &column);

    column.pszText = LPWSTR(L"New Value"); 
    column.cx = 100;
    ListView_InsertColumn(hListView, 2, &column);
}

void SaveRegistrySnapshot(const std::wstring& path) {

    // saving snapshot here;
    std::wofstream file(path);
    if (file.is_open()) {
        file << L"Registry snapshot saved at " << path;
        file.close();
    }
}

void CompareRegistrySnapshots(const std::wstring& currentPath, const std::wstring& savedPath) {
    // Здесь должна быть реализация для сравнения текущего состояния с сохраненным
    // Это будет просто пример, на самом деле вам потребуется логика сравнения
    std::wstring changes = L"Changes detected between " + currentPath + L" and " + savedPath;

    // Отображение изменений в ListView
    // Здесь вам нужно добавить элементы в ListView с найденными изменениями
}

void CreateRegFile(const std::wstring& changes, const std::wstring& fileName) {
    // Реализуйте логику создания reg-файла на основе изменений
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_REGISTRYUTILITY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGISTRYUTILITY));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REGISTRYUTILITY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_REGISTRYUTILITY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ShowMessage(HWND hWnd, const std::string& message) {
    MessageBoxA(hWnd, message.c_str(), "Information", MB_OK | MB_ICONINFORMATION);
}

std::string WideToMultiByte(const std::wstring& wideString) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), static_cast<int>(wideString.size()), nullptr, 0, nullptr, nullptr);
    std::string converted(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), static_cast<int>(wideString.size()), &converted[0], size_needed, nullptr, nullptr);
    return converted;
}

void ReadRegistry(HWND hWnd, const std::wstring& regKey) {
    HKEY hKey;
    std::wstring subKey = L"";

    if (regKey.find(L"HKEY_CURRENT_USER") == 0) {
        hKey = HKEY_CURRENT_USER;
        subKey = regKey.substr(wcslen(L"HKEY_CURRENT_USER\\"));
    } else if (regKey.find(L"HKEY_LOCAL_MACHINE") == 0) {
        hKey = HKEY_LOCAL_MACHINE;
        subKey = regKey.substr(wcslen(L"HKEY_LOCAL_MACHINE\\"));
    } else if (regKey.find(L"HKEY_CLASSES_ROOT") == 0) {
        hKey = HKEY_CLASSES_ROOT;
        subKey = regKey.substr(wcslen(L"HKEY_CLASSES_ROOT\\"));
    } else if (regKey.find(L"HKEY_USERS") == 0) {
        hKey = HKEY_USERS;
        subKey = regKey.substr(wcslen(L"HKEY_USERS\\"));
    } else if (regKey.find(L"HKEY_CURRENT_CONFIG") == 0) {
        hKey = HKEY_CURRENT_CONFIG;
        subKey = regKey.substr(wcslen(L"HKEY_CURRENT_CONFIG\\"));
    } else {
        ShowMessage(hWnd, "Unsupported root key.");
        return;
    }

    if (subKey.empty()) {
        ShowMessage(hWnd, "No subkey specified.");
        return;
    }

    std::string subKeyStr = WideToMultiByte(subKey);
    std::vector<RegistryComparer::RegistryEntry> entries = comparer.getRegistryEntries(hKey, subKeyStr);

    if (!entries.empty()) {
        std::string output;
        for (const auto& entry : entries) {
            output += entry.key + "\\" + entry.name + " = ";

            if (entry.type == REG_SZ) {
                output += std::string(reinterpret_cast<const char*>(entry.data.data()), entry.data.size());
            } else if (entry.type == REG_DWORD) {
                DWORD value = *(DWORD*)(entry.data.data());
                output += std::to_string(value);
            } else if (entry.type == REG_BINARY) {
                output += "BINARY DATA";
            } else {
                output += "UNKNOWN DATA TYPE";
            }

            output += "\r\n";
        }
        SetWindowTextA(hEditOutput, output.c_str());

    } else {
        ShowMessage(hWnd, "Failed to open or read the registry key.");
    }
}

void ReadRegistry(HWND hWnd) {
    wchar_t regKey[256];
    GetWindowTextW(hEditKeyInput, regKey, 256);

    std::wstring keyStr = std::wstring(regKey);
    ReadRegistry(hWnd, keyStr);
}

void ShowRegistryChanges(HWND hWnd) {
    //snapshotAfter = snapshoter.saveRegistrySnapshot(HKEY_CURRENT_USER,"Software", "");
    //comparer.compareEntries(snapshotBefore, snapshotAfter);
    //std::string output;
    //for (const auto& change : changes) {
    //    output += "Key: " + change.key + "\n";
    //    if (change.type == RegistryChange::Added) { // compares valuetype and registrychange, fix later
    //        output += "Added: " + change.name + " = " + change.newValue + "\n";
    //    }
    //    else if (change.type == RegistryChange::Modified) {
    //        output += "Modified: " + change.name + " from " + change.oldValue + " to " + change.newValue + "\n";
    //    }
    //    else if (change.type == RegistryChange::Deleted) {
    //        output += "Deleted: " + change.name + "\n";
    //    }
    //    output += "\n";
    //}
    //SetWindowTextA(hEditOutput, output.c_str());
}

void ExportToRegFile(HWND hWnd) {
    //if (changes.empty()) {
    //    ShowMessage(hWnd, "No changes to export.");
    //    return;
    //}
    //comparer.exportToRegFile("exported_changes.reg", changes);
    //ShowMessage(hWnd, "Changes exported to exported_changes.reg.");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                switch (wmId) {
                    case ID_BUTTON_READ:
                        ReadRegistry(hWnd);
                        SaveRegistrySnapshot(L"snapshot.reg");
                        break;
                    case ID_BUTTON_COMPARE:
                        ShowRegistryChanges(hWnd);
                        break;
                    case ID_BUTTON_EXPORT:
                        ExportToRegFile(hWnd);
                        break;
                    case IDM_ABOUT:
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                        break;
                    case ID_MENU_SELECT_PATH: {
                        OPENFILENAME ofn;
                        wchar_t szFile[260];

                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hWnd;
                        ofn.lpstrFile = szFile;
                        ofn.lpstrFile[0] = '\0';
                        ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrFilter = L"Registry Files (*.reg)\0*.reg\0All Files (*.*)\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.lpstrFileTitle = NULL;
                        ofn.nMaxFileTitle = 0;
                        ofn.lpstrInitialDir = NULL;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

                        if (GetOpenFileName(&ofn)) {
                            savedPath = ofn.lpstrFile;
                            MessageBox(hWnd, L"Path selected", L"Info", MB_OK);
                        }
                        break;
                    }
                    case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        case WM_CREATE:
            CreateControls(hWnd);
            break;

        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if (nmhdr->hwndFrom == hListView) {
                OnNotifyListView(nmhdr);
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    return (INT_PTR)FALSE;
}