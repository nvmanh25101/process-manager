#include <windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <commctrl.h>

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);//Lresult là một giá trị nguyên mà chương trình của bạn trở lại WINDOW chứa phản hồi từ chương trình cho một tin nhắn cụ thể (mã thông báo)
//Có 4 tham số :
//hwnd là một handle tới window.
//message là mã message.
//wParam and lParam chứa dữ liệu bổ sung liên quan đến message.
char szClassName[] = "ProcessMngr";

int x = GetSystemMetrics(SM_CXSCREEN) / 3,// chiều rộng của màn hình hiển thị chính / 3
y = GetSystemMetrics(SM_CYSCREEN) / 3;// chiều cao của màn hình hiển thị chính / 3

int iSelect;
int _procID;
HWND proc_list;
LPWSTR _processName;//L:Long// P:Pointer//W:Wide(Trong c++ thì wide được định nghĩa cho ký tự unicode)//STR:String
wchar_t _processID[10];//Kiểu dữ liệu
wchar_t data[10];//Kiểu dữ liệu
char _buffProcessID[10];//Kiểu dữ liệu
HMENU hPopupMenu;// menu
POINT p;
//hàm hiển thị tiến trình
int _showProcesses();
//hàm ngắt tiến trình
BOOL _killProcess(int _procID);
//hàm thêm cột
void AddColumn(HWND hView, LPSTR text, int Col, int Width, DWORD dStyle);
//hàm thêm index
void AddIndex(HWND hView, int Index);
//hàm thêm dòng
void AddRow(HWND hView, LPWSTR text, int Index, int Col);
//hàm tự động làm mới các tiến trình
DWORD WINAPI _autoRefresh(LPVOID);
//CreateFont() tạo một HFONT, một handle tới một Logical Font trong bộ nhớ.
HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, FF_DONTCARE, L"Arial");
//hàm winmain hiển thị ra window
int WINAPI WinMain(HINSTANCE hThisInstance, //Quản lý việc thực thi của chương trình, nó quản lý tài nguyên của chương trình đang cài đặt
    HINSTANCE hPrevInstance, //Con trỏ trỏ đến việc thực thi trước đó
    LPSTR lpszArgument, //Command line arguments, chuỗi ký tự bình thường
    int iCmdShow) //1 số nguyên để cung cấp cho hàm ShowWindow()
{
    HWND hwnd; //handle window
    MSG messages; //chứa thông tin message
    // tạo window class
    WNDCLASSEX wincl; //chứa thông tin window class, được dùng với RegisterClassEx

    wincl.hInstance = hThisInstance;// quản lý thông tin của cửa sổ, tương đương với giá trị khai báo ở WinMain()
    wincl.lpszClassName = (LPCWSTR)szClassName;//Tên của window class
    wincl.lpfnWndProc = WindowProcedure;//con trỏ trỏ đến WindowProcedure
    wincl.style = CS_DBLCLKS; //kiểu của class
    wincl.cbSize = sizeof(WNDCLASSEX); //kích thước trên bộ nhớ của wincl
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);// icon lớn của class
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // icon nhỏ của class
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);//Tải con trỏ mũi tên
    wincl.lpszMenuName = NULL;// Không cần một menu/ con trỏ trỏ về dữ liệu của các thanh menu
    wincl.cbClsExtra = 0; // số lượng dữ liệu tối đa được cài đặt cho class, thường có giá trị 0
    wincl.cbWndExtra = 0; // số lượng dữ liệu tối đa được cài đặt cho class, thường có giá trị 0
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;// màu background

    // kiểm tra việc đăng ký window class có thành công không
    if (!RegisterClassEx(&wincl)) // đăng ký 1 window class để sử dụng cho CreateWindowEx
        return 0;

    // đăng ký 1 handle cho window class, khi tương tác vs handle này cũng có nghĩa là đang tương tác với chính wincl
    hwnd = CreateWindowEx( //Tạo một cửa sổ con,cửa sổ bật lên hoặc cửa sổ con trồng lên nhau với kiểu cửa sổ mở rộng hoặc giống như create window(tạo một cửa sổ con)
        WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, // kiểu mở rộng của window style
        (LPCWSTR)szClassName, // Tên của Class, để hệ thống hiểu được ta đang thao tác với Class nào
        L"Process Manager", // Tiêu đề
        WS_SYSMENU | WS_MINIMIZEBOX, // kiểu của window style
        x, // tọa độ x 
        y, // tọa độ y
        325, // chiều rộng 
        390, // chiều cao
        HWND_DESKTOP,
        NULL, // Handle của menu chương trình
        hThisInstance, // Instance của chương trình
        NULL // Con trỏ dùng để gửi các thông tin bổ sung
    );

    ShowWindow(hwnd, iCmdShow); // hiển thị cửa sổ, đặt trạng thái hiển thị của window được chỉ định.


    while (GetMessage(&messages, NULL, 0, 0)) // hàm lấy thông tin từ message queue
    {
        TranslateMessage(&messages);// Chuyển các tin nhắn phím ảo thành các tin nhắn ký tự. Các thông điệp ký tự được đăng vào hàng đợi tin nhắn của chuỗi gọi, sẽ được đọc vào lần tiếp theo chuỗi gọi hàm GetMessage
        DispatchMessage(&messages);// Hàm này sẽ xác định message này được gửi đến cửa sổ nào và thực thi nó. Gửi một thông báo đến một thủ tục của window. Nó thường được sử dụng để gửi một tin nhắn được truy xuất bởi chức năng GetMessage
    }

    return messages.wParam;// trả về wParam của message
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)//gọi thủ tục thông báo của window//HWND là xử lý cho window//UNIT là mã nhắn tin//wparam//lparam chứa dữ liệu bổ sung liên quan đến tin nhắn
{
    switch (message) // bắt các trường hợp của message
    {
    case WM_DESTROY://nó được gửi đến thủ tục cửa sổ của windows khi một cửa sổ bị hủy
        PostQuitMessage(0);//Cho hệ thống biết rằng một luồng đã đưa ra yêu cầu kết thúc (thoát)
        break;

    case WM_CREATE://Khởi tạo cửa sổ
        InitCommonControls();//Đăng ký và khởi tạo một số lớp cửa sổ điều khiển chung nhất định

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

        SendMessage(proc_list, WM_SETFONT, (WPARAM)hFont, 0);//Gửi thông báo được chỉ định đến một cửa sổ hoặc các cửa sổ. Hàm SendMessage gọi thủ tục cửa sổ cho cửa sổ được chỉ định và không trả về cho đến khi thủ tục cửa sổ xử lý thông báo.

        ListView_SetExtendedListViewStyle(proc_list, LVS_EX_FULLROWSELECT);//Dùng để sử dụng điều khiển chế độ xem danh sách mở rộng

        AddColumn(proc_list, (LPSTR)L"Name", 0, 230, LVCFMT_LEFT);//Thêm cột Name
        AddColumn(proc_list, (LPSTR)L"ID", 1, 70, LVCFMT_LEFT);//Thêm cột ID

        _showProcesses();
        CreateThread(NULL, 0, _autoRefresh, 0, 0, NULL);//Tạo một chuỗi để thực thi trong không gian địa chỉ ảo của quá trình gọi.
        break;

    case WM_NOTIFY: // message được gửi từ control 
        if (((LPNMHDR)lParam)->hwndFrom == proc_list && ((LPNMHDR)lParam)->code == NM_RCLICK) //nếu message là tiến trình và click chuột phải
        {
            iSelect = SendMessage(proc_list, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);//Gửi thông báo được chỉ định đến một cửa sổ hoặc các cửa sổ. Hàm SendMessage gọi thủ tục cửa sổ cho cửa sổ được chỉ định và không trả về cho đến khi thủ tục cửa sổ xử lý thông báo.

            ListView_GetItemText(proc_list, iSelect, 1, data, sizeof(data))//Nhận văn bản hoặc trang con ở chế độ xem danh sách

                _procID = _wtoi(data);//Chuyển đổi một chuỗi thành số nguyên

            if (iSelect >= 0) //nếu có chọn tiến trình
            {
                GetCursorPos(&p);//Truy xuất vị trí con trỏ trên vị trí màn hình

                hPopupMenu = CreatePopupMenu();//Tạo menu thả xuống, menu con

                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Terminate");//Chèn một mục menu mới vào menu, di chuyển các mục khác xuống menu.

                int choice = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y + 23, 0, hwnd, NULL);//Hiển thị menu lối tắt tại vị trí đã chỉ định và theo dõi việc lựa chọn các mục trên menu.Menu lối tắt xuất hiện ở bất kỳ đâu trên màn hình

                switch (choice) //kiểm tra nếu chọn vào mục ở trên menu thì sẽ gọi đến hàm _killProcess
                {
                case 1:
                    _killProcess(_procID);
                    break;
                }

                DestroyMenu(hPopupMenu); //Huỷ menu và giải phóng bộ nhớ mà menu chiếm
            }

            ListView_SetItemState(proc_list, iSelect, FALSE, LVIS_FOCUSED);//Thay đổi trạng thái của một mục trong điều khiển dạng xem danh sách
        }
        return false;

    default:
        // để thư viện win32 api tự xử lý
        return DefWindowProc(hwnd, message, wParam, lParam); //Hàm này như một phần của quá trình xử lý tin nhắn trong trường hợp như vậy ứng dụng có thể sửa đổi các tham số thông báo trước khi chuyển thông báo tới DefWindowProc
    }

    return 0;
}

