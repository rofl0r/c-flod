#include "ByteArray.h"
#include "../neoart/flod/flod_internal.h"
#include "endianness.h"

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
#ifdef IS_LITTLE_ENDIAN
	self->sys_endian = BAE_LITTLE;
#else
	self->sys_endian = BAE_BIG;
#endif
}

struct ByteArray* ByteArray_new(void) {
	CLASS_NEW_BODY(ByteArray);
}

off_t ByteArray_get_position(struct ByteArray* self) {
	return self->pos;
}

static int seek_error() {
	perror("seek error!\n");
}

static int neg_off() {
	fprintf(stderr, "negative seek attempted");
}

int ByteArray_set_position_rel(struct ByteArray* self, int rel) {
	if((int) self->pos - rel < 0) {
		neg_off();
		rel = 0;
	}
	return ByteArray_set_position(self, self->pos - rel);
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

static int read_error() {
	perror("read error!\n");
}

static int read_error_short() {
	perror("read error (short)!\n");
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
	return 1;
}

void ByteArray_readMultiByte(struct ByteArray* self, char* buffer, size_t len) {
	if(self->type == BAT_FILESTREAM) {
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
	ByteArray_readMultiByte(self, buf.charval, 4);
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
	ByteArray_readMultiByte(self, buf.charval, 4);
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
	ByteArray_readMultiByte(self, buf.charval, 2);
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
	ByteArray_readMultiByte(self, buf.charval, 2);
	if(self->endian != self->sys_endian) {
		buf.intval = byteswap16(buf.intval);
	}
	return buf.intval;
}

unsigned char ByteArray_readUnsignedByte(struct ByteArray* self) {
	union {
		unsigned char intval;
	} buf;
	ByteArray_readMultiByte(self, &buf.intval, 1);
	return buf.intval;
}

signed char ByteArray_readByte(struct ByteArray* self) {
	union {
		signed char intval;
	} buf;
	ByteArray_readMultiByte(self, &buf.intval, 1);
	return buf.intval;
}
