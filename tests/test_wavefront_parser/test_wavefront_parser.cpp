#include "../../tmwindow/wavefront_parser.cpp"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
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

    WavefrontParser("../models/medium_african_head.obj",
                    &vertex_buffer,
                    &face_buffer);

    int vertex_offset = 0;
    while (true)
    {
        if (vertex_offset != vertex_buffer.allocation_offset)
        {
            Vertex *vertex = (Vertex *)(vertex_buffer.data + vertex_offset);
            printf("%lf %lf %lf\n", vertex->v0, vertex->v1, vertex->v2);
            vertex_offset += sizeof(Vertex);
        }
        else
        {
            break;
        }
    }

    int face_offset = 0;
    while (true)
    {
        if (face_offset != face_buffer.allocation_offset)
        {
            Face *face = (Face *)(face_buffer.data + face_offset);
            printf("%i %i %i\n",
                   (face->vertex_indices)[0],
                   (face->vertex_indices)[1],
                   (face->vertex_indices)[2]);
            face_offset += sizeof(Face);
        }
        else
        {
            break;
        }
    }
}