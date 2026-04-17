#include <windows.h>

const int radius = 2;
HWND g_hwnd = NULL;
bool g_running = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        int centerX = (rect.left + rect.right) / 2;
        int centerY = (rect.top + rect.bottom) / 2;
        
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);
        
        Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
        
        DeleteObject(hPen);
        DeleteObject(hBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    
    case WM_ERASEBKGND:
        return 1;
        
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
        
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SetClickThrough(HWND hwnd)
{
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 创建互斥量，确保只有一个实例运行
    HANDLE hMutex = CreateMutex(NULL, TRUE, "RedDotOverlay_Mutex");
    
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // 已有实例在运行，激活已有窗口并退出
        HWND hExistingWnd = FindWindow(NULL, "RedDot");
        if (hExistingWnd)
        {
            ShowWindow(hExistingWnd, SW_SHOW);
            SetForegroundWindow(hExistingWnd);
        }
        CloseHandle(hMutex);
        return 0;
    }
    
    const char CLASS_NAME[] = "RedDotOverlay";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    g_hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        CLASS_NAME, "RedDot", WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hwnd)
    {
        CloseHandle(hMutex);
        return 0;
    }
    
    SetClickThrough(g_hwnd);
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    
    MSG msg;
    while (g_running && GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // 程序退出时释放互斥量
    CloseHandle(hMutex);
    return 0;
}