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

CThemeManager::ThemeMap_t CThemeManager::m_theme_map =
{
	{
		// Default theme
		CThemeManager::Theme::Default,
		{
			"Default",
			{
				ImColor( 15,  15,  15, 240), 
				ImColor(  0,   0,   0,   0), 
				ImColor(  0,   0,   0,   0), 
			}
		}
	},
};

void CThemeManager::initialize()
{
	// For now we'll set theme to default. Later on we'll have
	// some system to load theme settings from a settings file.
	set_new_theme(Theme::Default);
}

void CThemeManager::set_new_theme(Theme theme)
{
	m_current_theme = theme;

	refresh_colors();
}

AppTheme& CThemeManager::get_current_theme()
{
	return get_theme_object(m_current_theme);
}

void CThemeManager::refresh_colors()
{
	auto& current = get_current_theme();

	current.fill_imgui_colors();

	CDebugConsole::get().output_message(std::format("Changed theme to {}", current.get_name()));
}

AppTheme& CThemeManager::get_theme_object(Theme theme)
{
	try
	{
		// Use at(), because [] operator creates new entry if the one isn't found.
		return m_theme_map.at(theme);
	}
	catch (...)
	{
		CDebugConsole::get().output_error(std::format("Tried to obtain invalid theme: {}", (int)theme));
		CDebugConsole::get().output_error("Returned default theme...");
	}

	// This should theoretically never happen
	return m_theme_map.at(Theme::Default);
}

void AppTheme::fill_imgui_colors()
{
	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	colors[ImGuiCol_WindowBg] = m_clr_window_bg;
	colors[ImGuiCol_TableBorderStrong] = m_clr_table_border_strong;
	colors[ImGuiCol_TableBorderLight] = m_clr_table_border_light;
}
