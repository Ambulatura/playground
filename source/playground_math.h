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

#define v2(...) v2 { __VA_ARGS__ }

#define PLAYGROUND_MATH_H
#endif
