#include <windows.h>
#include <stdint.h>

struct {
    int width;
    int height;
    uint32_t *pixels;
} frame = {0};

#if RAND_MAX == 32767
#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))
#else
#define Rand32() rand()
#endif

static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

const char g_szClassName[] = "myWindowClass";

// Step 4: the Window Procedure
LRESULT CALLBACK 
WndProc(HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
{
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
                       "This program is:",
                       MB_OK | MB_ICONINFORMATION); 
        } break;
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        } break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
        case WM_PAINT: {
            static PAINTSTRUCT paint;
            static HDC device_context;
            device_context = BeginPaint(hwnd, &paint);
            BitBlt(device_context,
                   paint.rcPaint.left,
                   paint.rcPaint.top,
                   paint.rcPaint.right  - paint.rcPaint.left,
                   paint.rcPaint.bottom - paint.rcPaint.top,
                   frame_device_context,
                   paint.rcPaint.left, paint.rcPaint.top,
                   SRCCOPY);
            EndPaint(hwnd, &paint);
        } break;
        case WM_SIZE: {
            frame_bitmap_info.bmiHeader.biWidth  = LOWORD(lParam);
            frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

            if(frame_bitmap)
            {
                DeleteObject(frame_bitmap);
            }
            frame_bitmap = CreateDIBSection(NULL,
                                            &frame_bitmap_info,
                                            DIB_RGB_COLORS,
                                            (void**)&frame.pixels, 0, 0);

            SelectObject(frame_device_context,
                         frame_bitmap);

            frame.width =  LOWORD(lParam);
            frame.height = HIWORD(lParam);
        } break;
        default:
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    return 0;
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
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL,
                   "Window Registration Failed!",
                   "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    frame_bitmap_info.bmiHeader.biPlanes = 1;
    frame_bitmap_info.bmiHeader.biBitCount = 32;
    frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    frame_device_context = CreateCompatibleDC(0);

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                          g_szClassName,
                          "tmeditor",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          500,
                          500,
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
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

        static unsigned int p = 0;
        for (int i = 0; i < 100; i++)
        {
            frame.pixels[i % (frame.width*frame.height)] = Rand32();
        }
        // frame.pixels[(p++)%(frame.width*frame.height)] = Rand32();
        // frame.pixels[Rand32()%(frame.width*frame.height)] = 0;

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
    }
    return Msg.wParam;
}