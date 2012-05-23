#include <stdint.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>

#include "wavewriter.h"
#include "../flashlib/endianness.h"

typedef union {
	uint8_t chars[2];
	uint16_t shrt;
} u_char2;

//all values are LITTLE endian unless otherwise specified...
typedef struct {
	uint8_t text_RIFF[4];
	/*
	ChunkSize:  36 + SubChunk2Size, or more precisely:
	4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
	This is the size of the rest of the chunk 
	following this number.  This is the size of the 
	entire file in bytes minus 8 bytes for the
	two fields not included in this count:
	ChunkID and ChunkSize.	 * */
	uint32_t filesize_minus_8;
	uint8_t text_WAVE[4];
} RIFF_HEADER;

typedef struct {
	uint8_t text_fmt[4];
	uint32_t formatheadersize; /* Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number. */
	u_char2 format; /* big endian, PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some 
                               form of compression. */
	uint16_t channels; //Mono = 1, Stereo = 2, etc
	uint32_t samplerate;
	uint32_t bytespersec; // ByteRate == SampleRate * NumChannels * BitsPerSample/8
	uint16_t blockalign; /* == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. */
	uint16_t bitwidth; //BitsPerSample    8 bits = 8, 16 bits = 16, etc.
} WAVE_HEADER;

typedef struct {
	uint8_t text_data[4]; //Contains the letters "data"
	uint32_t data_size; /* == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this 
                               number.*/
} RIFF_SUBCHUNK2_HEADER;

typedef struct {
	RIFF_HEADER riff_hdr;
	WAVE_HEADER wave_hdr;
	RIFF_SUBCHUNK2_HEADER sub2;
} WAVE_HEADER_COMPLETE;

static const WAVE_HEADER_COMPLETE wave_hdr = {
	{
		{ 'R', 'I', 'F', 'F'},
		0,
		{ 'W', 'A', 'V', 'E'},
	},
	{
		{ 'f', 'm', 't', ' '},
		16,
		{1, 0},
		0,
		44100,
		0,
		0,
		16,
	},
	{
		{ 'd', 'a', 't', 'a' },
		0
	},
};

static int do_write(struct WaveWriter *self, void* buffer, size_t bufsize) {
	if(self->fd == -1) return 0;
	if(write(self->fd, buffer, bufsize) != (ssize_t) bufsize) {
		close(self->fd);
		self->fd = -1;
		return 0;
	}
	return 1;
}

int WaveWriter_init(struct WaveWriter *self, char* filename) {
	self->fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0660);
	return do_write(self, (void*) &wave_hdr, sizeof(wave_hdr));
}

int WaveWriter_write(struct WaveWriter *self, void* buffer, size_t bufsize) {
	int success = 0;
	if(do_write(self, buffer, bufsize)) {
		self->written += bufsize;
		success = 1;
	}
	return success;
}

#ifdef IS_LITTLE_ENDIAN
#define le32(X) (X)
#define le16(X) (X)
#else
#define le32(X) byteswap32(X)
#define le16(X) byteswap16(X)
#endif

int WaveWriter_close(struct WaveWriter *self) {
	const int channels = 2;
	const int samplerate = 44100;
	const int bitwidth = 16;
	
	WAVE_HEADER_COMPLETE hdr = wave_hdr;
	size_t size = self->written;
	
	if(self->fd == -1) return 0;
	if(lseek(self->fd, 0, SEEK_SET) == -1) {
		close(self->fd);
		return 0;
	}
	
	hdr.wave_hdr.channels = le16(channels);
	hdr.wave_hdr.samplerate = le32(samplerate);
	hdr.wave_hdr.bitwidth = le16(bitwidth);
	
	hdr.wave_hdr.blockalign = le16(channels * (bitwidth / 8));
	hdr.wave_hdr.bytespersec = le32(samplerate * channels * (bitwidth / 8));
	hdr.riff_hdr.filesize_minus_8 = le32(sizeof(WAVE_HEADER_COMPLETE) + size - 8);
	hdr.sub2.data_size = le32(size);
	
	if(!do_write(self, &hdr, sizeof(hdr))) return 0;
	
	close(self->fd);
	self->fd = -1;
	return 1;
}
