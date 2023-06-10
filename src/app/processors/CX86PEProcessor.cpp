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

bool CX86PEProcessor::process(const std::filesystem::path& filepath)
{
	if (!on_processing_start(filepath))
		return false;

	if (!process_dos())
		return false;

	if (!process_nt_headers())
		return false;

	process_image_strings();

	on_processing_end();

	return true;
}

void CX86PEProcessor::render_gui()
{
	ImGui::Columns(2, NULL, false);

	ImGui::SetColumnWidth(0, gui_leftmost_column_size());

	CGUIWidgets::get().add_centered_text("DOS Header", CGUIWidgets::get().apply_imgui_window_x_padding(gui_leftmost_column_size()));
	ImGui::BeginChild("DOS_Header_child", { 0, 90 }, true);
	{
		CGUIWidgets::get().add_undecorated_simple_table(
			"DOS_header", 2,
			[&]()
			{
				CGUIWidgets::get().add_double_entry("Magic", m_dos_magic.name());
				CGUIWidgets::get().add_double_entry_dec("Pages", m_number_of_pages);
				CGUIWidgets::get().add_double_entry("Header size", m_dos_headers_size);
				CGUIWidgets::get().add_double_entry("Stub size", m_dos_stub_size);
			});

		ImGui::EndChild();
	}

	CGUIWidgets::get().add_centered_text("NT headers", CGUIWidgets::get().apply_imgui_window_x_padding(gui_leftmost_column_size()));
	ImGui::BeginChild("NT_headers_child", { 0.f, 380.f }, true);
	{
		CGUIWidgets::get().add_undecorated_simple_table(
			"NT_headers_signature", 2,
			[&]()
			{
				CGUIWidgets::get().add_double_entry("Signature", m_nt_sig.name());
			});

		CGUIWidgets::get().add_section_separator("File header");

		CGUIWidgets::get().add_undecorated_simple_table(
			"NT_headers_fileheader", 2,
			[&]()
			{
				CGUIWidgets::get().add_double_entry("Machine", m_nt_machine.name());
				CGUIWidgets::get().add_double_entry_dec("Sections", m_num_sections);
				CGUIWidgets::get().add_double_entry("Creation time", m_nt_time_date_stamp.as_string());

				static uint32_t active_characteristics = 0;
				render_named_bitfield_constant_generic(m_nt_characteristics, "Characteristics", "Characteristics_child", active_characteristics);
			});

		CGUIWidgets::get().add_section_separator("Optional header");

		CGUIWidgets::get().add_undecorated_simple_table(
			"NT_headers_optionalheader", 2,
			[&]()
			{
				CGUIWidgets::get().add_double_entry("Magic", m_nt_magic.name());
				CGUIWidgets::get().add_double_entry("Image base", m_img_base);
				CGUIWidgets::get().add_double_entry("Code base VA", m_code_base_va);
				CGUIWidgets::get().add_double_entry("Data base VA", m_data_base_va);
				CGUIWidgets::get().add_double_entry("Entry point VA", m_entry_point_va);
				CGUIWidgets::get().add_double_entry("Uninit data base", m_uninitialized_data_size);
				CGUIWidgets::get().add_double_entry("Init data base", m_initialized_data_size);
				CGUIWidgets::get().add_double_entry("Code size", m_code_size);
				CGUIWidgets::get().add_double_entry("Section alignment", m_section_alignment);
				CGUIWidgets::get().add_double_entry("File alignment", m_file_alignment);
				CGUIWidgets::get().add_double_entry("Size of image", m_size_of_image);
				CGUIWidgets::get().add_double_entry("Size of headers", m_size_of_hdrs);

				if (m_image_checksum_real != m_image_checksum_calc)
					CGUIWidgets::get().add_double_entry_colored("Checksum (calc.)", std::format("{} ({})", m_image_checksum_real.as_string(), m_image_checksum_calc.as_string()), ImColor(230, 0, 0, 200));
				else
					CGUIWidgets::get().add_double_entry("Checksum", m_image_checksum_real);

				CGUIWidgets::get().add_double_entry("SubSystem", m_nt_subsystem.name());

				static uint32_t active_characteristics = 0;
				CGUIWidgets::get().add_double_entry_hex_hoverable(
					"DLL Characteristics", m_nt_dll_characteristics.get_val(),
					[&]()
					{
						ImGui::BeginChild("Characteristics_child", { 400.f, 17 * (float)active_characteristics });
						{
							active_characteristics = 0;
							for (uint32_t i = 0; i < m_nt_dll_characteristics.get_size_bits(); i++)
							{
								// True if current bit in the bitfield is set
								if (!m_nt_dll_characteristics.is_bit_present(i)) // Display only bits that are set
									continue;

								const auto& str = m_nt_dll_characteristics.get_string_at(i);
								ImGui::TextWrapped("%02d: %s", i, str.c_str());
								active_characteristics++;
							}
						}
						ImGui::EndChild();
					});
			});

		ImGui::EndChild();
	}

	CGUIWidgets::get().add_centered_text("Data directories", CGUIWidgets::get().apply_imgui_window_x_padding(gui_leftmost_column_size()));
	ImGui::BeginChild("Data_directories_child", { 0.f, 0.f }, true);
	{
		for (const auto& [key, dir] : m_data_dirs)
		{
			if (!dir.is_present())
				continue;

			if (ImGui::TreeNodeEx(dir.m_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
			{
				CGUIWidgets::get().add_undecorated_simple_table(
					dir.m_name.c_str(), 2,
					[&]()
					{
						CGUIWidgets::get().add_double_entry("Base RVA", dir.m_address);
						CGUIWidgets::get().add_double_entry("Size", dir.m_size);
					});

				ImGui::TreePop();
			}
		}

		ImGui::EndChild();
	}

	ImGui::NextColumn();

	render_tabs();

	ImGui::Columns(1);
}

void CX86PEProcessor::render_tabs()
{
	static const auto tabs_flags =
		ImGuiTabBarFlags_NoCloseWithMiddleMouseButton |
		ImGuiTabBarFlags_FittingPolicyScroll;

	ImGui::BeginTabBar("Tabs", tabs_flags);

	render_tab_sections();
	render_tab_exports();
	render_tab_imports();
	render_tab_certificates();
	render_tab_relocations();
	render_tab_debug();
	render_tab_load_cfg();
	render_tab_strings();
	render_tab_misc();

	ImGui::EndTabBar();
}

void CX86PEProcessor::render_tab_sections()
{
	render_content_tab(
		"Sections",
		true, // Assume that we have at least one section
		[&]()
		{
			ImGui::BeginChild("Sections_child_low", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);
			{
				CGUIWidgets::get().add_table(
					"Exports_child_up_table", 7,
					ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame,
					[&]()
					{
						CGUIWidgets::get().table_setup_column("Name");
						CGUIWidgets::get().table_setup_column("Base VA");
						CGUIWidgets::get().table_setup_column("End VA");
						CGUIWidgets::get().table_setup_column("Size");
						CGUIWidgets::get().table_setup_column("Raw data RVA");
						CGUIWidgets::get().table_setup_column("Raw size");
						CGUIWidgets::get().table_setup_column("Zero padding");

						ImGui::TableHeadersRow();
					},
					[&]()
					{
						for (const auto& [key, sec] : m_sections)
						{
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_name.c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_va_base.as_string().c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.virtual_end_addr().as_string().c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_va_size.as_string().c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_raw_data_ptr.as_string().c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_raw_data_size.as_string().c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(sec.m_zero_padding.as_string().c_str());
						}
					});
			}
			ImGui::EndChild();
		}
	);
}

void CX86PEProcessor::render_tab_exports()
{
	render_content_tab(
		"Exports",
		m_data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT].is_present(),
		[&]()
		{
			ImGui::BeginChild("Exports_child_up", { 0, 70.f });
			{
				CGUIWidgets::get().add_undecorated_simple_table(
					"Exports_child_up_table", 2,
					[&]()
					{
						CGUIWidgets::get().add_double_entry("Exported functions", std::format("{} entries", m_number_of_exported_functions));
						CGUIWidgets::get().add_double_entry("Ordinal base number", m_export_starting_ordinal_num.as_string());
						CGUIWidgets::get().add_double_entry("Export DLL name", m_export_dll_name);
						CGUIWidgets::get().add_double_entry("Export creation time", m_export_creation_timestamp.as_string());
					});
			}
			ImGui::EndChild();

			ImGui::Separator();

			ImGui::BeginChild("Exports_child_low", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);
			{
				const auto is_meant_to_be_colorized_or_not = [this](const ImageExport& exp) -> bool
				{
					if (exp.m_address == m_entry_point_va)
						return true;

					// Look for common function names
					static const char* s_blacklisted_function_names[] =
					{
						"DllInitialize",
						"main",
						"DllMain"
					};

					for (const char* name : s_blacklisted_function_names)
					{
						if (exp.m_name == name)
							return true;
					}

					return false;
				};

				for (const auto& exp : m_image_exports)
				{
					bool colorize_function = is_meant_to_be_colorized_or_not(exp);

					if (colorize_function)
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(ImColor(230, 50, 50, 230)));

					CGUIWidgets::get().add_copyable_selectable(exp.m_name.c_str());

					if (colorize_function)
						ImGui::PopStyleColor();

					// Render tooltip on hover
					if (!ImGui::IsItemHovered())
						continue;

					CGUIWidgets::get().inside_tooltip(
						[&]()
						{
							CGUIWidgets::get().add_undecorated_simple_table(
								"Exports_child_low_table", 2,
								[&]()
								{
									CGUIWidgets::get().add_double_entry("VA", exp.m_address.as_string());
									CGUIWidgets::get().add_double_entry("Ordinal", std::format("{} ({})", exp.m_ordinal.as_string(), exp.m_ordinal.as_string()));
									CGUIWidgets::get().add_double_entry("Forwarded", exp.is_forwarded ? "yes" : "no");
								});
						});
				}
			}
			ImGui::EndChild();
		});
}

void CX86PEProcessor::render_tab_imports()
{
	render_content_tab(
		"Imports",
		true,
		[&]()
		{
			auto render_imports_tab_contents = [&](bool delayed)
			{
				ImGui::BeginChild("Imports_child_up", { 0, delayed ? 35.f : 70.f });
				{
					CGUIWidgets::get().add_undecorated_simple_table(
						"Imports_child_up_table", 2,
						[&]()
						{
							CGUIWidgets::get().add_double_entry_dec("Import descriptors", m_number_of_import_descriptors);
							CGUIWidgets::get().add_double_entry("Imported by Name/Ordinal", std::format("{}/{} (total {})", m_number_of_import_thunk_ordinals, m_number_of_import_thunk_names, m_number_of_import_thunk_ordinals + m_number_of_import_thunk_names));

							if (!delayed)
							{
								CGUIWidgets::get().add_double_entry("Number of IAT entries", std::format("{} (thunks & descriptors)", m_iat_entries.size()));
								CGUIWidgets::get().add_double_entry("IAT base VA", m_imports_iat_base);
							}
						});
				}
				ImGui::EndChild();

				ImGui::Separator();

				ImGui::BeginChild("Imports_child_low", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);
				{
					if (delayed)
					{
						for (const auto& imp : m_image_delayed_import_descriptors)
						{
							CGUIWidgets::get().add_section_separator(std::format("{} ({})", imp.m_image_name, imp.m_descriptor_va.as_string()));

							for (const auto& thunk : imp.m_import_thunks)
							{
								CGUIWidgets::get().add_copyable_selectable(thunk.imported_by_name() ? thunk.m_name.c_str() : std::format("{}", thunk.m_ordinal.val()).c_str());

								// Render tooltip on hover
								if (!ImGui::IsItemHovered())
									continue;
								
								CGUIWidgets::get().inside_tooltip(
									[&]()
									{
										CGUIWidgets::get().add_undecorated_simple_table(
											"Imports_child_low_table", 2,
											[&]()
											{
												if (thunk.imported_by_name())
												{
													CGUIWidgets::get().add_double_entry_colored("Imported by", "Name", ImColor(50, 230, 50, 200));
													CGUIWidgets::get().add_double_entry("Hint", std::format("{} ({})", thunk.m_hint.as_string(), thunk.m_hint.as_string()));
												}
												else
												{
													CGUIWidgets::get().add_double_entry_colored("Exported by", "Ordinal", ImColor(50, 125, 230, 200));
													CGUIWidgets::get().add_double_entry("Ordinal", std::format("{} ({})", thunk.m_ordinal.as_string(), thunk.m_ordinal.as_string()));
												}


												CGUIWidgets::get().add_double_entry("VA", thunk.m_import_va);
											});
									});
							}
						}
					}
					else
					{
						for (const auto& imp : m_image_import_descriptors)
						{
							CGUIWidgets::get().add_section_separator(std::format("{} ({})", imp.m_image_name, imp.m_descriptor_va.as_string()));

							for (const auto& thunk : imp.m_import_thunks)
							{
								CGUIWidgets::get().add_copyable_selectable(thunk.imported_by_name() ? thunk.m_name.c_str() : thunk.m_ordinal.as_string().c_str());

								// Render tooltip on hover
								if (!ImGui::IsItemHovered())
									continue;

								CGUIWidgets::get().inside_tooltip(
									[&]()
									{
										CGUIWidgets::get().add_undecorated_simple_table(
											"Imports_child_low_table", 2,
											[&]()
											{
												if (thunk.imported_by_name())
												{
													CGUIWidgets::get().add_double_entry_colored("Imported by", "Name", ImColor(50, 230, 50, 200));
													CGUIWidgets::get().add_double_entry("Hint", std::format("{} ({})", thunk.m_hint.as_string(), thunk.m_hint.as_string()));
												}
												else
												{
													CGUIWidgets::get().add_double_entry_colored("Exported by", "Ordinal", ImColor(50, 125, 230, 200));
													CGUIWidgets::get().add_double_entry("Ordinal", std::format("{} ({})", thunk.m_ordinal.as_string(), thunk.m_ordinal.as_string()));
												}

												CGUIWidgets::get().add_double_entry("VA", thunk.m_import_va);
											});
									});
							}
						}
					}
				}
				ImGui::EndChild();
			};

			static bool render_delayed_imports = false;

			ImGui::Columns(2, NULL, false);

			if (ImGui::Button("Non-delayed", { -1, 0 }))
				render_delayed_imports = false;

			ImGui::NextColumn();

			if (ImGui::Button("Delayed", { -1, 0 }))
				render_delayed_imports = true;

			ImGui::Columns(1);

			ImGui::Spacing();
			CGUIWidgets::get().add_centered_text(render_delayed_imports ? "Delayed" : "Non-Delayed",
												 CGUIWidgets::get().apply_imgui_window_x_padding(ImGui::GetWindowSize().x));
			ImGui::Separator();

			if (render_delayed_imports)
			{
				if (m_data_dirs[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].is_present())
					render_imports_tab_contents(true);
				else
					CGUIWidgets::get().add_window_centered_disabled_text("No data");
			}
			else
			{
				if (m_data_dirs[IMAGE_DIRECTORY_ENTRY_IMPORT].is_present())
					render_imports_tab_contents(false);
				else
					CGUIWidgets::get().add_window_centered_disabled_text("No data");
			}
		});
}

