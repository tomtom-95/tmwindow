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
#include <ctype.h>

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
    //                that uses mmap() for Unix platform and see the difference
    vertex_buffer.vertex = (Vertex *)malloc(len);
    if (vertex_buffer.len)
    {
        vertex_buffer.len = len;
    }
    else
    {
        printf("VertexBufferAllocate failed\n");
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

/*
    Vertex Parsing: first method
    Args:
        data: buffer with the content of a wavefront obj file
        idx_start: starts reading from data at this index
        vertex: pointer to the Vertex where result of the parsing is stored
    Return:
        index of position in data one after the last character read
*/
int
ParseVertexV1(char *data, int idx_start, Vertex *vertex)
{
    double nums[3] = {};
    char *number_end = NULL;
    for (int i = 0; i < 3; i++)
    {
        for (;;)
        {
            if (isdigit(data[idx_start]) || data[idx_start] == '-')
            {
                nums[i] = strtold(data + idx_start, &number_end);
                idx_start = number_end - data;
                break;
            }
            else
            {
                idx_start++;
            }
        }
    }
    int idx_end = number_end - data;

    vertex->v0 = nums[0];
    vertex->v1 = nums[1];
    vertex->v2 = nums[2];

    return idx_end;
}

/*
    Let's try another implementation for vertex parsing
    This time we pass char **data and we do not use and idx
    to keep track of where we are in the buffer
    we can return a struct Vertex
*/
Vertex
ParseVertexV2(char **data)
{
    double nums[3] = {};
    char *number_end = NULL;
    for (int i = 0; i < 3; i++)
    {
        for (;;)
        {
            if (isdigit(**data) || **data == '-')
            {
                nums[i] = strtold(*data, &number_end);
                *data = number_end;
                break;
            }
            else
            {
                (*data)++;
            }
        }
    }

    Vertex vertex = {nums[0], nums[1], nums[2]};
    return vertex;
}

Face
ParseFace(char **data)
{
    Face face = {};
    int nums[3] = {};

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (;;)
            {
                if (isdigit(**data))
                {
                    char *ptr = *data;
                    while (isdigit(*ptr))
                    {
                        ptr++;
                    }
                    nums[j] = atoi(*data);
                    *data = ptr;
                    break;
                }
                else
                {
                    (*data)++;
                }
            }
        }

        face.vertex_indices[i]  = nums[0];
        face.texture_indices[i] = nums[1];
        face.normal_indices[i]  = nums[2];
    }

    return face;
}


static void
WavefrontParser(char *filepath)
{
    Buffer filebuffer = ReadEntireFile(filepath);

    VertexBuffer vertex_buffer = {};
    vertex_buffer.vertex = (Vertex *)VirtualAlloc(0, 10000 * sizeof(Vertex),
                                                  MEM_RESERVE, PAGE_READWRITE);
    int bytes_committed = 0;

    char *ptr = filebuffer.data;
    while (ptr - filebuffer.data != filebuffer.len)
    {
        if (*ptr == 'v' && *(ptr + 1) == ' ')
        {
            Vertex vertex = ParseVertexV2(&ptr);
        }
        else if (*ptr == 'f')
        {
            Face face = ParseFace(&ptr);
        }
        else
        {
            ptr++;
        }
    }

    /*
    size_t idx = 0;
    while (idx != filebuffer.len)
    {
        if (filebuffer.data[idx] == 'v' && filebuffer.data[idx + 1] == ' ')
        {
            struct Vertex vertex = {};
            idx = ParseVertexV1(filebuffer.data, idx + 2, &vertex);
        }
        else
        {
            idx++;
        }
    }
    */

    /*
    size_t idx = 0;
    while (idx != filebuffer.len)
    {
        if (filebuffer.data[idx] == 'v' && filebuffer.data[idx + 1] == ' ')
        {
            char *ptr = filebuffer.data + idx + 2;
            Vertex vertex = ParseVertexV2(&ptr);
            idx = ptr - filebuffer.data;
        }
        else
        {
            idx++;
        }
    }
    */

}

#endif // WAVEFRONT_PARSER_CPP