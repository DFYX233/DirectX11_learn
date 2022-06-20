#include <windows.h>
#include<tchar.h> 
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <iostream>

typedef wchar_t WCHAR;


//ȫ�ֱ���
HINSTANCE hInst;
static std::string charList;



LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL InitInstance(HINSTANCE instance, int nCmdShow);
ATOM RegisterClass(HINSTANCE instance);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance,
    _In_ PSTR szCmdLine, _In_ int  nCmdShow)
{

    
    //WNDCLASSEX wndclass;  //������ṹ��

    RegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }


    MSG msg;
    while (TRUE)
    {
        //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        //{
        //    TranslateMessage(&msg);
        //    DispatchMessage(&msg);
        //}
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);//���յ�����Ϣת��Ϊ"WM_+��Ϣ����"����ʽ
            DispatchMessage(&msg);//���յ�����Ϣ���͸��¼�������
            if (msg.message == WM_QUIT)
                break;
        }
        else
        {

        }

    }

    return (int)msg.wParam;
}

ATOM RegisterClass(HINSTANCE instance)
{
    WNDCLASSEXW wnd;
    wnd.cbSize = sizeof(WNDCLASSEX);

    wnd.style = CS_HREDRAW | CS_VREDRAW;  // ���ڷ����ƶ���仯ʱʹ��ȱ仯, ���»��ƴ��� ���ڷ����ƶ���仯ʱʹ�߶ȱ仯
    wnd.lpfnWndProc = WndProc;
    wnd.cbClsExtra = 0; //����ռ�
    wnd.cbWndExtra = 0; //����ռ�
    wnd.hInstance = instance;//ģ������ֵ
    wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wnd.lpszMenuName = L"Test";
    wnd.lpszClassName = L"HXY Engine";  //��������
    wnd.hIconSm = LoadCursor(instance, IDC_ARROW);;

    return RegisterClassExW(&wnd);
}

BOOL InitInstance(HINSTANCE instance, int nCmdShow)
{
    hInst = instance;//// ��ʵ������洢��ȫ�ֱ�����

    RECT wr = { 0,0,1280,720 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowW(L"HXY Engine", L"Test", WS_OVERLAPPEDWINDOW,
        100, 100, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, instance, nullptr);

    if (!hwnd)
    {
        return FALSE;
    }
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return TRUE;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)  //������ �յ��� "WM" ��Ϣ wParam��lParam: MSG�и�����Ϣ, �봰�ڵ���Ϣ, �¼������й�
{
    switch (msg)
    {
    case WM_QUIT:
    {DestroyWindow(hWnd); }break;
    case WM_DESTROY:
    {PostQuitMessage(0); }break;
    case WM_CHAR:
    {

        charList.push_back((char)wParam);
        wchar_t* wc = new wchar_t[charList.size()];
        swprintf(wc, 200, L"%S", charList.c_str());
        SetWindowText(hWnd, wc);
    }
        break;
    case WM_KEYUP:
    {
        charList.clear();
        SetWindowText(hWnd, L"NO interactive");
    }
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
//{
//
//}