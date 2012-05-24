#include <math.h>
#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define denormalA(X) ((float)((X) + 0x1.8p23f) - 0x1.8p23f)
#define denormalB(X) ((float)((X) + 0x1p-103  ) - 0x1p-103  )
#define denormalC(X) ((float)((X) + 1.e-18f  ) - 1.e-18f  )
#define denormalZ(X) (X)
#define denormal(X) denormalB(X)

union float_repr { float __f; __uint32_t __i; };
#define floatrepr(f) (((union __float_repr){ (float)(f) }).__i)


static int clip(int sample) {
	if(unlikely(sample > 32767)) return 32767;
	else if(unlikely(sample < -32767)) return -32768;
	return sample;
}

static inline int convert_sample16(Number sample) {
	int ret = lrintf(denormal(sample) * 32767.f);
	return clip(ret);
	//return clip(floatrepr((denormal(sample) * 32767.f) + 0x1.8p23f) & 0x3fffff );
	//
	
	/*
	Number mul;
	if (sample > 1.0) return 32767;
	if (sample < -1.0) return -32768;
	mul = 32767.f;
	if(sample < 0.0) mul = 32768.f;
	return sample * mul; */
}
