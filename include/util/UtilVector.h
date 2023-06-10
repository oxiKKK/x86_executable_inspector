/*
*	x86executable_inspector developed by oxiKKK
*	Copyright (c) 2023
*
*	This program is licensed under the MIT license. By downloading, copying,
*	installing or using this software you agree to this license.
*
*	License Agreement
*
*	Permission is hereby granted, free of charge, to any person obtaining a
*	copy of this software and associated documentation files (the "Software"),
*	to deal in the Software without restriction, including without limitation
*	the rights to use, copy, modify, merge, publish, distribute, sublicense,
*	and/or sell copies of the Software, and to permit persons to whom the
*	Software is furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included
*	in all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
*	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
*	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
*	IN THE SOFTWARE.
*/

#ifndef EXEIN_UTIL_VECTOR_H
#define EXEIN_UTIL_VECTOR_H

#pragma once

class Vec2D
{
public:
	constexpr Vec2D() noexcept :
		x(0.0),
		y(0.0)
	{
	}

	constexpr ~Vec2D() noexcept
	{
	}

	constexpr Vec2D(float X, float Y) noexcept :
		x(X),
		y(Y)
	{}

	constexpr Vec2D(float p[2]) noexcept
	{
		if (p)
		{
			x = p[0];
			y = p[1];
		}
		else
		{
			x = 0.0;
			y = 0.0;
		}
	};

	constexpr Vec2D(const Vec2D& in) noexcept :
		x(in.x),
		y(in.y)
	{
	}

	constexpr Vec2D(Vec2D& in) noexcept :
		x(in.x),
		y(in.y)
	{
	}

	constexpr Vec2D(Vec2D&& in) noexcept :
		x(in.x),
		y(in.y)
	{}

public:
	//-----------------------------------------------------------------------------
	// 
	// Conversion operators
	// 
	//-----------------------------------------------------------------------------

	constexpr inline operator float*() noexcept
	{
		return &x;
	}

	constexpr inline operator const float*() const noexcept
	{
		return &x;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator=
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator=(const Vec2D& other) noexcept
	{
		x = other.x;
		y = other.y;

		return *this;
	}

	constexpr inline auto& operator=(float p[2]) noexcept
	{
		if (p)
		{
			x = p[0];
			y = p[1];
		}
		else
		{
			x = y = 0.0;
		}

		return *this;
	}

	constexpr inline auto& operator=(float f) noexcept
	{
		x = y = f;

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator+=
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator+=(const Vec2D& other) noexcept
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	constexpr inline auto& operator+=(float p[2]) noexcept
	{
		if (p)
		{
			x += p[0];
			y += p[1];
		}

		return *this;
	}

	constexpr inline auto& operator+=(float f) noexcept
	{
		x += f;
		y += f;

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator-=
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator-=(const Vec2D& other) noexcept
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	constexpr inline auto& operator-=(float p[2]) noexcept
	{
		if (p)
		{
			x -= p[0];
			y -= p[1];
		}

		return *this;
	}

	constexpr inline auto& operator-=(float f) noexcept
	{
		x -= f;
		y -= f;

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator*=
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator*=(const Vec2D& other) noexcept
	{
		x *= other.x;
		y *= other.y;

		return *this;
	}

	constexpr inline auto& operator*=(float p[2]) noexcept
	{
		if (p)
		{
			x *= p[0];
			y *= p[1];
		}

		return *this;
	}

	constexpr inline auto& operator*=(float f) noexcept
	{
		x *= f;
		y *= f;

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator/=
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator/=(const Vec2D& other) noexcept
	{
		if (other.x != 0.0 && other.y != 0.0)
		{
			x /= other.x;
			y /= other.y;
		}

		return *this;
	}

	constexpr inline auto& operator/=(float p[2]) noexcept
	{
		if (p)
		{
			if (p[0] != 0.0 && p[1] != 0.0)
			{
				x /= p[0];
				y /= p[1];
			}
		}

		return *this;
	}

	constexpr inline auto& operator/=(float f) noexcept
	{
		if (f != 0.0)
		{
			x /= f;
			y /= f;
		}

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator+
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto operator+(const Vec2D& other) const noexcept
	{
		return Vec2D(x + other.x, y + other.y);
	}

	constexpr inline auto operator+(float p[2]) const noexcept
	{
		if (p)
			return Vec2D(x + p[0], y + p[1]);

		return *this;
	}

	constexpr inline auto operator+(float f) const noexcept
	{
		return Vec2D(x + f, y + f);
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator-
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto operator-(const Vec2D& other) const noexcept
	{
		return Vec2D(x - other.x, y - other.y);
	}

	constexpr inline auto operator-(float p[2]) const noexcept
	{
		if (p)
			return Vec2D(x - p[0], y - p[1]);

		return *this;
	}

	constexpr inline auto operator-(float f) const noexcept
	{
		return Vec2D(x - f, y - f);
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator- (negation)
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto operator-() const noexcept
	{
		return Vec2D(-x, -y);
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator*
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto operator*(const Vec2D& other) const noexcept
	{
		return Vec2D(x * other.x, y * other.y);
	}

	constexpr inline auto operator*(float p[2]) const noexcept
	{
		if (p)
			return Vec2D(x * p[0], y * p[1]);

		return *this;
	}

	constexpr inline auto operator*(float f) const noexcept
	{
		return Vec2D(x * f, y * f);
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator/
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto operator/(const Vec2D& other) const noexcept
	{
		if (other.x != 0.0 && other.y != 0.0)
			return Vec2D(x / other.x, y / other.y);

		return *this;
	}

	constexpr inline auto operator/(float p[2]) const noexcept
	{
		if (p)
		{
			if (p[0] != 0.0 && p[1] != 0.0)
				return Vec2D(x / p[0], y / p[1]);
		}

		return *this;
	}

	constexpr inline auto operator/(float f) const noexcept
	{
		if (f != 0.0)
			return Vec2D(x / f, y / f);

		return *this;
	}

	//-----------------------------------------------------------------------------
	// 
	// Operator[]
	// 
	//-----------------------------------------------------------------------------

	constexpr inline auto& operator[](int i) const noexcept
	{
		if (i >= 0 && i < 2)
			return ((float*)this)[i];

		return ((float*)this)[0];
	}

	//-----------------------------------------------------------------------------
	// 
	// Boolean operators
	// 
	//-----------------------------------------------------------------------------

	constexpr inline bool operator!() const noexcept
	{
		return x == 0.0 && y == 0.0;
	}

	constexpr inline bool operator==(const Vec2D& other) const noexcept
	{
		return x == other.x && y == other.y;
	}

	constexpr inline bool operator!=(const Vec2D& other) const noexcept
	{
		return x != other.x || y != other.y;
	}

	constexpr inline bool operator<(const Vec2D& other) const noexcept
	{
		return x < other.x && y < other.y;
	}

	constexpr inline bool operator>(const Vec2D& other) const noexcept
	{
		return x > other.x && y > other.y;
	}

public:
	float x, y;
};



#endif