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

#ifndef EXEIN_C_PE_PROCESSOR
#define EXEIN_C_PE_PROCESSOR

#pragma once

class IntegerTimestamp
{
public:
	IntegerTimestamp(uint32_t timestamp);

	IntegerTimestamp() = default;

public:
	inline const auto& as_string() const { return m_str_timestamp; }

	inline bool is_valid() const { return m_dw_timestamp != 0; }

private:
	uint32_t m_dw_timestamp;
	std::string m_str_timestamp;
};

class GUIDString
{
public:
	GUIDString() : 
		m_str_guid("uninitialized") 
	{
	}

	void stringify_me(const GUID& guid);

public:
	inline const auto& get() const { return m_str_guid; }

private:
	std::string m_str_guid;
};

class ImageDataDir
{
public:
	inline bool is_present() const { return m_address && m_size; }

public:
	std::string m_name;

	// Can be a relative, virtual, or file pointer address
	SmartHexValue<uint32_t> m_address;
	SmartDecValue<uint32_t> m_size;
};

class ImageExport
{
public:
	std::string m_name;
	SmartDecValue<uint16_t> m_ordinal;
	SmartHexValue<uint32_t> m_address;
	bool is_forwarded;
};

class ImageImportThunk
{
public:
	inline bool imported_by_ordinal() const { return m_ordinal != 0; }
	inline bool imported_by_name() const { return !m_name.empty(); }

public:
	std::string m_name; // Can be imported either by name or by an ordinal
	
	SmartDecValue<uint16_t> m_ordinal;
	SmartDecValue<uint16_t> m_hint; // This comes with the name

	// VA to an entry inside IAT
	SmartHexValue<uint32_t> m_import_va;
};

class ImageImportDescriptor
{
public:
	inline bool has_thunks() const { return !m_import_thunks.empty(); }

public:
	std::string m_image_name;
	std::vector<ImageImportThunk> m_import_thunks;

	// VA to an entry inside IAT. Descriptors also have one.
	SmartHexValue<uint32_t> m_descriptor_va;
};

class ImageDelayedImportDescriptor
{
public:
	inline bool has_thunks() const { return !m_import_thunks.empty(); }

public:
	DLAttr m_attributes;

	std::string m_image_name;
	SmartHexValue<uint32_t> m_module_handle_rva;

	std::vector<ImageImportThunk> m_import_thunks;

	// VA to an entry inside IAT. Descriptors also have one.
	SmartHexValue<uint32_t> m_descriptor_va;

	IntegerTimestamp m_timestamp;
};

class ImageIATEntry
{
public:
	SmartHexValue<uint32_t> m_va;

	std::string m_refered_thunk_or_descriptor_name;
};

class ImageSection
{
public:
	void process_characteristics(uint32_t characteristics);

public:
	// Returns end address of the section
	inline auto virtual_end_addr() const 
	{ 
		return SmartHexValue<uint32_t>(m_va_base + m_va_size - 1);
	}

	inline bool is_valid() const 
	{
		return !m_name.empty() && m_va_size != NULL;
	}

public:
	std::string m_name;

	SmartHexValue<uint32_t> m_va_base;
	SmartHexValue<uint32_t> m_va_size;

	SmartHexValue<int32_t> m_zero_padding;

	SmartHexValue<uint32_t> m_raw_data_ptr;
	SmartHexValue<uint32_t> m_raw_data_size;

	SmartHexValue<uint32_t> m_line_numbers_ptr;
	SmartHexValue<uint16_t> m_line_numbers_num;

	SmartHexValue<uint32_t> m_relocs_ptr;
	SmartHexValue<uint16_t> m_relocs_num;

	NamedBitfieldConstant<uint32_t> m_characteristics;
};

class ImageWinCertificate
{
public:
	void process_revision_version(uint16_t rev_version);
	void process_cert_type(uint16_t cert_type);

public:
	// Not aligned entry length. Should be aligned to an 8-byte boundary when
	// offsetting to the next entry.
	SmartDecValue<uint32_t> m_entry_length;

	SmartHexValue<uint32_t> m_cert_raw_data_va;

	NamedConstant<uint16_t> m_revision_version;
	NamedConstant<uint16_t> m_cert_type;
};

class ImageRelocationBlock
{
public:
	SmartHexValue<uint32_t> m_page_rva;
	SmartDecValue<uint32_t> m_size; // block size

	struct BlockInfo
	{
		void process_block_info_type(uint32_t type);

		NamedConstant<uint32_t> m_type;
		SmartHexValue<uint32_t> m_offset;
	};

	// Type & offset
	std::vector<BlockInfo> m_block_info_entries;
};

