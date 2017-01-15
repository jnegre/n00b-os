/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 3GiB + 1 MiB */
	. = 3*1024M + 1M;
	
	/* These labels get exported to the code file */
	kernel_start = .;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	/* Loads AT 1MiB */
	.text BLOCK(4K) : AT(ADDR(.text) - 3*1024M) ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data. */
	/* Loads AT 1MiB */
	.rodata BLOCK(4K) : AT(ADDR(.rodata) - 3*1024M) ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	/* Loads AT 1MiB */
	.data BLOCK(4K) : AT(ADDR(.data) - 3*1024M) ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	/* Loads AT 1MiB */
	.bss BLOCK(4K) : AT(ADDR(.bss) - 3*1024M) ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */

	kernel_end = .;
}

