#ifndef PLAYGROUND_MATH_H

union v2
{
	f32 elements[2];

	struct
	{
		f32 x;
		f32 y;
	};
};

union v3
{
	f32 elements[3];

	struct
	{
		f32 x;
		f32 y;
		f32 z;
	};
};

union v4
{
	f32 elements[4];

	struct
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	struct
	{
		f32 r;
		f32 g;
		f32 b;
		f32 a;
	};
};

inline f32 Square(f32 value)
{
	f32 result = value * value;
	
	return result;
}

inline f32 SafeDivideN(f32 numerator, f32 divisor, f32 value)
{
	f32 result = value;

	if (divisor != 0.0f) {
		result = numerator / divisor;
	}

	return result;
}

inline f32 SafeDivide0(f32 numerator, f32 divisor)
{
	f32 result = SafeDivideN(numerator, divisor, 0.0f);
	
	return result;
}

inline f32 SafeDivide1(f32 numerator, f32 divisor)
{
	f32 result = SafeDivideN(numerator, divisor, 1.0f);
	
	return result;
}

inline u32 SafeModU32N(u32 numerator, u32 divisor, u32 value)
{
	u32 result = value;

	if (divisor != 0) {
		result = numerator % divisor;
	}

	return result;
}

inline u32 SafeModU320(u32 numerator, u32 divisor)
{
	u32 result = SafeModU32N(numerator, divisor, 0);
	
	return result;
}

inline v2 operator-(v2 v)
{
	v2 result;

	result.x = -v.x;
	result.y = -v.y;

	return result;
}

inline v2 operator+(v2 a, v2 b)
{
	v2 result;

	result.x = a.x + b.x;
	result.y = a.y + b.y;

	return result;
}

inline v2 operator-(v2 a, v2 b)
{
	v2 result;

	result.x = a.x - b.x;
	result.y = a.y - b.y;

	return result;
}

inline v2 operator*(v2 a, f32 b)
{
	v2 result;

	result.x = a.x * b;
	result.y = a.y * b;

	return result;
}

inline v2 operator*(f32 a, v2 b)
{
	v2 result = b * a;

	return result;
}

inline v2& operator+=(v2& a, v2 b)
{
	a = a + b;
	
	return a;
}

inline v2& operator-=(v2& a, v2 b)
{
	a = a - b;
	
	return a;
}

inline v2& operator*=(v2& a, f32 b)
{
	a = a * b;
	
	return a;
}

inline f32 Dot(v2 a, v2 b)
{
	f32 result = a.x * b.x + a.y * b.y;

	return result;
}

inline f32 LengthSquared(v2 v)
{
	f32 result = Dot(v, v);

	return result;
}

inline f32 Length(v2 v)
{
	f32 result = SquareRoot(LengthSquared(v));

	return result;
}

inline v2 HadamardProduct(v2 a, v2 b)
{
	v2 result = {
		a.x * b.x,
		a.y * b.y
	};

	return result;
}

// NOTE(SSJSR):  Rectangle2

struct Rectangle2
{
	v2 min;
	v2 max;
};

inline Rectangle2 Rectangle2MinMax(v2 min, v2 max)
{
	Rectangle2 result;
	
	result.min = min;
	result.min = max;

	return result;
}

inline Rectangle2 Rectangle2CenterHalfDimension(v2 center, v2 half_dimension)
{
	Rectangle2 result;
	
	result.min = center - half_dimension;
	result.max = center + half_dimension;

	return result;
}

inline Rectangle2 Rectangle2CenterDimension(v2 center, v2 dimension)
{
	Rectangle2 result = Rectangle2CenterHalfDimension(center, dimension * 0.5f);

	return result;
}

inline b32 IsInRectangle2(Rectangle2 rect2, v2 point)
{
	b32 result = ((rect2.min.x <= point.x) &&
				  (rect2.min.y <= point.y) &&
				  (rect2.max.x > point.x) &&
				  (rect2.max.y > point.y));

	return result;
}

inline Rectangle2 AddDimensionTo(Rectangle2 rect2, v2 dimension)
{
	Rectangle2 result;
	result.min = rect2.min - dimension;
	result.max = rect2.max + dimension;

	return result;
}

inline Rectangle2 SubtractDimensionTo(Rectangle2 rect2, v2 dimension)
{
	Rectangle2 result;
	result.min = rect2.min + dimension;
	result.max = rect2.max - dimension;

	return result;
}

#define v2(...) v2 { __VA_ARGS__ }
#define v3(...) v3 { __VA_ARGS__ }
#define v4(...) v4 { __VA_ARGS__ }

#define PLAYGROUND_MATH_H
#endif
