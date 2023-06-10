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

#ifndef EXEIN_PCH_H
#define EXEIN_PCH_H

#pragma once

//===========================================================================
// 
// Public header files
// 
//===========================================================================

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <map>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <bitset>
#include <array>
#include <Windows.h>
#include <delayimp.h>
#include <WinTrust.h> // For certificate data

// Needs to be included at the beginning
#include "CDebugConsole.h"
#include "CUtilityFuncs.h"

//===========================================================================
// 
// Portable file dialogs
// 
//===========================================================================

#include "portable_file_dialogs/portable-file-dialogs.h"

//===========================================================================
// 
// ImGUI
// 
//===========================================================================

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl2.h>

//===========================================================================
// 
// GLFW
// 
//===========================================================================

#include <GLFW/glfw3.h>

//===========================================================================
// 
// SOIL
// 
//===========================================================================

#include <SOIL/SOIL.h>

//===========================================================================
// 
// Util
// 
//===========================================================================

#include "util/UtilByteBuffer.h"
#include "util/UtilNamedConstant.h"
#include "util/UtilSmartValue.h"
#include "util/UtilVector.h"

//===========================================================================
// 
// Assets
// 
//===========================================================================

#include "assets/assets.h"

//===========================================================================
// 
// Our header files
// 
//===========================================================================

// Processors
#include "processors/IBaseProcessor.h"
#include "processors/CX86PEProcessor.h"

// GUI code
#include "GUI/CThemeManager.h"
#include "GUI/CImGUIInternalLayer.h"
#include "GUI/CGUIWidgets.h"
#include "GUI/CGUI.h"

// Others
#include "CApplication.h"

#endif