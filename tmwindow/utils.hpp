#ifndef UTILS_HPP
#define UTILS_HPP

#include <windows.h>

#define KiloByte (1024LL)
#define MegaByte (1024LL * 1024LL)
#define GigaByte (1024LL * 1024LL * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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

struct GrowableBuffer
{
    int allocation_count;
    int allocation_size;
    int allocation_offset;
    char *data;
};

typedef struct Vertex
{
    double v0;
    double v1;
    double v2;
} Vertex;

typedef struct Face
{
    int vertex_indices[3];
    int texture_indices[3];
    int normal_indices[3];
} Face;

void
IntSwap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

#endif // UTILS_HPP