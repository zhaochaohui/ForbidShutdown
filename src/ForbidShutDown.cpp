﻿#include "header.h"
#include "Resource.h"
#include <shellapi.h>
#include "CKeepAwake.h"

#define MAX_LOADSTRING 100
#define UM_TRAY_NOTIFY (WM_USER + WM_USER)
#define UM_END 0xBFFE

HINSTANCE hInst = NULL;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING] = { L"ForbidShutDown" }; // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING] = { L"ForbidShutDown" }; // 主窗口类名
CKeepAwake* g_pKeepAwake = nullptr;

BOOL             InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM             MyRegisterClass(HINSTANCE hInstance);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 保证单例
    auto h = ::CreateEvent(NULL, FALSE, TRUE, _T("52E6F1E6-26CB-4182-BC0F-60EA520B0EA3"));
    auto err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS)
    {
        return 0;
    }

    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, SW_HIDE))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FORBIDSHUTDOWN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_FORBIDSHUTDOWN);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_FORBIDSHUTDOWN));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT lpClickPoint;

    switch (message)
    {
    case WM_CREATE:
    {
        NOTIFYICONDATA IconData = { 0 };
        IconData.cbSize = sizeof(NOTIFYICONDATA);
        IconData.hWnd = (HWND)hWnd;
        IconData.uID = 0;
        IconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        IconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FORBIDSHUTDOWN));
        IconData.uCallbackMessage = UM_TRAY_NOTIFY;
        lstrcpy(IconData.szTip, L"避免休眠或阻止关机");
        Shell_NotifyIcon(NIM_ADD, &IconData);

        g_pKeepAwake = CKeepAwake::GetInstance();
        g_pKeepAwake->Init(hWnd);
        break;
    }

    case UM_TRAY_NOTIFY:
    {
        if (lParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&lpClickPoint);
            HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TRAY_MENU));
            if (hMenu)
            {
                HMENU hSubMenu = GetSubMenu(hMenu, 0);
                if (hSubMenu) {
                    SetForegroundWindow(hWnd); // 避免菜单弹出后, 如果不点击则不消失的问题。
                    TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
                }
                DestroyMenu(hMenu);
            }
        }

        if (lParam == WM_LBUTTONDOWN)
        {
            NOTIFYICONDATA IconData = { 0 };
            IconData.cbSize = sizeof(NOTIFYICONDATA);
            IconData.hWnd = (HWND)hWnd;
            IconData.uFlags = NIF_INFO;
            lstrcpy(IconData.szInfo, _T("111"));
            lstrcpy(IconData.szInfoTitle,  _T("222"));
            Shell_NotifyIcon(NIM_MODIFY, &IconData);
            return 0;
        }
        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDM_EXIT)
        {
            PostMessage(hWnd, WM_DESTROY, 0, 0);
        }
		else if (LOWORD(wParam) == IDM_LINK)
		{
			system("rundll32 url.dll,FileProtocolHandler https://github.com/zhaochaohui");
		}
        break;
    }

    case WM_DESTROY:
    {
        CKeepAwake::DelInstance();

        NOTIFYICONDATA IconData = { 0 };
        IconData.cbSize = sizeof(NOTIFYICONDATA);
        IconData.hWnd = (HWND)hWnd;
        IconData.uID = 0;
        lstrcpy(IconData.szTip, L"避免休眠或阻止关机");

        Shell_NotifyIcon(NIM_DELETE, &IconData);

        PostQuitMessage(0);
        break;
    }

    default:
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}