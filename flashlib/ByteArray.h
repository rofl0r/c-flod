#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

enum ByteArray_Endianess {
	BAE_BIG,
	BAE_LITTLE,
};

enum ByteArray_Type {
	BAT_MEMSTREAM,
	BAT_FILESTREAM,
};

struct ByteArray {
	int type;
	off_t pos;
	off_t size;
	enum ByteArray_Endianess endian;
	enum ByteArray_Endianess sys_endian;
	union {
		char* start_addr;
		int fd;
	};
	void (*readMultiByte)(struct ByteArray*, char*, size_t);
	unsigned int (*readUnsignedInt)(struct ByteArray*);
	signed int (*readInt)(struct ByteArray*);
	unsigned short (*readUnsignedShort)(struct ByteArray*);
	signed short (*readShort)(struct ByteArray*);
	unsigned char (*readUnsignedByte)(struct ByteArray*);
	signed char (*readByte)(struct ByteArray*);
	//off_t (*bytesAvailable)(struct ByteArray*);
};

void ByteArray_defaults(struct ByteArray* self);
void ByteArray_ctor(struct ByteArray* self);
struct ByteArray* ByteArray_new(void);
off_t ByteArray_get_position(struct ByteArray* self);
int ByteArray_set_position(struct ByteArray* self, off_t pos);
int ByteArray_set_position_rel(struct ByteArray* self, int rel);
int ByteArray_open_file(struct ByteArray* self, char* filename);
int ByteArray_open_mem(struct ByteArray* self, char* data, size_t size);
void ByteArray_readMultiByte(struct ByteArray* self, char* buffer, size_t len);
off_t ByteArray_bytesAvailable(struct ByteArray* self);
unsigned int ByteArray_readUnsignedInt(struct ByteArray* self);
int ByteArray_readInt(struct ByteArray* self);
unsigned short ByteArray_readUnsignedShort(struct ByteArray* self);
short ByteArray_readShort(struct ByteArray* self);
unsigned char ByteArray_readUnsignedByte(struct ByteArray* self);
signed char ByteArray_readByte(struct ByteArray* self);



#endif
