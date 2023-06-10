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

#include "exein_pch.h"

#define FANCY_TIMESTAMP "{}"

uint16_t CWinBaseConsole::s_color_translation[] =
{
	/*White   */FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	/*Red     */FOREGROUND_INTENSITY | FOREGROUND_RED,
	/*Yellow  */FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN,
	/*NonPrint*/FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
};

bool CWinBaseConsole::create(int32_t max_lines)
{
	AllocConsole();

	// So that we can use printf and other std features
	if (!redirect_con_io())
		return false;

	m_max_lines = max_lines;

	resize_con_buffer(NULL, m_max_lines);

	return true;
}

void CWinBaseConsole::destroy()
{
	FreeConsole();
}

void CWinBaseConsole::output(const std::string& fmt, Type type, bool fancy_timestamp, bool newline)
{
	m_output_lock.lock();

	auto clock_now = std::chrono::system_clock::now();

	time_t now = std::chrono::system_clock::to_time_t(clock_now);

	// Duration in ms
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(clock_now.time_since_epoch()).count();
	msec %= 1000;

	push_color(color_by_type(type));

	if (fancy_timestamp)
		std::cout << '[' << std::put_time(std::localtime(&now), "%T") << '.' << std::format("{:03}", msec) << ']' << ' ';

	std::cout << fmt.c_str();
	
	if (newline)
		std::cout << std::endl;

	// Reached the end
	if (++m_total_lines == m_max_lines)
		m_total_lines--;

	pop_color();

	m_output_lock.unlock();
}

void CWinBaseConsole::change_title(const std::string& title)
{
	SetConsoleTitleA(title.c_str());
}

void CWinBaseConsole::resize_con_buffer(int32_t x, int32_t y)
{
	// Set the screen buffer to be big enough to scroll some text
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	if (x != NULL)
		csbi.dwSize.X = x;

	if (y != NULL)
	{
		csbi.dwSize.Y = y;
		m_max_lines = y;
	}

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);
}

void CWinBaseConsole::change_color(Color color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), s_color_translation[(int)color]);
}

// https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
bool CWinBaseConsole::redirect_con_io()
{
	bool result = true;
	FILE* fp;

	// Redirect STDIN if the console has an input handle
	if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
		if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
			result = false;
		else
			setvbuf(stdin, NULL, _IONBF, 0);

	// Redirect STDOUT if the console has an output handle
	if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
		if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
			result = false;
		else
			setvbuf(stdout, NULL, _IONBF, 0);

	// Redirect STDERR if the console has an error handle
	if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
		if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
			result = false;
		else
			setvbuf(stderr, NULL, _IONBF, 0);

	// Make C++ standard streams point to console as well.
	std::ios::sync_with_stdio();

	// Clear the error state for each of the C++ standard streams.
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();

	return result;
}

bool CDebugConsole::create(int32_t total_lines)
{
	if (!CWinBaseConsole::create(total_lines))
		return false;

	output_message("Debug console initialized");

	return true;
}

void CDebugConsole::destroy()
{
	CWinBaseConsole::destroy();
}

void CDebugConsole::update()
{
	static auto t = std::chrono::system_clock::now();

	if (std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - t).count() < 250)
		return;

	static const char animation[] = { '|', '/', '-', '\\' };
	static int32_t animidx = 0;

	// Title animation
	change_title(std::format("{:c} Debug Console | {} lines", animation[animidx++], m_total_lines));

	if (animidx >= ARRAYSIZE(animation))
		animidx = 0;

	t = std::chrono::system_clock::now();
}

void CDebugConsole::output_message(const std::string& fmt, bool newline)
{
	output(fmt, CWinBaseConsole::Type::Message, !m_no_fancy_timestamp, newline);
}

void CDebugConsole::output_error(const std::string& fmt, bool newline)
{
	output(fmt, CWinBaseConsole::Type::Error, !m_no_fancy_timestamp, newline);
}

void CDebugConsole::output_info(const std::string& fmt, bool newline)
{
	output(fmt, CWinBaseConsole::Type::Info, !m_no_fancy_timestamp, newline);
}

void CDebugConsole::output_newline()
{
	output("", CWinBaseConsole::Type::NonPrint, !m_no_fancy_timestamp, true);
}
