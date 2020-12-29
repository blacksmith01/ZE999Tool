#include "SirTool.hpp"

#include "SirReader.hpp"
#include "SirXmlReader.hpp"
#include "SirWriter.hpp"
#include "SirXmlWriter.hpp"
#include "SirPngWriter.hpp"

bool SirTool::Unpack(const fs::path& src_path, const fs::path& dst_dir_path)
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

	SirXmlWriter::WriteAll(org_set.dlgs, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.names, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.fonts, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.items, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.msgs, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.descs, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.fcharts, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.docs, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.maps, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.credits, dst_dir_path);
	SirXmlWriter::WriteAll(org_set.rooms, dst_dir_path);

	return true;
}

bool SirTool::Repack(const fs::path& src_path, const fs::path& dst_dir_path)
{
	if (!fs::is_directory(dst_dir_path)) {
		return false;
	}

	if (fs::is_regular_file(src_path)) {
		if (src_path.extension() == ".xml") {
			ReadXml(src_path, org_set);
		}
	}
	else {
		for (auto i : fs::recursive_directory_iterator{ src_path }) {
			if (i.is_regular_file() && i.path().extension() == ".xml") {
				ReadXml(i.path(), org_set);
			}
		}
	}

	if (!fs::exists(dst_dir_path)) {
		fs::create_directory(dst_dir_path);
	}

	SirWriter::WriteAll(org_set.dlgs, dst_dir_path);
	SirWriter::WriteAll(org_set.names, dst_dir_path);
	SirWriter::WriteAll(org_set.fonts, dst_dir_path);
	SirWriter::WriteAll(org_set.items, dst_dir_path);
	SirWriter::WriteAll(org_set.msgs, dst_dir_path);
	SirWriter::WriteAll(org_set.descs, dst_dir_path);
	SirWriter::WriteAll(org_set.fcharts, dst_dir_path);
	SirWriter::WriteAll(org_set.docs, dst_dir_path);
	SirWriter::WriteAll(org_set.maps, dst_dir_path);
	SirWriter::WriteAll(org_set.credits, dst_dir_path);
	SirWriter::WriteAll(org_set.rooms, dst_dir_path);

	return true;
}

bool SirTool::CopyValid(const fs::path& org_dir_path, const fs::path& dst_dir_path)
{
	if (!fs::is_directory(org_dir_path) || !fs::is_directory(dst_dir_path)) {
		return false;
	}

	ReadSirDir(org_dir_path);

	std::vector<std::string> filenames;
	for (auto& sir : org_set.dlgs) filenames.push_back(sir->filename);
	for (auto& sir : org_set.names) filenames.push_back(sir->filename);
	for (auto& sir : org_set.fonts) filenames.push_back(sir->filename);
	for (auto& sir : org_set.items) filenames.push_back(sir->filename);
	for (auto& sir : org_set.msgs) filenames.push_back(sir->filename);
	for (auto& sir : org_set.descs) filenames.push_back(sir->filename);
	for (auto& sir : org_set.fcharts) filenames.push_back(sir->filename);
	for (auto& sir : org_set.docs) filenames.push_back(sir->filename);
	for (auto& sir : org_set.maps) filenames.push_back(sir->filename);
	for (auto& sir : org_set.credits) filenames.push_back(sir->filename);
	for (auto& sir : org_set.rooms) filenames.push_back(sir->filename);

	for (auto& fn : filenames) {
		fs::copy_file(fs::path(org_dir_path).append(fn + ".sir"), fs::path(dst_dir_path).append(fn + ".sir"), fs::copy_options::overwrite_existing);
	}

	return true;
}

