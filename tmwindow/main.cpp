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
    /*
        Allocation for vertices
    */
    GrowableBuffer vertex_buffer = {};
    vertex_buffer.allocation_count  = 0;
    vertex_buffer.allocation_size   = sizeof(Vertex) * 10000;
    vertex_buffer.allocation_offset = 0;

    vertex_buffer.data = (char *)VirtualAlloc(0, GigaByte,
                                              MEM_RESERVE, PAGE_READWRITE);

    VirtualAlloc(vertex_buffer.data,
                 vertex_buffer.allocation_size,
                 MEM_COMMIT, PAGE_READWRITE);
    vertex_buffer.allocation_count++;

    /*
        Allocation for faces
    */
    GrowableBuffer face_buffer = {};
    face_buffer.allocation_count  = 0;
    face_buffer.allocation_size   = sizeof(Face) * 10000;
    face_buffer.allocation_offset = 0;
    face_buffer.data = (char *)VirtualAlloc(0, GigaByte,
                                            MEM_RESERVE, PAGE_READWRITE);

    VirtualAlloc(face_buffer.data,
                 face_buffer.allocation_size,
                 MEM_COMMIT, PAGE_READWRITE);
    face_buffer.allocation_count++;

    /*
        Parsing of wavefront object file
    */
    WavefrontParser("./models/african_head.obj",
                    &vertex_buffer,
                    &face_buffer);

    WNDCLASSA window_class = {0};

    // repaint the whole window when I resize, not just the new section
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowProc;
    window_class.hInstance = instance;
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

                // int face_offset = 0;
                // while (true)
                // {
                //     if (face_offset == face_buffer.allocation_offset)
                //     {
                //         break;
                //     }
                //     else
                //     {
                //         Face *face = (Face *)(face_buffer.data + face_offset);
                //         Vertex *vertex0 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[0] - 1));
                //         Vertex *vertex1 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[1] - 1));
                //         Vertex *vertex2 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[2] - 1));

                //         DrawTriangle(global_back_buffer, *vertex0, *vertex1, *vertex2);

                //         face_offset += sizeof(Face);
                //     }
                // }
                ColorWireFrameObj(global_back_buffer, face_buffer, vertex_buffer);

                HDC device_context = GetDC(window);
                win32_window_dimension window_dimension = GetWindowDimension(window);
                Win32DisplayBufferInWindow(
                    device_context,
                    window_dimension.width,
                    window_dimension.height,
                    global_back_buffer
                );
                ReleaseDC(window, device_context);
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