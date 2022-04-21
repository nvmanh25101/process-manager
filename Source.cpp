#include <windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <commctrl.h>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

char szClassName[] = "ProcessMngr";

int x = GetSystemMetrics(SM_CXSCREEN) / 3,
y = GetSystemMetrics(SM_CYSCREEN) / 3;

int iSelect;
int _procID;

HWND proc_list;

LPWSTR _processName;

wchar_t _processID[10];
wchar_t data[10];

char _buffProcessID[10];

HMENU hPopupMenu;

POINT p;

int _showProcesses();
BOOL _killProcess(int _procID);
void AddColumn(HWND hView, LPSTR text, int Col, int Width, DWORD dStyle);
void AddIndex(HWND hView, int Index);
void AddRow(HWND hView, LPWSTR text, int Index, int Col);
DWORD WINAPI _autoRefresh(LPVOID);

HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, FF_DONTCARE, L"Arial");

int WINAPI WinMain(HINSTANCE hThisInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszArgument,
    int nFunsterStil)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = (LPCWSTR)szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

    if (!RegisterClassEx(&wincl))
        return 0;

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME,
        (LPCWSTR)szClassName,
        L"Process Manager",
        WS_SYSMENU | WS_MINIMIZEBOX,
        x,
        y,
        325,
        390,
        HWND_DESKTOP,
        NULL,
        hThisInstance,
        NULL
    );

    ShowWindow(hwnd, nFunsterStil);

    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CREATE:
        InitCommonControls();

        proc_list = CreateWindowEx(
            0,
            WC_LISTVIEW,
            0,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
            0,
            0,
            306,
            349,
            hwnd,
            NULL,
            0,
            NULL
        );

        SendMessage(proc_list, WM_SETFONT, (WPARAM)hFont, 0);

        ListView_SetExtendedListViewStyle(proc_list, LVS_EX_FULLROWSELECT);

        AddColumn(proc_list, (LPSTR)L"Name", 0, 230, LVCFMT_LEFT);
        AddColumn(proc_list, (LPSTR)L"ID", 1, 70, LVCFMT_LEFT);

        _showProcesses();
        CreateThread(NULL, 0, _autoRefresh, 0, 0, NULL);
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->hwndFrom == proc_list && ((LPNMHDR)lParam)->code == NM_RCLICK)
        {
            iSelect = SendMessage(proc_list, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);

            ListView_GetItemText(proc_list, iSelect, 1, data, sizeof(data))

                _procID = _wtoi(data);

            if (iSelect >= 0)
            {
                GetCursorPos(&p);

                hPopupMenu = CreatePopupMenu();

                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Terminate");

                int choice = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y + 23, 0, hwnd, NULL);

                switch (choice)
                {
                case 1:
                    _killProcess(_procID);
                    break;
                }

                DestroyMenu(hPopupMenu);
            }

            ListView_SetItemState(proc_list, iSelect, FALSE, LVIS_FOCUSED);
        }
        return false;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

int _showProcesses()
{
    ListView_DeleteAllItems(proc_list);

    typedef BOOL(WINAPI* TH32_PROCESS)
        (HANDLE hSnapShot, LPPROCESSENTRY32 lppe);

    static TH32_PROCESS pProcess32First = NULL;
    static TH32_PROCESS pProcess32Next = NULL;

    PROCESSENTRY32 pe32 = { 0 };

    HANDLE hSnapshot = NULL;

    HINSTANCE hDll = LoadLibrary(L"kernel32.dll");

    if (hDll == 0)
    {
        MessageBox(NULL, L"Error getting processes !", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    pProcess32First = (TH32_PROCESS)GetProcAddress(hDll, "Process32First");
    pProcess32Next = (TH32_PROCESS)GetProcAddress(hDll, "Process32Next");

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != (HANDLE)-1)
    {
        pe32.dwSize = sizeof(PROCESSENTRY32);

        int proc_cnt = 0, thrd_cnt = 0;

        if (pProcess32First(hSnapshot, &pe32))
        {
            while (pProcess32Next(hSnapshot, &pe32))
            {
                _processName = new wchar_t[pe32.dwSize];

                sprintf(_buffProcessID, "%d", pe32.th32ProcessID);
                mbstowcs(_processID, _buffProcessID, 10);

                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pe32.szExeFile, pe32.dwSize, _processName, pe32.dwSize);

                AddIndex(proc_list, 0);
                AddRow(proc_list, _processName, 0, 0);
                AddRow(proc_list, _processID, 0, 1);
            }
        }
    }
    CloseHandle(hSnapshot);

    return 0;
}

BOOL _killProcess(int procID)
{
    HANDLE hProcessSnap;
    HANDLE hProcess;

    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    pe32.dwSize = sizeof(PROCESSENTRY32);

    Process32First(hProcessSnap, &pe32);

    while (Process32Next(hProcessSnap, &pe32))
    {
        if (procID == pe32.th32ProcessID)
        {
            hProcess = OpenProcess(PROCESS_TERMINATE, 0, pe32.th32ProcessID);

            TerminateProcess(hProcess, 0);

            ListView_DeleteItem(proc_list, iSelect);
        }
    }
    CloseHandle(hProcessSnap);
    CloseHandle(hProcess);

    return 0;
}

DWORD WINAPI _autoRefresh(LPVOID)
{
    while (1)
    {
        Sleep(7500);
        _showProcesses();
    }
}

void AddColumn(HWND hView, LPSTR text, int Col, int Width, DWORD dStyle)
{
    LVCOLUMN lvc = { 0 };
    memset(&lvc, 0, sizeof(LVCOLUMN));
    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = dStyle;

    lvc.iSubItem = Col;
    lvc.cx = Width;
    lvc.pszText = (LPWSTR)text;

    ListView_InsertColumn(hView, Col, &lvc);
}

void AddIndex(HWND hView, int Index)
{
    LVITEM lv;
    memset(&lv, 0, sizeof(LVITEM));

    lv.mask = LVIF_TEXT;
    lv.iSubItem = 0;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.iItem = Index;

    ListView_InsertItem(proc_list, &lv);
}

void AddRow(HWND hView, LPWSTR text, int Index, int Col)
{
    ListView_SetItemText(hView, Index, Col, (LPWSTR)text);
}