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

#ifndef EXEIN_BASE_PROCESSOR_H
#define EXEIN_BASE_PROCESSOR_H

#pragma once

// A base class that every single processor for specific file 
// type has to derive from.
class IBaseProcessor
{
public:
	// Digs out as much information as possible from the binary stream
	virtual bool process(const std::filesystem::path& filepath) { return false; }
	
	// Render contents specific to information we gained from this file.
	virtual void render_gui() {}

	// To tell outside code whenether we've finished or not
	inline bool finished_processing() const { return m_processed; }

	inline const auto& get_input_file() const { return m_input_file; }

protected:
	// Has to be called every time we start parsing!
	// Creates the file buffer for reading.
	bool on_processing_start(const std::filesystem::path& filepath)
	{
		m_input_file = filepath;

		CDebugConsole::get().output_message(std::format("Input file: {}", m_input_file.string()));

		if (!check_file_permissions())
		{
			CDebugConsole::get().output_error("Not enough permissions to process the file.");
			return false;
		}

		m_start_timestamp = std::chrono::high_resolution_clock::now();
		uint32_t filesize;

		// This should be the first call to filesystem. We'll catch errors here if any.
		try
		{
			filesize = std::filesystem::file_size(m_input_file);
		}
		catch (std::filesystem::filesystem_error& err)
		{
			CDebugConsole::get().output_error(err.what());
			return false;
		}

		m_byte_buffer.create(m_input_file, filesize);

		CDebugConsole::get().output_message("Created file data to read");

		return true;
	}

	// Has to be called when the processor has finished parsing
	void on_processing_end()
	{
		m_processed = true;

		m_end_timestamp = std::chrono::high_resolution_clock::now();

		m_byte_buffer.destroy();

		CDebugConsole::get().output_message(std::format("Took {} seconds to process", get_process_time_sec()));
	}

	// Return processing time in seconds since the processing has started
	inline double get_process_time_sec() const
	{
		return std::chrono::duration<double>(m_end_timestamp - m_start_timestamp).count();
	}

private:
	// Some system files and other cannot be processed. Verify if we can
	// process this one.
	bool check_file_permissions()
	{
		auto perms = std::filesystem::status(m_input_file).permissions();

		// Check if we have read permissions
		if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none)
		{
			CDebugConsole::get().output_error("Insufficient amount of permissions to read from the file.");
			return false;
		}

		// Check for unknown permissions. Some highly protected files like the system kernel
		// have this type of permissions.
		//if ((perms & std::filesystem::perms::unknown) != std::filesystem::perms::none)
		//{
		//	CDebugConsole::get().output_error("The file is highly protected and cannot be processed.");
		//	return false;
		//}

		CDebugConsole::get().output_message("Enough permission to process the file.");

		return true;
	}

protected:
	ByteBuffer<uint32_t> m_byte_buffer;

	bool m_processed = false;

	using TimePoint_t = std::chrono::high_resolution_clock::time_point;
	TimePoint_t	m_start_timestamp, m_end_timestamp;

	std::filesystem::path m_input_file;
};

#endif