class ImageDebugDirectory
{
public:
	void process_type(uint32_t type);

	void process_type_cv(const ByteBuffer<uint32_t>& byte_buffer);

public:
	IntegerTimestamp m_timestamp;

	SmartDecValue<uint16_t> m_major_ver, m_minor_ver;

	NamedConstant<uint32_t> m_type;

	SmartHexValue<uint32_t> m_size_of_data;
	SmartHexValue<uint32_t> m_address_of_raw_data_va;
	SmartHexValue<uint32_t> m_file_pointer_to_raw_data;

public:
	class CodeView
	{
	public:
		// CodeView magic signatures
		static inline constexpr const ULONG k_sigRSDS = 'SDSR';
		static inline constexpr const ULONG k_sigNB09 = '90BN';
		static inline constexpr const ULONG k_sigNB10 = '01BN';
		static inline constexpr const ULONG k_sigNB11 = '11BN';

	public:
		NamedConstant<ULONG> m_magic_signature;

		// RSDS
		GUIDString m_rsds_guid_pdb_sig;

		// NB10
		SmartHexValue<uint32_t> m_nb10_pdb_sig;

		// Shared
		SmartDecValue<uint32_t> m_pdb_age;
		std::string m_pdb_path;
	};

public:
	CodeView m_cv;
};

class ImageString
{
public:
	std::string m_string;
	SmartHexValue<uint32_t> m_va;

	// Section where we did find this string
	std::string m_parent_sec_name;
};

class ImageLoadCFGCodeIntegrity
{
public:
	SmartHexValue<uint16_t> m_flags;
	SmartHexValue<uint16_t> m_catalog;
	SmartHexValue<uint32_t> m_catalog_offs;
};

class CX86PEProcessor final : public IBaseProcessor
{
public:
	bool process(const std::filesystem::path& filepath) override;

	void render_gui() override;

private:
	void render_tabs();
	void render_tab_sections();
	void render_tab_exports();
	void render_tab_imports();
	void render_tab_certificates();
	void render_tab_relocations();
	void render_tab_debug();
	void render_tab_load_cfg();
	void render_tab_strings();
	void render_tab_misc();
	void render_content_tab(const std::string& label, bool does_exist, const std::function<void()>& pfn_contents);

	template<typename T>
	void render_named_bitfield_constant_generic(const T& flags, const std::string& label, const char* child, uint32_t& active_flags_static);

	bool process_dos();
	bool process_dos_magic(uint16_t magic);
	void process_dos_stub(uint32_t off, uint32_t size);

	bool process_nt_headers();
	bool process_nt_sig(uint32_t sig);
	bool process_nt_file_hdr(const IMAGE_FILE_HEADER* pfile_hdr);
	bool process_nt_opt_hdr(const IMAGE_OPTIONAL_HEADER* popt_hdr);
	void process_nt_characteristics(uint16_t characs);
	bool process_nt_data_dir_entries(const IMAGE_OPTIONAL_HEADER* popt_hdr);

	void process_nt_data_directories();
	void process_nt_data_dir_exports(const ImageDataDir& dir_entry);
	void process_nt_data_dir_imports(const ImageDataDir& dir_entry);
	void process_nt_data_dir_resources(const ImageDataDir& dir_entry);
	void process_nt_data_dir_exceptions(const ImageDataDir& dir_entry);
	void process_nt_data_dir_certificates(const ImageDataDir& dir_entry);
	void process_nt_data_dir_relocs(const ImageDataDir& dir_entry);
	void process_nt_data_dir_debug(const ImageDataDir& dir_entry);
	void process_nt_data_dir_arch(const ImageDataDir& dir_entry);
	void process_nt_data_dir_global_ptr(const ImageDataDir& dir_entry);
	void process_nt_data_dir_tls(const ImageDataDir& dir_entry);
	void process_nt_data_dir_load_cfg(const ImageDataDir& dir_entry);
	void process_nt_data_dir_bound_import(const ImageDataDir& dir_entry);
	void process_nt_data_dir_iat(const ImageDataDir& dir_entry);
	void process_nt_data_dir_delay_import(const ImageDataDir& dir_entry);
	void process_nt_data_dir_clr_runtime(const ImageDataDir& dir_entry);

	void process_nt_machine(uint16_t machine);
	bool process_nt_magic(uint16_t magic);
	void process_nt_subsystem(uint32_t subsystem);
	void process_nt_dll_characteristics(uint16_t dll_characteristics);

	bool process_sections();

	void process_image_strings();

	void process_load_cfg_guard_flags(uint32_t flags);
	void process_load_cfg_process_heap_flags(uint32_t flags);

private:
	// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#coff-file-header-object-and-image
	static inline uint32_t max_number_of_sections() { return 96; }

