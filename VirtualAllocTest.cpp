#ifndef VIRTUAL_ALLOC_TEST_CPP
#define VIRTUAL_ALLOC_TEST_CPP

#include "utils.hpp"

#define KiloByte 1024LL
#define MegaByte (1024LL * 1024LL)
#define GigaByte (1024LL * 1024LL * 1024LL)

void
VirtualAllocTest()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    int page_size = 4 * KiloByte;
    int vertex_per_buffer = 10000;

    int allocation_count  = 0;
    int allocation_size   = sizeof(Vertex) * vertex_per_buffer;
    int allocation_offset = 8;

    char *arena = (char *)VirtualAlloc(0, GigaByte,
                                       MEM_RESERVE, PAGE_READWRITE);

    VirtualAlloc(arena + allocation_offset,
                 allocation_size,
                 MEM_COMMIT, PAGE_READWRITE);
    allocation_count += 1;
    

    Vertex *vertex;

    for (int i = 0; i < 20000; i++)
    {
        if (allocation_offset >= allocation_count * allocation_size)
        {
            VirtualAlloc(arena + allocation_offset,
                         allocation_size,
                         MEM_COMMIT, PAGE_READWRITE);
            allocation_count++;
        }

        vertex = (Vertex *)(arena + allocation_offset);
        vertex->v0 = i + 0.1;
        vertex->v1 = i + 0.2;
        vertex->v2 = i + 0.3;

        allocation_offset += sizeof(Vertex);
    }
}

int
main(void)
{
    VirtualAllocTest();
    return 0;
}

#endif // VIRTUAL_ALLOC_TEST_CPP