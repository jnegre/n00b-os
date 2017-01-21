Address | What | Note
--------|------|-----
0xC0000000 | Start of low 1M memory mapping |
0xC00B8000 | VGA screen |
0xC00FFFFF | Last byte of low 1M memory mapping |
0xC0100000 | kernel_start | 
? | mm_freepage | 4k to be used by the mm (4k aligned)
? | kernel_end |
....||
0xFFBFE000 | | start of the 1st page of stack (TBD)
0xFFBFE00C | | canari (4) (TBD)
0xFFBFE010 | | top of stack (TBD)
....|| stack, almost 8k (TBD)
0xFFBFFFF0 | | bottom of stack, goes towards lower memory (8k - 16 aligned) (TBD)
0xFFBFFFF0 | | canari (4) (TBD)
0xFFBFFFF8 | | task_info (init) (=(ESP and FFFFE000) or 3FF8) (TBD)
0xFFC00000 | Page Table #0 | 4k
0xFFC01000 | Page Table #1 | 4k
....||
0xFFF00000 | Page Table #768 | 4k (for the kernel at 3G)
....||
0xFFFFF000 | Page Directory / Page Table #1023 | Recursive Page Directory