void CX86PEProcessor::render_tab_certificates()
{
	render_content_tab(
		"Certificates",
		m_data_dirs[IMAGE_DIRECTORY_ENTRY_SECURITY].is_present(),
		[&]()
		{
			uint32_t n = 0;
			for (const auto& cert : m_image_certificates)
			{
				CGUIWidgets::get().add_centered_text(cert.m_cert_type.name(), CGUIWidgets::get().apply_imgui_window_x_padding(ImGui::GetColumnWidth(1)));

				ImGui::BeginChild(std::format("Certificates_child_{}", n).c_str(), { 0, 70.f }, true);
				{
					CGUIWidgets::get().add_undecorated_simple_table(
						"Certificates_child_table", 2,
						[&]()
						{
							CGUIWidgets::get().add_double_entry("Certificate version", cert.m_revision_version.name());
							CGUIWidgets::get().add_double_entry("Entry length", cert.m_entry_length);
							CGUIWidgets::get().add_double_entry("Raw data VA", cert.m_cert_raw_data_va);
						});
				}
				ImGui::EndChild();

				n++;
			}
		});
}

void CX86PEProcessor::render_tab_relocations()
{
	render_content_tab(
		"Relocations",
		m_data_dirs[IMAGE_DIRECTORY_ENTRY_BASERELOC].is_present(),
		[&]()
		{
			uint32_t reloc_num = 0, blockinfo_num = 0;
			for (const auto& reloc : m_relocation_blocks)
			{
				CGUIWidgets::get().add_undecorated_simple_table(
					"Relocations_table", 2,
					[&]()
					{
						CGUIWidgets::get().add_double_entry("Page RVA", reloc.m_page_rva);
						CGUIWidgets::get().add_double_entry("Size", reloc.m_size);
					});

				if (!reloc.m_block_info_entries.empty())
				{
					if (ImGui::TreeNodeEx(std::format("Block info entries ({})", reloc.m_block_info_entries.size()).c_str(),
										  ImGuiTreeNodeFlags_SpanFullWidth))
					{
						for (const auto& info : reloc.m_block_info_entries)
						{
							CGUIWidgets::get().add_section_separator(std::format("Block info #{}", blockinfo_num));

							CGUIWidgets::get().add_undecorated_simple_table(
								"Relocations_table1", 2,
								[&]()
								{
									CGUIWidgets::get().add_double_entry("Type", info.m_type.name());
									CGUIWidgets::get().add_double_entry("Offset", info.m_offset);
								});

							blockinfo_num++;
						}

						ImGui::TreePop();
					}
				}
				else
				{
					ImGui::Text("No block info entries");
				}

				ImGui::Separator();

				reloc_num++;
			}
		});
}

void CX86PEProcessor::render_tab_debug()
{
	render_content_tab(
		"Debug",
		m_data_dirs[IMAGE_DIRECTORY_ENTRY_DEBUG].is_present(),
		[&]()
		{
			uint32_t num_debug_dirs = 0;
			for (const auto& debug : m_debug_directories)
			{
				CGUIWidgets::get().add_section_separator(std::format("Directory #{}", num_debug_dirs));

				CGUIWidgets::get().add_undecorated_simple_table(
					"Debug_table", 2,
					[&]()
					{
						CGUIWidgets::get().add_double_entry("Timestamp", debug.m_timestamp.as_string());
						CGUIWidgets::get().add_double_entry("Major/Minor version", std::format("{}/{}", debug.m_major_ver.val(), debug.m_minor_ver.val()));
						CGUIWidgets::get().add_double_entry("Type", debug.m_type.name());
						CGUIWidgets::get().add_double_entry("Size of data", debug.m_size_of_data);
						CGUIWidgets::get().add_double_entry("VA of data", debug.m_address_of_raw_data_va);
						CGUIWidgets::get().add_double_entry("File pointer of data", debug.m_file_pointer_to_raw_data);
					});

				ImGui::PushID(num_debug_dirs);

				if (ImGui::TreeNodeEx("View debug data", ImGuiTreeNodeFlags_SpanFullWidth))
				{
					CGUIWidgets::get().add_undecorated_simple_table(
						"Debug_table_data", 2,
						[&]()
						{
							// Now look for all possible debug directory types
							if (debug.m_type.is(IMAGE_DEBUG_TYPE_CODEVIEW))
							{
								const auto& cv = debug.m_cv;
								const auto& sig = cv.m_magic_signature;

								CGUIWidgets::get().add_double_entry("Signature", sig.name());

								// The codeview type is also divided into more possible sub-types determinted
								// by a signature.
								if (sig.is(ImageDebugDirectory::CodeView::k_sigRSDS))
								{
									CGUIWidgets::get().add_double_entry("PDB GUID signature", cv.m_rsds_guid_pdb_sig.get());
								}
								// These two are without any structure
								else if (sig.is(ImageDebugDirectory::CodeView::k_sigNB09) ||
										 sig.is(ImageDebugDirectory::CodeView::k_sigNB11))
								{
									// Nothing here to display
								}
								else if (sig.is(ImageDebugDirectory::CodeView::k_sigNB10))
								{
									CGUIWidgets::get().add_double_entry("PDB Signature", cv.m_nb10_pdb_sig);
								}

								// These are shared with either RSDS or NBXX signatures
								CGUIWidgets::get().add_double_entry("PDB age", cv.m_pdb_age);
								CGUIWidgets::get().add_double_entry("PDB path", cv.m_pdb_path);
							}
						});

					ImGui::TreePop();
				}

				ImGui::PopID();

				num_debug_dirs++;
			}
		});
}

void CX86PEProcessor::render_tab_load_cfg()
{
	render_content_tab(
		"Load CFG",
		m_data_dirs[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].is_present(),
		[&]()
		{
			CGUIWidgets::get().add_undecorated_simple_table(
				"LoadCFG_table", 2,
				[&]()
				{
					CGUIWidgets::get().add_double_entry("Timestamp", m_load_cfg_timestamp.as_string());
					CGUIWidgets::get().add_double_entry("Global flags clear", m_load_cfg_global_flags_clear);
					CGUIWidgets::get().add_double_entry("Global flags set", m_load_cfg_global_flags_set);
					CGUIWidgets::get().add_double_entry("Default critical sectionf timeout (ms)", m_load_cfg_cs_default_timeout);
					CGUIWidgets::get().add_double_entry("Process heap DeCommit free block threshold", m_load_cfg_decommit_free_block_threshold);
					CGUIWidgets::get().add_double_entry("Process heap DeCommit total free threshold", m_load_cfg_decommit_total_block_threshold);
					CGUIWidgets::get().add_double_entry("Lock prefix table VA", m_load_cfg_lock_prefix_table_va);
					CGUIWidgets::get().add_double_entry("Max process heap allocation size", m_load_cfg_max_proc_heap_alloc_size);
					CGUIWidgets::get().add_double_entry("Max process VirtualAlloc threshold", m_load_cfg_vm_threshold);

					static uint32_t active_procflags = 0;
					CGUIWidgets::get().add_double_entry_hex_hoverable(
						"Guard flags", m_load_cfg_process_heap_flags.get_val(),
						[&]()
						{
							ImGui::BeginChild("LoadCFGProcHeapFlags_child", { 400.f, 17 * (float)active_procflags });
							{
								active_procflags = 0;
								for (uint32_t i = 0; i < m_load_cfg_process_heap_flags.get_size_bits(); i++)
								{
									// True if current bit in the bitfield is set
									if (!m_load_cfg_process_heap_flags.is_bit_present(i)) // Display only bits that are set
										continue;

									const auto& str = m_load_cfg_process_heap_flags.get_string_at(i);
									ImGui::TextWrapped("%02d: %s", i, str.c_str());
									active_procflags++;
								}
							}
							ImGui::EndChild();
						});

					CGUIWidgets::get().add_double_entry("Process affinity mask", m_load_cfg_process_affinity_mask);
					CGUIWidgets::get().add_double_entry("Service pakc version id", m_load_cfg_service_pack_ver_id);
					CGUIWidgets::get().add_double_entry("Dependent load flags", m_load_cfg_dependent_load_flags);
					CGUIWidgets::get().add_double_entry("Edit list VA", m_load_cfg_edit_list_va);
					CGUIWidgets::get().add_double_entry("Security cookie VA", m_load_cfg_security_cookie_va);
					CGUIWidgets::get().add_double_entry("SEH table VA", m_load_cfg_seh_table_va);
					CGUIWidgets::get().add_double_entry("SEH table handlers", m_load_cfg_seh_handlers_count);
					CGUIWidgets::get().add_double_entry("Control Flow Guard check-function pointer VA", m_load_cfg_guard_cfg_check_func_ptr_va);
					CGUIWidgets::get().add_double_entry("Control Flow Guard dispatch-function pointer VA", m_load_cfg_guard_cfg_dispatch_func_ptr_va);
					CGUIWidgets::get().add_double_entry("Guard CFG function table VA", m_load_cfg_guard_cfg_func_table_va);
					CGUIWidgets::get().add_double_entry("Guard CFG function table entries", m_load_cfg_guard_cfg_func_table_entries);

					static uint32_t active_gflags = 0;
					CGUIWidgets::get().add_double_entry_hex_hoverable(
						"Guard flags", m_load_cfg_guard_flags.get_val(),
						[&]()
						{
							ImGui::BeginChild("LoadCFGGuardFlags_child", { 400.f, 17 * (float)active_gflags });
							{
								active_gflags = 0;
								for (uint32_t i = 0; i < m_load_cfg_guard_flags.get_size_bits(); i++)
								{
									// True if current bit in the bitfield is set
									if (!m_load_cfg_guard_flags.is_bit_present(i)) // Display only bits that are set
										continue;

									const auto& str = m_load_cfg_guard_flags.get_string_at(i);
									ImGui::TextWrapped("%02d: %s", i, str.c_str());
									active_gflags++;
								}
							}
							ImGui::EndChild();
						});

					// Code integrity
					CGUIWidgets::get().add_double_entry("Code integrity flags", m_load_cfg_ci.m_flags);
					CGUIWidgets::get().add_double_entry("Code integrity catalog", m_load_cfg_ci.m_catalog);
					CGUIWidgets::get().add_double_entry("Code integrity catalog offset", m_load_cfg_ci.m_catalog_offs);

					CGUIWidgets::get().add_double_entry("Control Flow Guard address taken IAT table VA", m_load_cfg_guard_addr_taken_iat_entry_table_va);
					CGUIWidgets::get().add_double_entry("Control Flow Guard address taken IAT table entries", m_load_cfg_guard_addr_taken_iat_entry_table_entries);
					CGUIWidgets::get().add_double_entry("Control Flow Guard long jump table VA", m_load_cfg_guard_long_jump_target_table_va);
					CGUIWidgets::get().add_double_entry("Control Flow Guard long jump table entries", m_load_cfg_guard_long_jump_target_table_entries);
					CGUIWidgets::get().add_double_entry("Dynamic value relocation table VA", m_load_cfg_dynamic_value_reloc_table_va);
					CGUIWidgets::get().add_double_entry("CHPE metadata", m_load_cfg_chpe_metadata);
					CGUIWidgets::get().add_double_entry("Guard RF failuire routine VA", m_load_cfg_guard_rf_failure_routine_va);
					CGUIWidgets::get().add_double_entry("Guard RF failuire routine function pointer VA", m_load_cfg_guard_rf_failure_routine_func_ptr_va);
					CGUIWidgets::get().add_double_entry("Dynamic value relocation table offset", m_load_cfg_dynamic_value_reloc_table_offs);
					CGUIWidgets::get().add_double_entry("Dynamic value relocation table section", m_load_cfg_dynamic_value_reloc_table_section);
					CGUIWidgets::get().add_double_entry("Guard RF verify stack pointer function pointer VA", m_load_cfg_guard_rf_verify_stack_ptr_func_ptr_va);
					CGUIWidgets::get().add_double_entry("Hot patch table offset", m_load_cfg_hot_patch_table_offs);
					CGUIWidgets::get().add_double_entry("Enclave config pointer VA", m_load_cfg_enclave_config_ptr_va);
					CGUIWidgets::get().add_double_entry("Volatile metadata pointer VA", m_load_cfg_volatile_metadata_pointer_va);
					CGUIWidgets::get().add_double_entry("Guard EH continuation table VA", m_load_cfg_guard_eh_continuation_table_va);
					CGUIWidgets::get().add_double_entry("Guard EH continuation table entries", m_load_cfg_guard_eh_continuation_table_entries);
				});
		});
}

