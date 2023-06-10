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

bool CGUI::startup(GLFWwindow* glfw_window)
{
	// Setup Dear ImGui context
	if (!ImGui::CreateContext())
	{
		CDebugConsole::get().output_message("Failed to create ImGui context");
		return false;
	}

	auto& io = ImGui::GetIO();

	// Disable generating imgui.ini
	io.IniFilename = io.LogFilename = nullptr;

	CThemeManager::get().initialize();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);

	ImGui_ImplOpenGL2_Init();

	CDebugConsole::get().output_message("Initialized GUI");

	return true;
}

void CGUI::shutdown()
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

void CGUI::update(int32_t app_width, int32_t app_height, const std::function<void()>& base_frame_contents_callback)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	render_base_frame(app_width, app_height, base_frame_contents_callback);

	//ImGui::ShowDemoWindow();

	ImGui::Render();

	glViewport(0, 0, app_width, app_height);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void CGUI::render_base_frame(int32_t app_width, int32_t app_height, const std::function<void()>& base_frame_contents_callback)
{
	ImGui::SetNextWindowSize({ (float)app_width, (float)app_height }, ImGuiCond_Always);
	ImGui::SetNextWindowPos({}, ImGuiCond_Always);

	static constexpr auto window_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("main_window", NULL, window_flags))
	{
		base_frame_contents_callback();

		ImGui::End();
	}
}
