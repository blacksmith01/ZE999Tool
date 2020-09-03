#pragma once

#include "Sir.hpp"

class SirReader {
public:
	static std::shared_ptr<SirDlg> ReadDlg(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirDlg>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos;

		if (buffer.size() < (curr_pos - orig_pos) + 20) {
			throw std::exception("invalid memory access");
		}
		memcpy(sir->header_sig, curr_pos, 4); curr_pos += 4;
		auto footer_beg = *(uint64_t*)curr_pos; curr_pos += 8;
		auto footer_end = *(uint64_t*)curr_pos; curr_pos += 8;
		auto content_pos = curr_pos;
		auto content_size = (orig_pos + footer_beg) - content_pos;

		if (buffer.size() < footer_end) {
			throw std::exception("invalid memory access");
		}
		curr_pos = orig_pos + footer_beg;

		std::vector<uint64_t> offsets;
		for (int i = 0; i < (footer_end - footer_beg); i += 8, curr_pos += 8) {
			auto offset = *(const uint64_t*)curr_pos;
			if (offset == 0) {
				/* zero */curr_pos += 16;
				/* footer_beg */curr_pos += 8;
				/* zero */curr_pos += 8;
				/* zero */curr_pos += 16;
				auto node_padding = (16 - ((curr_pos - orig_pos) & 0x0F)) & 0x0F; curr_pos += node_padding;
				break;
			}
			offsets.push_back(offset);
		}

		sir->nodes.reserve(offsets.size());
		for (int i = 0; i < offsets.size(); i += 4) {

			sir->nodes.push_back(ReadDlgNode(buffer, offsets[i]));
		}

		return sir;
	}
	static std::shared_ptr<SirDlg::Node> ReadDlgNode(const std::span<char>& buffer, uint64_t offset)
	{
		const char* orig_pos = buffer.data();
		auto curr_pos = buffer.data() + offset;
		auto n = std::make_shared<SirDlg::Node>();

		std::array<std::string*, 4> str_array = { &n->id__, &n->type, &n->name, &n->text };
		for (auto& str : str_array) {
			if (buffer.size() < (curr_pos - orig_pos) + strlen(curr_pos) + 1) {
				throw std::exception("invalid memory access");
			}
			*str = curr_pos; curr_pos += strlen(curr_pos) + 1;
		}
		return n;
	}
	static bool IsValidDlg(const std::span<char>& buffer)
	{
		try {
			uint64_t offset = 4 + 8 + 8;
			auto n = ReadDlgNode(buffer, offset);
			auto dlg_id_len = n->id__.length();
			if (dlg_id_len < 15 || n->id__.find('_') == std::string::npos) {
				return false;
			}
			if (!isdigit(n->id__.at(dlg_id_len - 2)) || !isdigit(n->id__.at(dlg_id_len - 1))) {
				return false;
			}
			if (n->type.length() == 0) {
				return false;
			}
			return true;
		}
		catch (const std::exception& e) {
			return false;
		}
	}

