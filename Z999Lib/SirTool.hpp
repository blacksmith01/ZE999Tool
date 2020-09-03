#pragma once

#include "Common.hpp"
#include "Sir.hpp"
#include "SirReader.hpp"
#include "SirWriter.hpp"
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
	std::array<uint8_t, 3> keycode = { 0xF0, 0x00, 0x00 };
};


class SirTool {
public:
	bool Unpack(fs::path src_path, fs::path dst_dir_path)
	{
		if (!fs::is_directory(dst_dir_path)) {
			return false;
		}

		if (fs::is_regular_file(src_path)) {
			if (src_path.extension() == ".sir") {
				ReadSirFile(src_path);
			}
		}
		else {
			ReadSirDir(src_path);
		}

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}

		for (auto& sir : dlgs) {
			SirXmlWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".dlg.xml"));
		}
		for (auto& sir : names) {
			SirXmlWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".name.xml"));
		}
		for (auto& sir : fonts) {
			SirXmlWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".font.xml"));
			SirPngWriter::Write(*sir, 0, fs::path(dst_dir_path).append(sir->filename + ".default.png"));
			SirPngWriter::Write(*sir, 1, fs::path(dst_dir_path).append(sir->filename + ".border.png"));
		}

		return true;
	}

	bool Repack(fs::path src_path, fs::path dst_dir_path)
	{
		if (!fs::is_directory(dst_dir_path)) {
			return false;
		}

		if (fs::is_regular_file(src_path)) {
			if (src_path.extension() == ".xml") {
				ReadXml(src_path);
			}
		}
		else {
			for (auto i : fs::recursive_directory_iterator{ src_path }) {
				if (i.is_regular_file() && i.path().extension() == ".xml") {
					ReadXml(i.path());
				}
			}
		}

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}

		for (auto& sir : dlgs) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
		for (auto& sir : names) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
		for (auto& sir : fonts) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}

		return true;
	}

	bool CopyValid(fs::path org_dir_path, fs::path dst_dir_path)
	{
		if (!fs::is_directory(org_dir_path) || !fs::is_directory(dst_dir_path)) {
			return false;
		}

		ReadSirDir(org_dir_path);

		std::vector<std::string> filenames;
		for (auto& sir : dlgs) filenames.push_back(sir->filename);
		for (auto& sir : names) filenames.push_back(sir->filename);
		for (auto& sir : fonts) filenames.push_back(sir->filename);

		for (auto& fn : filenames) {
			fs::copy_file(fs::path(org_dir_path).append(fn + ".sir"), fs::path(dst_dir_path).append(fn + ".sir"), fs::copy_options::overwrite_existing);
		}

		return true;
	}

	bool EditFontSize(fs::path org_dir_path, uint8_t size, fs::path dst_dir_path)
	{
		if (!fs::is_directory(dst_dir_path)) {
			return false;
		}

		ReadSirDir(org_dir_path);

		for (auto& sir : fonts) {
			sir->ModifySize(size);
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}

		return true;
	}

	bool GeneratePatchFontChars(fs::path org_dir_path, fs::path patch_dir_path, fs::path dst_file_path)
	{
		// kor
		wchar_t scope_min = 0xAC00;
		wchar_t scope_max = 0xD7AF;

		ReadSirDir(org_dir_path);

		for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
			auto ip = i.path();
			if (i.is_regular_file() && ip.extension() == ".xml") {
				if (StrCmpEndWith(ip.string(), ".dlg.xml") || StrCmpEndWith(ip.string(), ".name.xml")) {
					ReadXml(ip, true);
				}
			}
		}

		std::wstring kormap_string;
		{
			std::set<wchar_t> w_keycodes;
			RetrievePatchChars(scope_min, scope_max, w_keycodes);
			kormap_string.reserve(w_keycodes.size() + 10);
			for (auto& w : w_keycodes) {
				kormap_string += w;
			}
		}

		std::wstring ansimap_string;
		{
			std::set<uint8_t> ansi_map;
			RetriveAnsiChars(ansi_map);
			ansimap_string.reserve(ansi_map.size() + 10);
			for (auto& ch : ansi_map) {
				ansimap_string += ch;
			}
		}

		std::wofstream wof;
		wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
		wof.open(dst_file_path.wstring().c_str());
		wof << ansimap_string;
		wof << kormap_string;
		wof.close();

		if (wof.bad()) {
			return false;
		}

		return true;
	}

	bool GeneratePatchFontData(fs::path org_dir_path, fs::path patch_dir_path, fs::path bmf_default_path, fs::path bmf_border_path, fs::path dst_dir_path)
	{
		if (!fs::is_directory(dst_dir_path) || !fs::is_regular_file(bmf_default_path) || !fs::is_regular_file(bmf_border_path)) {
			return false;
		}

		ReadSirDir(org_dir_path);

		for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
			auto ip = i.path();
			if (i.is_regular_file() && ip.extension() == ".xml") {
				if (StrCmpEndWith(ip.string(), ".dlg.xml") || StrCmpEndWith(ip.string(), ".name.xml")) {
					ReadXml(ip, true);
				}
			}
		}

		std::array<fs::path, 2> bmf_path = { bmf_default_path, bmf_border_path };
		std::array<BMFont, 2> bmf;
		std::array<std::vector<std::vector<uint8_t>>, 2> png_buffer;
		std::array<uint32_t, 2> png_width{};
		std::array<uint32_t, 2> png_height{};
		std::array<uint32_t, 2> ch_width{};
		std::array<uint32_t, 2> ch_height{};

		for (int i = 0; i < 2; i++) {
			BMFontXmlReader::Read(bmf_path[i], bmf[i]);
			ch_width[i] = bmf[i].GetCharMaxWidthForRender();
			ch_height[i] = bmf[i].common.lineHeight;
			png_buffer[i].resize(bmf[i].pages.size());
			int pcount = 0;
			for (auto& p : bmf[i].pages) {
				PngReader::Read(bmf_path[i].parent_path().append(p.file), png_width[i], png_height[i], png_buffer[i][pcount]);
				pcount++;
			}
		}

		JpKeycodeAllocator keycode_alloc(0xF0, 0x00);
		char temp_jp_char[3] = {};
		for (auto& s : fonts) {
			for (auto& n : s->nodes) {
				if (n->keycode[1] != 0) {
					temp_jp_char[0] = n->keycode[1];
					temp_jp_char[1] = n->keycode[0];
					keycode_alloc.jp_map.emplace(temp_jp_char);
				}
			}
		}

		// kor
		wchar_t scope_min = 0xAC00;
		wchar_t scope_max = 0xD7AF;

		auto func_alloc_data = [&](std::shared_ptr<SirFont::Node>& fn, wchar_t ch) {
			for (int i = 0; i < 2; i++) {
				if (auto bmf_ch = stdext::FindPtr<BMFont::Char>(bmf[i].chars, [ch](const auto& elm) { return elm.id == ch; })) {
					fn->wsize[i] = bmf_ch->width;
					fn->hsize[i] = bmf_ch->height;
					fn->data[i].resize(fn->wsize[i] * fn->hsize[i], 0);
					auto& src_buf = png_buffer[i][bmf_ch->page];
					for (int y = 0; y < bmf_ch->height; y++) {
						for (int x = 0; x < bmf_ch->width; x++) {
							fn->data[i][y * bmf_ch->width + x] = src_buf[(bmf_ch->y + y) * png_width[i] + bmf_ch->x + x];
						}
					}
				}
				else {
					fn->wsize[i] = ch_width[i];
					fn->hsize[i] = ch_height[i];
					fn->data[i].resize(ch_width[i] * ch_height[i], 0);
				}
			}
		};

		SirFont new_f;

		std::set<uint8_t> ansi_map;
		RetriveAnsiChars(ansi_map);
		for (auto& ch : ansi_map) {
			auto fn = std::make_shared<SirFont::Node>();
			fn->keycode[0] = (char)ch;
			func_alloc_data(fn, ch);
			new_f.nodes.push_back(fn);
		}

		std::set<wchar_t> w_keycodes;
		RetrievePatchChars(scope_min, scope_max, w_keycodes);
		for (auto& w : w_keycodes) {
			keycode_alloc.Alloc();
			auto fn = std::make_shared<SirFont::Node>();
			fn->keycode[0] = keycode_alloc.keycode[1];
			fn->keycode[1] = keycode_alloc.keycode[0];
			fn->patch_keycode = wcs_to_utf8(std::wstring(1, w));
			func_alloc_data(fn, w);
			new_f.nodes.push_back(fn);
		}

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}

		for (auto& f : fonts) {
			new_f.filename = f->filename;
			new_f.footer_unknown_values = f->footer_unknown_values;
			SirXmlWriter::Write(new_f, fs::path(dst_dir_path).append(new_f.filename + ".font.xml"));
			SirPngWriter::Write(new_f, 0, fs::path(dst_dir_path).append(new_f.filename + ".default.png"));
			SirPngWriter::Write(new_f, 1, fs::path(dst_dir_path).append(new_f.filename + ".border.png"));
		}

		return true;
	}

	void RetrievePatchChars(wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes)
	{
		for (auto& ps : patch_dlgs) {
			auto d = stdext::FindPtr<SirDlg>(dlgs, [&ps](std::shared_ptr<SirDlg> f) {
				return f->filename == ps->filename;
			});
			if (d == nullptr) {
				continue;
			}

			for (auto& pn : ps->nodes) {
				auto n = stdext::FindPtr<SirDlg::Node>(d->nodes, [&pn](auto& arg) {
					return arg->id__ == pn->id__ && arg->type == pn->type;
				});
				if (n == nullptr) {
					continue;
				}

				if (pn->text != n->text) {
					auto modlen = pn->patch_text.length();
					for (std::size_t i = 0; i < modlen; i++) {
						if (pn->patch_text[i] >= scope_min && pn->patch_text[i] <= scope_max) {
							w_keycodes.insert(pn->patch_text[i]);
						}
					}
				}
			}
		}

		for (auto& ps : patch_names) {
			auto d = stdext::FindPtr<SirName>(names, [&ps](std::shared_ptr<SirName> f) {
				return f->filename == ps->filename;
			});
			if (d == nullptr) {
				continue;
			}

			for (auto& pn : ps->nodes) {
				auto n = stdext::FindPtr<SirName::Node>(d->nodes, [&pn](auto& arg) {
					return arg->key_name == pn->key_name && arg->key_msg == pn->key_msg;
				});
				if (n == nullptr) {
					continue;
				}

				if (pn->name != n->name) {
					auto modlen = pn->patch_name.length();
					for (std::size_t i = 0; i < modlen; i++) {
						if (pn->patch_name[i] >= scope_min && pn->patch_name[i] <= scope_max) {
							w_keycodes.insert(pn->patch_name[i]);
						}
					}
				}
			}
		}
	}

	void RetriveAnsiChars(std::set<uint8_t>& ansi_map)
	{
		for (auto& s : fonts) {
			for (auto& n : s->nodes) {
				if (n->keycode[1] == 0) {
					if ((uint8_t)n->keycode[0] <= 0x7E) {
						ansi_map.insert((uint8_t)n->keycode[0]);
					}
				}
			}
		}
	}


	bool Patch(fs::path org_dir_path, fs::path patch_dir_path, std::string option, fs::path dst_dir_path)
	{
		if (!fs::is_directory(org_dir_path) || !fs::is_directory(patch_dir_path) || !fs::is_directory(dst_dir_path)) {
			return false;
		}

		ReadSirDir(org_dir_path);

		for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
			if (i.is_regular_file()) {
				if (i.path().extension() == ".xml") {
					ReadXml(i.path(), true);
				}
			}
		}

		auto mod_size = (std::size_t)std::atoi(option.c_str());

		if (mod_size != 0) {
			for (auto& f : fonts) {
				f->ModifySize(mod_size);
			}
		}

		std::vector<SirDlg*> patched_dlgs;
		std::vector<SirName*> patched_names;
		std::vector<SirFont*> patched_fonts;

		for (auto& ps : patch_fonts) {
			auto f = stdext::FindPtr<SirFont>(fonts, [&ps](auto& elm) {
				return elm->filename == ps->filename;
			});
			if (f == nullptr) {
				continue;
			}

			for (auto& pn : ps->nodes) {
				if (pn->keycode[1] == 0) {
					auto idx = stdext::FindIdx(f->nodes, [&pn](auto& elm) { return elm->keycode[1] == 0 && elm->keycode[0] == pn->keycode[0]; });
					if (idx >= 0) {
						f->nodes[idx] = pn;
					}
				}
				else {
					patch_glyphs.emplace(utf8_to_wcs(pn->patch_keycode).front(), mbs_to_wcs(pn->SjisString(), L"").front());
					f->nodes.push_back(pn);
				}
			}
			patched_fonts.push_back(f);
		}

		for (auto& ps : patch_dlgs) {
			auto d = stdext::FindPtr<SirDlg>(dlgs, [&ps](std::shared_ptr<SirDlg> elm) {
				return elm->filename == ps->filename;
			});
			if (d == nullptr) {
				continue;
			}

			for (auto& pn : ps->nodes) {
				auto n = stdext::FindPtr<SirDlg::Node>(d->nodes, [&pn](auto& elm) {
					return elm->id__ == pn->id__ && elm->type == pn->type;
				});
				if (n == nullptr) {
					continue;
				}

				if (pn->text != n->text) {
					std::wstring fwtext;
					auto wlen = pn->patch_text.length();
					fwtext.reserve(wlen + 10);

					for (std::size_t i = 0; i < wlen; i++) {
						auto it = patch_glyphs.find(pn->patch_text[i]);
						if (it == patch_glyphs.end()) {
							fwtext += pn->patch_text[i];
							continue;
						}
						fwtext += it->second;
					}

					n->text = wcs_to_mbs(fwtext, "");
				}
			}
			patched_dlgs.push_back(d);
		}

		for (auto& ps : patch_names) {
			auto d = stdext::FindPtr<SirName>(names, [&ps](auto& elm) {
				return elm->filename == ps->filename;
			});
			if (d == nullptr) {
				continue;
			}

			for (auto& pn : ps->nodes) {
				auto n = stdext::FindPtr<SirName::Node>(d->nodes, [&pn](auto& elm) {
					return elm->key_name == pn->key_name && elm->key_msg == pn->key_msg;
				});
				if (n == nullptr) {
					continue;
				}

				if (pn->name != n->name) {
					auto wlen = pn->patch_name.length();

					std::wstring fwtext;
					fwtext.reserve(wlen + 10);

					for (std::size_t i = 0; i < wlen; i++) {
						auto it = patch_glyphs.find(pn->patch_name[i]);
						if (it == patch_glyphs.end()) {
							fwtext += pn->patch_name[i];
							continue;
						}
						fwtext += it->second;
					}

					n->name = wcs_to_mbs(fwtext, "");
				}
			}
			patched_names.push_back(d);
		}

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}

		for (auto sir : patched_dlgs) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
		for (auto sir : patched_names) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
		for (auto sir : patched_fonts) {
			SirWriter::Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}

		return true;
	}

	void ReadSirDir(fs::path dir_path)
	{
		for (auto i : fs::recursive_directory_iterator{ dir_path }) {
			if (i.is_directory()) {
				ReadSirDir(i);
			}
			else if (i.is_regular_file() && i.path().extension() == ".sir") {
				ReadSirFile(i);
			}
		}
	}

	bool ReadSirFile(fs::path file_path)
	{
		auto file_size = fs::file_size(file_path);
		if (file_size <= 0)
			return false;

		std::ifstream ifs(file_path, std::ifstream::binary);
		std::vector<char> buffer;
		buffer.resize(file_size + 1);
		buffer[file_size] = 0;
		ifs.rdbuf()->sgetn(&buffer[0], file_size);

		try {
			auto rbuffer = std::span(buffer.data(), file_size);
			if (SirReader::IsValidDlg(rbuffer)) {
				dlgs.push_back(SirReader::ReadDlg(file_path.stem().string(), rbuffer));
			}
			else if (SirReader::IsValidName(rbuffer)) {
				names.push_back(SirReader::ReadName(file_path.stem().string(), rbuffer));
			}
			else if (SirReader::IsValidFont(rbuffer)) {
				fonts.push_back(SirReader::ReadFont(file_path.stem().stem().string(), rbuffer));
			}
			else {
				return false;
			}
		}
		catch (const std::exception& e) {
			return false;
		}

		return true;
	}

	void ReadXml(fs::path file_path, int is_patch = false)
	{
		auto filename = file_path.filename().string();

		if (StrCmpEndWith(filename, ".dlg.xml")) {
			(is_patch ? patch_dlgs : dlgs).push_back(SirXmlReader::ReadDlg(file_path));
		}
		else if (StrCmpEndWith(filename, ".name.xml")) {
			(is_patch ? patch_names : names).push_back(SirXmlReader::ReadName(file_path));
		}
		else if (StrCmpEndWith(filename, ".font.xml")) {
			auto filename_without_ext = filename.substr(0, filename.length() - 9);
			(is_patch ? patch_fonts : fonts).push_back(SirXmlReader::ReadFont(
				file_path,
				fs::path(file_path.parent_path()).append(filename_without_ext + ".default.png"),
				fs::path(file_path.parent_path()).append(filename_without_ext + ".border.png")));
		}
	}

	std::vector<std::shared_ptr<SirDlg>> dlgs;
	std::vector<std::shared_ptr<SirName>> names;
	std::vector<std::shared_ptr<SirFont>> fonts;

	std::vector<std::shared_ptr<SirDlg>> patch_dlgs;
	std::vector<std::shared_ptr<SirName>> patch_names;
	std::vector<std::shared_ptr<SirFont>> patch_fonts;
	std::unordered_map<wchar_t, wchar_t> patch_glyphs;
};