	static inline float gui_leftmost_column_size() { return 390; }

	uint32_t calculate_image_checksum();

	std::string data_dir_name_by_idx(uint32_t i);

	// Accepts rva address and returns file pointer to the address 
	// relative to the start of appropriate section. If the rva is
	// out of bounds of all sections, 0 is returned.
	uint32_t offset_relative_to_a_section(uint32_t rva);

	inline const ImageSection& get_section_by_name(const std::string& section_name) const
	{
		static ImageSection dummy_section;
		try
		{
			return m_sections.at(section_name);
		}
		catch (...)
		{
			CDebugConsole::get().output_error(std::format("Failed to obtain section: {}", section_name));
			return dummy_section;
		}
	}

	inline bool image_has_tls() const { return m_tls_raw_data_start_va && m_tls_raw_data_size; }

	inline uint32_t va_as_rva(uint32_t va) const { return va - m_img_base; }
	inline uint32_t rva_as_va(uint32_t rva) const { return rva + m_img_base; }

private:
	// DOS header
	NamedConstant<uint16_t> m_dos_magic;
	uint16_t m_number_of_pages;
	SmartHexValue<int32_t> m_dos_headers_size; // Also e_lfanew
	ByteBuffer<uint32_t> m_dos_stub_byte_buffer;
	SmartDecValue<uint32_t> m_dos_stub_size;

	// NT headers
	NamedConstant<uint16_t> m_nt_sig;

	// File header
	NamedConstant<uint16_t> m_nt_machine; // CPU type
	uint32_t m_num_sections;
	IntegerTimestamp m_nt_time_date_stamp; // Creation of the file
	NamedBitfieldConstant<uint16_t> m_nt_characteristics;

	// Optional header
	NamedConstant<uint16_t> m_nt_magic;
	SmartHexValue<uint32_t> m_img_base;
	SmartHexValue<uint32_t> m_code_base_va;
	SmartHexValue<uint32_t> m_code_size;
	SmartHexValue<uint32_t> m_data_base_va;
	SmartHexValue<uint32_t> m_uninitialized_data_size; // Uninitialized data section (.bss)
	SmartHexValue<uint32_t> m_initialized_data_size;
	SmartHexValue<uint32_t> m_entry_point_va;
	SmartHexValue<uint32_t> m_section_alignment, m_file_alignment;
	SmartHexValue<uint32_t> m_size_of_image; // Size including all headers as the image is loaded in memory
	SmartHexValue<uint32_t> m_size_of_hdrs;
	SmartHexValue<uint32_t> m_image_checksum_calc, m_image_checksum_real;
	NamedConstant<uint32_t> m_nt_subsystem;
	NamedBitfieldConstant<uint16_t> m_nt_dll_characteristics;
	uint32_t m_number_of_data_directories;

	// Sections
	std::unordered_map<std::string, ImageSection> m_sections;

	// Data directories (Use MAGE_DIRECTORY_ENTRY_* macros as keys)
	std::unordered_map<uint32_t, ImageDataDir> m_data_dirs;

	// Exports
	uint32_t m_number_of_exported_functions;
	uint32_t m_number_of_exported_names;
	SmartDecValue<uint32_t> m_export_starting_ordinal_num; // Obtained by IMAGE_EXPORT_DIRECTORY::Base
	std::string m_export_dll_name; // Obtained by IMAGE_EXPORT_DIRECTORY::Name
	IntegerTimestamp m_export_creation_timestamp;
	std::vector<ImageExport> m_image_exports;

	// Imports
	uint32_t m_number_of_import_descriptors;
	uint32_t m_number_of_import_thunk_ordinals, m_number_of_import_thunk_names; // number of imports snapped by ordinal or name. The sum of these two is the total sum
	SmartHexValue<uint32_t> m_imports_iat_base;
	std::vector<ImageImportDescriptor> m_image_import_descriptors;

	// Delayed Imports
	uint32_t m_number_of_delayed_import_descriptors;
	uint32_t m_number_of_delayed_import_thunk_ordinals, m_number_of_delayed_import_thunk_names; // number of imports snapped by ordinal or name. The sum of these two is the total sum
	std::vector<ImageDelayedImportDescriptor> m_image_delayed_import_descriptors;

	// Import Address Table (IAT)
	std::vector<ImageIATEntry> m_iat_entries;

	// Certificates
	std::vector<ImageWinCertificate> m_image_certificates;

	// Global register pointer
	SmartHexValue<uint32_t> m_global_ptr_register;

