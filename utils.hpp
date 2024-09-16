#ifndef UTILS_HPP
#define UTILS_HPP

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

struct Buffer
{
    size_t len;
    char *data;
};

typedef struct Vertex
{
    int v0;
    int v1;
    int v2;
} Vertex;

typedef struct VertexBuffer
{
    size_t len;
    Vertex *vertex;
} VertexBuffer;


void
IntSwap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

#endif // UTILS_HPP