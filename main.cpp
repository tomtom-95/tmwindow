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

typedef enum
{
    RED   = 0x00ff0000,
    GREEN = 0x0000ff00,
    BLUE  = 0x000000ff,
    WHITE = 0x00ffffff
} Color;

global_variable bool running;
global_variable win32_offscreen_buffer global_back_buffer;

void
IntSwap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

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
RenderWeirdGradient(win32_offscreen_buffer buffer, int blue_offset, int green_offset)
{
    // let's see what optimizer does
    uint8 *row = (uint8 *)buffer.memory;
    for (int y = 0; y < buffer.height; y++)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer.width; x++)
        {
            uint8 Blue = (x + blue_offset);
            uint8 Green = (y + green_offset);

            *pixel++ = ((Green << 8) | Blue);
        }

        row += 4 * buffer.pitch;
    }
}

// Simple but slow implementation of drawing a line
internal void
DrawLineSlowly(win32_offscreen_buffer buffer,
               int x0, int x1,
               int y0, int y1,
               Color color)
{
    uint32_t *pixel = (uint32_t *)buffer.memory;
    for (float t = 0.; t < 1.; t += .001)
    {
        int x = x0 + (x1 - x0) * t;
        int y = y0 + (y1 - y0) * t;
        *(pixel + y * buffer.pitch + x) = color;
    }
}

// Draw line which is garanteed to have slope greater than 1
// or less than -1
internal void
DrawLineHigh(win32_offscreen_buffer buffer,
             int x0, int x1,
             int y0, int y1,
             Color color)
{
    // f(x, y) = A * x + B * y + C
    int A = y1 - y0;
    int B = x0 - x1;
    int C = (y0 - y1) * x0 + (x1 - x0) * y0;

    uint32_t *pixel;
    int x = x0;
    if (x1 > x0)
    {
        for (int y = y0; y < y1; y++)
        {
            int f = A * x + B * y + C; 
            if (f < 0)
            {
                x++;
            }
            pixel = (uint32_t *)buffer.memory + buffer.pitch * y + x;
            *pixel = WHITE;
        }
    }
    else
    {
        for (int y = y0; y < y1; y++)
        {
            int f = A * x + B * y + C; 
            if (f > 0)
            {
                x--;
            }
            pixel = (uint32_t *)buffer.memory + buffer.pitch * y + x;
            *pixel = WHITE;
        }
    }
}

// Draw line which is garanteed to have slope between -1 and 1
internal void
DrawLineLow(win32_offscreen_buffer buffer,
            int x0, int x1,
            int y0, int y1,
            Color color)
{
    // f(x, y) = A * x + B * y + C
    int A = y1 - y0;
    int B = x0 - x1;
    int C = (y0 - y1) * x0 + (x1 - x0) * y0;

    uint32_t *pixel;
    int y = y0;
    if (y1 > y0)
    {
        for (int x = x0; x < x1; x++)
        {
            int f = A * x + B * y + C; 
            if (f > 0)
            {
                y++;
            }
            pixel = (uint32_t *)buffer.memory + buffer.pitch * y + x;
            *pixel = WHITE;
        }
    }
    else
    {
        for (int x = x0; x < x1; x++)
        {
            int f = A * x + B * y + C; 
            if (f < 0)
            {
                y--;
            }
            pixel = (uint32_t *)buffer.memory + buffer.pitch * y + x;
            *pixel = WHITE;
        }
    }
}

internal void
DrawLineBresenham(win32_offscreen_buffer buffer,
                  int x0, int x1,
                  int y0, int y1,
                  Color color)
{
    if (abs(x1 - x0) > abs(y1 - y0))
    {
        if (x1 > x0)
        {
            DrawLineLow(buffer, x0, x1, y0, y1, color);
        }
        else
        {
            DrawLineLow(buffer, x1, x0, y1, y0, color);
        }
    }
    else
    {
        if (y1 > y0)
        {
            DrawLineHigh(buffer, x0, x1, y0, y1, color);
        }
        else
        {
            DrawLineHigh(buffer, x1, x0, y1, y0, color);
        }
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

internal void
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