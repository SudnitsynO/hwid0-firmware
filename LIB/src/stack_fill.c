
#include <stack_fill.h>

extern uint8_t _estack;
extern uint8_t _ebss;
extern uint32_t _Min_Stack_Size;

void fill_stack(uint32_t pattern)
{
    uint32_t *top, *start;
    __asm__ volatile("mov %[top], sp" : [top] "=r"(top) : :);
    start = (uint32_t *)(&_estack - (uint32_t)(&_Min_Stack_Size));
    while (start < top)
    {
        *(start++) = pattern;
    }
}

void fill_heap(uint32_t pattern)
{
    uint32_t *top, *start;
    top = (uint32_t *)(&_estack - (uint32_t)(&_Min_Stack_Size));
    start = (uint32_t*)&_ebss;
    while (start < top)
    {
        *(start++) = pattern;
    }
}