int _showProcesses()
{
    ListView_DeleteAllItems(proc_list);//Xóa tất cả các danh sách tiến trình

    typedef BOOL(WINAPI* TH32_PROCESS)//Thêm tên mới vào kiểu có sẵn
        (HANDLE hSnapShot, LPPROCESSENTRY32 lppe);

    static TH32_PROCESS pProcess32First = NULL;//static là khai báo biến chung áp dụng chung cho đối tượng
    static TH32_PROCESS pProcess32Next = NULL;

    PROCESSENTRY32 pe32 = { 0 };// danh sách các tiến trình 

    HANDLE hSnapshot = NULL;

    HINSTANCE hDll = LoadLibrary(L"kernel32.dll"); //xử lý cho hDll//LoadLibrary tải lên các thư viện của Window//kernel32.dll Tạo thành mã cơ bản mà được xây dựng và cung cấp các chức năng cơ bản như quản lý bộ nhớ,xử lý ngắt vào ra chuyển chúng thành các lệnh CPU và hướng dẫn cho các thành phần điện tử

        if (hDll == 0)
        {
            MessageBox(NULL, L"Error getting processes !", L"Error", MB_OK | MB_ICONERROR); //Hiển thị thông báo lỗi
                return 0;
        }

    pProcess32First = (TH32_PROCESS)GetProcAddress(hDll, "Process32First");//Truy xuất về thông tin quy trình đầu tiên
    pProcess32Next = (TH32_PROCESS)GetProcAddress(hDll, "Process32Next");//Truy xuất thông tin quy trình tiếp theo

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);// chụp lại quy trình được chỉ định,cũng như moodun và luồng được sử dụng bởi quy trình này

    if (hSnapshot != (HANDLE)-1)
    {
        pe32.dwSize = sizeof(PROCESSENTRY32);//Đặt kích thước cấu trúc trước khi sử dụng

        if (pProcess32First(hSnapshot, &pe32))
        {
            while (pProcess32Next(hSnapshot, &pe32))
            {
                _processName = new wchar_t[pe32.dwSize];

                sprintf(_buffProcessID, "%d", pe32.th32ProcessID); // ghi vào dạng chuỗi
                mbstowcs(_processID, _buffProcessID, 10); // chuyển đổi sang chuỗi ký tự

                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pe32.szExeFile, pe32.dwSize, _processName, pe32.dwSize);//MultiByteToWideChar là Ánh xạ một chuỗi ký tự thành một chuỗi UTF-16(Ký tự rộng) chuỗi ký tự không nhất thiết phải từ một bộ ký tự nhiều byte

                AddIndex(proc_list, 0); //Thêm thuộc tính vào chỉ mục được sử dụng để tìm kiếm
                AddRow(proc_list, _processName, 0, 0);//Thêm một hàng mới vào cuối giá trị cột
                AddRow(proc_list, _processID, 0, 1);//Thêm một hàng mới vào cuối giá
                AddRow(proc_list, _processName, 0, 0);//Thêm một hàng mới vào cuối giá
            }
        }
    }
    CloseHandle(hSnapshot);

    return 0;
}

