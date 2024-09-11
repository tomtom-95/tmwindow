#include <windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool running;
global_variable BITMAPINFO bitmap_info;
global_variable void *bitmap_memory;
global_variable HBITMAP bitmap_handle;
global_variable HDC bitmap_device_context;

internal void
Win32ResizeDIBSection(int width, int height)
{
    // TODO(tommaso): bulletproof this, maybe don't free first, free after, then free first if it fails
    // TODO(tommaso): free out DIBSection
    if (bitmap_handle)
    {
        DeleteObject(bitmap_handle); 
    }
    if (!bitmap_device_context)
    {
        bitmap_device_context = CreateCompatibleDC(0);
    }

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = width;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    bitmap_handle = CreateDIBSection(
        bitmap_device_context,
        &bitmap_info,
        DIB_RGB_COLORS,
        &bitmap_memory,
        0, 0
    );
}

internal void
Win32UpdateWindow(HDC device_context,
                  int x,
                  int y,
                  int width,
                  int height)
{
    StretchDIBits(
        device_context,
        x, y, width, height,
        x, y, width, height,
        bitmap_memory,
        &bitmap_info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

LRESULT CALLBACK
MainWindowProc(HWND window,
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
                window,
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
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int height = paint.rcPaint.bottom - paint.rcPaint.top; 
            int width = paint.rcPaint.right - paint.rcPaint.left;
            Win32UpdateWindow(
                device_context,
                x, y,
                width, height
            );
            local_persist DWORD operation = WHITENESS;
            if (operation == WHITENESS)
            {
                operation = BLACKNESS;
            }
            else
            {
                operation = WHITENESS;
            }
            PatBlt(
                device_context,
                x, y, width, height,
                operation
            );
            EndPaint(window, &paint);
        } break;
        default:
        {
            OutputDebugStringA("default\n");
            result = DefWindowProcA(
                window,
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
    WNDCLASS window_class = {0};

    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowProc;
    window_class.hInstance = instance;
    // window_class.hIcon = ;
    window_class.lpszClassName = "tmWindow";

    if (RegisterClass(&window_class))
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