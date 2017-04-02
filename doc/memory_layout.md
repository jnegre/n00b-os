Address | What | Note
--------|------|-----
0x00000000 | Unmappable memory | 4k
0x00001000 | Start of user space |
....||
0xC0000000 | Start of kernel space; Maps low 1M memory |
0xC00B8000 | VGA screen |
0xC00FFFFF | Last byte of low 1M memory mapping |
0xC0100000 | kernel_start | 
? | mm_freepage | 4k to be used by the mm (4k aligned)
? | kernel_end |
? | mm_info->heap_start |
....|| Heap
? | mm_info->heap_end |
....||
0xFFBFC000 | | start of the 1st page of stack
0xFFBFC00C | | canary 0xDEADBEEF (4)
0xFFBFC010 | | top of stack (almost 16k)
0xFFBFD000 | | start of 2nd page of stack
0xFFBFE000 | | start of 3rd page of stack
0xFFBFF000 | stack_bottom | start of 4th page of stack
0xFFBFFFF0 | | bottom of stack, goes towards lower memory (16k - 16 aligned)
0xFFBFFFF0 | | canary 0xDEADBEEF (4)
0xFFBFFFF8 | | task_info (init) (=(ESP and FFFFD000) or 3FF8) (TBD)
0xFFC00000 | mm_info->stacks_start; Page Table #0 | 4k
0xFFC01000 | Page Table #1 | 4k
....||
0xFFF00000 | Page Table #768 | 4k (for the kernel at 3G)
....||
0xFFFFF000 | Page Directory / Page Table #1023 | Recursive Page Directory

