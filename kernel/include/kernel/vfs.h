#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stdint.h>
#include <stddef.h>


/*
 * Data structures
 */

typedef struct vfs {
	struct vfs* next;
	const struct vfsops* ops;
	void* data;
} vfs_t;

enum vtype {VREG, VDIR};
typedef struct vnode {
	const struct vnodeops* ops;
	enum vtype type;
	void* data;
} vnode_t;

typedef struct credentials {
	//TODO add some stuff
} credentials_t;

// TODO move to uio.h + implement uiomove() to copy between user and kernel space
enum uio_rw {UIO_READ, UIO_WRITE};
typedef struct uio {
	size_t offset;
	char* buffer; // in-out
	size_t length; // in-out
} uio_t;

struct vfsops {
	int (*mount)(vfs_t*, const char*, void*);
	int (*root)(vfs_t*, vnode_t**);
};
#define VFS_MOUNT(vfsp, path, datap) ((vfsp)->ops->mount((vfsp), (path), (datap)))
#define VFS_ROOT(vfsp, vpp) ((vfsp)->ops->root((vfsp), (vpp)))

struct vnodeops {
	int (*open)(vnode_t**, const int, const credentials_t*);
	int (*close)(vnode_t**, const int, const credentials_t*);
	int (*lookup)(vnode_t*, const char*, vnode_t**, const credentials_t*);
	int (*rdwr)(vnode_t*,uio_t*, enum uio_rw, const int, const credentials_t*);
};
#define VN_OPEN(vpp, f, c) ((*vpp)->ops->open((vpp), (f), (c)))
#define VN_CLOSE(vpp, f, c) ((*vpp)->ops->close((vpp), (f), (c)))
#define VN_LOOKUP(vp, nm, vpp, c) ((vp)->ops->lookup((vp), (nm), (vpp), (c)))
#define VN_RDWR(vp, uiop, rw, f, c) ((vp)->ops->rdwr((vp), (uiop), (rw), (f), (c)))

// for the pcb + kernel

struct fpos {
	size_t pos;
};

typedef struct file_handle {
	vnode_t* node;
	struct fpos pos;
	int flags;
	int error;
	int eof;
} file_handle_t;

#define VFS_OPEN_FILES_LENGTH 7
typedef struct vfs_open_files {
	file_handle_t* handles[VFS_OPEN_FILES_LENGTH];
	struct vfs_open_files* next;
} vfs_open_files_t;

typedef struct vfs_info {
	vnode_t* root;
	vnode_t* cwd;
	vfs_open_files_t* files;
} vfs_info_t;



/*
 * Kernel operations
 */
void vfs_create_initial_fs(void*);

int vn_open(file_handle_t** handlepp, const char* path, const int f, const credentials_t* c);
int vn_rdwr(file_handle_t* handlep, enum uio_rw rw, char* buffer, size_t* length, const credentials_t* c);

#endif