	static std::shared_ptr<SirName> ReadName(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirName>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos;

		if (buffer.size() < (curr_pos - orig_pos) + 20) {
			throw std::exception("invalid memory access");
		}
		memcpy(sir->header_sig, curr_pos, 4); curr_pos += 4;
		auto footer_beg = *(uint64_t*)curr_pos; curr_pos += 8;
		auto footer_end = *(uint64_t*)curr_pos; curr_pos += 8;
		auto content_pos = curr_pos;
		auto content_size = (orig_pos + footer_beg) - content_pos;

		if (buffer.size() < footer_end) {
			throw std::exception("invalid memory access");
		}
		curr_pos = orig_pos + footer_beg;

		std::vector<uint64_t> node_types;
		std::vector<uint64_t> offsets;
		for (int i = 0; i < (footer_end - footer_beg); i += 8, curr_pos += 8) {
			auto offset = *(const uint64_t*)curr_pos;
			if (offset == 0) {
				/* zero */curr_pos += 16;
				/* footer_beg */curr_pos += 8;
				/* zero */curr_pos += 8;
				/* zero */curr_pos += 16;
				auto node_padding = (16 - ((curr_pos - orig_pos) & 0x0F)) & 0x0F; curr_pos += node_padding;
				break;
			}
			if ((i / 8) % 5 == 3) {
				node_types.push_back(offset);
			}
			else {
				if (offset >= buffer.size()) {
					throw std::exception("invalid memory access");
				}
				offsets.push_back(offset);
			}
		}

		if (offsets.empty() || offsets.size() / 4 > node_types.size()) {
			throw std::exception("invalid memory access");
		}

		sir->nodes.reserve(offsets.size());
		for (int i = 0; i < offsets.size(); i += 4) {
			sir->nodes.push_back(ReadNameNode(buffer, offsets[i]));
			auto uv = node_types[sir->nodes.size() - 1];
			sir->nodes.back()->unknown_value[0] = *(((const uint32_t*)&uv) + 0);
			sir->nodes.back()->unknown_value[1] = *(((const uint32_t*)&uv) + 1);
		}

		auto special_codes = uintvar(footer_beg - 12).bytes;

		return sir;
	}
	static std::shared_ptr<SirName::Node> ReadNameNode(const std::span<char>& buffer, uint64_t offset)
	{
		const char* orig_pos = buffer.data();
		auto curr_pos = buffer.data() + offset;
		auto n = std::make_shared<SirName::Node>();

		std::array<std::string*, 4> str_array = { &n->key_name, &n->name, &n->key_msg, &n->msg };
		for (auto& str : str_array) {
			if (buffer.size() < (curr_pos - orig_pos) + strlen(curr_pos) + 1) {
				throw std::exception("invalid memory access");
			}
			*str = curr_pos; curr_pos += strlen(curr_pos) + 1;
		}
		return n;
	}
	static bool IsValidName(const std::span<char>& buffer)
	{
		try {
			uint64_t offset = 4 + 8 + 8;
			auto n = ReadNameNode(buffer, offset);
			if (n->key_name != "&NONE") {
				return false;
			}
			if (n->key_msg.empty() || n->key_name.empty() || n->msg.empty() || n->name.empty()) {
				return false;
			}
			return true;
		}
		catch (const std::exception& e) {
			return false;
		}
	}

	static std::shared_ptr<SirFont> ReadFont(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirFont>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos;

		if (buffer.size() < (curr_pos - orig_pos) + 20) {
			throw std::exception("invalid memory access");
		}
		memcpy(sir->header_sig, curr_pos, 4); curr_pos += 4;
		auto footer_beg = *(uint64_t*)curr_pos; curr_pos += 8;
		auto footer_end = *(uint64_t*)curr_pos; curr_pos += 8;
		auto content_pos = curr_pos;
		auto content_size = (orig_pos + footer_beg) - content_pos;

		if (buffer.size() < footer_end) {
			throw std::exception("invalid memory access");
		}
		auto footer_count = *((uint64_t*)(orig_pos + footer_beg));
		memcpy(&sir->footer_unknown_values[0], orig_pos + footer_beg + 8, 12);

		std::vector<uint32_t> footer_font_values;
		for (auto i = 0; i < footer_count; i++) {
			footer_font_values.push_back(*(uint32_t*)(orig_pos + footer_beg + 8 + 12 + i * 4));
		}
		std::vector<uint8_t> special_codes;
		special_codes.resize(16);
		memcpy(special_codes.data(), orig_pos + buffer.size() - 16, +16);

		sir->nodes.reserve(footer_font_values.size());
		uint64_t offset = 4 + 8 + 8;
		while (offset < footer_beg) {
			sir->nodes.push_back(ReadFontNode(buffer, offset));
			auto& back = sir->nodes.back();
			offset += back->Size();
		}

		return sir;
	}
	static std::shared_ptr<SirFont::Node> ReadFontNode(const std::span<char>& buffer, uint64_t offset)
	{
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos + offset;
		auto n = std::make_shared<SirFont::Node>();
		if (buffer.size() < (curr_pos - orig_pos) + 10) {
			throw std::exception("invalid memory access");
		}
		memcpy(n->keycode, curr_pos, 6); curr_pos += 6;
		n->wsize[0] = *(uint8_t*)curr_pos; curr_pos += 1;
		n->hsize[0] = *(uint8_t*)curr_pos; curr_pos += 1;
		n->wsize[1] = *(uint8_t*)curr_pos; curr_pos += 1;
		n->hsize[1] = *(uint8_t*)curr_pos; curr_pos += 1;
		auto f1_size = n->wsize[0] * n->hsize[0];
		auto f2_size = n->wsize[1] * n->hsize[1];
		if (f1_size <= 0 || f2_size <= 0) {
			throw std::exception("invalid memory access");
		}
		if (buffer.size() < (curr_pos - orig_pos) + f1_size + f2_size) {
			throw std::exception("invalid memory access");
		}
		n->data[0].resize(f1_size);
		n->data[1].resize(f2_size);
		memcpy(&n->data[0][0], curr_pos, f1_size); curr_pos += f1_size;
		memcpy(&n->data[1][0], curr_pos, f2_size); curr_pos += f2_size;
		auto padding = n->Padding();
		if (buffer.size() < (curr_pos - orig_pos) + padding) {
			throw std::exception("invalid memory access");
		}
		curr_pos += n->Padding();
		return n;
	}
	static bool IsValidFont(const std::span<char>& buffer)
	{
		try {
			uint64_t offset = 4 + 8 + 8;
			auto n = ReadFontNode(buffer, offset);
			if (n->wsize[0] == 0 || n->hsize[0] == 0) {
				return false;
			}
			for (int i = 0; i < 4; i++) {
				if (n->keycode[i + 2] != 0) {
					return false;
				}
			}
			return true;
		}
		catch (const std::exception& e) {
			return false;
		}
	}
};

