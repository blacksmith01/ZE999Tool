#pragma once

#include "Common.hpp"
#include "Sir.hpp"
#include "BMFont.hpp"

class JpKeycodeAllocator
{
public:
	JpKeycodeAllocator(uint8_t h, uint8_t l) : keycode({ h ,l, 0 }) {}

	const char* Alloc()
	{
		do {
			keycode[1]++;
			if (keycode[1] == 0x00) {
				keycode[0]++;
				keycode[1] = 0x01;
			}
			if (keycode[0] == 0xFF) {
				break;
			}

			if (jp_map.contains((const char*)&keycode[0])) {
				continue;
			}

			if (sjis_valid((const char*)&keycode[0])) {
				break;
			}

		} while (true);

		return (const char*)keycode.data();
	}

	std::unordered_set<std::string> jp_map;
	std::array<uint8_t, 3> keycode;
};


class SirTool {
public:
	bool Unpack(const fs::path& src_path, const fs::path& dst_dir_path);
	bool Repack(const fs::path& src_path, const fs::path& dst_dir_path);
	bool CopyValid(const fs::path& org_dir_path, const fs::path& dst_dir_path);
	bool Patch(const fs::path& org_dir_path, const fs::path& patch_dir_path, std::string option, const fs::path& dst_dir_path);
	bool ExePatch(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& exe_file_path, const fs::path& dst_dir_path);
	bool GeneratePatchFontChars(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& dst_file_path);
	bool GeneratePatchFontData(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& bmf_default_path, const fs::path& bmf_border_path, const fs::path& dst_dir_path);

//private:
	void RetrievePatchChars(wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes);
	void RetrieveExPatchChars(const fs::path& patch_dir_path, wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes);
	void RetrievePatchChars(const std::wstring& text, wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes);
	
	std::string PatchText(const std::wstring& text);

	void ReadExePatchFile(const fs::path& file_path, std::map<std::string, std::string>& map);
	void RetriveAnsiChars(std::set<uint8_t>& ansi_map);

	void ReadSirDir(const fs::path& dir_path);
	bool ReadSirFile(const fs::path& file_path);
	void ReadXml(const fs::path& file_path, int is_patch = false);

	template<typename T>
	T* FindSirPtr(const std::vector<std::shared_ptr<T>>& container, std::string_view filename)
	{
		for (auto& c : container) {
			if (c->filename == filename) {
				return &(*c);
			}
		}
		return nullptr;
	}

//private:
	std::vector<std::shared_ptr<SirDlg>> dlgs;
	std::vector<std::shared_ptr<SirName>> names;
	std::vector<std::shared_ptr<SirMap>> maps;
	std::vector<std::shared_ptr<SirFont>> fonts;
	std::vector<std::shared_ptr<SirItem>> items;
	std::vector<std::shared_ptr<SirMsg>> msgs;
	std::vector<std::shared_ptr<SirDesc>> descs;
	std::vector<std::shared_ptr<SirFChart>> fcharts;
	std::vector<std::shared_ptr<SirDoc>> docs;

	std::vector<std::shared_ptr<SirDlg>> patch_dlgs;
	std::vector<std::shared_ptr<SirName>> patch_names;
	std::vector<std::shared_ptr<SirMap>> patch_maps;
	std::vector<std::shared_ptr<SirFont>> patch_fonts;
	std::vector<std::shared_ptr<SirItem>> patch_items;
	std::vector<std::shared_ptr<SirMsg>> patch_msgs;
	std::vector<std::shared_ptr<SirDesc>> patch_descs;
	std::vector<std::shared_ptr<SirFChart>> patch_fcharts;
	std::vector<std::shared_ptr<SirDoc>> patch_docs;

	std::unordered_map<wchar_t, wchar_t> patch_glyphs;
};