/*
    Parser for the wavefront object format file
    Used to read the mesh from the african_head.obj
    in models folder
*/

#ifndef WAVEFRONT_PARSER_CPP
#define WAVEFRONT_PARSER_CPP

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "utils.hpp"

struct Buffer
BufferAllocate(size_t count)
{
    struct Buffer buffer = {};

    // NOTE(tommaso): If I want to start wasting time I could write two path
    //                one that uses VirtualAlloc() for WIN32 platform and one
    //                that uses mmap() for Unix platform
    //                would be fine if I have some time to really dig into the
    //                differences between VirtualAlloc() and mmap()
    buffer.data = (char *)malloc(count);
    if (buffer.data)
    {
        buffer.len = count;
    }
    else
    {
        // TODO(tommaso): Logging
    }

    return buffer;
}

void
BufferFree(struct Buffer *buffer)
{
    if (buffer -> data)
    {
        free(buffer -> data);
    }
    *buffer = {};
}

struct VertexBuffer
VertexBufferAllocate(size_t len)
{
    struct VertexBuffer vertex_buffer = {};

    // NOTE(tommaso): If I want to start wasting time I could write two path
    //                one that uses VirtualAlloc() for WIN32 platform and one
    //                that uses mmap() for Unix platform
    //                would be fine if I have some time to really dig into the
    //                differences between VirtualAlloc() and mmap()
    vertex_buffer.vertex = (Vertex *)malloc(len);
    if (vertex_buffer.len)
    {
        vertex_buffer.len = len;
    }
    else
    {
        // TODO(tommaso): Logging
    }

    return vertex_buffer;
}

void
VertexBufferFree()
{
    return;
}

struct Buffer
ReadEntireFile(const char *filename)
{
    struct Buffer buffer = {};
        
    FILE *file = fopen(filename, "rb");
    if (file)
    {
#if _WIN32
        struct __stat64 stat;
        _stat64(filename, &stat);
#else
        struct stat stat;
        stat(filename, &stat);
#endif
        buffer = BufferAllocate(stat.st_size);
        if (buffer.data)
        {
            if (fread(buffer.data, buffer.len, 1, file) != 1)
            {
                // TODO(tommaso): Logging
                BufferFree(&buffer);
            }
        }
        
        fclose(file);
    }
    else
    {
        // TODO(tommaso): Logging
    }
    
    return buffer;
}

size_t
GetIdxLineEnd(char *data, size_t data_len, size_t line_start)
{
    size_t line_end = line_start;
    for (;;)
    {
        if (line_end == data_len)
        {
            break;
        }
        else
        {
#if _WIN32
            if (data[line_end] == '\r' && data[line_end + 1] == '\n')
            {
                line_end++;
                break;
            }
#else
            if (data[line_end] == '\n')
            {
                break;
            }
#endif // _WIN32
            else
            {
                line_end++;
            }
        }
    }

    return line_end;
}

static void
WavefrontParser(char *filepath)
{
    struct Buffer filebuffer = ReadEntireFile(filepath);

    size_t line_start = 0;
    size_t line_end = 0;
    while (line_end != filebuffer.len)
    {
        line_end = GetIdxLineEnd(
            filebuffer.data,
            filebuffer.len,
            line_start
        );
        line_start = line_end + 1;
    }
}

#endif // WAVEFRONT_PARSER_CPP