bool SirTool::GeneratePatchFontChars(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& dst_file_path)
{
	// kor
	wchar_t scope_min = 0xAC00;
	wchar_t scope_max = 0xD7AF;

	ReadSirDir(org_dir_path);

	for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
		auto ip = i.path();
		if (i.is_regular_file() && ip.extension() == ".xml") {
			if (!StrCmpEndWith(ip.string(), SirFont::XmlExtension)) {
				ReadXml(ip, patch_set);
			}
		}
	}

	std::wstring kormap_string;
	{
		std::set<wchar_t> w_keycodes;
		RetrievePatchChars(scope_min, scope_max, w_keycodes);
		RetrieveExPatchChars(patch_dir_path, scope_min, scope_max, w_keycodes);
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

bool SirTool::GeneratePatchFontData(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& bmf_default_path, const fs::path& bmf_border_path, const fs::path& dst_dir_path)
{
	if (!fs::is_directory(dst_dir_path) || !fs::is_regular_file(bmf_default_path) || !fs::is_regular_file(bmf_border_path)) {
		return false;
	}

	ReadSirDir(org_dir_path);

	for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
		auto ip = i.path();
		if (i.is_regular_file() && ip.extension() == ".xml") {
			if (!StrCmpEndWith(ip.string(), SirFont::XmlExtension)) {
				ReadXml(ip, patch_set);
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
	for (auto& s : org_set.fonts) {
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

	std::array<int, 2> min_yoffsets = { bmf[0].GetCharMinYoffset(), bmf[1].GetCharMinYoffset() };

	auto func_alloc_data = [&](std::shared_ptr<SirFont::Node>& fn, wchar_t ch) {
		for (int i = 0; i < 2; i++) {
			if (auto bmf_ch = stdext::FindPtr<BMFont::Char>(bmf[i].chars, [ch, &min_yoffsets](const auto& elm) { return elm.id == ch; })) {
				int xoffset_mod = 0;
				int yoffset_mod = 0;
				if (bmf[i].force_offsets_to_zero) {
					fn->wsize[i] = bmf_ch->width;
					fn->hsize[i] = bmf_ch->height;
				}
				else {
					xoffset_mod = std::max(bmf_ch->xoffset + (int)bmf[i].info.padding[3], 0);
					yoffset_mod = std::max(bmf_ch->yoffset - min_yoffsets[i], 0);
					fn->wsize[i] = std::max(xoffset_mod + bmf_ch->width, bmf_ch->xadvance + bmf[i].info.padding[1] + bmf[i].info.padding[3]);
					fn->hsize[i] = yoffset_mod + bmf_ch->height;
				}
				fn->data[i].resize(fn->wsize[i] * fn->hsize[i], 0);
				auto& src_buf = png_buffer[i][bmf_ch->page];

				for (int y = 0; y < bmf_ch->height; y++) {
					for (int x = 0; x < bmf_ch->width; x++) {
						fn->data[i][(yoffset_mod + y) * fn->wsize[i] + xoffset_mod + x] = src_buf[(bmf_ch->y + y) * png_width[i] + bmf_ch->x + x];
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
	RetrieveExPatchChars(patch_dir_path, scope_min, scope_max, w_keycodes);
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

	for (auto& f : org_set.fonts) {
		new_f.filename = f->filename;
		new_f.footer_unknown_values = f->footer_unknown_values;
		SirXmlWriter::Write(new_f, fs::path(dst_dir_path).append(new_f.filename + SirFont::XmlExtension));
		SirPngWriter::Write(new_f, 0, fs::path(dst_dir_path).append(new_f.filename + SirFont::DefaultPngExtension));
		SirPngWriter::Write(new_f, 1, fs::path(dst_dir_path).append(new_f.filename + SirFont::BorderPngExtension));
	}

	return true;
}

void SirTool::RetrievePatchChars(wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes)
{
	for (auto& ps : patch_set.dlgs) {
		if (auto s = FindSirPtr(org_set.dlgs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirDlg::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn) && (an->text != pn->text); })) {
					RetrievePatchChars(pn->patch_text, scope_min, scope_max, w_keycodes);
				}
			}
		}
	}

	for (auto& ps : patch_set.names) {
		if (auto s = FindSirPtr(org_set.names, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (stdext::FindPtr<SirName::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn) && an->name != pn->name; })) {
					RetrievePatchChars(pn->patch_name, scope_min, scope_max, w_keycodes);
				}
			}
		}
	}

	for (auto& ps : patch_set.items) {
		if (auto s = FindSirPtr(org_set.items, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirItem::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirItem::Node::Item>(n->items, [&pi](std::shared_ptr<SirItem::Node::Item> f) { return f->Equal(*pi) && f->text1 != pi->text1; })) {
							RetrievePatchChars(pi->patch_text, scope_min, scope_max, w_keycodes);
						}
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.msgs) {
		if (auto s = FindSirPtr(org_set.msgs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirMsg::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn) && an->AllText() != pn->AllText(); })) {
					for (auto& pt : pn->patch_texts) {
						RetrievePatchChars(pt, scope_min, scope_max, w_keycodes);
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.descs) {
		if (auto s = FindSirPtr(org_set.descs, ps->filename)) {
			for (auto& pt : ps->texts) {
				if (auto t = stdext::FindPtr<SirDesc::Text>(s->texts, [&pt](auto& at) { return at->Equal(*pt) && at->value != pt->value; })) {
					RetrievePatchChars(pt->patch_text, scope_min, scope_max, w_keycodes);
				}
			}
		}
	}

	for (auto& ps : patch_set.fcharts) {
		if (auto s = FindSirPtr(org_set.fcharts, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirFChart::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn); })) {
					if (pn->text != n->text) {
						RetrievePatchChars(pn->patch_text, scope_min, scope_max, w_keycodes);
					}

					auto item_size = n->items.size();
					if (pn->items.size() == item_size) {
						for (std::size_t i = 0; i < item_size; i++) {
							auto& item = n->items[i];
							auto& pitem = pn->items[i];
							if (pitem->text != item->text) {
								RetrievePatchChars(pitem->patch_text, scope_min, scope_max, w_keycodes);
							}
						}
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.docs) {
		if (auto s = FindSirPtr(org_set.docs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirDoc::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn) && an->AllText() != pn->AllText(); })) {
					RetrievePatchChars(pn->patch_text1, scope_min, scope_max, w_keycodes);
					RetrievePatchChars(pn->patch_text2, scope_min, scope_max, w_keycodes);
					for (auto& pt : pn->patch_contents) {
						RetrievePatchChars(pt, scope_min, scope_max, w_keycodes);
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.maps) {
		if (auto s = FindSirPtr(org_set.maps, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirMap::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirMap::Node::Item>(n->items, [&pi](auto& ai) {return ai->Equal(*pi) && ai->text != pi->text; })) {
							RetrievePatchChars(pi->patch_text, scope_min, scope_max, w_keycodes);
						}
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.credits) {
		if (auto s = FindSirPtr(org_set.credits, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirCredit::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirCredit::Node::Item>(n->items, [&pi](auto& ai) {return ai->Equal(*pi) && ai->text != pi->text; })) {
							RetrievePatchChars(pi->patch_text, scope_min, scope_max, w_keycodes);
						}
					}
				}
			}
		}
	}

	for (auto& ps : patch_set.rooms) {
		if (auto s = FindSirPtr(org_set.rooms, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirRoom::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirRoom::Node::Item>(n->items, [&pi](auto& ai) {return ai->Equal(*pi) && ai->text != pi->text; })) {
							RetrievePatchChars(pi->patch_text, scope_min, scope_max, w_keycodes);
						}
					}
				}
			}
		}
	}
}

