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


//ȫ�ֱ���
HINSTANCE hInst;
static std::string charList;

IDXGISwapChain* swapChain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backBuffer;//COM���󣬱������й���ȾĿ�����Ϣ

//Shader����
ID3D11VertexShader* pVS;
ID3D11PixelShader* pPS;
ID3D11Buffer* pVBuffer;//���ڷ����ִ�
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
        RenderFrame();
    }

    ClearD3D();

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



void InitD3D(HWND hWnd)
{
    //D3D��ʼ��
    DXGI_SWAP_CHAIN_DESC scd; //���潻��������Ϣ

    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = SCREEN_WIDTH;
    scd.BufferDesc.Height = SCREEN_HEIGHT;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//����ȫ�л�

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &scd, &swapChain, &dev, NULL, &devcon);


    //������ȾĿ��
    ID3D11Texture2D* pBackBuffer; 
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);//�ڽ��������Һ󻺳����������û����������������
                        //��������š�COM���������ID������λ��
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

    devcon->OMSetRenderTargets(1, &backBuffer, NULL);//������ȾĿ��
                       //��ȾĿ��������ָ����ȾĿ����ͼ�����б��ָ�룬����



    //����viewport
    D3D11_VIEWPORT viewPort;
    ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));

    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = SCREEN_WIDTH;
    viewPort.Height = SCREEN_HEIGHT;

    devcon->RSSetViewports(1, &viewPort);//����viewPort,
                //viewProt������viewport��ַ

    InitPipeine();
    InitGraphic();
}


void ClearD3D()
{
    swapChain->SetFullscreenState(FALSE, NULL);//FALSE ���� TRUE ȫ��   , ѡ�����Ƶ������

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


    devcon->Draw(3, 0);//���ƵĶ�����  Ҫ���Ƶĵ�һ������



    swapChain->Present(0, 0);
}

void InitPipeine()
{
    //���ļ��м��ز���������shader
    ID3D10Blob* VS, * PS;
    D3DX11CompileFromFile(L"myShaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);//��Shaders�ļ����ҵ�VShader�������4.0�汾��洢��blod vs��
    D3DX11CompileFromFile(L"myShaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

    //��Shader��װ��Shader������
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
            //�ѱ�������ݵ�ַ   ��С     shader�����ַ

    //����Ϊ�shader
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);



    //�������벼��          D3D11_INPUT_ELEMENT_DESC����������Ԫ�صĽṹ��
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
    };
    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(),&pLayout);
    //�������벼��
    devcon->IASetInputLayout(pLayout);
}

void InitGraphic()
{

    D3D11_BUFFER_DESC bd;  //�������������ԵĽṹ��
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;//CPU GPU����Ȩ��
    bd.ByteWidth = sizeof(VERTEX) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;//�������㻺����
    bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;//����CPUд�뻺����

    dev->CreateBuffer(&bd, NULL, &pVBuffer);

    //��䶥�㻺����
    D3D11_MAPPED_SUBRESOURCE ms;  // ӳ�仺����ʱ�����������ṹ�壬�������йػ���������Ϣ                  ms.pDataָ�򻺳���
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);//ӳ�仺����
    memcpy(ms.pData, VERTEXARRAY, sizeof(VERTEXARRAY));//��������
    devcon->Unmap(pVBuffer, NULL);
}


//�Զ��崴��indexBuffer
ID3D11Buffer* CreateIndexBuffer(UINT size, bool dynamic, D3D11_SUBRESOURCE_DATA* pData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //����ʹ�ó������� usage �� CPU access
    if (dynamic)//�Ƿ���Ҫ��̬����������
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    }
    else
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
    }


    //����Buffer
    ID3D11Buffer* pBuffer;
    HRESULT hr = dev->CreateBuffer(&desc, NULL, &pBuffer);

    if (FAILED(hr))
    {
        return 0;
    }
    return pBuffer;
}

//�Զ��崴������������
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
    else if (dynamic && !CPUupdates)//��ĳЩ�������D3D11DevieContext::CopyStructureCount ��Ҫ������׷�ӵ���������������Ҫʹ��Ĭ�� usage
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

//�����Զ���structured Buffer
ID3D11Buffer* CreateStructuredBuffer(UINT count,UINT structsize, bool CPUWritable, bool GPUWritable, D3D10_SUBRESOURCE_DATA* pData)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = count * structsize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = structsize;

    if (!CPUWritable && !GPUWritable) //��̬����
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
    }
    else if (CPUWritable && !GPUWritable) //CPU����
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else if (!CPUWritable && GPUWritable)  //GPU��Ҫ���㣬������ģ�⣬��Ҫ����д�뻺��
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
    }
    else if (CPUWritable && GPUWritable)//������ͬʱ����
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

//����ShaderResourView ��Ҫ����SRV�����ṹ��
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

//����UAV ��Ҫ����SRV�����ṹ��
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