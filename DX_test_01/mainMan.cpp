#include <windows.h>
#include<tchar.h> 
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <iostream>

typedef wchar_t WCHAR;
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600


//全局变量
HINSTANCE hInst;
static std::string charList;

IDXGISwapChain* swapChain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backBuffer;//COM对象，保存了有关渲染目标的信息

//Shader对象
ID3D11VertexShader* pVS;
ID3D11PixelShader* pPS;
ID3D11Buffer* pVBuffer;//用于访问现存
ID3D11InputLayout* pLayout;


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL InitInstance(HINSTANCE instance, int nCmdShow);
ATOM RegisterClass(HINSTANCE instance);
void InitD3D(HWND hWnd);
void ClearD3D();
void RenderFrame();
void InitGraphic();
void InitPipeine();


#pragma region Data
struct VERTEX
{
    FLOAT X, Y, Z;
    D3DXCOLOR Color;
};

VERTEX VERTEXARRAY[] =
{
    {0.0f,0.5f,0.0f,D3DXCOLOR(1.0f,0.0f,0.0f,1.0f)},
    {0.45f,-0.5f,0.0f,D3DXCOLOR(0.0f,1.0f,0.0f,1.0f)},
    {-0.45f,-0.5f,0.0f,D3DXCOLOR(0.0f,0.0f,1.0f,1.0f)}
};
#pragma endregion


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance,
    _In_ PSTR szCmdLine, _In_ int  nCmdShow)
{

    
    //WNDCLASSEX wndclass;  //窗口类结构体

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
            TranslateMessage(&msg);//将收到的消息转换为"WM_+消息内容"的形式
            DispatchMessage(&msg);//将收到的消息发送给事件处理函数
            if (msg.message == WM_QUIT)
                break;
        }
        else
        {

        }
        RenderFrame();
    }

    ClearD3D();

    return (int)msg.wParam;
}

ATOM RegisterClass(HINSTANCE instance)
{
    WNDCLASSEXW wnd;
    wnd.cbSize = sizeof(WNDCLASSEX);

    wnd.style = CS_HREDRAW | CS_VREDRAW;  // 窗口发生移动或变化时使宽度变化, 重新绘制窗口 窗口发生移动或变化时使高度变化
    wnd.lpfnWndProc = WndProc;
    wnd.cbClsExtra = 0; //缓存空间
    wnd.cbWndExtra = 0; //缓存空间
    wnd.hInstance = instance;//模块句柄赋值
    wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wnd.lpszMenuName = L"Test";
    wnd.lpszClassName = L"HXY Engine";  //窗口类名
    wnd.hIconSm = LoadCursor(instance, IDC_ARROW);;

    return RegisterClassExW(&wnd);
}

BOOL InitInstance(HINSTANCE instance, int nCmdShow)
{
    hInst = instance;//// 将实例句柄存储在全局变量中

    RECT wr = { 0,0,1280,720 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowW(L"HXY Engine", L"Test", WS_OVERLAPPEDWINDOW,
        100, 100,wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, instance, nullptr);

    if (!hwnd)
    {
        return FALSE;
    }
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    InitD3D(hwnd);

    return TRUE;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)  //父窗口 收到的 "WM" 消息 wParam与lParam: MSG中附加消息, 与窗口的信息, 事件处理有关
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



void InitD3D(HWND hWnd)
{
    //D3D初始化
    DXGI_SWAP_CHAIN_DESC scd; //保存交换链的信息

    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = SCREEN_WIDTH;
    scd.BufferDesc.Height = SCREEN_HEIGHT;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//允许全切换

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &scd, &swapChain, &dev, NULL, &devcon);


    //设置渲染目标
    ID3D11Texture2D* pBackBuffer; 
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);//在交换链上找后缓冲区，并且用缓冲区创建纹理对象
                        //缓冲区编号。COM对象的类型ID，对象位置
    if (FAILED(hr))
    {
        OutputDebugString(L"error");
    }

    //D3D11_TEXTURE2D_DESC desc;
    //desc.Width = SCREEN_WIDTH;
    //desc.Height = SCREEN_HEIGHT;
    //desc.MipLevels = 1;
    //desc.ArraySize = 1;
    //desc.Format = DXGI_FORMAT_D32_FLOAT;
    //desc.SampleDesc.Count = 1;
    //desc.SampleDesc.Quality = 0;
    //desc.Usage = D3D11_USAGE_DEFAULT;
    //desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    //desc.CPUAccessFlags = 0;
    //desc.MiscFlags = 0;

    //D3D11_DEPTH_STENCIL_DESC depthDesc;


    //ID3D11Texture2D* pDepthStencilBuffer = 0;
    //HRESULT hr = dev->CreateTexture2D(&desc, 0, &pDepthStencilBuffer);

    //ID3D11DepthStencilView* pDepthview = 0;
    //HRESULT hr = dev->CreateDepthStencilView(pDepthStencilBuffer,c,&pDepthview)

    dev->CreateRenderTargetView(pBackBuffer, NULL, &backBuffer);
    
    pBackBuffer->Release();

    devcon->OMSetRenderTargets(1, &backBuffer, NULL);//设置渲染目标
                       //渲染目标数量，指向渲染目标视图对象列表的指针，暂无



    //设置viewport
    D3D11_VIEWPORT viewPort;
    ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));

    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = SCREEN_WIDTH;
    viewPort.Height = SCREEN_HEIGHT;

    devcon->RSSetViewports(1, &viewPort);//激活viewPort,
                //viewProt数量，viewport地址

    InitPipeine();
    InitGraphic();
}


