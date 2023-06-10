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

#ifndef EXEIN_C_THEME_MANAGER_H
#define EXEIN_C_THEME_MANAGER_H

#pragma once

struct ThemeColorContainer
{
	ImVec4 m_clr_window_bg;

	ImVec4 m_clr_table_border_strong;
	ImVec4 m_clr_table_border_light;
};

class AppTheme : public ThemeColorContainer
{
public:
	AppTheme(const std::string& name, const ThemeColorContainer& colors) :
		m_name(name), 
		ThemeColorContainer(colors)
	{
	}

	AppTheme() = delete;

public:
	inline const auto& get_name() const { return m_name; }

	// Fill ImGui color fields with our data
	void fill_imgui_colors();

private:
	std::string m_name;
};

class CThemeManager
{
public:
	static auto& get()
	{
		static CThemeManager theme_mgr;
		return theme_mgr;
	}

public:
	enum class Theme
	{
		Default,
		// ...
	};

private:
	using ThemeMap_t = std::unordered_map<Theme, AppTheme>;

public:
	void initialize();

	void set_new_theme(Theme theme);

	AppTheme& get_current_theme();

private:
	// Fills ImGui colors with current theme
	void refresh_colors();

	// Exception-care getter for theme objects.
	AppTheme& get_theme_object(Theme theme);

private:
	static ThemeMap_t	m_theme_map;
	Theme				m_current_theme = Theme::Default;
};

#endif