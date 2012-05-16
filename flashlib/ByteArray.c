#include "ByteArray.h"
#include "../neoart/flod/flod_internal.h"
#include "endianness.h"
#include <assert.h>

void ByteArray_defaults(struct ByteArray* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void ByteArray_ctor(struct ByteArray* self) {
	CLASS_CTOR_DEF(ByteArray);
	// original constructor code goes here
	self->readMultiByte = ByteArray_readMultiByte;
	self->readByte = ByteArray_readByte;
	self->readUnsignedByte = ByteArray_readUnsignedByte;
	self->readShort = ByteArray_readShort;
	self->readUnsignedShort = ByteArray_readUnsignedShort;
	self->readInt = ByteArray_readInt;
	self->readUnsignedInt = ByteArray_readUnsignedInt;
	
	self->writeInt = ByteArray_writeInt;
	self->writeUnsignedInt = ByteArray_writeUnsignedInt;
	self->writeShort = ByteArray_writeShort;
	self->writeUnsignedShort = ByteArray_writeUnsignedShort;
	self->writeByte = ByteArray_writeByte;
	self->writeUnsignedByte = ByteArray_writeUnsignedByte;;
	self->writeMem = ByteArray_writeMem;
	self->writeUTFBytes = ByteArray_writeUTFBytes;
	self->writeBytes = ByteArray_writeBytes;
	self->writeFloat = ByteArray_writeFloat;
	
#ifdef IS_LITTLE_ENDIAN
	self->sys_endian = BAE_LITTLE;
#else
	self->sys_endian = BAE_BIG;
#endif
}

struct ByteArray* ByteArray_new(void) {
	CLASS_NEW_BODY(ByteArray);
}

void ByteArray_clear(struct ByteArray* self) {
	fprintf(stderr, "clear called");
}

off_t ByteArray_get_position(struct ByteArray* self) {
	return self->pos;
}

static void seek_error() {
	perror("seek error!\n");
	__asm__("int3");
}

static void neg_off() {
	fprintf(stderr, "negative seek attempted\n");
	__asm__("int3");
}

static void oob() {
	fprintf(stderr, "oob access attempted\n");
	__asm__("int3");
}

void ByteArray_set_length(struct ByteArray* self, off_t len) {
	if(len > self->size) {
		oob();
		return;
	}
	self->size = len;
}

off_t ByteArray_get_length(struct ByteArray* self) {
	return self->size;
}

int ByteArray_set_position_rel(struct ByteArray* self, int rel) {
	if((int) self->pos + rel < 0) {
		neg_off();
		rel = -self->pos;
	}
	return ByteArray_set_position(self, self->pos + rel);
}

int ByteArray_set_position(struct ByteArray* self, off_t pos) {
	if(pos == self->pos) return 1;
	if(pos > self->size) return 0;
	if(self->type == BAT_FILESTREAM) {
		off_t ret = lseek(self->fd, pos, SEEK_SET);
		if(ret == (off_t) -1) {
			seek_error();
			return 0;
		}
	}
	self->pos = pos;
	return 1;
}

static void read_error() {
	perror("read error!\n");
	__asm__("int3");
}

static void read_error_short() {
	perror("read error (short)!\n");
	__asm__("int3");
}

int ByteArray_open_file(struct ByteArray* self, char* filename) {
	struct stat st;
	self->type = BAT_FILESTREAM;
	self->pos = 0;
	self->size = 0;
	if(stat(filename, &st) == -1) return 0;
	self->size = st.st_size;
	self->fd = open(filename, O_RDONLY);
	return (self->fd != -1);
}

int ByteArray_open_mem(struct ByteArray* self, char* data, size_t size) {
	self->pos = 0;
	self->size = size;
	self->type = BAT_MEMSTREAM;
	self->start_addr = data;
	return 1;
}

void ByteArray_readMultiByte(struct ByteArray* self, char* buffer, size_t len) {
	if(self->type == BAT_MEMSTREAM) {
		assert(self->start_addr);
		assert(self->pos + len < self->size);
		memcpy(buffer, &self->start_addr[self->pos], len);
	} else {
		ssize_t ret = read(self->fd, buffer, len);
		if(ret == -1) {
			read_error();
			return;
		} else if(ret != len) {
			read_error_short();
			self->pos += len;
			return;
		}
	}
	self->pos += len;
}

off_t ByteArray_bytesAvailable(struct ByteArray* self) {
	if(self->pos < self->size) return self->size - self->pos;
	return 0;
}

unsigned int ByteArray_readUnsignedInt(struct ByteArray* self) {
	union {
		unsigned int intval;
		unsigned char charval[sizeof(unsigned int)];
	} buf;
	ByteArray_readMultiByte(self, (char*) buf.charval, 4);
	if(self->endian != self->sys_endian) {
		buf.intval = byteswap32(buf.intval);
	}
	return buf.intval;
}

int ByteArray_readInt(struct ByteArray* self) {
	union {
		unsigned int intval;
		unsigned char charval[sizeof(unsigned int)];
	} buf;
	ByteArray_readMultiByte(self, (char*) buf.charval, 4);
	if(self->endian != self->sys_endian) {
		buf.intval = byteswap32(buf.intval);
	}
	return buf.intval;
}

unsigned short ByteArray_readUnsignedShort(struct ByteArray* self) {
	union {
		unsigned short intval;
		unsigned char charval[sizeof(unsigned short)];
	} buf;
	ByteArray_readMultiByte(self, (char*) buf.charval, 2);
	if(self->endian != self->sys_endian) {
		buf.intval = byteswap16(buf.intval);
	}
	return buf.intval;
}

short ByteArray_readShort(struct ByteArray* self) {
	union {
		unsigned short intval;
		unsigned char charval[sizeof(unsigned short)];
	} buf;
	ByteArray_readMultiByte(self, (char*) buf.charval, 2);
	if(self->endian != self->sys_endian) {
		buf.intval = byteswap16(buf.intval);
	}
	return buf.intval;
}

unsigned char ByteArray_readUnsignedByte(struct ByteArray* self) {
	union {
		unsigned char intval;
	} buf;
	ByteArray_readMultiByte(self, (char*) &buf.intval, 1);
	return buf.intval;
}

signed char ByteArray_readByte(struct ByteArray* self) {
	union {
		signed char intval;
	} buf;
	ByteArray_readMultiByte(self, (char*) &buf.intval, 1);
	return buf.intval;
}

void ByteArray_writeByte(struct ByteArray* self, signed char what) {
	ByteArray_writeMem(self, (unsigned char*) &what, 1);
}

void ByteArray_writeUnsignedByte(struct ByteArray* self, unsigned char what) {
	ByteArray_writeMem(self, (unsigned char*) &what, 1);
}

void ByteArray_writeShort(struct ByteArray* self, signed short what) {
	union {
		short intval;
		unsigned char charval[sizeof(what)];
	} u;
	u.intval = what;
	if(self->sys_endian != self->endian) {
		u.intval = byteswap16(u.intval);
	}
	ByteArray_writeMem(self, u.charval, sizeof(what));
}

void ByteArray_writeUnsignedShort(struct ByteArray* self, unsigned short what) {
	union {
		unsigned short intval;
		unsigned char charval[sizeof(what)];
	} u;
	u.intval = what;
	if(self->sys_endian != self->endian) {
		u.intval = byteswap16(u.intval);
	}
	ByteArray_writeMem(self, u.charval, sizeof(what));
}

void ByteArray_writeInt(struct ByteArray* self, signed int what) {
	union {
		int intval;
		unsigned char charval[sizeof(what)];
	} u;
	u.intval = what;
	if(self->sys_endian != self->endian) {
		u.intval = byteswap32(u.intval);
	}
	ByteArray_writeMem(self, u.charval, sizeof(what));
}

void ByteArray_writeUnsignedInt(struct ByteArray* self, unsigned int what) {
	union {
		unsigned int intval;
		unsigned char charval[sizeof(what)];
	} u;
	u.intval = what;
	if(self->sys_endian != self->endian) {
		u.intval = byteswap32(u.intval);
	}
	ByteArray_writeMem(self, u.charval, sizeof(what));
}

void ByteArray_writeMem(struct ByteArray* self, unsigned char* what, size_t len) {
	if(self->type == BAT_FILESTREAM) {
		fprintf(stderr, "tried to write to file!\n");
		__asm__("int3");
		return;
	}
	if(self->pos + len > self->size) {
		fprintf(stderr, "oob write attempted");
		__asm__("int3");
		return;
	}
	assert(self->start_addr);

	memcpy(&self->start_addr[self->pos], what, len);
	self->pos += len;
}

void ByteArray_writeUTFBytes(struct ByteArray* self, char* what) {
	ByteArray_writeMem(self, (unsigned char*) what, strlen(what));
}

void ByteArray_writeBytes(struct ByteArray* self, struct ByteArray* what) {
	if(what->type == BAT_FILESTREAM) {
		fprintf(stderr, "tried to write from non-memory stream\n");
		__asm__("int3");
		abort();
	} else {
		ByteArray_writeMem(self, (unsigned char*) &what->start_addr[what->pos], what->size - what->pos);
	}
}

void ByteArray_writeFloat(struct ByteArray* self, float what) {
	union {
		float floatval;
		unsigned int intval;
		unsigned char charval[sizeof(what)];
	} u;
	u.floatval = what;
	if(self->sys_endian != self->endian) {
		u.intval = byteswap32(u.intval);
	}
	ByteArray_writeMem(self, u.charval, sizeof(what));
}

void ByteArray_dump_to_file(struct ByteArray* self, char* filename) {
	assert(self->type == BAT_MEMSTREAM);
	int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 666);
	write(fd, self->start_addr, self->size);
	close(fd);
}


