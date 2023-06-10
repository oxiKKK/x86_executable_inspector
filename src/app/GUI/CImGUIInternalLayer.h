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

#ifndef EXEIN_C_IMGUI_INTERNAL_LAYER_H
#define EXEIN_C_IMGUI_INTERNAL_LAYER_H

#pragma once

// Extension over already existing imgui APIs within our application's code.
class CImGUIInternalLayer
{
public:
	static inline CImGUIInternalLayer& get()
	{
		static CImGUIInternalLayer layer;
		return layer;
	}

public:
	void add_text_unformatted_wrapped(const char* text);
	void add_text_unformatted_wrapped_colored(const ImVec4& col, const char* text);
	void add_text_unformatted_colored(const ImVec4& col, const char* text);

	// Disable word-wrap on functions that are meant to process wordwrap.
	inline void entry_push_disable_wordwrap() { m_entry_text_wrappable = false; }
	inline void entry_pop_disable_wordwrap() { m_entry_text_wrappable = true; }

private:
	bool m_entry_text_wrappable = true;
};

#endif