void CX86PEProcessor::render_tab_strings()
{
	render_content_tab(
		"Strings",
		!m_image_strings.empty(),
		[&]()
		{
			// We use clipping in order to just process what is in the view however, 
			// this approach is very fast but we cannot filter through the whole list.
			static bool filterable = false;
			static ImGuiTextFilter filter;

			ImGui::BeginChild("Strings_child_up", { 0, filterable ? 85.f : 60.f });
			{
				CGUIWidgets::get().add_undecorated_simple_table(
					"Strings_child_up_table", 2,
					[&]()
					{
						CGUIWidgets::get().add_double_entry_dec("Amount of strings", m_image_strings.size());
						CGUIWidgets::get().add_double_entry("Sections", m_image_strings_found_in);
						CGUIWidgets::get().add_double_entry_checkbox("Enable search (slow)", &filterable);
					});

				if (filterable)
				{
					filter.Draw("Filter by a string");
				}
			}
			ImGui::EndChild();

			ImGui::Separator();

			ImGui::BeginChild("Strings_child_low", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);
			{
				CGUIWidgets::get().add_table(
					"Strings_child_up_table", 3,
					ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_BordersOuter,
					[&]()
					{
						CGUIWidgets::get().table_setup_column_fixed_width("VA", 70.f);
						CGUIWidgets::get().table_setup_column_fixed_width("Section", 60.f);
						CGUIWidgets::get().table_setup_column("String");
						ImGui::TableSetupScrollFreeze(0, 1);

						ImGui::TableHeadersRow();
					},
					[&]()
					{
						const auto render_string_column = [](const ImageString& str)
						{
							ImGui::TableNextColumn(); CGUIWidgets::get().add_copyable_selectable(str.m_va.as_string().c_str(), str.m_string);
							ImGui::TableNextColumn(); ImGui::TextUnformatted(str.m_parent_sec_name.c_str());
							ImGui::TableNextColumn(); ImGui::TextUnformatted(str.m_string.c_str());
						};

						// If the user wants to search through the list of strings, we have to disable
						// clipping.
						if (filterable)
						{
							for (const auto& str : m_image_strings)
							{
								if (!filter.PassFilter(str.m_string.c_str()))
									continue;

								render_string_column(str);
							}
						}
						else
						{
							ImGuiListClipper clipper;
							clipper.Begin(m_image_strings.size());
							while (clipper.Step())
							{
								for (uint32_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
								{
									const auto& str = m_image_strings[i];
									render_string_column(str);
								}
							}
						}
					});

			}
			ImGui::EndChild();
		});
}

void CX86PEProcessor::render_tab_misc()
{
	render_content_tab(
		"Misc",
		true, // Always exist
		[&]()
		{
			CGUIWidgets::get().add_centered_text("TLS data", CGUIWidgets::get().apply_imgui_window_x_padding(ImGui::GetColumnWidth(1)));
			ImGui::BeginChild("Misc_child_tls", { 0, 200 }, true, ImGuiWindowFlags_HorizontalScrollbar);
			{
				if (image_has_tls())
				{
					CGUIWidgets::get().add_undecorated_simple_table(
						"Misc_child_tls_table", 2,
						[&]()
						{
							CGUIWidgets::get().add_centered_text("TLS data", CGUIWidgets::get().apply_imgui_window_x_padding(ImGui::GetColumnWidth(1)));

							CGUIWidgets::get().add_double_entry("Raw data start", m_tls_raw_data_start_va);
							CGUIWidgets::get().add_double_entry("Raw data end", m_tls_raw_data_end_va);
							CGUIWidgets::get().add_double_entry("Raw data size", m_tls_raw_data_size);
							CGUIWidgets::get().add_double_entry("TLS index VA", m_tls_index_va);
							CGUIWidgets::get().add_double_entry("Size of zero fill", m_tls_size_of_zero_fill);
							CGUIWidgets::get().add_double_entry("TLS callbacks VA", m_tls_base_of_callbacks_va);
						});

					if (ImGui::TreeNodeEx(std::format("Callback functions ({})", m_tls_callbacks_va.size()).c_str(),
										  ImGuiTreeNodeFlags_SpanFullWidth))
					{
						CGUIWidgets::get().add_undecorated_simple_table(
							"Misc_child_tls_table_callbacks", 2,
							[&]()
							{
								uint32_t n = 0;
								for (const auto& func : m_tls_callbacks_va)
								{
									CGUIWidgets::get().add_double_entry(std::format("Callback VA #{:02}", n), func);

									n++;
								}
							});

						ImGui::TreePop();
					}
				}
				else
				{
					CGUIWidgets::get().add_window_centered_disabled_text("No data");
				}
			}
			ImGui::EndChild();

			CGUIWidgets::get().add_centered_text("Global ptr", CGUIWidgets::get().apply_imgui_window_x_padding(ImGui::GetColumnWidth(1)));
			ImGui::BeginChild("Misc_child_globalptr", { 0, 40 }, true, ImGuiWindowFlags_HorizontalScrollbar);
			{
				if (m_global_ptr_register)
				{
					CGUIWidgets::get().add_undecorated_simple_table(
						"Misc_child_globalptr_table", 2,
						[&]()
						{
							CGUIWidgets::get().add_double_entry("Global pointer register", m_global_ptr_register);
						});
				}
				else
				{
					CGUIWidgets::get().add_window_centered_disabled_text("No data");
				}
			}
			ImGui::EndChild();
		});
}

void CX86PEProcessor::render_content_tab(const std::string& label, bool does_exist, const std::function<void()>& pfn_contents)
{
	if (!does_exist)
		return;

	if (ImGui::BeginTabItem(label.c_str()))
	{
		ImGui::BeginChild(label.c_str(), { 0, 0 }, true, ImGuiWindowFlags_HorizontalScrollbar);
		if (pfn_contents)
			pfn_contents();

		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}

template<typename T>
void CX86PEProcessor::render_named_bitfield_constant_generic(const T& flags, const std::string& label, const char* child, uint32_t& active_flags_static)
{
	static uint32_t active_procflags = 0;
	CGUIWidgets::get().add_double_entry_hex_hoverable(
		label, flags.get_val(),
		[&]()
		{
			ImGui::BeginChild(child, {400.f, 17 * (float)active_flags_static });
			{
				active_flags_static = 0;
				for (uint32_t i = 0; i < flags.get_size_bits(); i++)
				{
					// True if current bit in the bitfield is set
					if (!flags.is_bit_present(i)) // Display only bits that are set
						continue;

					const auto& str = flags.get_string_at(i);
					ImGui::TextWrapped("%02d: %s", i, str.c_str());
					active_flags_static++;
				}
			}
			ImGui::EndChild();
		});
}

bool CX86PEProcessor::process_dos()
{
	auto pdos_hdr = m_byte_buffer.get_at<IMAGE_DOS_HEADER>(0);

	if (!process_dos_magic(pdos_hdr->e_magic))
		return false;

	m_number_of_pages = pdos_hdr->e_cp;

	// Size of DOS header including the stub
	m_dos_headers_size = pdos_hdr->e_lfanew;

	if (!m_dos_headers_size || m_dos_headers_size < sizeof(IMAGE_DOS_HEADER))
	{
		CDebugConsole::get().output_message(std::format("Invalid PE header offset: {}", m_dos_headers_size.val()));
		return false;
	}

	// Offset of dos stub is right after the header
	uint32_t dos_hdr_size = sizeof(IMAGE_DOS_HEADER);
	CDebugConsole::get().output_message(std::format("Sizeof DOS header: {} bytes", dos_hdr_size));

	uint32_t dos_stub_size = m_dos_headers_size - sizeof(IMAGE_DOS_HEADER);
	CDebugConsole::get().output_message(std::format("Sizeof DOS stub: {} bytes", dos_stub_size));

	process_dos_stub(dos_hdr_size, dos_stub_size);

	CDebugConsole::get().output_message("DOS header processed");

	return true;
}

bool CX86PEProcessor::process_dos_magic(uint16_t magic)
{
	m_dos_magic = { magic, std::format("0x{:04X}", magic) };

	switch (magic)
	{
		case IMAGE_DOS_SIGNATURE:
			m_dos_magic = { IMAGE_DOS_SIGNATURE, "MZ" };
			break;
		case IMAGE_OS2_SIGNATURE:
			m_dos_magic = { IMAGE_OS2_SIGNATURE, "NE" };
			break;
		case IMAGE_OS2_SIGNATURE_LE:
			m_dos_magic = { IMAGE_OS2_SIGNATURE_LE, "LE" };
			break;
		default:
			m_dos_magic = { magic, std::format("0x{:04X}", magic) };
			CDebugConsole::get().output_error(std::format("Unknown DOS magic: {}", m_dos_magic.name()));
			return false;
	}

	CDebugConsole::get().output_message(std::format("DOS header: {}", m_dos_magic.name()));
	return true;
}

void CX86PEProcessor::process_dos_stub(uint32_t off, uint32_t size)
{
	if (!m_dos_stub_byte_buffer.create(m_byte_buffer.get_raw() + off, size))
		return;

	m_dos_stub_size = m_dos_stub_byte_buffer.get_size();

	CDebugConsole::get().output_message("DOS stub bytes: ");
	CDebugConsole::get().output_message("", false);
	CDebugConsole::get().push_no_timestamp();

	for (uint32_t i = 0; i < m_dos_stub_size; i++)
	{
		uint8_t byte = *m_dos_stub_byte_buffer.get_at<uint8_t>(i);

		CDebugConsole::get().output_message(std::format("{:02X} ", byte), false);
	}

	CDebugConsole::get().output_newline();
	CDebugConsole::get().pop_no_timestamp();
}

bool CX86PEProcessor::process_nt_headers()
{
	auto pnt_hdrs = m_byte_buffer.get_at<IMAGE_NT_HEADERS>(m_dos_headers_size);

	if (!process_nt_sig(pnt_hdrs->Signature))
		return false;

	// File header
	if (!process_nt_file_hdr(&pnt_hdrs->FileHeader))
		return false;

	// Optional header
	if (!process_nt_opt_hdr(&pnt_hdrs->OptionalHeader))
		return false;

	CDebugConsole::get().output_message("NT headers processed");

	return true;
}

bool CX86PEProcessor::process_nt_sig(uint32_t sig)
{
	// AFAIK, all signatures we are aware of are 16-bit wide.
	uint16_t realsig = sig;

	switch (realsig)
	{
		case IMAGE_DOS_SIGNATURE: // TODO
			CDebugConsole::get().output_error("Unable to process IMAGE_DOS_SIGNATURE-type images YET.");
			return false;
		case IMAGE_OS2_SIGNATURE: // TODO
			CDebugConsole::get().output_error("Unable to process IMAGE_OS2_SIGNATURE-type images YET.");
			return false;
		case IMAGE_OS2_SIGNATURE_LE: // Or IMAGE_VXD_SIGNATURE, TODO
			CDebugConsole::get().output_error("Unable to process IMAGE_OS2_SIGNATURE_LE-type images YET.");
			return false;
		case IMAGE_NT_SIGNATURE:
			m_nt_sig = { IMAGE_NT_SIGNATURE, "PE00" };
			break;
		default:
			m_nt_sig = { realsig, std::format("0x{:04X}", realsig) };
			CDebugConsole::get().output_message(std::format("Invalid NT signature: 0x{:08X}", realsig));
			return false;
	}

	m_nt_sig = { IMAGE_NT_SIGNATURE, "PE00" };
	CDebugConsole::get().output_message(std::format("NT signature: {}", m_nt_sig.name()));

	return true;
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#coff-file-header-object-and-image
bool CX86PEProcessor::process_nt_file_hdr(const IMAGE_FILE_HEADER* pfile_hdr)
{
	// We won't fail on this if it's unrecognized
	process_nt_machine(pfile_hdr->Machine);

	if (!m_nt_machine.is(IMAGE_FILE_MACHINE_I386))
	{
		CDebugConsole::get().output_error("We can process only 32-bit images.");
		return false;
	}

	m_num_sections = pfile_hdr->NumberOfSections;

	if (m_num_sections >= max_number_of_sections())
	{
		CDebugConsole::get().output_error(std::format("Number of sections exceeds the limit! sec={} limit{}", m_num_sections, max_number_of_sections()));
		return false;
	}

	m_nt_time_date_stamp = pfile_hdr->TimeDateStamp;

	if (!m_nt_time_date_stamp.is_valid())
		CDebugConsole::get().output_info("Got null NT file header creation timestamp");

	// These should be zero according to msdn
	if (pfile_hdr->NumberOfSymbols != NULL || pfile_hdr->PointerToSymbolTable != NULL)
		CDebugConsole::get().output_info("Warning, symbol fields inside file header aren't NULL.");

	// This is critical and must match with our version
	if (pfile_hdr->SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER))
	{
		CDebugConsole::get().output_error(std::format("Mismatch in size of optional header: our={} img={}", sizeof(IMAGE_OPTIONAL_HEADER), pfile_hdr->SizeOfOptionalHeader));
		return false;
	}

	process_nt_characteristics(pfile_hdr->Characteristics);

	CDebugConsole::get().output_message("NT file header processed");

	return true;
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#optional-header-windows-specific-fields-image-only
bool CX86PEProcessor::process_nt_opt_hdr(const IMAGE_OPTIONAL_HEADER* popt_hdr)
{
	if (!process_nt_magic(popt_hdr->Magic))
		return false;

	m_img_base = popt_hdr->ImageBase;

	// Image base must be 64K aligned
	if (!m_img_base.is_aligned(0x10000))
	{
		CDebugConsole::get().output_error(std::format("Image base address isn't 64K aligned: {}", m_img_base.as_string()));
		return false;
	}

	CDebugConsole::get().output_message(std::format("Image base address: {}", m_img_base.as_string()));

	// code
	m_code_base_va = rva_as_va(popt_hdr->BaseOfCode);
	m_code_size = popt_hdr->SizeOfCode;

	CDebugConsole::get().output_message(std::format("Size of code: {} bytes", m_code_size.val()));

	// data
	m_data_base_va = rva_as_va(popt_hdr->BaseOfData);
	m_uninitialized_data_size = popt_hdr->SizeOfUninitializedData;
	m_initialized_data_size = popt_hdr->SizeOfInitializedData;

	CDebugConsole::get().output_message(std::format("Size of data (init/uninit): {}/{} bytes", m_initialized_data_size.val(), m_uninitialized_data_size.val()));

	// entry point
	m_entry_point_va = rva_as_va(popt_hdr->AddressOfEntryPoint);

	CDebugConsole::get().output_message(std::format("Entry point VA: {}", m_entry_point_va.as_string()));

	m_section_alignment = popt_hdr->SectionAlignment;
	m_file_alignment = popt_hdr->FileAlignment;

	// File alignment must be power of two and between 512 & 64K
	if (!m_file_alignment.is_power_of_two() ||
		(m_file_alignment < 0x200 && m_file_alignment > 0x10000))
	{
		CDebugConsole::get().output_error(std::format("File alignment must be power of two and between 512 and 64K inclusively. file align={}", m_file_alignment.as_string()));
		return false;
	}

	// Section alignment must be greater or equal to file alignment.
	if (m_section_alignment < m_file_alignment)
	{
		CDebugConsole::get().output_error(std::format("Section alignment must be greater than or equal to file alignment! sec align={} file align={}",
													  m_section_alignment.as_string(), m_file_alignment.as_string()));
		return false;
	}

	CDebugConsole::get().output_message(std::format("Section alignment: {} bytes", m_section_alignment.val()));
	CDebugConsole::get().output_message(std::format("File alignment: {} bytes", m_file_alignment.val()));

	m_size_of_image = popt_hdr->SizeOfImage;

	// Must be aligned to section alignment
	if (!m_size_of_image.is_aligned(m_section_alignment))
	{
		CDebugConsole::get().output_error(std::format("Size of the image must be section-alignment aligned. sizeof image={}", m_size_of_image.as_string()));
		return false;
	}

	CDebugConsole::get().output_message(std::format("Size of image: {}", m_size_of_image.as_string()));

	m_size_of_hdrs = popt_hdr->SizeOfHeaders;

	// Must be aligned to file alignment
	if (!m_size_of_hdrs.is_aligned(m_file_alignment))
	{
		CDebugConsole::get().output_error(std::format("Size of headers must be file-alignment aligned. sizeof image={}", m_size_of_hdrs.as_string()));
		return false;
	}

	CDebugConsole::get().output_message(std::format("Size of headers: {}", m_size_of_hdrs.as_string()));

	m_image_checksum_real = popt_hdr->CheckSum;
	m_image_checksum_calc = calculate_image_checksum();

	if (m_image_checksum_real != m_image_checksum_calc)
	{
		CDebugConsole::get().output_error(std::format("Calculated checksum doesn't match the real one. real={} calc={}", m_image_checksum_real.as_string(), m_image_checksum_calc.as_string()));
		// This isn't critical, we don't have to return from this.
	}

	CDebugConsole::get().output_message(std::format("Image CheckSum: {}", m_image_checksum_real.as_string()));

	process_nt_subsystem(popt_hdr->Subsystem);

	process_nt_dll_characteristics(popt_hdr->DllCharacteristics);

	if (!process_sections())
		return false;

	if (!process_nt_data_dir_entries(popt_hdr))
		return false;

	process_nt_data_directories();

	return true;
}

void CX86PEProcessor::process_nt_characteristics(uint16_t characs)
{
	m_nt_characteristics =
	{
		characs,
		{
			{ NULL, "None" },
			{ IMAGE_FILE_RELOCS_STRIPPED,			"No reloc" },
			{ IMAGE_FILE_EXECUTABLE_IMAGE,			"Executable" },
			{ IMAGE_FILE_LINE_NUMS_STRIPPED,		"No line numbers" },
			{ IMAGE_FILE_LOCAL_SYMS_STRIPPED,		"No symbols" },
			{ IMAGE_FILE_AGGRESIVE_WS_TRIM,			"Aggressively trim working set" },
			{ IMAGE_FILE_LARGE_ADDRESS_AWARE,		">2gb addrs" },
			{ IMAGE_FILE_BYTES_REVERSED_LO,			"Machine word bytes reversed low" },
			{ IMAGE_FILE_32BIT_MACHINE,				"32-bit word machine" },
			{ IMAGE_FILE_DEBUG_STRIPPED,			"No debug (inside .DBG file instead)" },
			{ IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP,	"If on removable media, copy & run from the swap file" },
			{ IMAGE_FILE_NET_RUN_FROM_SWAP,			"If on Net, copy & run from the swap file" },
			{ IMAGE_FILE_SYSTEM,					"System File" },
			{ IMAGE_FILE_DLL,						"DLL" },
			{ IMAGE_FILE_UP_SYSTEM_ONLY,			"Should only be run on a UP machine" },
			{ IMAGE_FILE_BYTES_REVERSED_HI,			"Machine word bytes are reversed high" },
		}
	};

	CDebugConsole::get().output_message(std::format("NT characteristics: {}", m_nt_characteristics.as_continuous_string()));
}

bool CX86PEProcessor::process_nt_data_dir_entries(const IMAGE_OPTIONAL_HEADER* popt_hdr)
{
	m_number_of_data_directories = popt_hdr->NumberOfRvaAndSizes;

	if (!m_number_of_data_directories)
	{
		CDebugConsole::get().output_error(std::format("Image without data directories!"));
		return false;
	}

	if (m_number_of_data_directories > IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
	{
		CDebugConsole::get().output_error(std::format("Image has more data directories than expected. got={} max={}", m_number_of_data_directories, IMAGE_NUMBEROF_DIRECTORY_ENTRIES));
		return false;
	}

	for (uint32_t i = 0; i < m_number_of_data_directories; i++)
	{
		ImageDataDir datadir;
		auto pdir_entry = const_cast<IMAGE_DATA_DIRECTORY*>(&popt_hdr->DataDirectory[i]);

		datadir.m_name = data_dir_name_by_idx(i);
		datadir.m_address = pdir_entry->VirtualAddress;
		datadir.m_size = pdir_entry->Size;

		CDebugConsole::get().output_message(std::format("#{:02d} {:<24} present={}", i, datadir.m_name, datadir.is_present() ? "yes" : "no"));
		m_data_dirs[i] = datadir;
	}

	CDebugConsole::get().output_message("Processed all data directory entries");

	return true;
}

void CX86PEProcessor::process_nt_data_directories()
{
	auto process_data_dir = [&](CX86PEProcessor* pobj, void(CX86PEProcessor::*process_func)(const ImageDataDir&), uint32_t i)
	{
		const auto& ddir = m_data_dirs[i];

		if (!ddir.is_present())
			return;

		CDebugConsole::get().output_message(std::format("Processing '{}'", ddir.m_name));

		(pobj->*process_func)(ddir);

		CDebugConsole::get().output_message(std::format("Processed '{}' data directory", ddir.m_name));
	};

	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_exports, IMAGE_DIRECTORY_ENTRY_EXPORT);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_imports, IMAGE_DIRECTORY_ENTRY_IMPORT);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_resources, IMAGE_DIRECTORY_ENTRY_RESOURCE);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_exceptions, IMAGE_DIRECTORY_ENTRY_EXCEPTION);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_certificates, IMAGE_DIRECTORY_ENTRY_SECURITY);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_relocs, IMAGE_DIRECTORY_ENTRY_BASERELOC);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_debug, IMAGE_DIRECTORY_ENTRY_DEBUG);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_arch, IMAGE_DIRECTORY_ENTRY_ARCHITECTURE);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_global_ptr, IMAGE_DIRECTORY_ENTRY_GLOBALPTR);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_tls, IMAGE_DIRECTORY_ENTRY_TLS);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_load_cfg, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_bound_import, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_iat, IMAGE_DIRECTORY_ENTRY_IAT);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_delay_import, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
	process_data_dir(this, &CX86PEProcessor::process_nt_data_dir_clr_runtime, IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

	// We get the function names/ordinals from the directory import entry however, 
	// addresses of individual imports are from the iat directory entry. Thus, we 
	// have to post-assign each import a corresponding address.
	//assign_addresses_to_imports();

	CDebugConsole::get().output_message("Processed all data directories");
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#export-directory-table
void CX86PEProcessor::process_nt_data_dir_exports(const ImageDataDir& dir_entry)
{
	auto ied = m_byte_buffer.get_at<IMAGE_EXPORT_DIRECTORY>(offset_relative_to_a_section(dir_entry.m_address));

	// Must be zero, this is reserved
	if (ied->Characteristics != 0)
	{
		CDebugConsole::get().output_error(std::format("Characteristics must be zero ({})", ied->Characteristics));
		return;
	}

	m_number_of_exported_functions = ied->NumberOfFunctions;
	m_number_of_exported_names = ied->NumberOfNames;

	if (!m_number_of_exported_names || !m_number_of_exported_functions)
	{
		CDebugConsole::get().output_error("Image doesn't have any exports but the directory exists");
		return;
	}

	// These two should be the same afaik. We're using the number of names further below.
	if (m_number_of_exported_names != m_number_of_exported_functions)
		CDebugConsole::get().output_info(std::format("Number of exported names & functions doesn't match. names={} functions={}", m_number_of_exported_names, m_number_of_exported_functions));

	m_export_starting_ordinal_num = ied->Base;

	// This is the base ordinal used inside the export address table. Should be 1 by default.
	if (m_export_starting_ordinal_num != 1)
	{
		CDebugConsole::get().output_info(std::format("Starting ordinal base should be 1, however it is: {}", m_export_starting_ordinal_num.val()));
	}

	m_export_dll_name = reinterpret_cast<const char*>(m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(ied->Name)));

	// This is the name of the dll (should be the name of processed file)
	if (m_export_dll_name.empty())
	{
		CDebugConsole::get().output_info("Export directory has no DLL name.");
	}

	CDebugConsole::get().output_message(std::format("Image has {} exports", m_number_of_exported_names));

	m_export_creation_timestamp = ied->TimeDateStamp;

	auto function_table_base = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(ied->AddressOfFunctions));
	auto name_table_base = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(ied->AddressOfNames));
	auto ordinal_table_base = m_byte_buffer.get_at<uint16_t>(offset_relative_to_a_section(ied->AddressOfNameOrdinals));

	CDebugConsole::get().output_message("Processing all exported functions");

	for (uint32_t i = 0; i < m_number_of_exported_names; i++)
	{
		ImageExport ex;

		auto name_entry_addr = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(name_table_base[i]));

		ex.m_name = reinterpret_cast<const char*>(name_entry_addr);

		if (ex.m_name.empty())
			CDebugConsole::get().output_info(std::format("Warning, got export without name: #{}", i));

		// Note: the ordinal is plus the base however, when actually accessing an entry, you don't use it? WTF Microsoft??! Gave me a headache
		ex.m_ordinal = ordinal_table_base[i] + m_export_starting_ordinal_num;
		ex.m_address = rva_as_va(function_table_base[ex.m_ordinal]);
		ex.is_forwarded = (function_table_base[ex.m_ordinal] >= dir_entry.m_address) && (function_table_base[ordinal_table_base[i]] < dir_entry.m_address + dir_entry.m_size);

		//if (!is_forwarded)
		//{
		//	ex.m_address = rva_as_va(function_table_base[ordinal_table_base[i]]);
		//	ex.m_forward_name.clear();
		//	CDebugConsole::get().output_message(std::format("[{}] (forwarded: no) {} ({})", 
		//													ex.m_ordinal.as_string(), ex.m_address.as_string(), ex.m_name));
		//}
		//else
		//{
		//	ex.m_address = rva_as_va(function_table_base[ordinal_table_base[i]]);
		//	ex.m_forward_name = m_byte_buffer.get_at<const char>(offset_relative_to_a_section(function_table_base[ordinal_table_base[i]]));
		//	CDebugConsole::get().output_message(std::format("[{}] (forwarded: yes) {} ({}/{})",
		//													ex.m_ordinal.as_string(), ex.m_address.as_string(), ex.m_name, ex.m_forward_name));
		//}

		//SmartHexValue address = function_table_base[ordinal_table_base[i]];

		m_image_exports.emplace_back(ex);
	}

	CDebugConsole::get().output_message("Done!");
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#the-idata-section
void CX86PEProcessor::process_nt_data_dir_imports(const ImageDataDir& dir_entry)
{
	auto iid = m_byte_buffer.get_at<IMAGE_IMPORT_DESCRIPTOR>(offset_relative_to_a_section(dir_entry.m_address));

	// Look through all import descriptors, each having many 'import thunks'.
	// Single import descriptor represents a single image from where the imports are imported.
	// We know that we're at the end when we come across the final descriptor, which has all
	// of its fields set to zero.
	while (iid->OriginalFirstThunk != NULL)
	{
		ImageImportDescriptor import_descriptor;

		auto descriptor_name_addr = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(iid->Name));

		import_descriptor.m_image_name = reinterpret_cast<const char*>(descriptor_name_addr);

		// Original first thunk is rva for the first that contains IMAGE_IMPORT_BY_NAME.
		auto thunk_data = m_byte_buffer.get_at<IMAGE_THUNK_DATA>(offset_relative_to_a_section(iid->OriginalFirstThunk));

		// First thunk contains the file pointer to import addresses for this descriptor. (inside IAT)
		uint32_t descriptor_import_addrs_base_va = rva_as_va(iid->FirstThunk);

		// Loop through all descriptor thunks (imported functions) and process them
		while (thunk_data->u1.AddressOfData != NULL)
		{
			ImageImportThunk import_thunk;

			// Check if the most-significant ordinal bit is set, if yes, we're importing
			// by an ordinal, not by a function name. We have to handle this..
			if (IMAGE_SNAP_BY_ORDINAL(thunk_data->u1.Ordinal))
			{
				import_thunk.m_ordinal = IMAGE_ORDINAL(thunk_data->u1.Ordinal);
				import_thunk.m_name = "";
				import_thunk.m_hint = 0;
				m_number_of_import_thunk_ordinals++;
			}
			else
			{
				// We can get the name only when not snapped by an ordinal
				auto import_name = m_byte_buffer.get_at<IMAGE_IMPORT_BY_NAME>(offset_relative_to_a_section(thunk_data->u1.AddressOfData));

				import_thunk.m_ordinal = 0;
				import_thunk.m_name = import_name->Name;
				import_thunk.m_hint = import_name->Hint;
				m_number_of_import_thunk_names++;
			}

			import_thunk.m_import_va = descriptor_import_addrs_base_va;

			import_descriptor.m_import_thunks.emplace_back(import_thunk);

			thunk_data++;
			descriptor_import_addrs_base_va += sizeof(uint32_t);
		}

		// At the end of thunk data, the virtual address corresponds not to
		// an import va but to descriptor va.
		import_descriptor.m_descriptor_va = descriptor_import_addrs_base_va;

		m_image_import_descriptors.emplace_back(import_descriptor);

		m_number_of_import_descriptors++;
		iid++;
	}

	CDebugConsole::get().output_message(std::format("Found {} thunks inside {} import descriptors.",
													m_number_of_import_thunk_ordinals + m_number_of_import_thunk_names, m_number_of_import_descriptors));
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#the-rsrc-section
void CX86PEProcessor::process_nt_data_dir_resources(const ImageDataDir& dir_entry)
{
	return; // TODO
	uint32_t rsc_base = offset_relative_to_a_section(dir_entry.m_address);

	auto ird_type = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY>(rsc_base);
	auto irde_type = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY_ENTRY>(rsc_base + sizeof(IMAGE_RESOURCE_DIRECTORY));

	// First level - type id
	for (uint16_t i = 0; i < ird_type->NumberOfIdEntries + ird_type->NumberOfNamedEntries; i++)
	{
		if (irde_type->DataIsDirectory)
		{
			CDebugConsole::get().output_message(std::format("  Directory type"));

			auto ird_name = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY>(rsc_base + irde_type->OffsetToDirectory);
			auto irde_name = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY_ENTRY>(rsc_base + irde_type->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY));

			// Second level - name id
			for (uint16_t k = 0; k < ird_name->NumberOfIdEntries + ird_name->NumberOfNamedEntries; k++)
			{
				if (irde_name->DataIsDirectory)
				{
					CDebugConsole::get().output_message(std::format("    Directory name"));

					auto ird_lang = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY>(rsc_base + irde_name->OffsetToDirectory);
					auto irde_lang = m_byte_buffer.get_at<IMAGE_RESOURCE_DIRECTORY_ENTRY>(rsc_base + irde_name->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY));

					// Third level - lang id
					for (uint16_t j = 0; j < ird_lang->NumberOfIdEntries + ird_lang->NumberOfNamedEntries; j++)
					{
						if (irde_lang->DataIsDirectory)
						{
							CDebugConsole::get().output_message(std::format("      Directory lang"));
						}
						else
						{
							CDebugConsole::get().output_message(std::format("      Data lang 0x{:08X}", irde_lang->OffsetToData));
						}

						irde_lang++;
					}
				}
				else
				{
					CDebugConsole::get().output_message(std::format("    Data name 0x{:08X}", irde_name->OffsetToData));
				}

				irde_name++;
			}
		}
		else
		{
			CDebugConsole::get().output_message(std::format("  Data type 0x{:08X}", irde_type->OffsetToData));
		}

		irde_type++;
	}
}