BOOL _killProcess(int procID)
{
    HANDLE hProcessSnap;//Tìm địa chỉ của window trong bộ nhớ
    HANDLE hProcess;

    PROCESSENTRY32 pe32;//Kiểu dữ liệu

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    //Hàm CreateToolhelp32Snapshot chụp nhanh các quy trình chỉ định ,modun được sử dụng bời quy trinh này

    pe32.dwSize = sizeof(PROCESSENTRY32);//Chứa kích thước của bộ đệm màn hình //Nhận một tham số bất kỳ kiểu dữ liệu nào trả về kích thước của kiểu dữ liệu đó

    Process32First(hProcessSnap, &pe32);//Truy xuất thông tin đầu tiên gặp phải trong hệ thống

    while (Process32Next(hProcessSnap, &pe32))//Hiển thị lên các quy trình trên hệ thống
    {
        if (procID == pe32.th32ProcessID)
        {
            hProcess = OpenProcess(PROCESS_TERMINATE, 0, pe32.th32ProcessID);//Hàm OpenProcess trả về xử lý của tiến trình //Bắt buộc phải kết thúc một quá trình bằng Terminalprocess(Chấm dứt quá trình được chỉ định và tất cả luồng của nó)
            TerminateProcess(hProcess, 0);//Chấm dứt quá trình được chỉ định và tất cả các luồng của nó.

            ListView_DeleteItem(proc_list, iSelect); //Xóa một mục khỏi một danh sách điều khiển        
        }
     }
        CloseHandle(hProcessSnap);//Đóng một xử lý
        CloseHandle(hProcess);

        return 0;
}

