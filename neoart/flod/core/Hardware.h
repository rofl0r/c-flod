#include <math.h>
#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif


static inline int clip(int sample) {
	if(unlikely(sample > 32767)) return 32767;
	else if(unlikely(sample < -32767)) return -32768;
	return sample;
}

static inline int convert_sample16(Number sample) {
	int ret = lrintf(sample * 32767.f);
	// clip(floatrepr(x+0x1.8p23f)&0x3fffff))
	return clip(ret);
	
	/*
	Number mul;
	if (sample > 1.0) return 32767;
	if (sample < -1.0) return -32768;
	mul = 32767.f;
	if(sample < 0.0) mul = 32768.f;
	return sample * mul; */
}
