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

#include <psapi.h>

//#include <iostream>

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

#define TextBufferSize	1000

struct KeyloggerRecord {
    DWORD processId;          // ID ��������
    wstring processPath;       // ���� � ����������� ��������
    wstring dateTime;          // ���� � ����� ��������
    int keyCode;              // ��� �������
    char keyChar;             // ������ �������
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

HWND hwndRecordButton;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void                MainWndAddMenues(HWND hwnd);
void                MainWndAddWidgets(HWND hwnd);

void                SetOpenFileParams(HWND hwnd);

void                DefineColumns(HWND hwndLV);
//void				PhoneBookFilling(HWND hwndListView, const vector<PhoneBookEntry>& phonebookData);

//void                PickTheFile(HWND hwndListView, HWND hwndOwner);
//void				LoadDataToTable(HWND hwndListView);
//void                OnSearchByField(HWND hEditControl, HWND hComboBox, HWND hListView);

void LoadKeyloggerRecordsFromFile(HWND hWnd);
void KeyloggerFilling(HWND hwndListView);

BOOL SelectFolderDialog(HWND hwnd, char* folderPath);
void CreateFileInSelectedFolder(HWND hwnd);

void LoadKeyloggerRecordsFromFile(HWND hwnd);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetKeyboardHook();
void RemoveKeyboardHook();

