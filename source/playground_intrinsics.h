#ifndef PLAYGROUND_INTRINSICS_H

#include <math.h>

struct LeastSignificantSetBitScan
{
	u32 index;
	b32 found;
};

inline LeastSignificantSetBitScan FindLeastSignificantSetBit(u32 value)
{
	LeastSignificantSetBitScan result = {};

	for (u32 test_bit = 0; test_bit < 32; ++test_bit) {
		if (value & (1 << test_bit)) {
			result.index = test_bit;
			result.found = true;
			
			break;
		}
	}

	return result;
}

inline u32 BitsRotateLeft(u32 value, i32 rotate_amount)
{
	rotate_amount &= 31;
	return (value << rotate_amount) | (value >> (32 - rotate_amount));
}

inline u32 BitsRotateRight(u32 value, i32 rotate_amount)
{
	rotate_amount &= 31;
	return (value >> rotate_amount) | (value << (32 - rotate_amount));
}


inline i32 RoundF32ToI32(f32 value)
{
	i32 result = (i32)roundf(value);

	return result;
}

inline u32 RoundF32ToU32(f32 value)
{
	u32 result = (u32)roundf(value);

	return result;
}

inline u32 CeilF32ToU32(f32 value)
{
	u32 result = (u32)ceilf(value);

	return result;
}

inline f32 SquareRoot(f32 value)
{
	f32 result = sqrtf(value);

	return result;
}

inline f32 AbsoluteOf(f32 value)
{
	f32 result = fabsf(value);

	return result;
}

#define PLAYGROUND_INTRINSICS_H
#endif
