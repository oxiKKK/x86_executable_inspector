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

#ifndef EXEIN_C_DEBUG_CONSOLE_H
#define EXEIN_C_DEBUG_CONSOLE_H

#pragma once

class CWinBaseConsole
{
public:
	enum class Type
	{
		Message, 
		Error, 
		Info, 
		NonPrint, 
	};

	enum class Color
	{
		White,	// Used as default color
		Red, 
		Yellow, 
	};

protected:
	virtual bool create(int32_t max_lines);
	virtual void destroy();

	// Use std::format with python-line replacement fields
	virtual void output(const std::string& fmt, Type type, bool fancy_timestamp, bool newline);

	void change_title(const std::string& title);

	// Passing NULL will have no effect on the specific coordinate (X or Y)
	void resize_con_buffer(int32_t x, int32_t y);

private:
	bool redirect_con_io();
	void change_color(Color color);

	inline void push_color(Color color)
	{
		change_color(color);
	}

	inline void pop_color()
	{
		change_color(Color::White);
	}

	// White if invalid type is specified
	inline Color color_by_type(Type type)
	{
		switch (type)
		{
			case Type::Message:
				return Color::White;
				break;

			case Type::Error:
				return Color::Red;
				break;

			case Type::Info:
				return Color::Yellow;
				break;

			default:
				return Color::White;
				break;
		}
	}

private:
	// Translation from our color table into winapi.
	static uint16_t s_color_translation[];

	int32_t m_max_lines;

	std::mutex m_output_lock;

protected:
	uint32_t m_total_lines;
};

class CDebugConsole : CWinBaseConsole
{
public:
	static auto& get()
	{
		static CDebugConsole console;
		return console;
	}

public:
	bool create(int32_t max_lines);
	void destroy();

	// Ran every # ms (constant in function)
	void update();

	void output_message(const std::string& fmt, bool newline = true);
	void output_error(const std::string& fmt, bool newline = true);
	void output_info(const std::string& fmt, bool newline = true);
	void output_newline();

	inline void push_no_timestamp() { m_no_fancy_timestamp = true; }
	inline void pop_no_timestamp() { m_no_fancy_timestamp = false; }

private:
	bool m_no_fancy_timestamp = false;
};

#endif