	// TLS data
	SmartHexValue<uint32_t> m_tls_raw_data_start_va, m_tls_raw_data_end_va;
	SmartHexValue<uint32_t> m_tls_raw_data_size;
	SmartHexValue<uint32_t> m_tls_index_va; // VA to the address of __tls_index
	SmartHexValue<uint32_t> m_tls_size_of_zero_fill;
	SmartHexValue<uint32_t> m_tls_base_of_callbacks_va; // VA to the base of callbacks
	std::vector<SmartValue<uint32_t>> m_tls_callbacks_va; // Vector containing VAs to individual callbacks

	// Relocations
	std::vector<ImageRelocationBlock> m_relocation_blocks;

	// Debug directories
	std::vector<ImageDebugDirectory> m_debug_directories;

	// Strings
	std::vector<ImageString> m_image_strings;
	std::string m_image_strings_found_in; // Silly name but basically separated string of section names in
	// which we found any strings.

	// Load CFG
	IntegerTimestamp m_load_cfg_timestamp;
	SmartHexValue<uint32_t> m_load_cfg_global_flags_clear;
	SmartHexValue<uint32_t> m_load_cfg_global_flags_set;
	SmartHexValue<uint32_t> m_load_cfg_cs_default_timeout; // critical section
	SmartHexValue<uint32_t> m_load_cfg_decommit_free_block_threshold;
	SmartHexValue<uint32_t> m_load_cfg_decommit_total_block_threshold;
	SmartHexValue<uint32_t> m_load_cfg_lock_prefix_table_va;
	SmartHexValue<uint32_t> m_load_cfg_max_proc_heap_alloc_size;
	SmartHexValue<uint32_t> m_load_cfg_vm_threshold; // virtual memory
	NamedBitfieldConstant<uint32_t> m_load_cfg_process_heap_flags;
	SmartHexValue<uint32_t> m_load_cfg_process_affinity_mask;
	SmartHexValue<uint16_t> m_load_cfg_service_pack_ver_id;
	SmartHexValue<uint16_t> m_load_cfg_dependent_load_flags;
	SmartHexValue<uint32_t> m_load_cfg_edit_list_va;
	SmartHexValue<uint32_t> m_load_cfg_security_cookie_va;
	SmartHexValue<uint32_t> m_load_cfg_seh_table_va;
	SmartDecValue<uint32_t> m_load_cfg_seh_handlers_count;
	SmartHexValue<uint32_t> m_load_cfg_guard_cfg_check_func_ptr_va;		// VA of Control Flow Guard check-function pointer
	SmartHexValue<uint32_t> m_load_cfg_guard_cfg_dispatch_func_ptr_va;	// VA of Control Flow Guard dispatch-function pointer
	SmartHexValue<uint32_t> m_load_cfg_guard_cfg_func_table_va;			// VA of the sorted table of RVAs of each Control Flow Guard function in the image
	SmartDecValue<uint32_t> m_load_cfg_guard_cfg_func_table_entries;
	NamedBitfieldConstant<uint32_t> m_load_cfg_guard_flags;
	ImageLoadCFGCodeIntegrity m_load_cfg_ci; // code integrity
	SmartHexValue<uint32_t> m_load_cfg_guard_addr_taken_iat_entry_table_va;			// VA of Control Flow Guard address taken IAT table
	SmartDecValue<uint32_t> m_load_cfg_guard_addr_taken_iat_entry_table_entries;
	SmartHexValue<uint32_t> m_load_cfg_guard_long_jump_target_table_va;				// VA of Control Flow Guard long jump target table
	SmartDecValue<uint32_t> m_load_cfg_guard_long_jump_target_table_entries;
	SmartHexValue<uint32_t> m_load_cfg_dynamic_value_reloc_table_va;
	SmartHexValue<uint32_t> m_load_cfg_chpe_metadata;
	SmartHexValue<uint32_t> m_load_cfg_guard_rf_failure_routine_va;
	SmartHexValue<uint32_t> m_load_cfg_guard_rf_failure_routine_func_ptr_va;
	SmartHexValue<uint32_t> m_load_cfg_dynamic_value_reloc_table_offs;
	SmartHexValue<uint16_t> m_load_cfg_dynamic_value_reloc_table_section;
	SmartHexValue<uint32_t> m_load_cfg_guard_rf_verify_stack_ptr_func_ptr_va;
	SmartHexValue<uint32_t> m_load_cfg_hot_patch_table_offs;
	SmartHexValue<uint32_t> m_load_cfg_enclave_config_ptr_va;
	SmartHexValue<uint32_t> m_load_cfg_volatile_metadata_pointer_va;
	SmartHexValue<uint32_t> m_load_cfg_guard_eh_continuation_table_va;
	SmartDecValue<uint32_t> m_load_cfg_guard_eh_continuation_table_entries;
};

#endif