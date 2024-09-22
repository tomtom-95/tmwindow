#ifndef UTILS_HPP
#define UTILS_HPP

#include <windows.h>

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
    double v0;
    double v1;
    double v2;
} Vertex;

typedef struct VertexBuffer
{
    size_t len;
    Vertex *vertex;
} VertexBuffer;

typedef struct LinkedListVertexBuffer
{
    VertexBuffer vertex_buffer;
    VertexBuffer *next;
} LinkedListVertexBuffer;

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