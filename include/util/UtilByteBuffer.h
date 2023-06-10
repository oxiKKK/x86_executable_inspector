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

#ifndef EXEIN_BYTE_BUFFER
#define EXEIN_BYTE_BUFFER

#pragma once

// Helper class that allows us to better manage raw byte buffer
// The template parameter is the addressing type on such file.
template<typename A> requires(std::is_integral<A>::value)
class ByteBuffer
{
public:
	// Create data from filepath
	bool create(const std::filesystem::path& filepath, A size);

	// Create data from a buffer
	bool create(uint8_t* data, A size);

	void destroy();

private:
	// Allocates # number of bytes for the byte buffer
	bool allocate(A size);

	// Deallocates previously allocated data
	void deallocate();

	// Copy data from filepath
	bool copy_data(const std::filesystem::path& filepath);

	// Copy data from buffer
	bool copy_data(uint8_t* data);

public:
	inline uint8_t* const get_raw() const { return m_byte_buffer; }
	inline auto get_size() const { return m_size; }

	// Returns pointer to the address at specified offset
	template<class T>
	inline T* get_at(A off) const
	{
		if (!m_byte_buffer)
			return nullptr;

		if (off >= m_size)
		{
			CDebugConsole::get().output_error(std::format("Offset too large: 0x{:08X} (limit=0x{:08X}, exceeded=0x{:08X})", 
														  off, m_size, off - m_size));
			return nullptr;
		}

		return reinterpret_cast<T*>(m_byte_buffer + off);
	}

	// Converts all bytes into readable string form
	std::string to_string(A at, const std::string& delimiter = " ") const;

private:
	uint8_t* m_byte_buffer;
	A m_size;
};

template<typename A> requires(std::is_integral<A>::value)
inline bool ByteBuffer<A>::create(const std::filesystem::path& filepath, A size)
{
	if (!allocate(size))
		return false;

	if (!copy_data(filepath))
		return false;

	CDebugConsole::get().output_message(std::format("Created byte buffer from {} of size {} bytes", filepath.string(), size));

	return true;
}

template<typename A> requires(std::is_integral<A>::value)
inline bool ByteBuffer<A>::create(uint8_t* data, A size)
{
	if (!allocate(size))
		return false;

	if (!copy_data(data))
		return false;

	CDebugConsole::get().output_message(std::format("Created byte buffer from raw buffer of size {} bytes", size));

	return true;
}

template<typename A> requires(std::is_integral<A>::value)
inline void ByteBuffer<A>::destroy()
{
	deallocate();
}

template<typename A> requires(std::is_integral<A>::value)
inline bool ByteBuffer<A>::allocate(A size)
{
	m_size = size;

	m_byte_buffer = new uint8_t[m_size];

	if (!m_byte_buffer)
	{
		CDebugConsole::get().output_error("Byte buffer allocation failed");
		return false;
	}

	CDebugConsole::get().output_message("Allocated byte buffer");

	return true;
}

template<typename A> requires(std::is_integral<A>::value)
inline void ByteBuffer<A>::deallocate()
{
	if (!m_byte_buffer)
	{
		CDebugConsole::get().output_error("Tried to free nullptr byte buffer");
		return;
	}

	delete[] m_byte_buffer;
	m_byte_buffer = nullptr;

	CDebugConsole::get().output_message("Deallocated byte buffer");
}

template<typename A> requires(std::is_integral<A>::value)
inline bool ByteBuffer<A>::copy_data(const std::filesystem::path& filepath)
{
	std::ifstream ifs(filepath, std::ios_base::in | std::ios_base::binary);

	if (ifs.bad())
	{
		CDebugConsole::get().output_error("Failed to open the file");
		return false;
	}

	ifs.read(reinterpret_cast<char*>(m_byte_buffer), m_size);

	CDebugConsole::get().output_message(std::format("Read {} from file to the buffer",
													CUtil::make_filesize_nice(m_size)));

	ifs.close();

	return true;
}

template<typename A> requires(std::is_integral<A>::value)
inline bool ByteBuffer<A>::copy_data(uint8_t* data)
{
	if (!data)
	{
		CDebugConsole::get().output_error("Tried to fill data from nullptr data stream");
		return false;
	}

	memcpy(m_byte_buffer, data, m_size);

	CDebugConsole::get().output_message(std::format("Copied {} from raw data stream",
													CUtil::make_filesize_nice(m_size)));

	return true;
}

template<typename A> requires(std::is_integral<A>::value)
inline std::string ByteBuffer<A>::to_string(A at, const std::string& delimiter) const
{
	if (!m_size)
		return "null";

	if (at >= m_size - at)
	{
		CDebugConsole::get().output_error(std::format("Byte buffer: offset too big: val={} off={}", at, m_size - at));
		return "null";
	}

	std::string ret;

	for (A i = at; i < m_size; i++)
	{
		uint8_t byte = *get_at<uint8_t>(i);

		ret += std::format("{:02X}", byte);
		if (!delimiter.empty())
			ret += delimiter;
	}

	return ret;
}

#endif