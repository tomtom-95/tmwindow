#include <windows.h>
#include <stdint.h>

#define local_persist static
#define global_variable static
#define internal static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

global_variable bool running;
global_variable BITMAPINFO bitmap_info;
global_variable void *bitmap_memory;
global_variable int bitmap_width;
global_variable int bitmap_height;

internal void
Win32ResizeDIBSection(int width, int height)
{
    if (bitmap_memory)
    {
        VirtualFree(bitmap_memory, 0, MEM_RELEASE);
    }

    bitmap_width = width;
    bitmap_height = height;

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = bitmap_width;
    bitmap_info.bmiHeader.biHeight = -bitmap_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bytes_per_pixel = 4;
    int bitmap_memory_size = bytes_per_pixel * (bitmap_width * bitmap_height);
    bitmap_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

    int pitch = width * bytes_per_pixel;
    uint8 *row = (uint8 *)bitmap_memory;
    for (int y = 0; y < bitmap_height; y++)
    {
        uint8 *pixel = (uint8 *)row;
        for (int x = 0; x < bitmap_width; x++)
        {
            /*
                Pixel in memory: RR GG BB xx
            */
            *pixel = 255;
            pixel++;

            *pixel = 0;
            pixel++;

            *pixel = 0;
            pixel++;

            *pixel = 0;
            pixel++;
        }

        row += pitch;
    }
}

internal void
Win32UpdateWindow(HDC device_context,
                  RECT *window_rect,
                  int x, int y,
                  int width, int height)
{
    int window_width = window_rect->right - window_rect->left;
    int window_height = window_rect->bottom - window_rect->top;
    StretchDIBits(
        device_context,
        /* 
        x, y, width, height,
        x, y, width, height,
        */
        0, 0, bitmap_width, bitmap_height,
        0, 0, window_width, window_height,
        bitmap_memory,
        &bitmap_info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

LRESULT CALLBACK
MainWindowProc(HWND hwnd,
               UINT message,
               WPARAM w_param,
               LPARAM l_param)
{
    LRESULT result = 0;

    switch(message)
    {
        case WM_SIZE:
        {
            RECT client_rect;
            GetClientRect(
                hwnd,
                &client_rect
            );
            int width = client_rect.right - client_rect.left;
            int height = client_rect.bottom - client_rect.top;
            Win32ResizeDIBSection(width, height);
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_DESTROY:
        {
            // TODO(tommaso): handle this with a message to the user
            running = false;
        } break;
        case WM_CLOSE:
        {
            // TODO(tommaso): handle this with a message to the user
            running = false;
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_PAINT:
        {
            OutputDebugStringA("WM_PAINT\n");

            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hwnd, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top; 

            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            Win32UpdateWindow(
                device_context,
                &client_rect,
                x, y, width, height
            );
            EndPaint(hwnd, &paint);
        } break;
        default:
        {
            OutputDebugStringA("default\n");
            result = DefWindowProcA(
                hwnd,
                message,
                w_param,
                l_param
            );
        } break;
    }

    return result;
}

int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code)
{
    WNDCLASSA window_class = {0};

    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowProc;
    window_class.hInstance = instance;
    // window_class.hIcon = ;
    window_class.lpszClassName = "tmWindow";

    if (RegisterClassA(&window_class))
    {
        HWND window_handle = CreateWindowExA(
            0,
            window_class.lpszClassName, 
            "New Window",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instance, 
            0
        );

        if (window_handle != NULL)
        {
            running = true;
            while (running)
            {
                MSG message;
                BOOL message_result = GetMessageA(&message, 0, 0, 0);
                if (message_result > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            // TODO(tommaso): logging
        }
    }
    else
    {
        // TODO(tommaso): logging
    }

    return 0;
}