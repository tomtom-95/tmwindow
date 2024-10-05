#ifndef DRAWING_CPP
#define DRAWING_CPP

#include <stdlib.h>
#include <stdio.h>

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

#endif // DRAWING_CPP