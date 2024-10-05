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

void
ParseVertex(char **data, GrowableBuffer *vertex_buffer)
{
    int allocation_offset = vertex_buffer->allocation_offset;
    int allocation_count  = vertex_buffer->allocation_count;
    int allocation_size   = vertex_buffer->allocation_size;
    if (allocation_offset + sizeof(Vertex) >= allocation_count * allocation_size)
    {
        VirtualAlloc(vertex_buffer->data + allocation_offset,
                     allocation_size,
                     MEM_COMMIT, PAGE_READWRITE);
        (vertex_buffer->allocation_count)++;
    }

    Vertex *v = (Vertex *)(vertex_buffer->data + allocation_offset);

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

    v->v0 = nums[0];
    v->v1 = nums[1];
    v->v2 = nums[2];

    vertex_buffer->allocation_offset += sizeof(Vertex);
}

void
ParseFace(char **data, GrowableBuffer *face_buffer)
{
    int allocation_offset = face_buffer->allocation_offset;
    int allocation_count  = face_buffer->allocation_count;
    int allocation_size   = face_buffer->allocation_size;
    if (allocation_offset >= allocation_count * allocation_size)
    {
        VirtualAlloc(face_buffer->data + allocation_offset,
                     allocation_size,
                     MEM_COMMIT, PAGE_READWRITE);
        (face_buffer->allocation_count)++;
    }

    Face *face = (Face *)(face_buffer->data + allocation_offset);

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

        face->vertex_indices[i]  = nums[0];
        face->texture_indices[i] = nums[1];
        face->normal_indices[i]  = nums[2];
    }

    face_buffer->allocation_offset += sizeof(Face);
}


static void
WavefrontParser(char *filepath, GrowableBuffer *vertex_buffer, GrowableBuffer *face_buffer)
{
    Buffer filebuffer = ReadEntireFile(filepath);

    char *ptr = filebuffer.data;
    while (ptr - filebuffer.data != filebuffer.len)
    {
        if (*ptr == 'v' && *(ptr + 1) == ' ')
        {
            ParseVertex(&ptr, vertex_buffer);
        }
        else if (*ptr == 'f' && *(ptr + 1) == ' ')
        {
            ParseFace(&ptr, face_buffer);
        }
        else
        {
            ptr++;
        }
    }
}

#endif // WAVEFRONT_PARSER_CPP