void CX86PEProcessor::process_nt_data_dir_exceptions(const ImageDataDir& dir_entry)
{
	auto irfe = m_byte_buffer.get_at<IMAGE_RUNTIME_FUNCTION_ENTRY>(offset_relative_to_a_section(dir_entry.m_address));

	// TODO: Couldn't find an executable that has this.

	DebugBreak();
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-attribute-certificate-table-image-only
void CX86PEProcessor::process_nt_data_dir_certificates(const ImageDataDir& dir_entry)
{
	if (!dir_entry.m_size.is_aligned(8))
	{
		CDebugConsole::get().output_error(std::format("Cannot process image certificates because the size of data directory isn't"
													  "aligned on the 8-byte boundary. The list seems to be corrupted. "
													  "Missed by {} bytes", dir_entry.m_size % 8));
		return;
	}

	// Total certificate size aligned to a 8-byte boundary
	uint32_t cert_total_size = 0, num_certs = 0;
	while (dir_entry.m_size > cert_total_size)
	{
		// The relative virtual addr is a file offset.
		auto iwce = m_byte_buffer.get_at<WIN_CERTIFICATE>(dir_entry.m_address + cert_total_size);

		// Offset to next entry, aligned on a 8-byte boundary
		uint32_t next_cert_aligned = iwce->dwLength;

		ImageWinCertificate win_cert;

		win_cert.m_entry_length = iwce->dwLength;
		win_cert.m_cert_raw_data_va = rva_as_va(dir_entry.m_address + offsetof(WIN_CERTIFICATE, bCertificate));

		if (!win_cert.m_entry_length.is_aligned(8))
		{
			next_cert_aligned = (next_cert_aligned + 7) & ~7; // align to a 8-byte boundary
			CDebugConsole::get().output_info("Warning, the windows certificate entry length wasn't aligned to an 8-byte boundary. "
											 "Rounding up to that boundary.");
		}

		win_cert.process_cert_type(iwce->wCertificateType);
		win_cert.process_revision_version(iwce->wRevision);

		m_image_certificates.emplace_back(win_cert);

		cert_total_size += next_cert_aligned;
		num_certs++;
	}

	CDebugConsole::get().output_message(std::format("Processed {} image certificate{}", num_certs, num_certs > 1 ? "s" : ""));
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-reloc-section-image-only
void CX86PEProcessor::process_nt_data_dir_relocs(const ImageDataDir& dir_entry)
{
	// Extended structure that also contains the word at the end.
	struct IMAGE_BASE_RELOCATION_EXTENDED
	{
		IMAGE_BASE_RELOCATION	base_reloc;
		WORD					type[1];
	};

	auto offset = offset_relative_to_a_section(dir_entry.m_address);
	auto ibre = m_byte_buffer.get_at<IMAGE_BASE_RELOCATION_EXTENDED>(offset);

	if (!ibre->base_reloc.SizeOfBlock)
	{
		CDebugConsole::get().output_error("Base relocation block without size!");
		return;
	}

	uint32_t m_num_reloc_blocks = 0, read_bytes = 0;
	while (read_bytes < dir_entry.m_size)
	{
		ImageRelocationBlock block;

		block.m_page_rva = ibre->base_reloc.VirtualAddress;
		block.m_size = ibre->base_reloc.SizeOfBlock;

		if (!block.m_size || !block.m_page_rva)
			break;

		uint16_t* type = &ibre->type[0];

		// Start here and walk the whole type list.
		uint32_t off = sizeof(IMAGE_BASE_RELOCATION);
		while (off <= block.m_size)
		{
			ImageRelocationBlock::BlockInfo info;

			info.process_block_info_type(((*type) >> 12) && 0xF);
			info.m_offset = (*type) & 0x0FFF;

			block.m_block_info_entries.emplace_back(info);

			type++;
			off += sizeof(uint16_t);
		}

		m_relocation_blocks.emplace_back(block);

		read_bytes += ibre->base_reloc.SizeOfBlock;
		ibre = m_byte_buffer.get_at<IMAGE_BASE_RELOCATION_EXTENDED>(offset_relative_to_a_section(dir_entry.m_address + read_bytes));

		m_num_reloc_blocks++;
	}

	CDebugConsole::get().output_message(std::format("Found {} relocation block{}", m_num_reloc_blocks, m_num_reloc_blocks > 1 ? "s" : ""));
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-debug-section
void CX86PEProcessor::process_nt_data_dir_debug(const ImageDataDir& dir_entry)
{
	auto idd = m_byte_buffer.get_at<IMAGE_DEBUG_DIRECTORY>(offset_relative_to_a_section(dir_entry.m_address));


	if (idd->Characteristics != NULL)
		CDebugConsole::get().output_info(std::format("Warning, characteristics field inside debug directory should be null: 0x{:08X}", idd->Characteristics));

	uint32_t num_debug_dirs = dir_entry.m_size / sizeof(IMAGE_DEBUG_DIRECTORY);

	for (uint32_t i = 0; i < num_debug_dirs; i++)
	{
		ImageDebugDirectory debug;

		debug.m_timestamp = idd->TimeDateStamp;
		debug.m_major_ver = idd->MajorVersion;
		debug.m_minor_ver = idd->MinorVersion;

		debug.process_type(idd->Type);

		debug.m_size_of_data = idd->SizeOfData;
		debug.m_address_of_raw_data_va = rva_as_va(idd->AddressOfRawData);
		debug.m_file_pointer_to_raw_data = idd->PointerToRawData;

		// Process each debug directory type..

		auto process_type_data = [&](ImageDebugDirectory* pobj, void(ImageDebugDirectory::*process_func)(const ByteBuffer<uint32_t>&),
									 const std::string& name, uint32_t supposed_type)
		{
			if (supposed_type != idd->Type)
				return;

			CDebugConsole::get().output_message(std::format("Processing '{}' debug directory type data", name));

			(pobj->*process_func)(m_byte_buffer);

			CDebugConsole::get().output_message(std::format("Processed '{}' debug directory type data", name));
		};

		process_type_data(&debug, &ImageDebugDirectory::process_type_cv, "CodeView", IMAGE_DEBUG_TYPE_CODEVIEW);

		// Note: Idk if there's actually point in processing more types. As I've looked through the 
		//		 NT 5.1 source code, a majority of these types aren't process anywhere. The codeView
		//		 type makes the most sense to process, then the misc type, which is I assume for DBG
		//		 files only, and that'a but it. I've seen structures even for FPO however, is it really
		//		 necessary to implement it at all?

		m_debug_directories.emplace_back(debug);

		idd++;
	}

	CDebugConsole::get().output_message(std::format("Found {} data director{}", num_debug_dirs, num_debug_dirs > 1 ? "ies" : "y"));
}

void CX86PEProcessor::process_nt_data_dir_arch(const ImageDataDir& dir_entry)
{
	if (dir_entry.m_size != 0 || dir_entry.m_address != 0)
		CDebugConsole::get().output_info("Warning, architecture data directory fields should be zero!");
}

void CX86PEProcessor::process_nt_data_dir_global_ptr(const ImageDataDir& dir_entry)
{
	auto gptr_addr = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(dir_entry.m_address));

	m_global_ptr_register = reinterpret_cast<uint32_t>(gptr_addr);

	// TODO: Test on an executable

	CDebugConsole::get().output_message(std::format("The global pointer register value is {}", m_global_ptr_register.as_string()));
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-tls-section
// Tested on C:\\Windows\\SysWOW64\\aadtb.dll
void CX86PEProcessor::process_nt_data_dir_tls(const ImageDataDir& dir_entry)
{
	auto itd = m_byte_buffer.get_at<IMAGE_TLS_DIRECTORY>(offset_relative_to_a_section(dir_entry.m_address));

	m_tls_raw_data_start_va = itd->StartAddressOfRawData;
	m_tls_raw_data_end_va = itd->EndAddressOfRawData;
	m_tls_raw_data_size = m_tls_raw_data_end_va - m_tls_raw_data_start_va;
	m_tls_index_va = itd->AddressOfIndex;
	m_tls_size_of_zero_fill = itd->SizeOfZeroFill;
	m_tls_base_of_callbacks_va = itd->AddressOfCallBacks;

	uint32_t num_callbacks = 0;

	auto callback_pfn = m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(va_as_rva(m_tls_base_of_callbacks_va)));

	// The first entry is nulled. We have to offset to the second entry 
	// in order to find rvas.
	callback_pfn = reinterpret_cast<uint32_t*>((uint8_t*)callback_pfn + sizeof(uint32_t));

	while (true)
	{
		// So at the memory location there's a RVA. This RVA is
		// relative to the base address of an image, so adding
		// the base address to it locates the start of the callback
		// function.
		auto callback_rva = *(uint32_t*)callback_pfn;
		auto callback_va = rva_as_va(callback_rva);

		if (callback_rva == NULL)
			break; // End of the list right here.

		m_tls_callbacks_va.emplace_back(callback_va);

		//CDebugConsole::get().output_message(std::format("Got callback at 0x{:08X} (rva=0x{:08X})", callback_va, callback_rva));

		// There's a one byte gap between each entry. 
		callback_pfn = reinterpret_cast<uint32_t*>((uint8_t*)callback_pfn + sizeof(uint32_t) + sizeof(uint8_t));
		num_callbacks++;
	}

	CDebugConsole::get().output_message(std::format("Processed {} callback{}", num_callbacks, num_callbacks > 1 ? "s" : ""));
}

// Tested on C:\Windows\SysWOW64\bcd.dll
void CX86PEProcessor::process_nt_data_dir_load_cfg(const ImageDataDir& dir_entry)
{
	auto ilcd = m_byte_buffer.get_at<IMAGE_LOAD_CONFIG_DIRECTORY>(offset_relative_to_a_section(dir_entry.m_address));

	if (ilcd->Size != sizeof(IMAGE_LOAD_CONFIG_DIRECTORY))
	{
		CDebugConsole::get().output_error(std::format("Size of load config directory isn't the same as ours. (ours 0x{:08X} but got 0x{:08X})", 
			sizeof(IMAGE_LOAD_CONFIG_DIRECTORY), ilcd->Size));
		return;
	}

	m_load_cfg_timestamp = ilcd->TimeDateStamp;
	m_load_cfg_global_flags_clear = ilcd->GlobalFlagsClear;
	m_load_cfg_global_flags_set = ilcd->GlobalFlagsSet;
	m_load_cfg_cs_default_timeout = ilcd->CriticalSectionDefaultTimeout;
	m_load_cfg_decommit_free_block_threshold = ilcd->DeCommitFreeBlockThreshold;
	m_load_cfg_decommit_total_block_threshold = ilcd->DeCommitTotalFreeThreshold;
	m_load_cfg_lock_prefix_table_va = ilcd->LockPrefixTable;
	m_load_cfg_max_proc_heap_alloc_size = ilcd->MaximumAllocationSize;
	m_load_cfg_vm_threshold = ilcd->VirtualMemoryThreshold;

	process_load_cfg_process_heap_flags(ilcd->ProcessHeapFlags);

	m_load_cfg_process_affinity_mask = ilcd->ProcessAffinityMask;
	m_load_cfg_service_pack_ver_id = ilcd->CSDVersion;
	m_load_cfg_dependent_load_flags = ilcd->DependentLoadFlags;
	m_load_cfg_edit_list_va = ilcd->EditList;
	m_load_cfg_security_cookie_va = ilcd->SecurityCookie;
	m_load_cfg_seh_table_va = ilcd->SEHandlerTable;
	m_load_cfg_seh_handlers_count = ilcd->SEHandlerCount;
	m_load_cfg_guard_cfg_check_func_ptr_va = ilcd->GuardCFCheckFunctionPointer;
	m_load_cfg_guard_cfg_dispatch_func_ptr_va = ilcd->GuardCFDispatchFunctionPointer;
	m_load_cfg_guard_cfg_func_table_va = ilcd->GuardCFFunctionTable;
	m_load_cfg_guard_cfg_func_table_entries = ilcd->GuardCFFunctionCount;

	process_load_cfg_guard_flags(ilcd->GuardFlags);

	// Code integrity
	auto ilcci = &ilcd->CodeIntegrity;
	m_load_cfg_ci.m_flags = ilcci->Flags;
	m_load_cfg_ci.m_catalog = ilcci->Catalog;
	m_load_cfg_ci.m_catalog_offs = ilcci->CatalogOffset;

	m_load_cfg_guard_addr_taken_iat_entry_table_va = ilcd->GuardAddressTakenIatEntryTable;
	m_load_cfg_guard_addr_taken_iat_entry_table_entries = ilcd->GuardAddressTakenIatEntryCount;
	m_load_cfg_guard_long_jump_target_table_va = ilcd->GuardLongJumpTargetTable;
	m_load_cfg_guard_long_jump_target_table_entries = ilcd->GuardLongJumpTargetCount;

	m_load_cfg_dynamic_value_reloc_table_va = ilcd->DynamicValueRelocTable;
	m_load_cfg_chpe_metadata = ilcd->CHPEMetadataPointer;
	m_load_cfg_guard_rf_failure_routine_va = ilcd->GuardRFFailureRoutine;
	m_load_cfg_guard_rf_failure_routine_func_ptr_va = ilcd->GuardRFFailureRoutineFunctionPointer;
	m_load_cfg_dynamic_value_reloc_table_offs = ilcd->DynamicValueRelocTableOffset;
	m_load_cfg_dynamic_value_reloc_table_section = ilcd->DynamicValueRelocTableSection;

	if (ilcd->Reserved2 != 0)
	{
		CDebugConsole::get().output_info(std::format("Warning, 0x{:08X} field inside IMAGE_LOAD_CONFIG_DIRECTORY (Reserved2) should be zero. (val=0x{:08X})",
			offsetof(IMAGE_LOAD_CONFIG_DIRECTORY, Reserved2), ilcd->Reserved2));
	}

	m_load_cfg_guard_rf_verify_stack_ptr_func_ptr_va = ilcd->GuardRFVerifyStackPointerFunctionPointer;
	m_load_cfg_hot_patch_table_offs = ilcd->HotPatchTableOffset;

	if (ilcd->Reserved3 != 0)
	{
		CDebugConsole::get().output_info(std::format("Warning, 0x{:08X} field inside IMAGE_LOAD_CONFIG_DIRECTORY (Reserved3) should be zero. (val=0x{:08X})",
			offsetof(IMAGE_LOAD_CONFIG_DIRECTORY, Reserved3), ilcd->Reserved3));
	}

	m_load_cfg_enclave_config_ptr_va = ilcd->EnclaveConfigurationPointer;
	m_load_cfg_volatile_metadata_pointer_va = ilcd->VolatileMetadataPointer;
	m_load_cfg_guard_eh_continuation_table_va = ilcd->GuardEHContinuationTable;
	m_load_cfg_guard_eh_continuation_table_entries = ilcd->GuardEHContinuationCount;
}

void CX86PEProcessor::process_nt_data_dir_bound_import(const ImageDataDir& dir_entry)
{
	// Unused in almost all images on the newer versions of the OS.. Probably no
	// need to process at all.

	DebugBreak();
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-address-table
void CX86PEProcessor::process_nt_data_dir_iat(const ImageDataDir& dir_entry)
{
	// Note: Theoretically, from our perspective, processing this is useless. 
	//		 We can get the same data from the import data directory.

	// The directory entry address is a file pointer to the start of the table
	// in this particular case..
	m_imports_iat_base = rva_as_va(dir_entry.m_address);

	uint32_t num_of_entries = dir_entry.m_size / sizeof(uint32_t);

	for (uint32_t i = 0; i < num_of_entries; i++)
	{
		ImageIATEntry iat_entry;

		iat_entry.m_va = m_imports_iat_base + (i * sizeof(uint32_t));

		if (!iat_entry.m_va)
			continue; // If the address is null, then it's most likely a descriptor entry.

		// Inside IAT, there are some regions between thunks and another descriptors where there aren't
		// any addresses. Basically sort of a padding or something. Ignore these.
		auto data_at_address = *m_byte_buffer.get_at<uint32_t>(offset_relative_to_a_section(dir_entry.m_address + (i * sizeof(uint32_t))));
		if (data_at_address == 0)
			continue;

		// set to true if we have found match inside the descriptor-thunk list.
		bool found_match = false;

		// Find the right import thunk by the rva.
		if (!m_image_import_descriptors.empty())
		{
			for (auto& descriptor : m_image_import_descriptors)
			{
				if (descriptor.has_thunks())
				{
					for (auto& thunk : descriptor.m_import_thunks)
					{
						if (thunk.m_import_va == iat_entry.m_va)
						{
							found_match = true;
							iat_entry.m_refered_thunk_or_descriptor_name = thunk.m_name;
						}
					}

					// If we still haven't found one, there's a possibility that
					// this va belongs to a descriptor, not to a thunk.
					if (!found_match)
					{
						if (iat_entry.m_va == descriptor.m_descriptor_va)
						{
							found_match = true;
							iat_entry.m_refered_thunk_or_descriptor_name = descriptor.m_image_name;
						}
					}
				}
				else
				{
					CDebugConsole::get().output_error(std::format("Can't assign iat entries to import thunks for {} because it has no thunks!", descriptor.m_image_name));
				}
			}
		}
		else
		{
			CDebugConsole::get().output_error("Can't assign iat entries to import thunks because there aren't any import descriptors!");
		}

		if (!found_match)
		{
			iat_entry.m_refered_thunk_or_descriptor_name = "unknown";
			CDebugConsole::get().output_error(std::format("Couldn't find a thunk for iat entry {}", iat_entry.m_va.as_string()));
		}

		m_iat_entries.emplace_back(iat_entry);
	}

	CDebugConsole::get().output_message(std::format("Processed {} IAT entries", num_of_entries));
}

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#delay-load-import-tables-image-only
void CX86PEProcessor::process_nt_data_dir_delay_import(const ImageDataDir& dir_entry)
{
	auto idd = m_byte_buffer.get_at<ImgDelayDescr>(offset_relative_to_a_section(dir_entry.m_address));

	while (idd->rvaINT != NULL)
	{
		ImageDelayedImportDescriptor import_descriptor;

		if (idd->grAttrs & dlattrRva)
			import_descriptor.m_image_name = m_byte_buffer.get_at<const char>(offset_relative_to_a_section(idd->rvaDLLName));
		else
			import_descriptor.m_image_name = m_byte_buffer.get_at<const char>(offset_relative_to_a_section(va_as_rva(idd->rvaDLLName)));

		PIMAGE_THUNK_DATA thunk_data;

		if (idd->grAttrs & dlattrRva)
			thunk_data = m_byte_buffer.get_at<IMAGE_THUNK_DATA>(offset_relative_to_a_section(idd->rvaINT));
		else
			thunk_data = m_byte_buffer.get_at<IMAGE_THUNK_DATA>(offset_relative_to_a_section(va_as_rva(idd->rvaINT)));

		import_descriptor.m_timestamp = idd->dwTimeStamp;

		// Base address of the iat for this delayed import descriptor.
		uint32_t descriptor_import_addrs_base_va = rva_as_va(idd->rvaIAT);

		// Loop through all descriptor thunks (imported functions) and process them
		while (thunk_data->u1.AddressOfData != NULL)
		{
			ImageImportThunk import_thunk;

			// Check if the most-significant ordinal bit is set, if yes, we're importing
			// by an ordinal, not by a function name. We have to handle this..
			if (IMAGE_SNAP_BY_ORDINAL(thunk_data->u1.Ordinal))
			{
				import_thunk.m_ordinal = IMAGE_ORDINAL(thunk_data->u1.Ordinal);
				import_thunk.m_name = "";
				import_thunk.m_hint = 0;
				m_number_of_delayed_import_thunk_ordinals++;
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME import_name;

				// We search for name only when thunk isn't snapped by an ordinal.
				if (idd->grAttrs & dlattrRva)
					import_name = m_byte_buffer.get_at<IMAGE_IMPORT_BY_NAME>(offset_relative_to_a_section(thunk_data->u1.AddressOfData));
				else
					import_name = m_byte_buffer.get_at<IMAGE_IMPORT_BY_NAME>(offset_relative_to_a_section(va_as_rva(thunk_data->u1.AddressOfData)));

				import_thunk.m_ordinal = 0;
				import_thunk.m_name = import_name->Name;
				import_thunk.m_hint = import_name->Hint;
				m_number_of_delayed_import_thunk_names++;
			}

			// Get us an address for this specific thunk inside the IAT.
			import_thunk.m_import_va = descriptor_import_addrs_base_va;

			import_descriptor.m_import_thunks.emplace_back(import_thunk);

			thunk_data++;
			descriptor_import_addrs_base_va += sizeof(uint32_t);
		}

		// At the end of thunk data, the virtual address corresponds not to
		// an import va but to descriptor va.
		import_descriptor.m_descriptor_va = descriptor_import_addrs_base_va;

		m_image_delayed_import_descriptors.emplace_back(import_descriptor);
		m_number_of_delayed_import_descriptors++;

		idd++;
	}

	CDebugConsole::get().output_message(std::format("Found {} thunks inside {} delayed import descriptors.",
													m_number_of_delayed_import_thunk_ordinals + m_number_of_delayed_import_thunk_names,
													m_number_of_delayed_import_descriptors));
}

void CX86PEProcessor::process_nt_data_dir_clr_runtime(const ImageDataDir& dir_entry)
{
	DebugBreak();
}

void CX86PEProcessor::process_nt_machine(uint16_t machine)
{
	switch (machine)
	{
		case IMAGE_FILE_MACHINE_UNKNOWN: m_nt_machine = { IMAGE_FILE_MACHINE_UNKNOWN, "Unknown" }; break;
		case IMAGE_FILE_MACHINE_AM33: m_nt_machine = { IMAGE_FILE_MACHINE_AM33, "AM33" }; break;
		case IMAGE_FILE_MACHINE_AMD64: m_nt_machine = { IMAGE_FILE_MACHINE_AMD64, "AM64" }; break;
		case IMAGE_FILE_MACHINE_ARM: m_nt_machine = { IMAGE_FILE_MACHINE_ARM, "ARM" }; break;
		case IMAGE_FILE_MACHINE_ARM64: m_nt_machine = { IMAGE_FILE_MACHINE_ARM64, "ARM64" }; break;
		case IMAGE_FILE_MACHINE_ARMNT: m_nt_machine = { IMAGE_FILE_MACHINE_ARMNT, "ARMNT" }; break;
		case IMAGE_FILE_MACHINE_EBC: m_nt_machine = { IMAGE_FILE_MACHINE_EBC, "EBC" }; break;
		case IMAGE_FILE_MACHINE_I386: m_nt_machine = { IMAGE_FILE_MACHINE_I386, "I386" }; break;
		case IMAGE_FILE_MACHINE_IA64: m_nt_machine = { IMAGE_FILE_MACHINE_IA64, "IA64" }; break;
		case IMAGE_FILE_MACHINE_M32R: m_nt_machine = { IMAGE_FILE_MACHINE_M32R, "M32R" }; break;
		case IMAGE_FILE_MACHINE_MIPS16: m_nt_machine = { IMAGE_FILE_MACHINE_MIPS16, "MIPS16" }; break;
		case IMAGE_FILE_MACHINE_MIPSFPU: m_nt_machine = { IMAGE_FILE_MACHINE_MIPSFPU, "MIPSFPU" }; break;
		case IMAGE_FILE_MACHINE_MIPSFPU16: m_nt_machine = { IMAGE_FILE_MACHINE_MIPSFPU16, "MIPSFPU16" }; break;
		case IMAGE_FILE_MACHINE_POWERPC: m_nt_machine = { IMAGE_FILE_MACHINE_POWERPC, "POWERPC" }; break;
		case IMAGE_FILE_MACHINE_POWERPCFP: m_nt_machine = { IMAGE_FILE_MACHINE_POWERPCFP, "POWERPCFP" }; break;
		case IMAGE_FILE_MACHINE_R4000: m_nt_machine = { IMAGE_FILE_MACHINE_R4000, "R4000" }; break;
		case IMAGE_FILE_MACHINE_SH3: m_nt_machine = { IMAGE_FILE_MACHINE_SH3, "SH3" }; break;
		case IMAGE_FILE_MACHINE_SH3DSP: m_nt_machine = { IMAGE_FILE_MACHINE_SH3DSP, "SH3DSP" }; break;
		case IMAGE_FILE_MACHINE_SH5: m_nt_machine = { IMAGE_FILE_MACHINE_SH5, "SH5" }; break;
		case IMAGE_FILE_MACHINE_SH4: m_nt_machine = { IMAGE_FILE_MACHINE_SH4, "SH4" }; break;
		case IMAGE_FILE_MACHINE_THUMB: m_nt_machine = { IMAGE_FILE_MACHINE_THUMB, "THUMB" }; break;
		case IMAGE_FILE_MACHINE_WCEMIPSV2: m_nt_machine = { IMAGE_FILE_MACHINE_WCEMIPSV2, "WCEMIPSV2" }; break;
		case IMAGE_FILE_MACHINE_TARGET_HOST: m_nt_machine = { IMAGE_FILE_MACHINE_TARGET_HOST, "TARGET_HOST" }; break;
		case IMAGE_FILE_MACHINE_ALPHA: m_nt_machine = { IMAGE_FILE_MACHINE_ALPHA, "ALPHA" }; break;
		case IMAGE_FILE_MACHINE_TRICORE: m_nt_machine = { IMAGE_FILE_MACHINE_TRICORE, "TRICORE" }; break;
		case IMAGE_FILE_MACHINE_CEF: m_nt_machine = { IMAGE_FILE_MACHINE_CEF, "CEF" }; break;
		case IMAGE_FILE_MACHINE_CEE: m_nt_machine = { IMAGE_FILE_MACHINE_CEE, "CEE" }; break;

		default:
			m_nt_machine = { machine, std::format("0x{:04X}", machine) };
			CDebugConsole::get().output_error("Invalid machine type");
			break;
	}

	CDebugConsole::get().output_message(std::format("Machine type: {}", m_nt_machine.name()));
}

bool CX86PEProcessor::process_nt_magic(uint16_t magic)
{
	switch (magic)
	{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC: m_nt_magic = { IMAGE_NT_OPTIONAL_HDR32_MAGIC, "PE32" }; break;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC: m_nt_magic = { IMAGE_NT_OPTIONAL_HDR64_MAGIC, "PE32+" }; break;
		case IMAGE_ROM_OPTIONAL_HDR_MAGIC: m_nt_magic = { IMAGE_ROM_OPTIONAL_HDR_MAGIC, "ROM" }; break;

		default: // This is critical, we have to fail
			m_nt_magic = { magic, std::format("0x{:04X}", magic) };
			CDebugConsole::get().output_error(std::format("Invalid NT optional hdr magic: {}", m_nt_magic.name()));
			return false;
	}

	CDebugConsole::get().output_message(std::format("NT optional header magic: {}", m_nt_magic.name()));
	return true;
}

void CX86PEProcessor::process_nt_subsystem(uint32_t subsystem)
{
	switch (subsystem)
	{
		case IMAGE_SUBSYSTEM_UNKNOWN: m_nt_subsystem = { IMAGE_SUBSYSTEM_UNKNOWN, "Unknown" }; break;
		case IMAGE_SUBSYSTEM_NATIVE: m_nt_subsystem = { IMAGE_SUBSYSTEM_NATIVE, "NATIVE" }; break;
		case IMAGE_SUBSYSTEM_WINDOWS_GUI: m_nt_subsystem = { IMAGE_SUBSYSTEM_NATIVE, "GUI" }; break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI: m_nt_subsystem = { IMAGE_SUBSYSTEM_WINDOWS_CUI, "CUI" }; break;
		case IMAGE_SUBSYSTEM_OS2_CUI: m_nt_subsystem = { IMAGE_SUBSYSTEM_OS2_CUI, "OS2 CUI" }; break;
		case IMAGE_SUBSYSTEM_POSIX_CUI: m_nt_subsystem = { IMAGE_SUBSYSTEM_POSIX_CUI, "POSIX CUI" }; break;
		case IMAGE_SUBSYSTEM_NATIVE_WINDOWS: m_nt_subsystem = { IMAGE_SUBSYSTEM_NATIVE_WINDOWS, "NATIVE WINDOWS" }; break;
		case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI: m_nt_subsystem = { IMAGE_SUBSYSTEM_WINDOWS_CE_GUI, "CE GUI" }; break;
		case IMAGE_SUBSYSTEM_EFI_APPLICATION: m_nt_subsystem = { IMAGE_SUBSYSTEM_EFI_APPLICATION, "EFI APPLICATION" }; break;
		case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER: m_nt_subsystem = { IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER, "BOOT SERVICE DRIVER" }; break;
		case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER: m_nt_subsystem = { IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER, "EFI RUNTIME DRIVER" }; break;
		case IMAGE_SUBSYSTEM_EFI_ROM: m_nt_subsystem = { IMAGE_SUBSYSTEM_EFI_ROM, "EFI ROM" }; break;
		case IMAGE_SUBSYSTEM_XBOX: m_nt_subsystem = { IMAGE_SUBSYSTEM_XBOX, "XBOX" }; break;
		case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION: m_nt_subsystem = { IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION, "BOOT APP" }; break;

		default:
			m_nt_subsystem = { subsystem, std::format("0x{:08X}", subsystem) };
			CDebugConsole::get().output_error("Invalid subsystem");
			break;
	}

	CDebugConsole::get().output_message(std::format("Subsystem is: {}", m_nt_subsystem.name()));
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#dll-characteristics
void CX86PEProcessor::process_nt_dll_characteristics(uint16_t dll_characteristics)
{
	m_nt_dll_characteristics =
	{
		dll_characteristics,
		{
			{ IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA,			"High entropy 64-bit VA space" },
			{ IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE,			"DLL with dynamic base" },
			{ IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY,			"Code integrity" },
			{ IMAGE_DLLCHARACTERISTICS_NX_COMPAT,				"NX compatible" },
			{ IMAGE_DLLCHARACTERISTICS_NO_ISOLATION,			"Image can't afford isolation" },
			{ IMAGE_DLLCHARACTERISTICS_NO_SEH,					"No SE handler" },
			{ IMAGE_DLLCHARACTERISTICS_NO_BIND,					"Not bindable" },
			{ IMAGE_DLLCHARACTERISTICS_APPCONTAINER,			"App container executable" },
			{ IMAGE_DLLCHARACTERISTICS_WDM_DRIVER,				"WDM driver model" },
			{ IMAGE_DLLCHARACTERISTICS_GUARD_CF,				"Control Flow Guard support" },
			{ IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE,	"Terminal server aware" },
		}
	};

	// First four must be zero, they're reserved.
	if (m_nt_dll_characteristics.are_any_bits_present(0, 3))
	{
		CDebugConsole::get().output_info(std::format("Warning, reserved bits inside DLL characteristics are set. They should be zero."));
	}

	CDebugConsole::get().output_message(std::format("DLL characteristics: {}", m_nt_dll_characteristics.as_continuous_string()));
}

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#section-table-section-headers
bool CX86PEProcessor::process_sections()
{
	if (sizeof(IMAGE_SECTION_HEADER) != IMAGE_SIZEOF_SECTION_HEADER)
	{
		CDebugConsole::get().output_error(std::format("Section header size missmatch. our={} should be={}", sizeof(IMAGE_SECTION_HEADER), IMAGE_SIZEOF_SECTION_HEADER));
		return false;
	}

	auto pnt_hdrs = m_byte_buffer.get_at<IMAGE_NT_HEADERS>(m_dos_headers_size);

	auto img_sec_hdr = IMAGE_FIRST_SECTION(pnt_hdrs);

	for (uint32_t i = 0; i < m_num_sections; i++)
	{
		ImageSection sec;

		sec.m_name = (const char*)img_sec_hdr->Name;

		if (sec.m_name.empty())
		{
			CDebugConsole::get().output_error("Empty section name!");
			return false;
		}

		sec.m_va_size = img_sec_hdr->Misc.VirtualSize;
		sec.m_va_base = rva_as_va(img_sec_hdr->VirtualAddress); // It's RVA in reality
		sec.m_raw_data_ptr = img_sec_hdr->PointerToRawData;
		sec.m_raw_data_size = img_sec_hdr->SizeOfRawData;

		if (sec.m_raw_data_size && sec.m_raw_data_ptr)
		{
			// Raw data size must be a multiple of file aligment
			if (!sec.m_raw_data_size.is_aligned(m_file_alignment))
			{
				CDebugConsole::get().output_error(std::format("Raw data size isn't {}-byte aligned. ({})", m_file_alignment.val(), sec.m_raw_data_size.val()));
				return false;
			}

			// pointer to raw size should be a multiple of file aligment
			if (!sec.m_raw_data_ptr.is_aligned(0x4))
			{
				CDebugConsole::get().output_info(std::format("Pointer to raw data should be 4-byte aligned. ({})", sec.m_raw_data_ptr.val()));
				return false;
			}

			int32_t zero_fill = (int32_t)sec.m_va_size - (int32_t)sec.m_raw_data_size;

			// If VA size is greater than raw data size, the remainder bytes of the section
			// are filled with zeroed padding. This is fine.
			if (zero_fill > 0)
			{
				sec.m_zero_padding = zero_fill;

				if (sec.m_zero_padding > 0)
				{
					CDebugConsole::get().output_message(std::format("Section's VA size is greater than raw data size. "
						"There's {}-byte zero padding at end. ({} > {})",
						sec.m_zero_padding.val(),
						sec.m_va_size.val(), sec.m_raw_data_size.val()));
				}
			}
		}
		else
		{
			// Can happen that these are NULL.
			CDebugConsole::get().output_info("Warning, null raw data.");
		}

		sec.m_line_numbers_ptr = img_sec_hdr->PointerToLinenumbers;
		sec.m_line_numbers_num = img_sec_hdr->NumberOfLinenumbers;

		// These should be zero for the executable image
		if (sec.m_line_numbers_ptr != 0 || sec.m_line_numbers_num != 0)
		{
			CDebugConsole::get().output_info(std::format("Line numbers pointer and number of line numbers should be both 0 for an executable image. "
														 "(ptr={} num={})",
														 sec.m_line_numbers_ptr.as_string(),
														 sec.m_line_numbers_num.val()));
		}

		sec.m_relocs_ptr = img_sec_hdr->PointerToRelocations;
		sec.m_relocs_num = img_sec_hdr->NumberOfRelocations;

		sec.process_characteristics(img_sec_hdr->Characteristics);

		CDebugConsole::get().output_message(std::format("section #{}: {:<{}}", i, sec.m_name, IMAGE_SIZEOF_SHORT_NAME));

		m_sections[sec.m_name] = sec;

		img_sec_hdr++;
	}

	return true;
}

void CX86PEProcessor::process_image_strings()
{
	auto is_ascii_fine = [](char c)
	{
		return c >= ' ' && c <= '~';
	};

	// Each section has it's policy of searching for strings in it.
	struct SectionSearchPolicy
	{
		char		m_section_name[IMAGE_SIZEOF_SHORT_NAME];
		uint32_t	min_len_tolerance; // Minimal string length that we will save for this section.
	};

	// Each section has it's minimal string length tolerance which we
	// then use when searching through partiral section.
	constexpr static const std::array<SectionSearchPolicy, 5> allowed_sections =
	{
		{
			{ ".data", 4 },
			{ ".rdata", 4 },
			{ ".idata", 4 },
			{ ".rsrc", 4 },
			{ ".text", 10 }, // .text contain alot of crap
		}
	};

	// Since vector insertion ins't thread-safe, we have to create a vector for each section and
	// then merge them into the global vector.
	std::array<std::vector<ImageString>, allowed_sections.size()> concurrent_string_containers;

	const auto process_section = [&](const auto& allowed_sec, uint32_t which) -> void
	{
		CDebugConsole::get().output_message(std::format("Searching in {}...", allowed_sec.m_section_name));

		const auto& section = get_section_by_name(allowed_sec.m_section_name);

		if (!section.is_valid())
		{
			CDebugConsole::get().output_info(std::format("Couldn't search for strings inside {} because the section is invalid or non-existent.",
				allowed_sec.m_section_name));
			
			return;
		}

		uint32_t sec_base_rva = section.m_raw_data_ptr;

		if (section.m_raw_data_size == NULL)
		{
			return;
		}

		uint32_t strings_per_sec = 0;
		for (uint32_t i = sec_base_rva; i < sec_base_rva + section.m_raw_data_size - 1;)
		{
			ImageString str;

			auto c = m_byte_buffer.get_at<const char>(i);

			uint32_t pushed_chars = 0;
			while (is_ascii_fine(*c))
			{
				str.m_string.push_back(*c);

				c++;
				pushed_chars++;
			}

			if (!str.m_string.empty())
			{
				if (str.m_string.length() > allowed_sec.min_len_tolerance)
				{
					str.m_va = rva_as_va(i);
					str.m_parent_sec_name = allowed_sec.m_section_name;
					concurrent_string_containers[which].push_back(str);

					strings_per_sec++;
					i += pushed_chars;
					//CDebugConsole::get().output_message(std::format("Found {}", str.m_string));
				}
				else
				{
					i += allowed_sec.min_len_tolerance;
				}
			}
			else
			{
				i++;
			}
		}

		if (strings_per_sec != NULL)
		{
			m_image_strings_found_in += allowed_sec.m_section_name;
			m_image_strings_found_in += "; ";
			CDebugConsole::get().output_message(std::format("Found {} strings in {}.", strings_per_sec, allowed_sec.m_section_name));
		}
		else
		{
			CDebugConsole::get().output_info(std::format("Didn't find any strings at all inside {}!", allowed_sec.m_section_name));
		}
	};

	std::vector<std::thread> string_processors_for_each_section(allowed_sections.size());

	uint32_t i = 0;
	for (const auto& allowed_sec : allowed_sections)
	{
		string_processors_for_each_section[i++] = std::thread(process_section, allowed_sec, i);
	}

	for (auto& thread : string_processors_for_each_section)
	{
		thread.join();
	}

	// Now merge all vectors
	for (const auto& strings : concurrent_string_containers)
	{
		m_image_strings.reserve(strings.size());
		m_image_strings.insert(m_image_strings.end(), strings.begin(), strings.end());
	}

	m_image_strings_found_in.pop_back(); // remove last ', '
	m_image_strings_found_in.pop_back();

	CDebugConsole::get().output_message(std::format("Found total {} of strings inside the executable", m_image_strings.size()));
}

void CX86PEProcessor::process_load_cfg_guard_flags(uint32_t flags)
{
	m_load_cfg_guard_flags =
	{
		flags,
		{
			{ IMAGE_GUARD_CF_INSTRUMENTED,						"Control-flow integrity checks" },
			{ IMAGE_GUARD_CFW_INSTRUMENTED,						"Control-float and write integrity checks" },
			{ IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT,			"Contains valid control-flow function table" },
			{ IMAGE_GUARD_SECURITY_COOKIE_UNUSED,				"Doesn't use /GS security cookie" },
			{ IMAGE_GUARD_PROTECT_DELAYLOAD_IAT,				"Supports read-only IAT" },
			{ IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION,		"Delayload import table in its own .didat section" },
			{ IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT,	"Contains suppressed export information" },
			{ IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION,			"Enabled suppression of exports" },
			{ IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT,			"Contains longjmp target information" },
			{ IMAGE_GUARD_RF_INSTRUMENTED,						"Contains return flow instrumentation and metadata" },
			{ IMAGE_GUARD_RF_ENABLE,							"Requests that the OS enable return flow protection" },
			{ IMAGE_GUARD_RF_STRICT,							"Requests that the OS enable return flow protection in strict mode" },
			{ IMAGE_GUARD_RETPOLINE_PRESENT,					"Was built with retpoline support" },
			{ IMAGE_GUARD_EH_CONTINUATION_TABLE_PRESENT,		"Contains EH continuation target information" },
		}
	};

	CDebugConsole::get().output_message(std::format("Load CFG guard flags: {}", m_load_cfg_guard_flags.as_continuous_string()));
}

void CX86PEProcessor::process_load_cfg_process_heap_flags(uint32_t flags)
{
	m_load_cfg_process_heap_flags =
	{
		flags,
		{
			{ HEAP_NO_SERIALIZE,				"" },
			{ HEAP_GROWABLE,					"" },
			{ HEAP_GENERATE_EXCEPTIONS,			"" },
			{ HEAP_ZERO_MEMORY,					"" },
			{ HEAP_REALLOC_IN_PLACE_ONLY,		"" },
			{ HEAP_TAIL_CHECKING_ENABLED,		"" },
			{ HEAP_FREE_CHECKING_ENABLED,		"" },
			{ HEAP_DISABLE_COALESCE_ON_FREE,	"" },
			{ HEAP_CREATE_HARDENED,				"" },
			{ HEAP_CREATE_SEGMENT_HEAP,			"" },
			{ HEAP_CREATE_ALIGN_16,				"" },
			{ HEAP_CREATE_ENABLE_TRACING,		"" },
			{ HEAP_CREATE_ENABLE_EXECUTE,		"" },
		}
	};

	CDebugConsole::get().output_message(std::format("Load CFG process heap flags: {}", m_load_cfg_process_heap_flags.as_continuous_string()));
}

uint32_t CX86PEProcessor::calculate_image_checksum()
{
	uint64_t checksum = 0;

	uint32_t checksumpos = m_dos_headers_size + offsetof(IMAGE_NT_HEADERS, OptionalHeader.CheckSum);

	for (uint32_t i = 0; i < m_byte_buffer.get_size(); i += sizeof(uint32_t))
	{
		if (i == checksumpos)
			continue;

		uint32_t dw = *m_byte_buffer.get_at<uint32_t>(i);

		checksum = (checksum & 0xffffffff) + dw + (checksum >> 32);
		if (checksum > (((uint64_t)(~0ul) + 1)))
			checksum = (checksum & 0xffffffff) + (checksum >> 32);
	}

	//Finish checksum
	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = (checksum)+(checksum >> 16);
	checksum = checksum & 0xffff;

	checksum += m_byte_buffer.get_size();
	return static_cast<uint32_t>(checksum);
}

std::string CX86PEProcessor::data_dir_name_by_idx(uint32_t i)
{
	switch (i)
	{
		case IMAGE_DIRECTORY_ENTRY_EXPORT:			return "Export";
		case IMAGE_DIRECTORY_ENTRY_IMPORT:			return "Import";
		case IMAGE_DIRECTORY_ENTRY_RESOURCE:		return "Resource";
		case IMAGE_DIRECTORY_ENTRY_EXCEPTION:		return "Exception";
		case IMAGE_DIRECTORY_ENTRY_SECURITY:		return "Security";
		case IMAGE_DIRECTORY_ENTRY_BASERELOC:		return "BaseReloc";
		case IMAGE_DIRECTORY_ENTRY_DEBUG:			return "Debug";
		case IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:	return "Arch";
		case IMAGE_DIRECTORY_ENTRY_GLOBALPTR:		return "Global PTR";
		case IMAGE_DIRECTORY_ENTRY_TLS:				return "Thread Local Storage";
		case IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:		return "Load config";
		case IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:	return "Bound import";
		case IMAGE_DIRECTORY_ENTRY_IAT:				return "Import address table";
		case IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:	return "Delay import";
		case IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:	return "COM descriptor";
		case 15:									return "Reserved (15)";
	}

	CDebugConsole::get().output_error(std::format("Unknown data directory index: {}", i));
	return "Unknown";
}

uint32_t CX86PEProcessor::offset_relative_to_a_section(uint32_t rva)
{
	for (const auto& [key, sec] : m_sections)
	{
		// Check if we're in-bounds
		if (rva >= va_as_rva(sec.m_va_base) && rva <= va_as_rva(sec.virtual_end_addr()))
			return (rva - va_as_rva(sec.m_va_base)) + sec.m_raw_data_ptr;
	}

	CDebugConsole::get().output_error(std::format("Error! Relative section offset couldn't be calculated! Addr=0x{:08X}", rva));
	return 0x00000000;
}

void ImageSection::process_characteristics(uint32_t characteristics)
{
	m_characteristics =
	{
		characteristics,
		{
			{ IMAGE_SCN_TYPE_NO_PAD,				"Not padded" },
			{ IMAGE_SCN_CNT_CODE,					"Executable code" },
			{ IMAGE_SCN_CNT_INITIALIZED_DATA,		"Initialized data" },
			{ IMAGE_SCN_CNT_UNINITIALIZED_DATA,		"Uninitialized data" },
			{ IMAGE_SCN_LNK_OTHER,					"Other (reserved)" },
			{ IMAGE_SCN_LNK_INFO,					"Comments (.drectve)" },
			{ IMAGE_SCN_LNK_REMOVE,					"Not part of image" },
			{ IMAGE_SCN_LNK_COMDAT,					"COMDAT data" },
			{ IMAGE_SCN_GPREL,						"Global Ptr data" },
			{ IMAGE_SCN_MEM_PURGEABLE,				"Purgeable (reserved)" },
			{ IMAGE_SCN_MEM_16BIT,					"16bit (reserved)" },
			{ IMAGE_SCN_MEM_LOCKED,					"Locked (reserved)" },
			{ IMAGE_SCN_MEM_PRELOAD,				"Preload (reserved)" },
			{ IMAGE_SCN_ALIGN_1BYTES,				"1-byte aligned" },
			{ IMAGE_SCN_ALIGN_2BYTES,				"2-byte aligned" },
			{ IMAGE_SCN_ALIGN_128BYTES,				"128-byte aligned" },
			{ IMAGE_SCN_LNK_NRELOC_OVFL,			"Extended relocs" },
			{ IMAGE_SCN_MEM_DISCARDABLE,			"Discardable" },
			{ IMAGE_SCN_MEM_NOT_CACHED,				"Uncachable" },
			{ IMAGE_SCN_MEM_NOT_PAGED,				"Unpageable" },
			{ IMAGE_SCN_MEM_SHARED,					"Memory-shareable" },
			{ IMAGE_SCN_MEM_EXECUTE,				"Executable" },
			{ IMAGE_SCN_MEM_READ,					"Readable" },
			{ IMAGE_SCN_MEM_WRITE,					"Writeable" },
		}
	};

	// Too verbose
	//CDebugConsole::get().output_message(std::format("Section characteristics: {}", m_characteristics.as_continuous_string()));
}

void ImageWinCertificate::process_revision_version(uint16_t rev_version)
{
	switch (rev_version)
	{
		case WIN_CERT_REVISION_1_0: m_revision_version = { WIN_CERT_REVISION_1_0, "1.0" }; break;
		case WIN_CERT_REVISION_2_0: m_revision_version = { WIN_CERT_REVISION_1_0, "2.0" }; break;

		default:
			m_revision_version = { rev_version, std::format("0x{:04X}", rev_version) };
			CDebugConsole::get().output_error("Invalid certificate revision version");
			break;
	}

	CDebugConsole::get().output_message(std::format("Certificate revision version is: {}", m_revision_version.name()));
}

void ImageWinCertificate::process_cert_type(uint16_t cert_type)
{
	switch (cert_type)
	{
		case WIN_CERT_TYPE_X509: m_cert_type = { WIN_CERT_TYPE_X509, "X.509" }; break;
		case WIN_CERT_TYPE_PKCS_SIGNED_DATA: m_cert_type = { WIN_CERT_TYPE_PKCS_SIGNED_DATA, "PKCS#7 SignedData" }; break;
		case WIN_CERT_TYPE_TS_STACK_SIGNED: m_cert_type = { WIN_CERT_TYPE_RESERVED_1, "Terminal Server Protocol Stack" }; break;

		default:
			m_cert_type = { cert_type, std::format("0x{:04X}", cert_type) };
			CDebugConsole::get().output_error("Invalid certificate type");
			break;
	}

	CDebugConsole::get().output_message(std::format("Certificate type is: {}", m_cert_type.name()));
}

void ImageRelocationBlock::BlockInfo::process_block_info_type(uint32_t type)
{
	switch (type)
	{
		case IMAGE_REL_BASED_ABSOLUTE: m_type = { IMAGE_REL_BASED_ABSOLUTE, "Absolute based" }; break;
		case IMAGE_REL_BASED_HIGH: m_type = { IMAGE_REL_BASED_HIGH, "High based" }; break;
		case IMAGE_REL_BASED_LOW: m_type = { IMAGE_REL_BASED_LOW, "Low based" }; break;
		case IMAGE_REL_BASED_HIGHLOW: m_type = { IMAGE_REL_BASED_HIGHLOW, "High-Low based" }; break;
		case IMAGE_REL_BASED_HIGHADJ: m_type = { IMAGE_REL_BASED_HIGHADJ, "High-Adj based" }; break;
		case IMAGE_REL_BASED_MACHINE_SPECIFIC_5: m_type = { IMAGE_REL_BASED_MACHINE_SPECIFIC_5, "Machine specific (5)" }; break;
		case IMAGE_REL_BASED_RESERVED: m_type = { IMAGE_REL_BASED_RESERVED, "Reserved" }; break;
		case IMAGE_REL_BASED_MACHINE_SPECIFIC_7: m_type = { IMAGE_REL_BASED_MACHINE_SPECIFIC_7, "Machine specific (7)" }; break;
		case IMAGE_REL_BASED_MACHINE_SPECIFIC_8: m_type = { IMAGE_REL_BASED_MACHINE_SPECIFIC_8, "Machine specific (8)" }; break;
		case IMAGE_REL_BASED_MACHINE_SPECIFIC_9: m_type = { IMAGE_REL_BASED_MACHINE_SPECIFIC_9, "Machine specific (9)" }; break;
		case IMAGE_REL_BASED_DIR64: m_type = { IMAGE_REL_BASED_DIR64, "Dir 64" }; break;

		default:
			m_type = { type, std::format("0x{:04X}", type) };
			CDebugConsole::get().output_error("Invalid relocation block info type");
			break;
	}

	// too verbose
	//CDebugConsole::get().output_message(std::format("Image relocation block info type is: {}", m_type.name()));
}

void ImageDebugDirectory::process_type(uint32_t type)
{
	switch (type)
	{
		case IMAGE_DEBUG_TYPE_UNKNOWN: m_type = { IMAGE_DEBUG_TYPE_UNKNOWN, "Unknown" }; break;
		case IMAGE_DEBUG_TYPE_COFF: m_type = { IMAGE_DEBUG_TYPE_COFF, "COFF" }; break;
		case IMAGE_DEBUG_TYPE_CODEVIEW: m_type = { IMAGE_DEBUG_TYPE_CODEVIEW, "Code view" }; break;
		case IMAGE_DEBUG_TYPE_FPO: m_type = { IMAGE_DEBUG_TYPE_FPO, "FPO" }; break;
		case IMAGE_DEBUG_TYPE_MISC: m_type = { IMAGE_DEBUG_TYPE_MISC, "Miscellaneous" }; break;
		case IMAGE_DEBUG_TYPE_EXCEPTION: m_type = { IMAGE_DEBUG_TYPE_EXCEPTION, "Exception" }; break;
		case IMAGE_DEBUG_TYPE_FIXUP: m_type = { IMAGE_DEBUG_TYPE_FIXUP, "Fixup" }; break;
		case IMAGE_DEBUG_TYPE_OMAP_TO_SRC: m_type = { IMAGE_DEBUG_TYPE_OMAP_TO_SRC, "OMAP to source" }; break;
		case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC: m_type = { IMAGE_DEBUG_TYPE_OMAP_FROM_SRC, "OMAP from source" }; break;
		case IMAGE_DEBUG_TYPE_BORLAND: m_type = { IMAGE_DEBUG_TYPE_BORLAND, "borland" }; break;
		case IMAGE_DEBUG_TYPE_RESERVED10: m_type = { IMAGE_DEBUG_TYPE_RESERVED10, "Reserved (10)" }; break;
		case IMAGE_DEBUG_TYPE_CLSID: m_type = { IMAGE_DEBUG_TYPE_CLSID, "Class ID (CLSID)" }; break;
		case IMAGE_DEBUG_TYPE_VC_FEATURE: m_type = { IMAGE_DEBUG_TYPE_VC_FEATURE, "VC feature" }; break;
		case IMAGE_DEBUG_TYPE_POGO: m_type = { IMAGE_DEBUG_TYPE_POGO, "POGO" }; break;
		case IMAGE_DEBUG_TYPE_ILTCG: m_type = { IMAGE_DEBUG_TYPE_ILTCG, "ILTCG" }; break;
		case IMAGE_DEBUG_TYPE_MPX: m_type = { IMAGE_DEBUG_TYPE_MPX, "MPX" }; break;
		case IMAGE_DEBUG_TYPE_REPRO: m_type = { IMAGE_DEBUG_TYPE_REPRO, "REPRO" }; break;
		case IMAGE_DEBUG_TYPE_EX_DLLCHARACTERISTICS: m_type = { IMAGE_DEBUG_TYPE_EX_DLLCHARACTERISTICS, "External DLL characteristics" }; break;

		default:
			m_type = { type, std::format("0x{:08X}", type) };
			CDebugConsole::get().output_error("Invalid debug directory type");
			break;
	}

	CDebugConsole::get().output_message(std::format("Debug directory type is: {}", m_type.name()));
}

void ImageDebugDirectory::process_type_cv(const ByteBuffer<uint32_t>& byte_buffer)
{
	auto& cv = m_cv;

	// The first dword is the signature.
	auto magic_signature = *byte_buffer.get_at<DWORD>(m_file_pointer_to_raw_data);

	switch (magic_signature)
	{
		case CodeView::k_sigRSDS:
		{
			cv.m_magic_signature = { CodeView::k_sigRSDS, "RSDS" };

			struct RSDSI // RSDS debug info
			{
				DWORD	magic;		// RSDS
				GUID	guidSig;	// pdg signature
				DWORD	age;		// pdg age
				CHAR	pdbpath[1];
			};

			auto rsdsi = byte_buffer.get_at<RSDSI>(m_file_pointer_to_raw_data);

			cv.m_rsds_guid_pdb_sig.stringify_me(rsdsi->guidSig);
			cv.m_pdb_age = rsdsi->age;
			cv.m_pdb_path = rsdsi->pdbpath;

			CDebugConsole::get().output_message("Processed RSDS codeview signature data.");
			break;
		}
		// These two guys have no structure or an uknown one
		case CodeView::k_sigNB09:
			cv.m_magic_signature = { CodeView::k_sigNB09, "NB09" };
		case CodeView::k_sigNB11:
		{
			cv.m_magic_signature = { CodeView::k_sigNB11, "NB11" };
			break;
		}
		case CodeView::k_sigNB10:
		{
			cv.m_magic_signature = { CodeView::k_sigNB10, "NB10" };

			struct NB10I // NB10 debug info
			{
				DWORD	magic;	// NB10
				DWORD	off;	// offset, always 0
				DWORD	sig;	// pdg signature
				DWORD	age;	// pdg age
				CHAR	pdbpath[1];
			};

			auto nb10i = byte_buffer.get_at<NB10I>(m_file_pointer_to_raw_data);

			cv.m_nb10_pdb_sig = nb10i->sig;
			cv.m_pdb_age = nb10i->age;
			cv.m_pdb_path = nb10i->pdbpath;

			CDebugConsole::get().output_message("Processed NBXX codeview signature data.");
			break;
		}
		default:
		{
			CDebugConsole::get().output_info(std::format("Got invalid codeview magic signature: 0x{:08X}", magic_signature));
			break;
		}
	}
}

void GUIDString::stringify_me(const GUID& guid)
{
	m_str_guid = CUtil::guid_to_string(guid);
}

IntegerTimestamp::IntegerTimestamp(uint32_t timestamp) :
	m_dw_timestamp(timestamp),
	m_str_timestamp("invalid")
{
	if (!timestamp)
	{
		CDebugConsole::get().output_info("Warning, got invalid (null) integer timestamp.");

		return; // Kinda no point in converting the string when it's null.
	}

	time_t time = m_dw_timestamp;

	tm* t = std::gmtime(&time);

	if (!t)
	{
		CDebugConsole::get().output_error(std::format("Got nullptr tm struct for {:08X} timestamp.", timestamp));
		return;
	}

	m_str_timestamp = std::asctime(t);
}