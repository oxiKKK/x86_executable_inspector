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

void CGUIWidgets::add_double_entry(const std::string& label_left, const std::string& label_right)
{
	ImGui::TableNextColumn();
	CImGUIInternalLayer::get().add_text_unformatted_wrapped(label_left.c_str());
	ImGui::TableNextColumn();
	CImGUIInternalLayer::get().add_text_unformatted_wrapped(label_right.c_str());
}

void CGUIWidgets::add_double_entry_colored(const std::string& label_left, const std::string& label_right, 
										   const ImColor& color)
{
	ImGui::TableNextColumn();
	CImGUIInternalLayer::get().add_text_unformatted_wrapped(label_left.c_str());
	ImGui::TableNextColumn();
	CImGUIInternalLayer::get().add_text_unformatted_wrapped_colored(color, label_right.c_str());
}

void CGUIWidgets::add_double_entry_hoverable(const std::string& label_left, const std::string& label_right, const std::function<void()>& callback)
{
	add_double_entry(label_left, label_right + " (Hover over)");

	if (ImGui::IsItemHovered())
	{
		inside_tooltip(callback);
	}
}

bool CGUIWidgets::add_double_entry_checkbox(const std::string& label_left, bool* p_selected)
{
	ImGui::TableNextColumn();
	CImGUIInternalLayer::get().add_text_unformatted_wrapped(label_left.c_str());
	ImGui::TableNextColumn();
	return ImGui::Checkbox(("##" + label_left).c_str(), p_selected);
}

void CGUIWidgets::inside_popup_context_menu(const std::string& label, const std::function<void()>& callback, const char* caption)
{
	add_hovered_tooltip(caption);

	if (ImGui::BeginPopupContextItem(label.c_str()))
	{
		if (callback)
			callback();

		ImGui::EndPopup();
	}
}

void CGUIWidgets::inside_tooltip(const std::function<void()>& callback)
{
	CImGUIInternalLayer::get().entry_push_disable_wordwrap(); // disable wwrap temporarily
	
	ImGui::BeginTooltip();

	if (callback)
		callback();

	ImGui::EndTooltip();

	CImGUIInternalLayer::get().entry_pop_disable_wordwrap(); // enable it back
}

void CGUIWidgets::add_hovered_tooltip(const char* label)
{
	if (!label)
		return;

	if (is_item_long_hovered())
		ImGui::SetTooltip(label);
}

void CGUIWidgets::open_modal_popup(const std::string& name)
{
	ImGui::OpenPopup(name.c_str());
}

void CGUIWidgets::add_modal_popup(const std::string& name, ImGuiWindowFlags flags, const std::function<void()>& callback)
{
	if (ImGui::BeginPopupModal(name.c_str(), NULL, flags))
	{
		if (callback)
			callback();

		ImGui::EndPopup();
	}
}

void CGUIWidgets::add_table(const std::string& name, uint32_t columns, ImGuiTableFlags flags,
							const std::function<void()>& header_callback, const std::function<void()>& body_callback)
{
	if (ImGui::BeginTable(name.c_str(), columns, flags))
	{
		if (header_callback)
			header_callback();

		if (body_callback)
			body_callback();

		ImGui::EndTable();
	}
}

void CGUIWidgets::add_undecorated_simple_table(const std::string& name, uint32_t columns, const std::function<void()>& body_callback)
{
	const auto header_callback = [&]()
	{
		for (uint32_t i = 0; i < columns; i++)
			table_setup_column(std::format("column{}", columns));
	};

	add_table(name, columns, ImGuiTableFlags_None, header_callback, body_callback);
}

void CGUIWidgets::table_setup_column_fixed_width(const std::string& name, float width)
{
	ImGui::TableSetupColumn(name.c_str(), ImGuiTableColumnFlags_WidthFixed, width);
}

void CGUIWidgets::table_setup_column(const std::string& name)
{
	ImGui::TableSetupColumn(name.c_str());
}

void CGUIWidgets::add_window_centered_disabled_text(const std::string& label)
{
	add_window_centered_text_ex(label, 
								ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]));
}

void CGUIWidgets::add_window_centered_text(const std::string& label)
{
	add_window_centered_text_ex(label,
								ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
}

void CGUIWidgets::add_window_centered_text_ex(const std::string& label, ImColor color)
{
	auto window_size = ImGui::GetWindowSize();
	auto cursor_screen = ImGui::GetCursorScreenPos();

	const auto psz_caption = label.c_str();
	auto label_size = ImGui::CalcTextSize(psz_caption);

	ImVec2 center_pos =
	{
		apply_imgui_window_x_padding(cursor_screen.x + window_size.x / 2.f - label_size.x / 2.f),
		apply_imgui_window_y_padding(cursor_screen.y + window_size.y / 2.f),
	};

	ImGui::GetWindowDrawList()->AddText(center_pos,
										color,
										psz_caption);
}

void CGUIWidgets::add_centered_text(const std::string& text, float contents_size)
{
	float contents_sz = (contents_size != 0.f) ? contents_size : ImGui::GetWindowSize().x;

	const std::string stylestr = "---";
	const float style_str_size_px = ImGui::CalcTextSize(stylestr.c_str()).x;
	const float style_pad = 10.f;

	ImVec2 last = ImGui::GetCursorPos();

	float text_size_px = ImGui::CalcTextSize(text.c_str()).x;
	float text_position_x_left = ImGui::GetCursorPosX() + (contents_size / 2.f) - (text_size_px / 2.f);
	float text_position_x_right = ImGui::GetCursorPosX() + (contents_size / 2.f) + (text_size_px / 2.f);
	
	ImGui::SetCursorPosX(text_position_x_left - style_pad - style_str_size_px);
	ImGui::TextColored(ImColor(230, 230, 230, 150), stylestr.c_str());

	ImGui::SetCursorPos({ text_position_x_left, last.y });
	ImGui::TextUnformatted(text.c_str());
	ImGui::SetCursorPosX(last.x);

	ImGui::SetCursorPos({ text_position_x_right + style_pad, last.y });
	ImGui::TextColored(ImColor(230, 230, 230, 150), stylestr.c_str());
	ImGui::SetCursorPosX(last.x);
}

void CGUIWidgets::add_section_separator(const std::string& label)
{
	ImGui::Spacing();
	ImGui::TextDisabled(label.c_str());
	ImGui::Separator();
}

bool CGUIWidgets::is_item_long_hovered()
{
	return ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal);
}

float CGUIWidgets::apply_imgui_window_x_padding(float to)
{
	auto& style = ImGui::GetStyle();

	return to - style.WindowPadding.x * 2.f;
}

float CGUIWidgets::apply_imgui_window_y_padding(float to)
{
	auto& style = ImGui::GetStyle();

	return to - style.WindowPadding.y * 2.f;
}

bool CGUIWidgets::add_copyable_selectable(const std::string& label, const std::string& what_to_copy)
{
	bool selected = ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

	inside_popup_context_menu(label.c_str(),
		[&]()
		{
			if (ImGui::Selectable("Copy"))
			{
				CUtil::copy_to_clipboard(what_to_copy.empty() ? label : what_to_copy);
				ImGui::CloseCurrentPopup();
			}
		},
		nullptr);

	return selected;
}
