#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/vfs.h>
#include <kernel/vfs/tarfs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static vfs_t* rootvfs = NULL;

void vfs_create_initial_fs(void* initrd) {
	// initrd
	vfs_t* initrdfs = malloc(sizeof(vfs_t));
	vfs_tarfs_init(initrdfs);

	if(VFS_MOUNT(initrdfs, "/", initrd)) {
		panic("Unable to create initial fs");
	}
	// TODO free initrd from memory (in the calling method), all usefull data has been copied
	rootvfs = initrdfs;

	// set pcb->root and cwd
	process_control_block_t* pcb = current_process_control_block();
	if(VFS_ROOT(rootvfs, &(pcb->vfs_info->root)) || VFS_ROOT(rootvfs, &(pcb->vfs_info->cwd))) {
		panic("Unable to get root of inital fs");
	}
}

static int put_file_handle(file_handle_t* file_handle) {
	vfs_open_files_t* vfs_open_files = current_process_control_block()->vfs_info->files;
	int index = 0;
	
	while(true) { //TODO rewrite this loop
		for(int i=0; i<VFS_OPEN_FILES_LENGTH; i++) {
			if(vfs_open_files->handles[i] == NULL) {
				//free spot!
				vfs_open_files->handles[i] = file_handle;
				return 0;
			}
		}
		if(vfs_open_files->next!=NULL) {
			vfs_open_files = vfs_open_files->next;
			index += VFS_OPEN_FILES_LENGTH;
		} else {
			break;
		}
	}
	//TODO
	panic("Must append another vfs_open_files_t");
}

static void remove_file_handle(file_handle_t* file_handle) {
	//iterate over all file handles since we don't store the index
	for(
		vfs_open_files_t* vfs_open_files = current_process_control_block()->vfs_info->files;
		vfs_open_files!=NULL;
		vfs_open_files=vfs_open_files->next) {
			for(int i=0; i<VFS_OPEN_FILES_LENGTH; i++) {
				if(vfs_open_files->handles[i] == file_handle) {
					vfs_open_files->handles[i] = NULL;
					return;
				}
			}
		}
		panic("Can't find file_handle to remove");
}

int vn_open(file_handle_t** handlepp, const char* path, const int f, const credentials_t* c) {
	int error;
	int length = strlen(path);
	if(length == 0) {
		// invalid path
		return -1;
	}
	if(path[length-1] == '/') {
		return -1; // can't open a dir
	}
	vnode_t** node = malloc(sizeof(vnode_t**));
	if(path[0] == '/') {
		*node = current_process_control_block()->vfs_info->root;
	} else {
		*node = current_process_control_block()->vfs_info->cwd;
	}

	// copy the **const** path since strtok_k will modify it
	char* workpath = malloc(length + 1);
	if(workpath == NULL) {
		free(node);
		return -1;
	}
	strcpy(workpath, path);

	char* saveptr;
	char* element;
	element = strtok_r(workpath, "/", &saveptr);
	while(element != NULL) {
		error = VN_LOOKUP(*node, element, node, c);
		if(error) {
			free(workpath);
			free(node);
			return error;
		}
		element = strtok_r(NULL, "/", &saveptr);
	}
	free(workpath);
	
	// call VN_OPEN on the node
	error = VN_OPEN(node, f, c);
	if(error) {
		free(node);
		return error;
	}
	// we now have the vnode, let's put it in the pcb for later
	file_handle_t* handle = malloc(sizeof(file_handle_t));
	if(!handle) {
		free(node);
		return -1;
	}
	handle->node = *node;
	handle->pos.pos = 0;
	handle->flags = f;
	handle->error = 0;
	handle->eof = 0;
	put_file_handle(handle);
	*handlepp = handle;
	return 0;
}

int vn_close(file_handle_t* handlep, const credentials_t* c) {
	int error = VN_CLOSE(handlep->node, handlep->flags, c);
	remove_file_handle(handlep);
	handlep->node = NULL; // just to be sure it's not used by mistake
	free(handlep);
	return error;
}

int vn_rdwr(file_handle_t* handlep, enum uio_rw rw, char* buffer, size_t* length, const credentials_t* c) {
	uio_t uio = {
		.offset = handlep->pos.pos,
		.buffer = buffer,
		.length = *length
	};
	int error = VN_RDWR(handlep->node, &uio, rw, handlep->flags, c);
	handlep->pos.pos += uio.length;
	*length = uio.length;
	return error;
}