class SirXmlReader
{
public:
	static std::shared_ptr<SirDlg> ReadDlg(fs::path file_path)
	{
		auto sir = std::make_shared<SirDlg>();
		sir->filename = file_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();
		auto node_dlgs = node_sir->first_node();
		auto node_dlg = node_dlgs->first_node();
		while (node_dlg != nullptr) {
			auto n = std::make_shared<SirDlg::Node>();
			auto attr_dlg = node_dlg->first_attribute();
			while (attr_dlg != nullptr) {
				if (strcmp(attr_dlg->name(), "id") == 0) {
					n->id__ = attr_dlg->value();
				}
				else if (strcmp(attr_dlg->name(), "type") == 0) {
					n->type = attr_dlg->value();
				}
				else if (strcmp(attr_dlg->name(), "name") == 0) {
					n->patch_name = utf8_to_wcs(attr_dlg->value());
					n->name = wcs_to_mbs(n->patch_name, "");
				}
				else if (strcmp(attr_dlg->name(), "text") == 0) {
					n->patch_text = utf8_to_wcs(attr_dlg->value());
					n->text = wcs_to_mbs(n->patch_text, "");
				}

				attr_dlg = attr_dlg->next_attribute();
			}

			sir->nodes.push_back(n);
			node_dlg = node_dlg->next_sibling();
		}

		return sir;
	}

	static std::shared_ptr<SirName> ReadName(fs::path file_path)
	{
		auto sir = std::make_shared<SirName>();
		sir->filename = file_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();
		auto node_dlgs = node_sir->first_node();
		auto node_dlg = node_dlgs->first_node();
		while (node_dlg != nullptr) {
			auto n = std::make_shared<SirName::Node>();
			auto attr_dlg = node_dlg->first_attribute();
			while (attr_dlg != nullptr) {
				if (strcmp(attr_dlg->name(), "key") == 0) {
					n->key_name = utf8_to_mbs(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "name") == 0) {
					n->patch_name = utf8_to_wcs(attr_dlg->value());
					n->name = wcs_to_mbs(n->patch_name, "");
				}
				else if (strcmp(attr_dlg->name(), "kye2") == 0) {
					n->key_msg = utf8_to_mbs(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "msg") == 0) {
					n->msg = utf8_to_mbs(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "unknown1") == 0) {
					n->unknown_value[0] = atoi(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "unknown2") == 0) {
					n->unknown_value[1] = atoi(attr_dlg->value());
				}

				attr_dlg = attr_dlg->next_attribute();
			}

			sir->nodes.push_back(n);
			node_dlg = node_dlg->next_sibling();
		}

		return sir;
	}