void SirTool::RetrieveExPatchChars(const fs::path& patch_dir_path, wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes)
{
	for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
		auto ip = i.path();
		if (i.is_regular_file() && ip.extension() == ".xml") {
			if (StrCmpEndWith(ip.string(), ".exe.xml")) {

				std::map<std::string, std::string> map;
				ReadExePatchFile(i, map);
				for (auto& p : map)
				{
					auto wtext = utf8_to_wcs(p.second);
					auto modlen = wtext.length();
					for (std::size_t i = 0; i < modlen; i++) {
						if (wtext[i] >= scope_min && wtext[i] <= scope_max) {
							w_keycodes.insert(wtext[i]);
						}
					}
				}

			}
		}
	}
}

void SirTool::RetrievePatchChars(const std::wstring& text, wchar_t scope_min, wchar_t scope_max, std::set<wchar_t>& w_keycodes)
{
	auto modlen = text.length();
	for (std::size_t i = 0; i < modlen; i++) {
		if (text[i] >= scope_min && text[i] <= scope_max) {
			w_keycodes.insert(text[i]);
		}
	}
}

void SirTool::ReadExePatchFile(const fs::path& file_path, std::map<std::string, std::string>& map)
{
	rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
	rapidxml::xml_document<char> doc;
	doc.parse<0>(xmlFile.data());

	auto node_dlgs = doc.first_node();
	auto node_dlg = node_dlgs->first_node();
	while (node_dlg != nullptr) {
		auto attr_dlg = node_dlg->first_attribute();
		std::string org_text;
		std::string new_text;
		while (attr_dlg != nullptr) {
			if (strcmp(attr_dlg->name(), "org") == 0) {
				auto hbytes = HexStringToBytes(attr_dlg->value());
				hbytes.push_back(0);
				org_text = (const char*)hbytes.data();
			}
			else if (strcmp(attr_dlg->name(), "new") == 0) {
				new_text = attr_dlg->value();
			}
			attr_dlg = attr_dlg->next_attribute();
		}
		if (!org_text.empty() && !new_text.empty() && mbs_to_wcs(org_text).size() >= utf8_to_wcs(new_text).size())
		{
			map.emplace(org_text, new_text);
		}
		node_dlg = node_dlg->next_sibling();
	}
}

