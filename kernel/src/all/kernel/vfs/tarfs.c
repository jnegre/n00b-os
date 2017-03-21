#include <kernel/vfs.h>
#include <kernel/vfs/tarfs.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/*
 * tarfs data structures
 */

struct tar_vfs_data {
	vnode_t* root;
};

struct tar_reg_data {
	unsigned int size;
	char* content;
};

struct tar_dir_entry {
	char* name;
	vnode_t* vnode;
	struct tar_dir_entry* next;
};

struct tar_dir_data {
	struct tar_dir_entry* entries;
};

/*
 * Tar utils
 */

struct tar_header {
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime [12];
	char checksum[8];
	char type[1];
	char linkedname[100];
	char ustar[6]; // "ustar\0"
	char ustarversion[2]; // "00"
	char user[32];
	char group[32];
	char major[8];
	char minor[8];
	char filenameprefix[155];
};

static bool is_empty_header(struct tar_header* header) {
	// just checks the filename
	return header->filename[0] == 0;
}

static bool is_valid_header(struct tar_header* header) {
	return strcmp(header->ustar, "ustar") == 0
		&& header->ustarversion[0] == '0'
		&& header->ustarversion[1] == '0';
}

static int read_octal(const char* field) {
	int res = 0;
	for(int i=0; field[i] != 0; i++) {
		res = res*8 + field[i] - '0';
	}
	return res;
}

static struct tar_header* next_header(struct tar_header* header) {
	int size = read_octal(header->size);
	struct tar_header* next = (void*)header + ((size / 512) + 1) * 512;
	if(size % 512) {
		next = (void*)next + 512;
	}
	return next;
}

/*
 * Vnode ops
 */
static int node_open(vnode_t** vpp, const int f, const credentials_t* c);
static int node_close(vnode_t** vpp, const int f, const credentials_t* c);
static int node_lookup(vnode_t* vp, const char* nm, vnode_t** vpp, const credentials_t* c);
static int node_rdwr(vnode_t* vp, uio_t* uiop, enum uio_rw rw, int f, const credentials_t* c);

static const struct vnodeops node_ops = {
	.open = node_open,
	.close = node_close,
	.lookup = node_lookup,
	.rdwr = node_rdwr
};

/** Returns a new empty dir, or NULL if OOM **/
static vnode_t* create_dir(void) {
	vnode_t* node = malloc(sizeof(vnode_t));
	if(node == NULL) {
		return NULL;
	}
	node->ops = &node_ops;
	node->type = VDIR;
	struct tar_dir_data* data = malloc(sizeof(struct tar_dir_data));
	if(data == NULL) {
		free(node);
		return NULL;
	}
	node->data = data;
	return node;
}

static vnode_t* create_file(struct tar_header* header) {
	vnode_t* node = malloc(sizeof(vnode_t));
	if(node == NULL) {
		return NULL;
	}
	node->ops = &node_ops;
	node->type = VREG;
	struct tar_reg_data* data = malloc(sizeof(struct tar_reg_data));
	if(data == NULL) {
		free(node);
		return NULL;
	}
	node->data = data;
	data->size = read_octal(header->size);
	// copy the content
	char *content = malloc(data->size);
	if(content == NULL) {
		free(data);
		free(node);
		return NULL;
	}
	data->content = memcpy(content, ((void*)header) + 512, data->size);
	return node;
}

static int add_to_dir(vnode_t* dir, const char* name, vnode_t* child) {
	struct tar_dir_data* data = dir->data;
	struct tar_dir_entry* entry = malloc(sizeof(struct tar_dir_entry));
	if(entry == NULL) {
		return -1;
	}
	char* name_copy = malloc(strlen(name)+1);
	if(name_copy == NULL) {
		free(entry);
		return -1;
	}
	memcpy(name_copy, name, strlen(name)+1);
	entry->name = name_copy;
	entry->vnode = child;
	entry->next = data->entries;
	data->entries = entry;
	return 0;
}

