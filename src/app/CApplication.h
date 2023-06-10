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

#ifndef EXEIN_C_APPLICATION_H
#define EXEIN_C_APPLICATION_H

#pragma once

class CApplication
{
public:
	static auto& get()
	{
		static CApplication app;
		return app;
	}

public:
	bool run();

	// Pauses the program
	void halt();

	inline void get_app_dims(int32_t* width, int32_t* height)
	{
		*width = m_app_width;
		*height = m_app_height;
	}

	inline double get_frametime() const
	{
		return std::chrono::duration<double>(frame_end - frame_start).count();
	}

	inline double get_fps() const
	{
		return 1.0 / get_frametime();
	}

private:
	bool on_frame();

	void glfw_update();

	bool invoke_window();
	void destroy_window();

	void update_app_resolution();

	void update_app_title();

	void compute_avg_fps();

	void create_new_active_processor(const std::filesystem::path& filepath);
	void close_active_processor();

	// Return false if the application has to be closed
	bool is_alive();

	void render_ui_contents();

	inline bool is_processor_active() const { return m_active_processor != nullptr; }

	// Always call is_processor_active before this one!
	inline const auto& get_input_file() const { return m_active_processor->get_input_file(); }

private:
	std::string m_app_title;
	int32_t		m_app_width, m_app_height;

	GLFWwindow* m_glfw_window;

	// For frametime & fps calculations
	std::chrono::high_resolution_clock::time_point frame_start, frame_end;
	double m_avg_fps = 1.0; // Uses moving-average algorithm

	IBaseProcessor* m_active_processor;

	bool m_failed_to_open_file = false;
};

#endif