void SirTool::RetriveAnsiChars(std::set<uint8_t>& ansi_map)
{
	for (auto& s : org_set.fonts) {
		for (auto& n : s->nodes) {
			if (n->keycode[1] == 0) {
				if ((uint8_t)n->keycode[0] <= 0x7E) {
					ansi_map.insert((uint8_t)n->keycode[0]);
				}
			}
		}
	}
}

std::string SirTool::PatchText(const std::wstring& text)
{
	std::wstring fwtext;
	auto wlen = text.length();
	fwtext.reserve(wlen + 10);

	for (std::size_t i = 0; i < wlen; i++) {
		auto it = patch_glyphs.find(text[i]);
		if (it == patch_glyphs.end()) {
			fwtext += text[i];
			continue;
		}
		fwtext += it->second;
	}

	return wcs_to_mbs(fwtext, "");
}

bool SirTool::Patch(const fs::path& org_dir_path, const fs::path& patch_dir_path, std::string option, const fs::path& dst_dir_path)
{
	if (!fs::is_directory(org_dir_path) || !fs::is_directory(patch_dir_path) || !fs::is_directory(dst_dir_path)) {
		return false;
	}

	ReadSirDir(org_dir_path);

	for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
		if (i.is_regular_file()) {
			if (i.path().extension() == ".xml") {
				ReadXml(i.path(), patch_set);
			}
		}
	}

	{
		std::wstring keep_jpchars;
		auto jpchars_path = fs::path(patch_dir_path).append("jpchars.txt");
		if (fs::exists(jpchars_path)) {
			std::vector<uint8_t> buffer;
			buffer.resize(fs::file_size(jpchars_path) + 1, 0);
			::ReadFile(jpchars_path, buffer.data());
			std::u8string ustr((char8_t*)buffer.data());
			keep_jpchars = utf8_to_wcs(std::string((char*)buffer.data()));
		}
		auto mod_size = (std::size_t)std::atoi(option.c_str());

		if (mod_size > 0) {
			for (auto& f : org_set.fonts) {
				f->ReduceKanjiSize(keep_jpchars, mod_size);
			}
		}
		else if (mod_size == 0) {
			for (auto& f : org_set.fonts) {
				f->RemoveKanji(keep_jpchars);
			}
		}
	}

	std::vector<SirDlg*> patched_dlgs;
	std::vector<SirName*> patched_names;
	std::vector<SirFont*> patched_fonts;
	std::vector<SirItem*> patched_items;
	std::vector<SirMsg*> patched_msgs;
	std::vector<SirDesc*> patched_descs;
	std::vector<SirFChart*> patched_fcharts;
	std::vector<SirDoc*> patched_docs;
	std::vector<SirMap*> patched_maps;
	std::vector<SirCredit*> patched_credits;
	std::vector<SirRoom*> patched_rooms;

	for (auto& ps : patch_set.fonts) {
		if (auto s = FindSirPtr(org_set.fonts, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (pn->keycode[1] == 0) {
					auto idx = stdext::FindIdx(s->nodes, [&pn](auto& elm) { return elm->keycode[1] == 0 && elm->keycode[0] == pn->keycode[0]; });
					if (idx >= 0) {
						s->nodes[idx] = pn;
					}
				}
				else {
					patch_glyphs.emplace(utf8_to_wcs(pn->patch_keycode).front(), mbs_to_wcs(pn->SjisString(), L"").front());
					s->nodes.push_back(pn);
				}
			}
			patched_fonts.push_back(s);
		}
	}

	for (auto& ps : patch_set.dlgs) {
		if (auto s = FindSirPtr(org_set.dlgs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirDlg::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn) && an->text != pn->text; })) {
					n->text = PatchText(pn->patch_text);
				}
			}
			patched_dlgs.push_back(s);
		}
	}

	for (auto& ps : patch_set.names) {
		if (auto s = FindSirPtr(org_set.names, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirName::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn) && an->name != pn->name; })) {
					n->name = PatchText(pn->patch_name);
				}
			}
			patched_names.push_back(s);
		}
	}

	for (auto& ps : patch_set.items) {
		if (auto s = FindSirPtr(org_set.items, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirItem::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirItem::Node::Item>(n->items, [&pi](auto& ai) { return ai->Equal(*pi) && ai->text1 != pi->text1; })) {
							i->text1 = PatchText(pi->patch_text);
						}
					}
				}
			}
			patched_items.push_back(s);
		}
	}

	for (auto& ps : patch_set.msgs) {
		if (auto s = FindSirPtr(org_set.msgs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirMsg::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn) && an->AllText() != pn->AllText(); })) {
					n->texts.clear();
					for (auto& pitem : pn->patch_texts) {
						n->texts.push_back(PatchText(pitem));
					}
				}
			}
			patched_msgs.push_back(s);
		}
	}

	for (auto& ps : patch_set.descs) {
		if (auto s = FindSirPtr(org_set.descs, ps->filename)) {
			for (auto& pn : ps->texts) {
				if (auto n = stdext::FindPtr<SirDesc::Text>(s->texts, [&pn](auto& an) { return an->Equal(*pn) && an->value != pn->value; })) {
					n->value = PatchText(pn->patch_text);
				}
			}
			patched_descs.push_back(s);
		}
	}

	for (auto& pl : patch_set.fcharts) {
		if (auto s = FindSirPtr(org_set.fcharts, pl->filename)) {
			for (auto& pn : pl->nodes) {
				if (auto n = stdext::FindPtr<SirFChart::Node>(s->nodes, [&pn](auto& an) {return an->Equal(*pn); })) {
					if (pn->text != n->text) {
						n->text = PatchText(pn->patch_text);
					}
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirFChart::Node::Item>(n->items, [&pi](auto& ai) { return ai->Equal(*pi) && ai->text != pi->text;})) {
							i->text = PatchText(pi->patch_text);
						}
					}
				}
			}
			patched_fcharts.push_back(s);
		}
	}

	for (auto& ps : patch_set.docs) {
		if (auto s = FindSirPtr(org_set.docs, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirDoc::Node>(s->nodes, [&pn](auto& an) { return an->Equal(*pn) && an->AllText() != pn->AllText(); })) {
					n->contents.clear();
					n->text1 = PatchText(pn->patch_text1);
					n->text2 = PatchText(pn->patch_text2);
					for (auto& pitem : pn->patch_contents) {
						n->contents.push_back(PatchText(pitem));
					}
				}
			}
			patched_docs.push_back(s);
		}
	}

	for (auto& ps : patch_set.maps) {
		if (auto s = FindSirPtr(org_set.maps, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirMap::Node>(s->nodes, [&pn](auto& elm) { return elm->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirMap::Node::Item>(n->items, [&pi](auto& ai) { return ai->Equal(*pi) && ai->text != pi->text; })) {
							i->text = PatchText(pi->patch_text);
						}
					}
				}
			}
			patched_maps.push_back(s);
		}
	}

	for (auto& ps : patch_set.credits) {
		if (auto s = FindSirPtr(org_set.credits, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirCredit::Node>(s->nodes, [&pn](auto& elm) { return elm->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirCredit::Node::Item>(n->items, [&pi](auto& ai) { return ai->Equal(*pi) && ai->text != pi->text; })) {
							i->text = PatchText(pi->patch_text);
						}
					}
				}
			}
			patched_credits.push_back(s);
		}
	}

	for (auto& ps : patch_set.rooms) {
		if (auto s = FindSirPtr(org_set.rooms, ps->filename)) {
			for (auto& pn : ps->nodes) {
				if (auto n = stdext::FindPtr<SirRoom::Node>(s->nodes, [&pn](auto& elm) { return elm->Equal(*pn); })) {
					for (auto& pi : pn->items) {
						if (auto i = stdext::FindPtr<SirRoom::Node::Item>(n->items, [&pi](auto& ai) { return ai->Equal(*pi) && ai->text != pi->text; })) {
							i->text = PatchText(pi->patch_text);
						}
					}
				}
			}
			patched_rooms.push_back(s);
		}
	}

	if (!fs::exists(dst_dir_path)) {
		fs::create_directory(dst_dir_path);
	}

	SirWriter::WriteAll(patched_dlgs, dst_dir_path);
	SirWriter::WriteAll(patched_names, dst_dir_path);
	SirWriter::WriteAll(patched_fonts, dst_dir_path);
	SirWriter::WriteAll(patched_items, dst_dir_path);
	SirWriter::WriteAll(patched_msgs, dst_dir_path);
	SirWriter::WriteAll(patched_descs, dst_dir_path);
	SirWriter::WriteAll(patched_fcharts, dst_dir_path);
	SirWriter::WriteAll(patched_docs, dst_dir_path);
	SirWriter::WriteAll(patched_maps, dst_dir_path);
	SirWriter::WriteAll(patched_credits, dst_dir_path);
	SirWriter::WriteAll(patched_rooms, dst_dir_path);

	return true;
}

