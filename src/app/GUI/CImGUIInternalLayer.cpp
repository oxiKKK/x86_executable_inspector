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

#include "imgui/imgui_internal.h"

using namespace ImGui;

void CImGUIInternalLayer::add_text_unformatted_wrapped(const char* text)
{
	ImGuiContext& g = *GImGui;
	bool need_backup = (g.CurrentWindow->DC.TextWrapPos < 0.0f); // Keep existing wrap position if one is already set
	if (need_backup && m_entry_text_wrappable)
		PushTextWrapPos(0.0f);

	TextEx(text, NULL, ImGuiTextFlags_NoWidthForLargeClippedText);

	if (need_backup && m_entry_text_wrappable)
		PopTextWrapPos();
}

void CImGUIInternalLayer::add_text_unformatted_wrapped_colored(const ImVec4& col, const char* text)
{
	PushStyleColor(ImGuiCol_Text, col);

	TextEx(text, NULL, ImGuiTextFlags_NoWidthForLargeClippedText);

	PopStyleColor();
}

void CImGUIInternalLayer::add_text_unformatted_colored(const ImVec4& col, const char* text)
{
	PushStyleColor(ImGuiCol_Text, col);

	TextEx(text, NULL, ImGuiTextFlags_NoWidthForLargeClippedText);

	PopStyleColor();
}
