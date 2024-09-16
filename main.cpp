#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#include <windows.h>

#include "utils.hpp"
#include "drawing.cpp"
#include "wavefront_parser.cpp"
#include "logger.c"

static bool running;
static win32_offscreen_buffer global_back_buffer;

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

static void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer,
                      int width,
                      int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;
    buffer->pitch = buffer->width;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
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

static void
Win32DisplayBufferInWindow(HDC device_context,
                           int window_width,
                           int window_height,
                           win32_offscreen_buffer buffer)
{
    StretchDIBits(
        device_context,
        0, 0, window_width, window_height,
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
    WavefrontParser("./models/little_african_head.obj");
    return 0;

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

            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hwnd, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top; 

            win32_window_dimension window_dimension = GetWindowDimension(hwnd);
            Win32DisplayBufferInWindow(
                device_context,
                window_dimension.width,
                window_dimension.height,
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

                // RenderWeirdGradient(global_back_buffer, x_offset, 0);
                int line_x0 = 10 % global_back_buffer.width;
                int line_x1 = 100 % global_back_buffer.width;
                int line_y0 = 500 % global_back_buffer.height;
                int line_y1 = 10 % global_back_buffer.height;
                DrawLineBresenham(
                    global_back_buffer,
                    line_x0,
                    line_x1,
                    line_y0,
                    line_y1,
                    WHITE
                );
                HDC device_context = GetDC(window);
                win32_window_dimension window_dimension = GetWindowDimension(window);
                Win32DisplayBufferInWindow(
                    device_context,
                    window_dimension.width,
                    window_dimension.height,
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