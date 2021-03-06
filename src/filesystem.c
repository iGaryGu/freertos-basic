#include "osdebug.h"
#include "filesystem.h"
#include "fio.h"
#include "clib.h"

#include <stdint.h>
#include <string.h>
#include <hash-djb2.h>

#define MAX_FS 16

struct fs_t {
    uint32_t hash;
    fs_open_t cb;
    fs_open_dir_t dcb;
    void * opaque;
};
extern const unsigned char _sromfs;
extern uint32_t get_unaligned(const uint8_t * d);
static struct fs_t fss[MAX_FS];

__attribute__((constructor)) void fs_init() {
    memset(fss, 0, sizeof(fss));
}

int register_fs(const char * mountpoint, fs_open_t callback, fs_open_dir_t dir_callback, void * opaque) {
    int i;
    DBGOUT("register_fs(\"%s\", %p, %p, %p)\r\n", mountpoint, callback, dir_callback, opaque);
    
    for (i = 0; i < MAX_FS; i++) {
        if (!fss[i].cb) {
            fss[i].hash = hash_djb2((const uint8_t *) mountpoint, -1);
            fss[i].cb = callback;
            fss[i].dcb = dir_callback;
            fss[i].opaque = opaque;
            return 0;
        }
    }
    
    return -1;
}

int fs_open(const char * path, int flags, int mode) {
    const char * slash;
    uint32_t hash;
    int i;
//    DBGOUT("fs_open(\"%s\", %i, %i)\r\n", path, flags, mode);
    
    while (path[0] == '/')
        path++;
    
    slash = strchr(path, '/');
    
    if (!slash)
        return -2;

    hash = hash_djb2((const uint8_t *) path, slash - path);
    path = slash + 1;

    for (i = 0; i < MAX_FS; i++) {
        if (fss[i].hash == hash)
            return fss[i].cb(fss[i].opaque, path, flags, mode);
    }
    
    return -2;
}

int get_fileName(const uint8_t * romfs){
	const uint8_t * meta;
	uint32_t level;

	//dir
	if(get_unaligned(romfs) == 2){
		const uint8_t *temp = romfs + get_unaligned( romfs + 16 ) + 21;
		level = get_unaligned(temp + 4);
	}
	//current dir
	else if (get_unaligned(romfs) == 1){
		level = get_unaligned( romfs + 4);
	}
	else
		return -1;

	for (meta = romfs; get_unaligned(meta) && get_unaligned(meta + 16); meta += get_unaligned(meta + 16) + 21) {
		if(get_unaligned(meta+4) == level){
			fio_printf(1,"%s\r\n",meta+20);
		}
	}
	return 0;
}

static int root_opendir(){
	int r = get_fileName(&_sromfs);
	if(r==-1)
		return OPENDIR_NOTFOUNDFS; 
	else
		return r;
}

const uint8_t *search_dir(const uint8_t *romfs , uint32_t hash){
	const uint8_t * meta;
	for(meta = romfs; get_unaligned(meta) && get_unaligned(meta + 16); meta += get_unaligned(meta + 16) + 21) {
		int hash_temp = hash_djb2((const uint8_t *)(meta+20) , 5381);
		if(hash_temp == hash){
			return meta;
		}
	}
	return (const uint8_t *)-1;
}

int fs_opendir(const char * path){
	uint32_t hash;
	const uint8_t *dir;

	if ( path[0] == '\0' || (path[0] == '/' && path[1] == '\0') ){
		return root_opendir();
	}

	hash = hash_djb2((const uint8_t *)path,5381);

	dir = search_dir(&_sromfs,hash);
	if(dir == (const uint8_t *)-1)
		return OPENDIR_NOTFOUNDFS;
	return get_fileName(dir);
	
}

