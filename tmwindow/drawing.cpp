#ifndef DRAWING_CPP
#define DRAWING_CPP

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "utils.hpp"

static void
RenderWeirdGradient(win32_offscreen_buffer buffer,
                    int blue_offset,
                    int green_offset)
{
    // let's see what optimizer does
    uint8_t *row = (uint8_t *)buffer.memory;
    for (int y = 0; y < buffer.height; y++)
    {
        uint32_t *pixel = (uint32_t *)row;
        for (int x = 0; x < buffer.width; x++)
        {
            uint8_t Blue = (x + blue_offset);
            uint8_t Green = (y + green_offset);

            *pixel++ = ((Green << 8) | Blue);
        }

        row += 4 * buffer.pitch;
    }
}

// Simple but slow implementation of drawing a line
static void
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
static void
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
static void
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

// I actually did't bother doing the check on y+1/2 or x+1/2 (should I?) 
static void
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

static void
DrawTriangle(win32_offscreen_buffer buffer,
             Vertex vertex0,
             Vertex vertex1,
             Vertex vertex2)
{
    int width_scale = (buffer.width - 1) / 2;
    int height_scale = (buffer.height - 1) / 2;

    DrawLineBresenham(
        buffer,
        (vertex0.v0 + 1) * width_scale,
        (vertex1.v0 + 1) * width_scale,
        (vertex0.v1 + 1) * height_scale,
        (vertex1.v1 + 1) * height_scale,
        WHITE
    );
    DrawLineBresenham(
        buffer,
        (vertex0.v0 + 1) * width_scale,
        (vertex2.v0 + 1) * width_scale,
        (vertex0.v1 + 1) * height_scale,
        (vertex2.v1 + 1) * height_scale,
        WHITE
    );
    DrawLineBresenham(
        buffer,
        (vertex1.v0 + 1) * width_scale,
        (vertex2.v0 + 1) * width_scale,
        (vertex1.v1 + 1) * height_scale,
        (vertex2.v1 + 1) * height_scale,
        WHITE
    );
}

// TODO(tommaso): too many doubles!!!
static Vector3D
VectorNormalize(Vector3D vector)
{
    double vector_modulus = sqrt(vector.x * vector.x + 
                                 vector.y * vector.y +
                                 vector.z * vector.z);
    vector.x = vector.x / vector_modulus;
    vector.y = vector.y / vector_modulus;
    vector.z = vector.z / vector_modulus;

    return vector;
}

static Vertex
VertexDenormalize(win32_offscreen_buffer buffer,
                  Vertex vertex)
{
    vertex.v0 = (vertex.v0 + 1.0) * (buffer.width / 2.0 - 1);
    vertex.v1 = (vertex.v1 + 1.0) * (buffer.height / 2.0 - 1);

    return vertex;
}

struct Vector3D
TriangleGetNormalVector(Vertex A,
                        Vertex B,
                        Vertex C)
{
    struct Vertex v0 = {C.v0 - A.v0, C.v1 - A.v1, C.v2 - A.v2};
    struct Vertex v1 = {B.v0 - A.v0, B.v1 - A.v1, B.v2 - A.v2};

    struct Vector3D normal_vector;

    // compute the cross product
    normal_vector.x =   v0.v1 * v1.v2 - v0.v2 * v1.v1;
    normal_vector.y = -(v0.v0 * v1.v2 - v0.v2 * v1.v0);
    normal_vector.z =   v0.v0 * v1.v1 - v0.v1 * v1.v0;

    return normal_vector;
}

int
GetTriangleDeterminant(Vertex v0,
                       Vertex v1,
                       Vertex v2)
{
    int determinant = (v1.v0 * v2.v1 + v0.v0 * v1.v1 + v0.v0 * v2.v0) -
                      (v0.v1 * v1.v0 + v1.v1 * v2.v0 + v0.v0 * v2.v1);
    return determinant;
}

int
GetEdgeFunction(Vertex A,
                Vertex B,
                Vertex C)
{
    return (B.v0 - A.v0) * (C.v1 - A.v1) - (B.v1 - A.v1) * (C.v0 - A.v0);
}

struct TriangleBoundingBox
GetTriangleBoundingBox(Vertex A,
                       Vertex B,
                       Vertex C)
{
    struct Vertex vertices[3] = {A, B, C};

    int x_min = A.v0;
    int x_max = A.v0;
    int y_min = A.v1;
    int y_max = A.v1;

    for (int i = 1; i < 3; i++)
    {
        if (vertices[i].v0 < x_min)
        {
            x_min = vertices[i].v0;
        }
        if (vertices[i].v0 > x_max)
        {
            x_max = vertices[i].v0;
        }
        if (vertices[i].v1 < y_min)
        {
            y_min = vertices[i].v1;
        }
        if (vertices[i].v1 > y_max)
        {
            y_max = vertices[i].v1;
        }
    }

    struct TriangleBoundingBox bounding_box = {x_min, x_max, y_min, y_max}; 
    return bounding_box;
}

void
ColorTriangle(win32_offscreen_buffer buffer,
              Vertex A,
              Vertex B,
              Vertex C,
              int color)
{
    struct TriangleBoundingBox bounding_box;
    bounding_box = GetTriangleBoundingBox(A, B, C);

    int edge = GetEdgeFunction(A, B, C);
    for (int i = bounding_box.x_min; i < bounding_box.x_max; i++)
    {
        for (int j = bounding_box.y_min; j < bounding_box.y_max; j++)
        {
            struct Vertex P = {i, j, 0};
            if (GetEdgeFunction(A, B, P) > 0 &&
                GetEdgeFunction(B, C, P) > 0 &&
                GetEdgeFunction(C, A, P) > 0)
            {
                uint32_t *pixel = (uint32_t *)buffer.memory + buffer.pitch * (int)(P.v1) + (int)(P.v0);
                *pixel = color;
            }
        }
    }
}

static void
ColorWireFrameObj(win32_offscreen_buffer buffer,
                  GrowableBuffer face_buffer,
                  GrowableBuffer vertex_buffer)
{
    int light_direction[3] = {0, 0, -1};

    int width_scale = (buffer.width - 1) / 2;
    int height_scale = (buffer.height - 1) / 2;

    int face_offset = 0;
    while (true)
    {
        if (face_offset == face_buffer.allocation_offset)
        {
            return;
        }
        else
        {
            Face *face = (Face *)(face_buffer.data + face_offset);
            Vertex *vertex0 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[0] - 1));
            Vertex *vertex1 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[1] - 1));
            Vertex *vertex2 = (Vertex *)(vertex_buffer.data + sizeof(Vertex) * ((face->vertex_indices)[2] - 1));

            struct Vector3D normal_vector = TriangleGetNormalVector(*vertex0,
                                                                    *vertex1,
                                                                    *vertex2);
            normal_vector = VectorNormalize(normal_vector);
            double intensity = normal_vector.x * light_direction[0] +
                               normal_vector.y * light_direction[1] +
                               normal_vector.z * light_direction[2];
            
            Vertex v0 = VertexDenormalize(buffer, *vertex0);
            Vertex v1 = VertexDenormalize(buffer, *vertex1);
            Vertex v2 = VertexDenormalize(buffer, *vertex2);

            if (intensity > 0)
            {
                int color = RED   * (uint8_t)(intensity * 255) |
                            GREEN * (uint8_t)(intensity * 255) |
                            BLUE  * (uint8_t)(intensity * 255);
                ColorTriangle(buffer,
                              v0, v1, v2,
                              color);
            }
        }
        face_offset += sizeof(Face);
    }
}

#endif // DRAWING_CPP