	static std::shared_ptr<SirFont> ReadFont(fs::path xmlfile_path, fs::path default_png_file_path, fs::path border_png_file_path)
	{
		auto sir = std::make_shared<SirFont>();
		sir->filename = xmlfile_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		std::vector<uint8_t> png_buffers[2];
		PngReader::Read(default_png_file_path, png_buffers[0]);
		PngReader::Read(border_png_file_path, png_buffers[1]);

		rapidxml::file<char> xmlFile(xmlfile_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();
		auto node_png = node_sir->first_node();
		std::array<uint32_t, 2> wcount{};
		std::array<uint32_t, 2> hcount{};
		std::array<uint32_t, 2> png_width{};
		std::array<uint32_t, 2> png_height{};
		std::array<uint32_t, 2> font_width{};
		std::array<uint32_t, 2> font_height{};
		for (int i = 0; i < 2; i++)
		{
			auto attr_png = node_png->first_attribute();
			while (attr_png != nullptr) {
				if (false) {}
				M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "png_width", png_width[i])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "png_height", png_height[i])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "font_width", font_width[i])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "font_height", font_height[i])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "font_wcount", wcount[i])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_png, "font_hcount", hcount[i])
					attr_png = attr_png->next_attribute();
			}

			node_png = node_png->next_sibling();;
		}

		auto node_etc = node_png;
		{
			auto attr_etc = node_etc->first_attribute();
			while (attr_etc != nullptr) {
				if (false) {}
				M_RAPID_XML_ATTR_IF_SET_INT(attr_etc, "unknown1", sir->footer_unknown_values[0])
					M_RAPID_XML_ATTR_IF_SET_INT(attr_etc, "unknown2", sir->footer_unknown_values[1])
					attr_etc = attr_etc->next_attribute();
			}
		}

		auto node_fonts = node_etc->next_sibling();
		auto node_font = node_fonts->first_node();
		int inode = 0;
		while (node_font != nullptr) {
			auto n = std::make_shared<SirFont::Node>();
			auto attr_font = node_font->first_attribute();
			while (attr_font != nullptr) {
				if (strcmp(attr_font->name(), "bin") == 0) {
					auto hex_bytes = HexStringToBytes(attr_font->value());
					hex_bytes.resize(2);
					if (hex_bytes[1] == 0) {
						n->keycode[0] = hex_bytes[0];
					}
					else {
						n->keycode[0] = hex_bytes[1];
						n->keycode[1] = hex_bytes[0];
					}
				}
				else if (strcmp(attr_font->name(), "ch") == 0) {
					n->patch_keycode = attr_font->value();
				}
				else if (strcmp(attr_font->name(), "default_w") == 0) {
					n->wsize[0] = atoi(attr_font->value());
				}
				else if (strcmp(attr_font->name(), "default_h") == 0) {
					n->hsize[0] = atoi(attr_font->value());
				}
				else if (strcmp(attr_font->name(), "border_w") == 0) {
					n->wsize[1] = atoi(attr_font->value());
				}
				else if (strcmp(attr_font->name(), "border_h") == 0) {
					n->hsize[1] = atoi(attr_font->value());
				}
				attr_font = attr_font->next_attribute();
			}
			for (int i = 0; i < 2; i++) {
				auto png_xpos = (inode % wcount[i]) * font_width[i];
				auto png_ypos = (inode / wcount[i]) * font_height[i];

				auto data_size = n->wsize[i] * n->hsize[i];
				n->data[i].resize(data_size);
				for (int y = 0; y < n->hsize[i]; y++) {
					for (int x = 0; x < n->wsize[i]; x++) {
						n->data[i][y * n->wsize[i] + x] = png_buffers[i][(png_ypos + y) * png_width[i] + png_xpos + x];
					}
				}
			}

			sir->nodes.push_back(n);
			node_font = node_font->next_sibling();
			inode++;
		}

		return sir;
	}
};