DWORD WINAPI _autoRefresh(LPVOID)//Dword là khai báo kiểu số nguyên không dấu//WINAPI là hàm gọi chức năng của API Window
{
    while (1)
    {
        Sleep(7500);
        _showProcesses();
    }
}

void AddColumn(HWND hView, LPSTR text, int Col, int Width, DWORD dStyle)//Thêm cột và chuyển chế độ chỉnh sửa
{
    LVCOLUMN lvc = { 0 };//Chứa thông tin về một cột trong chế độ xem báo cáo. Cấu trúc này được sử dụng cho cả việc tạo và thao tác với các cột
    memset(&lvc, 0, sizeof(LVCOLUMN));
    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT; // cho phép pszText, iSubItem, cx, fmt
    lvc.fmt = dStyle;

    lvc.iSubItem = Col;
    lvc.cx = Width;
    lvc.pszText = (LPWSTR)text;

    ListView_InsertColumn(hView, Col, &lvc);//Chèn một cột mới trong điều khiển chế độ xem danh sách
}

void AddIndex(HWND hView, int Index)
{
    LVITEM lv; //Chỉ định nhận các thuộc tính của một mục dạng xem danh sách.cấu trúc này đã được cập nhật mới để hỗ trợ giá trị mặt nạ mới cho phép thụt lề mục
        memset(&lv, 0, sizeof(LVITEM)); //lấp đầy một khối bộ nhớ với một giá trị cụ thể
    lv.mask = LVIF_TEXT;
    lv.iSubItem = 0;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.iItem = Index;

    ListView_InsertItem(proc_list, &lv);//chèn một mục mới vào  danh sách
}

void AddRow(HWND hView, LPWSTR text, int Index, int Col)//Thêm một dòng trống vào bảng. 
{
    ListView_SetItemText(hView, Index, Col, (LPWSTR)text);//Thay đổi văn bản của một mục hoặc trang con ở chế độ xem danh sách
}
