#ifndef _KERNEL_VFS_TARFS_H
#define _KERNEL_VFS_TARFS_H

#include <kernel/vfs.h>

/* Initializes a new vfs. The vfs_t must be allocated. */
int vfs_tarfs_init(vfs_t*);

#endif
