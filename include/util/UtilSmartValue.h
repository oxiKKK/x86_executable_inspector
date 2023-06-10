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

#ifndef EXEIN_SMART_VALUE_H
#define EXEIN_SMART_VALUE_H

#pragma once

//
// Smart value declaration
//
//	Wrapper around type T that contains helper methods while 
//	sizeof(SmartValue<T>) == sizeof(T).
//
//	Note: Don't use this raw class, use pre-defined classes below.
//
template<typename T> requires(std::is_integral<T>::value)
class SmartValue
{
public:
	constexpr SmartValue(T val) :
		m_value(val)
	{}
	constexpr SmartValue() :
		m_value((T)0)
	{}

public:
	// These operators has to be overrided by the class that derives this.
	virtual inline constexpr operator T() const { return m_value; };
	virtual inline constexpr bool operator=(T val) { return m_value = val; };

	// Returns a string depending on the overrided as_string() function.
	inline constexpr operator std::string() const { return as_string(); }

public:
	inline constexpr T val() const { return m_value; }

	template<typename U>
	inline constexpr U as() const { return static_cast<U>(m_value); }

	inline constexpr uint8_t as_u8() const { return as<uint8_t>(); }
	inline constexpr uint16_t as_u16() const { return as<uint16_t>(); }
	inline constexpr uint32_t as_u32() const { return as<uint32_t>(); }
	inline constexpr uint64_t as_u64() const { return as<uint64_t>(); }

	inline constexpr int8_t as_i8() const { return as<int8_t>(); }
	inline constexpr int16_t as_i16() const { return as<int16_t>(); }
	inline constexpr int32_t as_i32() const { return as<int32_t>(); }
	inline constexpr int64_t as_i64() const { return as<int64_t>(); }

	// True if value is aligned to 'to'.
	inline constexpr bool is_aligned(T to) const { return (m_value % to) == 0; }

	// True if value is power of 2
	inline constexpr bool is_power_of_two() const { return m_value != 0 && !(m_value & (m_value - 1)); }

public:
	// Override this function to output custom-formatted values in a form of std::string
	// when this object is implicitly casted into std::string.
	virtual std::string as_string() const { return m_value ? std::to_string(m_value) : "null"; }

protected:
	T m_value;
};

//
// Hexadecimal smart value declaration
//

template<typename T> requires(std::is_integral<T>::value)
class SmartHexValue : public SmartValue<T>
{
private:
	using SmartValue<T>::m_value;

public:
	// These has to be declared in order for the compiler to figure stuff out.
	constexpr SmartHexValue(T val) : SmartValue<T>(val) {}
	constexpr SmartHexValue() : SmartValue<T>() {}

	inline constexpr operator T() const override { return m_value; }
	inline constexpr bool operator=(T val) { return m_value = val; }

public:
	inline std::string as_string() const override { return m_value ? std::format("0x{:0>{}X}", m_value, sizeof(T) * 2) : "null"; }
};

//
// Decimal smart value declaration
//

template<typename T> requires(std::is_integral<T>::value)
class SmartDecValue : public SmartValue<T>
{
private:
	using SmartValue<T>::m_value;

public:
	// These has to be declared in order for the compiler to figure stuff out.
	constexpr SmartDecValue(T val) : SmartValue<T>(val) {}
	constexpr SmartDecValue() : SmartValue<T>() {}

	inline constexpr operator T() const override { return m_value; }
	inline constexpr bool operator=(T val) { return m_value = val; }

public:
	inline std::string as_string() const override { return m_value ? std::to_string(m_value) : "null"; }
};

//
// Binary smart value declaration
//

template<typename T> requires(std::is_integral<T>::value)
class SmartBinValue : public SmartValue<T>
{
private:
	using SmartValue<T>::m_value;

public:
	// These has to be declared in order for the compiler to figure stuff out.
	constexpr SmartBinValue(T val) : SmartValue<T>(val) {}
	constexpr SmartBinValue() : SmartValue<T>() {}

	inline constexpr operator T() const override { return m_value; }
	inline constexpr bool operator=(T val) { return m_value = val; }

public:
	inline std::string as_string() const override { return m_value ? std::format("0b{:b}", m_value) : "null"; }
};

#endif