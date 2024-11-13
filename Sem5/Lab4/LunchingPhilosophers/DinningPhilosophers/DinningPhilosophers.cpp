//#include "Philosophers.cpp"
#include <string>
#include "framework.h"
#include "DinningPhilosophers.h"

#define MAX_LOADSTRING 100



HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HANDLE hwndOutput;
HANDLE hwndEatTime;
bool running;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    auto CLASS_NAME = convert_to_LPCWSTR("PhilosopherWindow");

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        convert_to_LPCWSTR("Dining Philosophers"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        450,
        300,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    int philosopers_amount = 5;
    int time_out_size = 1000; // in ms

    //LaunchinhPhilosophers lp = LaunchinhPhilosophers(philosopers_amount, time_out_size);
    return 0;
}

//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//                     _In_opt_ HINSTANCE hPrevInstance,
//                     _In_ LPWSTR    lpCmdLine,
//                     _In_ int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    // TODO: Разместите код здесь.
//
//    // Инициализация глобальных строк
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_DINNINGPHILOSOPHERS, szWindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);
//
//    // Выполнить инициализацию приложения:
//    if (!InitInstance (hInstance, nCmdShow))
//    {
//        return FALSE;
//    }
//
//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DINNINGPHILOSOPHERS));
//
//    MSG msg;
//
//    // Цикл основного сообщения:
//    while (GetMessage(&msg, nullptr, 0, 0))
//    {
//        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }
//
//    return (int) msg.wParam;
//}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DINNINGPHILOSOPHERS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DINNINGPHILOSOPHERS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LPCWSTR convert_to_LPCWSTR(std::string str) {
    int length = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t* wideStr = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideStr, length);

    return LPCWSTR(wideStr);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        hwndOutput = CreateWindow(convert_to_LPCWSTR("EDIT"), convert_to_LPCWSTR(""), WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL,
            10, 10, 400, 200, hwnd, NULL, NULL, NULL);

        hwndEatTime = CreateWindow(convert_to_LPCWSTR("EDIT"), convert_to_LPCWSTR("1000"), WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 220, 100, 20, hwnd, NULL, NULL, NULL);

        CreateWindow(convert_to_LPCWSTR("BUTTON"), convert_to_LPCWSTR("Change Eat Time"), WS_CHILD | WS_VISIBLE,
            120, 220, 150, 20, hwnd, (HMENU)1, NULL, NULL);

        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            //ChangeEatTime(hwnd);
        }
        return 0;
    case WM_DESTROY:
        running = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
