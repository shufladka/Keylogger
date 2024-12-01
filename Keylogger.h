#pragma once

#include "resource.h"
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <fstream>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <ctime>
#include <unordered_map>
#include <shellapi.h>
#include <psapi.h>


using namespace std;

#define MAX_LOADSTRING  100
#define IDC_LISTVIEW    101

#define OnClearedField	1
#define OnClearedList	2
#define OnSearch    	3
#define OnReadFile		4
#define OnLoadDatabase  5

#define OnCreateFile	6
#define OnRecordAction	7
#define OnOpenFile  	8

#define TextBufferSize	1000

struct KeyloggerRecord {
    DWORD processId;          // ID процесса
    wstring processPath;       // Путь к запущенному процессу
    wstring dateTime;          // Дата и время действия
    int keyCode;              // Код клавиши
    wstring keyChar;             // Символ клавиши

    // Конструктор для инициализации значений
    KeyloggerRecord(DWORD pid = 0, wstring path = L"", wstring dt = L"", int code = 0, wstring key = L"")
        : processId(pid), processPath(path), dateTime(dt), keyCode(code), keyChar(key) {
    }
};

vector<KeyloggerRecord> keyloggerRecords;  // Глобальный контейнер для хранения записей
const char delimeter = ';';  // Разделитель для записи

bool fileOpened;								// Флаг открытия файла для записи
bool isRecordStarted = false;					// Флаг начала записи действий в файл

HHOOK keyboardHook;                             // Хук клавиатуры
HWND hEditControl;								// Поле ввода
HWND hComboBox;									// Выпадающее меню
char filename[MAX_PATH];						// Для хранения пути к выбранному файлу

OPENFILENAMEA ofn;								// Структура для диалога открытия файла
HINSTANCE hInst;                                // Текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // Имя класса главного окна
HWND hwndListView;                              // Переменная для ListView

HWND hwndRecordButton;                          // Кнопка начала/останова записи нажатий клавиш
HWND hwndFilePathLabel;                         // Указатель на виджет, отображающий путь файла

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void                MainWndAddMenues(HWND hwnd);
void                MainWndAddWidgets(HWND hwnd);

void                SetOpenFileParams(HWND hwnd);

void                DefineColumns(HWND hwndLV);


void LoadKeyloggerRecordsFromFile(HWND hWnd);
void KeyloggerFilling(HWND hwndListView);

BOOL SelectFolderDialog(HWND hwnd, char* folderPath);
void CreateFileInSelectedFolder(HWND hwnd);
void WriteToFileANSI(const KeyloggerRecord& record);

void LoadKeyloggerRecordsFromFile(HWND hwnd);

wstring GetKeyStringFromCode(int keyCode);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetKeyboardHook();
void RemoveKeyboardHook();

std::wstring ANSIToWideString(const std::string& ansiStr);
void OpenTextFile();
void UpdateFilePathLabel();

