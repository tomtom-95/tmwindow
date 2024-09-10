#include <windows.h>
#include <stdint.h>

// struct {
//     int width;
//     int height;
//     uint32_t *pixels;
// } frame = {0};

#if RAND_MAX == 32767
#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))
#else
#define Rand32() rand()
#endif

static BITMAPINFO bitmap_info;
static void *bitmap_memory;
static int bitmap_width;
static int bitmap_height;

static void
Win32ResizeDIBSection(int width, int height)
{
    if (bitmap_memory)
    {
        VirtualFree(bitmap_memory,
                    NULL,
                    MEM_RELEASE);
    }

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = bitmap_width;
    bitmap_info.bmiHeader.biHeight = bitmap_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bytes_per_pixel = 4;
    int bitmap_memory_size = (width * height) * bytes_per_pixel;

    bitmap_memory = VirtualAlloc(0,
                                 bitmap_memory_size,
                                 MEM_COMMIT,
                                 PAGE_READWRITE);
}

static void
Win32UpdateWindow(HDC device_context,
                  RECT *window_rect,
                  int x,
                  int y,
                  int width,
                  int height)
{
    int window_width = window_rect->right - window_rect->left;
    int window_height = window_rect->bottom - window_rect->top;
    StretchDIBits(device_context,
                  0, 0, bitmap_width, bitmap_height,
                  0, 0, bitmap_width, bitmap_height,
                  bitmap_memory,
                  &bitmap_info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

// Step 4: the Window Procedure
LRESULT CALLBACK 
WndProc(HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
{
    LRESULT result = 0;
    switch(msg)
    {
        case WM_LBUTTONDOWN:
        {
            char szFileName[MAX_PATH];
            HINSTANCE hInstance = GetModuleHandle(NULL);

            GetModuleFileName(hInstance,
                              szFileName,
                              MAX_PATH);
            MessageBox(hwnd,
                       szFileName,
                       "Cazzo clicchi coglione:",
                       MB_OK | MB_ICONINFORMATION);
        } break;
        case WM_CLOSE:
        {
            OutputDebugString("WM_CLOSE\n");
            PostQuitMessage(0);
        } break;
        case WM_DESTROY:
        {
            OutputDebugString("WM_DESTROY\n");
            PostQuitMessage(0);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hwnd, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            Win32UpdateWindow(hwnd, x, y, width, height);
            PatBlt(device_context,
                   paint.rcPaint.left,
                   paint.rcPaint.top,
                   paint.rcPaint.right  - paint.rcPaint.left,
                   paint.rcPaint.bottom - paint.rcPaint.top,
                   SRCCOPY);
            SetPixel(device_context,
                     100,
                     100,
                     RGB(255, 0, 255));
            EndPaint(hwnd, &paint);
        } break;
        case WM_SIZE:
        {
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            int width = client_rect.right - client_rect.left;
            int height = client_rect.bottom - client_rect.top;
            Win32ResizeDIBSection(width, height);

            OutputDebugString("WM_SIZE\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            result = DefWindowProc(hwnd,
                                   msg,
                                   wParam,
                                   lParam);
        } break;
        default:
        {
            result = DefWindowProc(hwnd,
                                   msg,
                                   wParam,
                                   lParam);
        }
    }
    return result;
}

int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    // TODO(tommaso): check if this flags still matter
    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_EXCLAMATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    // wc.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "MyWindow";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL,
                   "Window Registration Failed!",
                   "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(0,
                          "MyWindow",
                          "tmeditor",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL,
                   "Window Creation Failed!",
                   "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg); // for keyboard messages NOTE(tommaso): will be important later
        DispatchMessage(&Msg);

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
    }
    return Msg.wParam;
}