void ClearD3D()
{
    swapChain->SetFullscreenState(FALSE, NULL);//FALSE 窗口 TRUE 全屏   , 选择的视频适配器

    swapChain->Release();
    dev->Release();
    devcon->Release();
    backBuffer->Release();
    pPS->Release();
    pVS->Release();
    pLayout->Release();
    pVBuffer->Release();

    devcon->ClearState();
    

}

void RenderFrame()
{
    devcon->ClearRenderTargetView(backBuffer, D3DXCOLOR(0.0f,0.2f,0.4f,1.0f));

    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

    devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    devcon->Draw(3, 0);//绘制的定点数  要绘制的第一个顶点



    swapChain->Present(0, 0);
}

void InitPipeine()
{
    //从文件中加载并编译两个shader
    ID3D10Blob* VS, * PS;
    D3DX11CompileFromFile(L"myShaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);//在Shaders文件中找到VShader并编译成4.0版本后存储在blod vs中
    D3DX11CompileFromFile(L"myShaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

    //把Shader封装到Shader对象中
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
            //已编译的数据地址   大小     shader对象地址

    //设置为活动shader
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);



    //创建输入布局          D3D11_INPUT_ELEMENT_DESC是描述输入元素的结构体
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
    };
    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(),&pLayout);
    //设置输入布局
    devcon->IASetInputLayout(pLayout);
}

void InitGraphic()
{

    D3D11_BUFFER_DESC bd;  //包含缓冲区属性的结构体
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;//CPU GPU访问权限
    bd.ByteWidth = sizeof(VERTEX) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;//用作顶点缓冲区
    bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;//允许CPU写入缓冲区

    dev->CreateBuffer(&bd, NULL, &pVBuffer);

    //填充顶点缓冲区
    D3D11_MAPPED_SUBRESOURCE ms;  // 映射缓冲区时，会填充这个结构体，保存了有关缓冲区的信息                  ms.pData指向缓冲区
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);//映射缓冲区
    memcpy(ms.pData, VERTEXARRAY, sizeof(VERTEXARRAY));//复制数据
    devcon->Unmap(pVBuffer, NULL);
}


//自定义创建indexBuffer
ID3D11Buffer* CreateIndexBuffer(UINT size, bool dynamic, D3D11_SUBRESOURCE_DATA* pData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //根据使用场景配置 usage 和 CPU access
    if (dynamic)//是否需要静态索引缓冲区
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    }
    else
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
    }


    //创建Buffer
    ID3D11Buffer* pBuffer;
    HRESULT hr = dev->CreateBuffer(&desc, NULL, &pBuffer);

    if (FAILED(hr))
    {
        return 0;
    }
    return pBuffer;
}

//自定义创建常量缓冲区
ID3D11Buffer* CreatConstantBuffer(UINT size, bool dynamic, bool CPUupdates, D3D11_SUBRESOURCE_DATA* pDate)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    if (dynamic && CPUupdates)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else if (dynamic && !CPUupdates)//在某些情况，如D3D11DevieContext::CopyStructureCount 需要把数据追加到常量缓冲区，需要使用默认 usage
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
    }
    else
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
    }

    ID3D11Buffer* pBuffer = 0;
    HRESULT hr = dev->CreateBuffer(&desc, NULL, &pBuffer);

    if (FAILED(hr))
    {
        return 0;
    }
    return pBuffer;
}

//创建自定义structured Buffer
ID3D11Buffer* CreateStructuredBuffer(UINT count,UINT structsize, bool CPUWritable, bool GPUWritable, D3D10_SUBRESOURCE_DATA* pData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = count * structsize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = structsize;

    if (!CPUWritable && !GPUWritable) //静态数据
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
    }
    else if (CPUWritable && !GPUWritable) //CPU访问
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else if (!CPUWritable && GPUWritable)  //GPU需要计算，如物理模拟，需要重新写入缓冲
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
    }
    else if (CPUWritable && GPUWritable)//不可以同时访问
    {
        //Handle the error
        // Resources can't be writable by both CPU and GPU simultaneously!
    }

    ID3D11Buffer* pBuffer = 0;
    HRESULT hr = dev->CreateBuffer(&desc, NULL, &pBuffer);

    if (FAILED(hr))
    {
        return 0;
    }
}

//创建ShaderResourView 需要定义SRV描述结构体
ID3D11ShaderResourceView* CreateBufferSRV(ID3D11Resource* pResource)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;

    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
    
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.ElementOffset = 0;
    desc.Buffer.ElementWidth = 100;

    ID3D11ShaderResourceView* pView = 0;
    HRESULT hr = dev->CreateShaderResourceView(pResource, &desc, &pView);
    return pView;

}

//创建UAV 需要定义SRV描述结构体
ID3D11UnorderedAccessView* CreateBufferUAV(ID3D11Resource* pResource)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;

    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = 100;

    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;

    ID3D11UnorderedAccessView* pView = 0;
    HRESULT hr = dev->CreateUnorderedAccessView(pResource, &desc, &pView);

    return pView;
}

ID3D11Buffer* CreateAppendConsumeBuffer(UINT size, UINT structsize, D3D11_SUBRESOURCE_DATA* pData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size * structsize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = structsize;

    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;


    ID3D11Buffer* pBuffer = 0;
    HRESULT hr = dev->CreateBuffer(&desc, NULL, &pBuffer);


    if (FAILED(hr))
    {
        return 0;
    }
    return pBuffer;
}