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

std::string CUtil::make_filesize_nice(uint32_t number)
{
	if (!number)
		return "NaN";

	std::string formatted;

	uint32_t decade = 0;
	for (uint32_t n = number; n; n /= 1000, decade++)
	{
		uint32_t num = n % 1000;

		bool first = n == number;
		std::string szdel = ",";

		if (num < 100)
			szdel.push_back('0');

		if (num < 10)
			szdel.push_back('0');

		bool end = n < 1000;
		formatted = (!end ? szdel : "") + std::to_string(num) + formatted;
	}

	if (decade > 1)
	{
		formatted.resize(formatted.size() - 4);
		formatted.append(" KiB");
	}
	else
		formatted.append(" B");

	return formatted;
}

void CUtil::copy_to_clipboard(const std::string& str_to_copy)
{
	HANDLE hHandle = GlobalAlloc(GMEM_FIXED, str_to_copy.size() + 1);
	memcpy(hHandle, str_to_copy.c_str(), str_to_copy.size() + 1);

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hHandle);
	CloseClipboard();
}

std::string CUtil::guid_to_string(const GUID& guid)
{
	return std::format("{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}",
					   guid.Data1, guid.Data2, guid.Data3,
					   guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
					   guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