bool SirTool::ExePatch(const fs::path& org_dir_path, const fs::path& patch_dir_path, const fs::path& exe_file_path, const fs::path& dst_dir_path)
{
	auto xml_file_path = fs::path(patch_dir_path).append("ze1.exe.xml");

	if (!fs::is_directory(org_dir_path) || !fs::exists(exe_file_path) || !fs::exists(xml_file_path)) {
		return false;
	}

	ReadSirDir(org_dir_path);

	for (auto i : fs::recursive_directory_iterator{ patch_dir_path }) {
		if (i.is_regular_file()) {
			if (StrCmpEndWith(i.path().filename().string(), SirFont::XmlExtension))
			{
				ReadXml(i.path(), patch_set);
			}
		}
	}

	std::vector<SirFont*> patched_fonts;

	for (auto& ps : patch_set.fonts) {
		auto f = FindSirPtr(org_set.fonts, ps->filename);
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


	std::map<std::string, std::string> replace_map;
	{
		std::map<std::string, std::string> temp_map;
		ReadExePatchFile(xml_file_path, temp_map);
		for (auto& p : temp_map)
		{
			auto wtext = utf8_to_wcs(p.second);
			auto wlen = wtext.length();

			std::wstring fwtext;
			fwtext.reserve(wlen + 10);

			for (std::size_t i = 0; i < wlen; i++) {
				auto it = patch_glyphs.find(wtext[i]);
				if (it == patch_glyphs.end()) {
					fwtext += wtext[i];
					continue;
				}
				fwtext += it->second;
			}

			std::string ftext = wcs_to_mbs(fwtext, "");
			if (!ftext.empty())
				replace_map.emplace(p.first, ftext);
		}
	}

	std::vector<char> buffer;
	buffer.resize(fs::file_size(exe_file_path));
	::ReadFile(exe_file_path, buffer.data());
	auto buffer_size = buffer.size();

	for (std::size_t i = 0; i < buffer_size; )
	{
		std::size_t changed = 0;
		for (auto& p : replace_map)
		{
			auto& org_text = p.first;
			if (org_text.front() == buffer[i] && i + org_text.size() < buffer_size)
			{
				if (strncmp(org_text.c_str(), buffer.data() + i, org_text.size()) == 0)
				{
					memset(buffer.data() + i, 0, org_text.size());
					memcpy(buffer.data() + i, p.second.c_str(), p.second.size());
					changed = org_text.size();
					break;
				}
			}
		}

		if (changed)
			i += changed;
		else
			i++;
	}

	if (!fs::exists(dst_dir_path))
		fs::create_directory(dst_dir_path);

	std::ofstream ofs(fs::path(dst_dir_path).append(exe_file_path.filename().string()), std::ios::binary);
	ofs.write((char*)buffer.data(), buffer.size());
	if (ofs.bad()) {
		return false;
	}

	return true;
}

void SirTool::ReadSirDir(const fs::path& dir_path)
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

bool SirTool::ReadSirFile(const fs::path& file_path)
{
	auto file_size = fs::file_size(file_path);
	if (file_size <= 20) // sir1 + footer_beg + footer_end
		return false;

	std::ifstream ifs(file_path, std::ifstream::binary);
	std::vector<char> buffer;
	buffer.resize(file_size + 1);
	buffer[file_size] = 0;
	ifs.rdbuf()->sgetn(&buffer[0], file_size);

	try {
		auto rbuffer = std::span(buffer.data(), file_size);
		if (SirReader::IsValidDlg(rbuffer)) {
			org_set.dlgs.push_back(SirReader::ReadDlg(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidName(rbuffer)) {
			org_set.names.push_back(SirReader::ReadName(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidFont(rbuffer)) {
			org_set.fonts.push_back(SirReader::ReadFont(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidItem(rbuffer)) {
			org_set.items.push_back(SirReader::ReadItem(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidMsg(rbuffer)) {
			org_set.msgs.push_back(SirReader::ReadMsg(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidDesc(rbuffer)) {
			org_set.descs.push_back(SirReader::ReadDesc(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidFChart(rbuffer)) {
			org_set.fcharts.push_back(SirReader::ReadFChart(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidDoc(rbuffer)) {
			org_set.docs.push_back(SirReader::ReadDoc(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidMap(rbuffer)) {
			org_set.maps.push_back(SirReader::ReadMap(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidCredit(rbuffer)) {
			org_set.credits.push_back(SirReader::ReadCredit(file_path.stem().string(), rbuffer));
		}
		else if (SirReader::IsValidRoom(rbuffer)) {
			org_set.rooms.push_back(SirReader::ReadRoom(file_path.stem().string(), rbuffer));
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

void SirTool::ReadXml(const fs::path& file_path, SirSet& set)
{
	auto filename = file_path.filename().string();

	if (StrCmpEndWith(filename, SirDlg::XmlExtension)) {
		set.dlgs.push_back(SirXmlReader::ReadDlg(file_path));
	}
	else if (StrCmpEndWith(filename, SirName::XmlExtension)) {
		set.names.push_back(SirXmlReader::ReadName(file_path));
	}
	else if (StrCmpEndWith(filename, SirFont::XmlExtension)) {
		auto filename_without_ext = filename.substr(0, filename.length() - 9);
		set.fonts.push_back(SirXmlReader::ReadFont(
			file_path,
			fs::path(file_path.parent_path()).append(filename_without_ext + SirFont::DefaultPngExtension),
			fs::path(file_path.parent_path()).append(filename_without_ext + SirFont::BorderPngExtension)));
	}
	else if (StrCmpEndWith(filename, SirItem::XmlExtension)) {
		set.items.push_back(SirXmlReader::ReadItem(file_path));
	}
	else if (StrCmpEndWith(filename, SirMsg::XmlExtension)) {
		set.msgs.push_back(SirXmlReader::ReadMsg(file_path));
	}
	else if (StrCmpEndWith(filename, SirDesc::XmlExtension)) {
		set.descs.push_back(SirXmlReader::ReadDesc(file_path));
	}
	else if (StrCmpEndWith(filename, SirFChart::XmlExtension)) {
		set.fcharts.push_back(SirXmlReader::ReadFChart(file_path));
	}
	else if (StrCmpEndWith(filename, SirDoc::XmlExtension)) {
		set.docs.push_back(SirXmlReader::ReadDoc(file_path));
	}
	else if (StrCmpEndWith(filename, SirMap::XmlExtension)) {
		set.maps.push_back(SirXmlReader::ReadMap(file_path));
	}
	else if (StrCmpEndWith(filename, SirCredit::XmlExtension)) {
		set.credits.push_back(SirXmlReader::ReadCredit(file_path));
	}
	else if (StrCmpEndWith(filename, SirRoom::XmlExtension)) {
		set.rooms.push_back(SirXmlReader::ReadRoom(file_path));
	}
}