static int lookup_parent(vnode_t* root, char* path, vnode_t** parent, char** name) {
	vnode_t* current = root;
	int length = strlen(path);
	if(path[length-1] == '/') {
		--length;
	}
	int index = 0;
	char nm[100]; // path is 100 chars max as per tar spec

	do {
		int nm_index = 0;
		while(index<length && path[index]!='/') {
			nm[nm_index] = path[index];
			++nm_index;
			++index;
		}
		nm[nm_index] = 0;
		if(index<length) {
			//not the last segment
			if(node_lookup(current, nm, &current, NULL)) {
				return -1;
			}
			++index;
		}
	} while(index<length);
	*parent = current;
	memcpy(*name, nm, strlen(nm)+1); // TODO implement strcpy
	return 0;
}

static int node_open(vnode_t** vpp, const int f, const credentials_t* c) {
	//TODO
	return 0;
}

static int node_close(vnode_t** vpp, const int f, const credentials_t* c) {
	//TODO
	return 0;
}

static int node_lookup(vnode_t* vp, const char* nm, vnode_t** vpp, const credentials_t* c) {
	if(vp->type != VDIR) {
		return -1; // wrong type
	}
	struct tar_dir_data* data = vp->data;
	for(struct tar_dir_entry* entry = data->entries; entry != NULL; entry = entry->next) {
		if(strcmp(nm, entry->name) == 0) {
			*vpp = entry->vnode;
			return 0;
		}
	}

	return -1;
}

static int node_rdwr(vnode_t* vp, uio_t* uiop, enum uio_rw rw, int f, const credentials_t* c) {
	if(vp->type != VREG) {
		return -1; // wrong type
	}
	if(rw == UIO_READ) {
		struct tar_reg_data* data = vp->data;
		if(uiop->length > data->size - uiop->offset) {
			uiop->length = data->size - uiop->offset;
		}
		memcpy(uiop->buffer, data->content+uiop->offset, uiop->length);
		return 0;
	} else {
		// not supported
		return -1;
	}
}

/*
 * VFS ops
 */

static void free_all(struct tar_vfs_data* vfs_data) {
	//TODO
}

static int fs_mount(vfs_t* vfsp, const char* path, void* datap) {
	printf("Mounting new tarfs on %s\n", path);

	// vfs data
	struct tar_vfs_data* vfs_data = malloc(sizeof(struct tar_vfs_data));
	if(vfs_data == NULL) {
		return -1;
	}
	vfsp->data = vfs_data;

	// create root vnode
	vnode_t* root = create_dir();
	if(root == NULL) {
		free_all(vfs_data);
		return -1;
	}
	vfs_data->root = root;

	for(struct tar_header* header = datap; !is_empty_header(header); header = next_header(header)) {
		int size = read_octal(header->size);
		char nm[100] = {0}; // TODO useless init
		char* bob =nm; //FIXME ugly
		vnode_t* parent = NULL; // TODO useless
		if(lookup_parent(root, header->filename, &parent, &bob)) {
			//lookup failed
			free_all(vfs_data);
			return -1;
		}
		vnode_t* new_node;
		if(header->type[0] == '0') {
			new_node = create_file(header);
		} else if(header->type[0] == '5') {
			new_node = create_dir();
		} else {
			free_all(vfs_data);
			return -1; // FIXME not implemented
		}
		if(new_node == NULL) {
			free_all(vfs_data);
			return -1;
		}
		add_to_dir(parent, nm, new_node);
	}
	return 0;
}

static int fs_root(vfs_t* vfsp, vnode_t** vpp) {
	struct tar_vfs_data* data = vfsp->data;
	*vpp = data->root;
	return 0;
}

static const struct vfsops fs_ops = {
	.mount = fs_mount,
	.root = fs_root
};

/*
 * tarfs.h functions
 */

int vfs_tarfs_init(vfs_t* vfsp) {
	if(vfsp == NULL) {
		return -1;
	}
	vfsp->next = NULL;
	vfsp->ops = &fs_ops;
	return 0;
}