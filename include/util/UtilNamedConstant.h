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

#ifndef EXEIN_NAMED_CONSTANT_H
#define EXEIN_NAMED_CONSTANT_H

#pragma once

// Class that allows to hold a constant value and a name associated to that value.
// This can be useful when printing named constans instead of actual values.
template<typename T>
class NamedConstant
{
public:
	NamedConstant(const T& constant, const std::string& name) :
		m_named_constant(constant, name)
	{
	}

	NamedConstant() :
		m_named_constant(NULL, "null")
	{
	}

public:
	inline const auto& value() const { return m_named_constant.first; }
	inline const auto& name() const { return m_named_constant.second; }
	inline bool is(T val) const { return value() == val; }

private:
	std::pair<T, std::string> m_named_constant;
};

// Used with values that are used to store flags as individual bits. Allows to
// add a simple description to each bit when constructing.
template<typename T> requires(sizeof(T) <= sizeof(uint64_t))
class NamedBitfieldConstant
{
public:
	// Initialize the bit field and string vector with values. Each bit has to 
	// have a string associated to it!
	NamedBitfieldConstant(T value, const std::vector<std::pair<T, std::string>>&& v_strings)
	{
		if (v_strings.size() > get_size_bits())
		{
			CDebugConsole::get().output_error("FATAL ERROR! Invalid arguments passed to " __FUNCTION__ "!");
			return;
		}

		// Set all bits individually depending on the value passed
		for (uint32_t i = 0; i < get_size_bits(); i++)
		{
			m_bit_field.set(i, value & (1 << i));

			// Search if we have a description for this bit
			const auto& iter = std::find_if(v_strings.begin(), v_strings.end(), [&](const std::pair<T, std::string>& pair)
				{
					const auto& [key, string] = pair;
					return key == (1 << i) && !string.empty();
				});

			// If we have, apply that description. Otherwise, apply the default one.
			if (iter != v_strings.end())
			{
				const auto& [key, string] = *iter;
				m_vec_strings.emplace_back(string);
			}
			else
			{
				m_vec_strings.emplace_back(i != 0 ? std::format("Reserved 0x{:X}", (uint32_t)(1 << i)) : "Null");
			}
		}
	}

	NamedBitfieldConstant() :
		m_bit_field(0),
		m_vec_strings({})
	{
	}

public:
	// Returns true if #th bit is present
	inline bool is_bit_present(uint32_t bit) const
	{
		return (m_bit_field.test(bit) == 1) ? true : false;
	}

	// Returns true if all bits inside this radius are present
	inline bool are_all_bits_present(uint32_t lo, uint32_t hi) const
	{
		if (lo > get_size_bits() || hi > get_size_bits())
		{
			CDebugConsole::get().output_error(std::format(__FUNCTION__ ": Bad parameters (lo={}, hi={})", lo, hi));
			return false;
		}

		for (uint32_t i = lo; i < hi; i++)
		{
			if (!is_bit_present(i))
				return false;
		}

		return true; // all bits were present
	}

	// Returns true if any bits inside this radius are present
	inline bool are_any_bits_present(uint32_t lo, uint32_t hi) const
	{
		if (lo > get_size_bits() || hi > get_size_bits())
		{
			CDebugConsole::get().output_error(std::format(__FUNCTION__ ": Bad parameters (lo={}, hi={})", lo, hi));
			return false;
		}

		for (uint32_t i = lo; i < hi; i++)
		{
			if (is_bit_present(i))
				return true;
		}

		return false; // not a single bit was present
	}

	// Returns compile-time size in bytes/bits of the type
	static inline constexpr T get_size_bytes() { return sizeof(T); }
	static inline constexpr T get_size_bits() { return get_size_bytes() * 8; }

	inline const auto& get_string_vec() const { return m_vec_strings; }

	// Returns raw value containing all bits
	inline T get_val() const
	{
		T ret = 0;

		if constexpr (get_size_bytes() <= sizeof(uint32_t))
			return ret |= static_cast<T>(m_bit_field.to_ulong());
		else if constexpr (get_size_bytes() >= sizeof(uint32_t))
			return ret |= static_cast<T>(m_bit_field.to_ullong());

		// This theoretically shouldn't ever happen
		static_assert("Bad type size");
		return 0;
	}

	// Returns the string associated to specified bit
	inline const std::string& get_string_at(T idx) const
	{
		try
		{
			return m_vec_strings.at(idx);
		}
		catch (...)
		{
			CDebugConsole::get().output_error(std::format("Tried to get string with invalid index: {}", idx));
			return m_vec_strings.at(0);
		}
	}

	// Returns an contiunous string containging strings of all set only bits.
	inline std::string as_continuous_string(const std::string& separator = "; ") const
	{
		std::string ret;

		for (uint32_t i = 1; i < m_vec_strings.size(); i++)
		{
			if (!is_bit_present(i))
				continue;

			ret += m_vec_strings[i] + separator;
		}

		if (!ret.empty())
		{
			for (uint32_t i = 0; i < separator.length(); i++)
				ret.pop_back();
		}

		return ret;
	}

private:
	std::bitset<get_size_bits()> m_bit_field;

	// Each string is for individual bits. Ranging from bit 0 to sizeof(T).
	std::vector<std::string> m_vec_strings;
};

#endif