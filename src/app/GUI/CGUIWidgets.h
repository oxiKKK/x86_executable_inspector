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

#ifndef EXEIN_C_GUI_WIDGETS_H
#define EXEIN_C_GUI_WIDGETS_H

#pragma once

// Static class that provides more extended functionality of imgui widgets
class CGUIWidgets
{
public:
	static auto& get()
	{
		static CGUIWidgets widgets;
		return widgets;
	}

public:
	//
	// Double entries
	//

	void add_double_entry(const std::string& label_left, const std::string& label_right);
	void add_double_entry_colored(const std::string& label_left, const std::string& label_right, const ImColor& color);
	void add_double_entry_hoverable(const std::string& label_left, const std::string& label_right, const std::function<void()>& callback);
	bool add_double_entry_checkbox(const std::string& label_left, bool* p_selected);

	//
	// Integral-value double entires
	//

	template<typename T> requires(std::is_integral<T>::value)
		inline void add_double_entry_hex(const std::string& label_left, T value)
	{
		add_double_entry(label_left, std::format("0x{:08X}", value));
	}

	template<typename T> requires(std::is_integral<T>::value)
		inline void add_double_entry_dec(const std::string& label_left, T value)
	{
		add_double_entry(label_left, std::to_string(value));
	}

	//
	// Hoverable integral double entires
	//

	template<typename T> requires(std::is_integral<T>::value)
		inline void add_double_entry_hex_hoverable(const std::string& label_left, T value,
												   const std::function<void()>& callback)
	{
		add_double_entry(label_left, value ? std::format("0x{:08X} (Hover over)", value) : "null");

		if (callback)
		{
			if (ImGui::IsItemHovered() && value != 0)
			{
				inside_tooltip(callback);
			}
		}
	}

	template<typename T> requires(std::is_integral<T>::value)
		inline void add_double_entry_dec_hoverable(const std::string& label_left, T value,
												   const std::function<void()>& callback)
	{
		add_double_entry(label_left,value ? std::format("{} (Hover over)", value) : "null");

		if (ImGui::IsItemHovered() && value != 0)
		{
			inside_tooltip(callback);
		}
	}

	//
	// Popups, tooltips, modals
	//

	void inside_popup_context_menu(const std::string& label, const std::function<void()>& callback, const char* caption = "Right-click on me!");

	void inside_tooltip(const std::function<void()>& callback);
	void add_hovered_tooltip(const char* label);

	void open_modal_popup(const std::string& name);
	void add_modal_popup(const std::string& name, ImGuiWindowFlags flags, const std::function<void()>& callback);

	//
	// Tables/lists
	//

	// Adds new imgui table
	void add_table(const std::string& name, uint32_t columns, ImGuiTableFlags flags, const std::function<void()>& header_callback, const std::function<void()>& body_callback);

	// Use for simple tables such as:
	// 
	// name       value
	// name1      value1
	// ...
	// nameN      valueN
	//
	void add_undecorated_simple_table(const std::string& name, uint32_t columns, const std::function<void()>& body_callback);

	void table_setup_column_fixed_width(const std::string& name, float width);
	void table_setup_column(const std::string& name);

	//
	// Centered text
	//

	// Renders text in the middle of a screen
	void add_window_centered_disabled_text(const std::string& label);
	void add_window_centered_text(const std::string& label);
	void add_window_centered_text_ex(const std::string& label, ImColor color);

	void add_centered_text(const std::string& text, float contents_size = 0.f);

	//
	// Others
	//

	void add_section_separator(const std::string& label);

	// True if item is hovered with ImGuiHoveredFlags_DelayNormal flag.
	bool is_item_long_hovered();

	float apply_imgui_window_x_padding(float to);
	float apply_imgui_window_y_padding(float to);

	// The text can be copied to clickboard on right click.
	bool add_copyable_selectable(const std::string& label, const std::string& what_to_copy = "");

private:
	bool m_entry_text_wrappable = true;
};

#endif