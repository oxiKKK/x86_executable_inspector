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

bool CApplication::run()
{
	static const int32_t con_size = 1024;
	if (!CDebugConsole::get().create(con_size))
		return false;

	CDebugConsole::get().output_message(std::format("Created console with {} line limit", con_size));

	if (!invoke_window())
	{
		CDebugConsole::get().output_error("Failed to invoke application window");
		return false;
	}

	if (!CGUI::get().startup(m_glfw_window))
	{
		CDebugConsole::get().output_error("Failed to startup GUI");
		return false;
	}

	while (on_frame()) { }

	CGUI::get().shutdown();

	destroy_window();

	CDebugConsole::get().destroy();

	return true;
}

void CApplication::halt()
{
	CDebugConsole::get().output_message("Press any key to continue...");
	std::cin.get();
}

bool CApplication::on_frame()
{
	frame_start = std::chrono::high_resolution_clock::now();

	// User has requested a window close or the window just
	// has to be closed now.
	if (is_alive() == false)
		return false;

	CDebugConsole::get().update();

	// GUI renderer inside here
	glfw_update();

	frame_end = std::chrono::high_resolution_clock::now();

	compute_avg_fps();
	
	return true;
}

void CApplication::glfw_update()
{
	glfwPollEvents();

	// Update the dims if user resized the window
	update_app_resolution();

	update_app_title();

	CGUI::get().update(m_app_width, m_app_height, 
					   [&]()
					   {
						   render_ui_contents();
					   });
	
	glfwMakeContextCurrent(m_glfw_window);
	glfwSwapBuffers(m_glfw_window);
}

bool CApplication::invoke_window()
{
	if (!glfwInit())
	{
		CDebugConsole::get().output_error("Failed to initialize GLFW");
		return false;
	}

	m_app_title = "x86 Binary Inspector";
	m_app_width = 1280;
	m_app_height = 720;

	m_glfw_window = glfwCreateWindow(
		m_app_width, m_app_height,
		m_app_title.c_str(),
		nullptr, nullptr);

	if (!m_glfw_window)
	{
		CDebugConsole::get().output_error("Failed to create GFLW window");
		return false;
	}

	CDebugConsole::get().output_message(std::format("Created window [{}: {:d}x{:d}]", m_app_title, m_app_width, m_app_height));

	// Pair context to current thread
	glfwMakeContextCurrent(m_glfw_window);

	GLFWimage icon;
	icon.pixels = SOIL_load_image_from_memory(s_icon_png_file_bytes, sizeof(s_icon_png_file_bytes), &icon.width, &icon.height, NULL, SOIL_LOAD_RGBA);
	glfwSetWindowIcon(m_glfw_window, 1, &icon);

	// Enable vsync
	glfwSwapInterval(1);

	CDebugConsole::get().output_message("Window created");

	return true;
}

void CApplication::destroy_window()
{
	glfwDestroyWindow(m_glfw_window);
	glfwTerminate();

	m_glfw_window = nullptr;
}

void CApplication::update_app_resolution()
{
	glfwGetFramebufferSize(m_glfw_window, &m_app_width, &m_app_height);
}

void CApplication::update_app_title()
{
	std::string title_text = std::format("x86executable_inspector ({:.3} fps)", m_avg_fps);

	// Also display active file
	if (is_processor_active())
	{
		const auto& input_file = get_input_file();

		if (!input_file.empty())
		{
			title_text = std::format("x86executable_inspector | {} ({:.3} fps)", input_file.filename().string(), m_avg_fps);
		}
	}

	glfwSetWindowTitle(m_glfw_window, title_text.c_str());
}

// https://en.wikipedia.org/wiki/Moving_average
void CApplication::compute_avg_fps()
{
	static constexpr double alpha = 0.6;

	m_avg_fps = alpha * m_avg_fps + (1.0 - alpha) * get_fps();
}

void CApplication::create_new_active_processor(const std::filesystem::path& filepath)
{
	m_failed_to_open_file = false;
	
	// Close current if there's any before be create a new one
	close_active_processor();

	m_active_processor = new CX86PEProcessor;
	if (!m_active_processor->process(filepath))
	{
		CDebugConsole::get().output_error(std::format("Failed to process \"{}\"", filepath.string()));

		close_active_processor();
		m_failed_to_open_file = true;
	}
}

void CApplication::close_active_processor()
{
	if (!m_active_processor)
		return;

	delete m_active_processor;
	m_active_processor = nullptr;
}

bool CApplication::is_alive()
{
	return !glfwWindowShouldClose(m_glfw_window);
}

void CApplication::render_ui_contents()
{
	std::thread processing_thread;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				auto sel = pfd::open_file::open_file(
					"Select a text file", ".",
					{ "Executable files (.dll, .exe)", "*.dll *.exe" },
					pfd::opt::none);

				if (!sel.result().empty())
				{
					create_new_active_processor(sel.result()[0]);
				}
			}

			if (ImGui::MenuItem("Close"))
			{
				close_active_processor();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Preferences"))
			{

			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (m_failed_to_open_file)
	{
		CGUIWidgets::get().open_modal_popup("Error ##Invalid_path_modal");

		CGUIWidgets::get().add_modal_popup(
			"Error ##Invalid_path_modal", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize,
			[&]()
			{
				ImGui::Text("Unable to process this type of file.");
				ImGui::Spacing();

				if (ImGui::Button("OK", { -1, 0 }))
				{
					m_failed_to_open_file = false;
					ImGui::CloseCurrentPopup();
				}
			});
	}

	if (processing_thread.joinable())
		processing_thread.join();

	if (is_processor_active())
	{
		m_active_processor->render_gui();
	}
	else
	{
		CGUIWidgets::get().add_window_centered_text("File extensions supported: Portable executable (.dll, .exe)");
	}
}
