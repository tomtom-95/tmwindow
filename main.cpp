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

typedef struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
} win32_offscreen_buffer;

typedef struct win32_window_dimension
{
    int width;
    int height;
} win32_window_dimension;

global_variable bool running;
global_variable win32_offscreen_buffer global_back_buffer;

win32_window_dimension
GetWindowDimension(HWND window)
{
    win32_window_dimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

internal void
RenderWeirdGradient(win32_offscreen_buffer buffer, int x_offset, int y_offset)
{
    // let's see what optimizer does
    uint8 *row = (uint8 *)buffer.memory;
    for (int y = 0; y < buffer.height; y++)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer.width; x++)
        {
            uint8 Blue = (x + x_offset);
            uint8 Green = (y + y_offset);

            *pixel++ = ((Green << 8) | Blue);
        }

        row += buffer.pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;
    buffer->pitch = buffer->bytes_per_pixel * buffer->width;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_area = buffer->width * buffer->height;
    int bitmap_memory_size = buffer->bytes_per_pixel * bitmap_area;
    buffer->memory = VirtualAlloc(
        0,
        bitmap_memory_size,
        MEM_COMMIT, PAGE_READWRITE
    );
}

internal void
Win32DisplayBufferInWindow(HDC device_context,
                           win32_window_dimension window_dimension,
                           win32_offscreen_buffer buffer)
{
    StretchDIBits(
        device_context,
        0, 0, window_dimension.width, window_dimension.height,
        0, 0, buffer.width, buffer.height,
        buffer.memory,
        &buffer.info,
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
            win32_window_dimension window_dimension = GetWindowDimension(hwnd);
            Win32ResizeDIBSection(
                &global_back_buffer,
                window_dimension.width,
                window_dimension.height
            );
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

            RenderWeirdGradient(global_back_buffer, 0, 0);
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hwnd, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top; 

            win32_window_dimension window_dimension = GetWindowDimension(hwnd);
            Win32DisplayBufferInWindow(
                device_context,
                window_dimension,
                global_back_buffer
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

    // repaint the whole window when I resize, not just the new section
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowProc;
    window_class.hInstance = instance;
    // window_class.hIcon = ;
    window_class.lpszClassName = "tmWindow";

    if (RegisterClassA(&window_class))
    {
        HWND window = CreateWindowExA(
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

        if (window != NULL)
        {
            int x_offset = 0;
            int y_offset = 0;

            running = true;
            while (running)
            {
                MSG message;
                while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                RenderWeirdGradient(global_back_buffer, x_offset, y_offset);
                HDC device_context = GetDC(window);
                win32_window_dimension window_dimension = GetWindowDimension(window);
                Win32DisplayBufferInWindow(
                    device_context,
                    window_dimension,
                    global_back_buffer
                );
                ReleaseDC(window, device_context);

                x_offset++;
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