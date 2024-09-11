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

struct win32_offscreen_buffer {
  // Pixels are alwasy 32-bits wide, memory Order BB GG RR XX, 0xXXRRGGBB
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

struct win32_window_dimension {
  int width;
  int height;
};


static BITMAPINFO bitmap_info;
static void *bitmap_memory;
static int bitmap_width;
static int bitmap_height;
static struct win32_offscreen_buffer globalBackBuffer;

static struct win32_window_dimension
Win32GetWindowDimension(HWND window)
{
  struct win32_window_dimension result;
  RECT clientRect;
  GetClientRect(window, &clientRect);
  result.width = clientRect.right - clientRect.left;
  result.height = clientRect.bottom - clientRect.top;
  return result;
}

static void
RenderWeirdGradient(int x_offset, int y_offset)
{
    /*
        MEMORY ORDER: RR GG BB xx
        LOADED IN:    xx BB GG RR
        WANTED:       AA RR GG BB
        MEMORY ORDER: BB GG RR AA
    */

    int width = bitmap_width; 
    int height = bitmap_height;

    unsigned int *pixel = (unsigned int *)bitmap_memory;
    for (int i = 0; i < 100; i++)
    {
        *pixel = 0x00ff0000;
        pixel++;
    }
}

static void
Win32ResizeDIBSection(int width, int height)
{
    if (bitmap_memory)
    {
        VirtualFree(bitmap_memory,
                    0,
                    MEM_RELEASE);
    }

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = bitmap_width;
    bitmap_info.bmiHeader.biHeight = -bitmap_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bytes_per_pixel = 4;
    int bitmap_memory_size = (width * height) * bytes_per_pixel;

    bitmap_memory = VirtualAlloc(
        0,
        bitmap_memory_size,
        MEM_COMMIT,
        PAGE_READWRITE
    );
}

static void
Win32UpdateWindow(
    HDC device_context,
    int window_width,
    int window_height,
    struct win32_offscreen_buffer *buffer)
{
    StretchDIBits(
        device_context,
        0, 0, 2 * buffer->width, 2 * buffer->height,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

// Step 4: the Window Procedure
LRESULT CALLBACK 
WndProc(HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    LRESULT result = 0;
    switch(message)
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
            struct win32_window_dimension dimension = Win32GetWindowDimension(hwnd);
            Win32UpdateWindow(
                device_context,
                dimension.width,
                dimension.height,
                &globalBackBuffer
            );
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
                                   message,
                                   wParam,
                                   lParam);
        } break;
        default:
        {
            result = DefWindowProc(hwnd,
                                   message,
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
    HWND hwnd;

    // Step 1: Registering the Window Class
    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "MyWindow";

    if(!RegisterClass(&wc))
    {
        MessageBox(NULL,
                   "Window Registration Failed!",
                   "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        "tmWindow",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if(hwnd == NULL)
    {
        MessageBox(NULL,
                   "Window Creation Failed!",
                   "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 3: The Message Loop
    int x_offset = 0;
    int y_offset = 0;
    MSG message;

    HDC device_context = GetDC(hwnd);

    while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        OutputDebugString("in the loop");
        if (message.message == WM_QUIT)
        {
            break;
        }

        TranslateMessage(&message); // for keyboard messages NOTE(tommaso): will be important later
        DispatchMessageA(&message);

        RenderWeirdGradient(x_offset, y_offset);
        HDC device_context = GetDC(hwnd);
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);
        int window_width = client_rect.right - client_rect.left;
        int window_height = client_rect.bottom - client_rect.top;
        Win32UpdateWindow(device_context,
                          &client_rect,
                          0,
                          0, 
                          window_width,
                          window_height);
        ReleaseDC(hwnd, device_context);
    }
    OutputDebugString("out the loop");
    return message.wParam;
}