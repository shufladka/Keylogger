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
        case OnClearedList:
            ListView_DeleteAllItems(hwndListView);
            break;
        case OnRecordAction:

            // Меняем текст кнопки в зависимости от состояния
            if (isRecordStarted) {
                SetWindowText(hwndRecordButton, L"Начать запись");
            }
            else {
                SetWindowText(hwndRecordButton, L"Остановить запись");
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
        //InitializeSharedMemory(hWnd);
        MainWndAddMenues(hWnd);
        MainWndAddWidgets(hWnd);
        break;
    case WM_DESTROY:
        //CleanupResources();
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
        { 325, L"Путь к запущенному процессу" },
        { 225, L"Дата и время действия" },
        { 105, L"Код влавиши" },
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

    // Заполняем таблицу данными
    for (size_t i = 0; i < keyloggerRecords.size(); ++i) {
        const auto& record = keyloggerRecords[i];

        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = static_cast<int>(i);  // Индекс элемента для вставки в ListView
        lvItem.iSubItem = 0;  // Индекс первого столбца (ID процесса)

        // Заполнение первого столбца (ID процесса)
        wstring processIdText = to_wstring(record.processId);
        lvItem.pszText = const_cast<LPWSTR>(processIdText.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        // 2. Путь к запущенному процессу (столбец 1)
        ListView_SetItemText(hwndListView, static_cast<int>(i), 1, const_cast<LPWSTR>(record.processPath.c_str()));

        // 3. Дата и время действия (столбец 2)
        ListView_SetItemText(hwndListView, static_cast<int>(i), 2, const_cast<LPWSTR>(record.dateTime.c_str()));

        // 4. Код клавиши (столбец 3)
        wstring keyCodeText = to_wstring(record.keyCode);
        ListView_SetItemText(hwndListView, static_cast<int>(i), 3, const_cast<LPWSTR>(keyCodeText.c_str()));

        // 5. Символ клавиши (столбец 4)
        wstring keyCharText(1, record.keyChar);
        ListView_SetItemText(hwndListView, static_cast<int>(i), 4, const_cast<LPWSTR>(keyCharText.c_str()));
    }
}

// Добавление пунктов меню
void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    AppendMenu(RootMenu, MF_STRING, OnCreateFile, L"Создать файл");
    AppendMenu(RootMenu, MF_STRING, OnReadFile, L"Выбрать файл");
    AppendMenu(RootMenu, MF_STRING, OnClearedList, L"Очистить список");
    AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Выход");
    SetMenu(hwnd, RootMenu);
}

// Добавление виджетов в рабочую область приложения
void MainWndAddWidgets(HWND hwnd) {


    hwndRecordButton = CreateWindowA("button", "Начать запись", WS_VISIBLE | WS_CHILD | ES_CENTER, 25, 10, 150, 40, hwnd, (HMENU)OnRecordAction, NULL, NULL);
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
vector<KeyloggerRecord> ParseKeyloggerData(const string& fileContent) {
    vector<KeyloggerRecord> keyloggerData;
    stringstream stream(fileContent);
    string line;

    // Очищаем текущий массив перед добавлением новых записей
    keyloggerRecords.clear();

    // Чтение каждой строки из файла
    while (getline(stream, line)) {
        if (line.empty()) continue;

        stringstream ss(line);  // Создаем поток для текущей строки
        string part;

        KeyloggerRecord record;

        // Чтение и разбор строки по разделителю ";"
        if (getline(ss, part, ';')) {
            record.processId = stoi(part); // ID процесса
        }
        if (getline(ss, part, ';')) {
            record.processPath = wstring(part.begin(), part.end()); // Путь к процессу
        }
        if (getline(ss, part, ';')) {
            record.dateTime = wstring(part.begin(), part.end()); // Дата и время
        }
        if (getline(ss, part, ';')) {
            record.keyCode = stoi(part); // Код клавиши
        }
        if (getline(ss, part, ';')) {
            record.keyChar = part[0]; // Символ клавиши
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
            const char* content = "Это новый текстовый файл!";
            DWORD bytesWritten;
            WriteFile(hFile, content, strlen(content), &bytesWritten, NULL);
            CloseHandle(hFile);

            MessageBoxA(hwnd, "Файл успешно создан!", "Успех", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBoxA(hwnd, "Не удалось создать файл.", "Ошибка", MB_OK | MB_ICONERROR);
        }
    }
    else {
        MessageBoxA(hwnd, "Выбор папки отменен.", "Отмена", MB_OK | MB_ICONINFORMATION);
    }
}
