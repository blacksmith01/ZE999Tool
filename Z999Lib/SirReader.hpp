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
		auto curr_pos = orig_pos + offset;
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

		return sir;
	}
	static std::shared_ptr<SirName::Node> ReadNameNode(const std::span<char>& buffer, uint64_t offset)
	{
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos + offset;
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

	static std::shared_ptr<SirItem> ReadItem(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirItem>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos;

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
		for (int i = 0; i < (footer_end - footer_beg); i += 16, curr_pos += 16) {
			auto offset = *(const uint64_t*)curr_pos;
			if (offset == 0) {
				break;
			}
			offsets.push_back(curr_pos - orig_pos);
		}

		sir->nodes.reserve(offsets.size());
		for (int i = 0; i < offsets.size(); i++) {
			sir->nodes.push_back(ReadItemNode(buffer, offsets[i]));
		}

		return sir;
	}
	static std::shared_ptr<SirItem::Node> ReadItemNode(const std::span<char>& buffer, uint64_t offset)
	{
		const char* orig_pos = buffer.data();

		auto data_offset = *((uint64_t*)(orig_pos + offset) + 0);
		auto info_offset = *((uint64_t*)(orig_pos + offset) + 1);

		MemReader reader(buffer);
		reader.Forward(data_offset);

		auto n = std::make_shared<SirItem::Node>();
		n->name = reader.Ptr();

		reader.Seek(info_offset);
		while (true) {
			auto offset_key = reader.Read<uint64_t>();
			if (offset_key == 0) {
				break;
			}

			auto offset_text1 = reader.Read<uint64_t>();
			auto offset_text2 = reader.Read<uint64_t>();
			if (offset_key >= buffer.size() || offset_text1 >= buffer.size() || offset_text2 >= buffer.size()) {
				throw std::exception("invalid memory access");
			}

			auto item = std::make_shared<SirItem::Node::Item>();
			item->key = orig_pos + offset_key;
			item->text1 = orig_pos + offset_text1;
			item->text2 = orig_pos + offset_text2;
			reader.ReadArray(item->unknowns);
			n->items.push_back(item);
		}

		return n;
	}
	static bool IsValidItem(const std::span<char>& buffer)
	{
		try {
			auto n = ReadItemNode(buffer, *(uint64_t*)(&buffer.front() + 4));
			if (n->items.empty()) {
				return false;
			}
			auto& front = n->items.front();
			if (front->key.empty() || front->key.front() != '^') {
				return false;
			}
			return true;
		}
		catch (const std::exception& e) {
			return false;
		}
	}

	static std::shared_ptr<SirMsg> ReadMsg(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirMsg>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();
		auto curr_pos = orig_pos;

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
		for (; curr_pos < orig_pos + footer_end; curr_pos += 32) {
			auto offset = *(const uint64_t*)curr_pos;
			if (offset == 0) {
				break;
			}
			offsets.push_back(curr_pos - orig_pos);
		}

		if (offsets.empty() || offsets.front() + 24 >= buffer.size()) {
			throw std::exception("invalid file");
		}

		auto data_max_pos = orig_pos + *((uint64_t*)(orig_pos + (offsets.front() + 24))) - 1;
		while (*data_max_pos == (char)0xAA) {
			data_max_pos--;
		}

		curr_pos += sizeof(uint64_t) * 3;
		sir->unknown = *(const uint64_t*)curr_pos;

		sir->nodes.reserve(offsets.size());
		for (int i = 0; i < offsets.size(); i++) {

			sir->nodes.push_back(ReadMsgNode(buffer, offsets[i], i + 1 == offsets.size() ? (data_max_pos - orig_pos) : *((uint64_t*)(orig_pos + offsets[i + 1]))));
		}

		return sir;
	}
	static std::shared_ptr<SirMsg::Node> ReadMsgNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset)
	{
		const char* orig_pos = buffer.data();

		MemReader reader(buffer);
		reader.Seek(offset);

		auto n = std::make_shared<SirMsg::Node>();

		auto data_offset = reader.Read<uint64_t>();
		reader.ReadArray(n->unknowns);
		auto value_offset = reader.Read<uint64_t>();

		reader.Seek(data_offset);

		n->key = reader.Ptr();
		reader.Forward(n->key.length() + 1);

		do {
			n->texts.push_back(reader.Ptr());
			reader.Forward(n->texts.back().length() + 1);
		} while (reader.Ptr() - orig_pos < next_offset);

		return n;
	}
	static bool IsValidMsg(const std::span<char>& buffer)
	{
		try {
			auto n = ReadMsgNode(buffer, *(uint64_t*)(&buffer.front() + 4), 0);
			if (n->key != "START_CREATE_FIRST") {
				return false;
			}
			return true;
		}
		catch (const std::exception& e) {
			return false;
		}
	}

	static std::shared_ptr<SirDesc> ReadDesc(std::string filename, const std::span<char>& buffer)
	{
		auto sir = std::make_shared<SirDesc>();
		sir->filename = filename;
		const char* orig_pos = buffer.data();

		MemReader reader(buffer);

		reader.ReadArray(4, sir->header_sig);
		auto footer_beg = reader.Read<uint64_t>();
		auto footer_end = reader.Read<uint64_t>();

		reader.Seek(footer_beg);

		auto sound_file_name_offset = reader.Read<uint64_t>();
		auto node_info_offset = reader.Read<uint64_t>();
		auto text_count = reader.Read<uint32_t>();

		auto text_offset = reader.Read<uint64_t>();
		auto start_offset = reader.Read<uint64_t>();
		auto var_offset = reader.Read<uint64_t>();

		if (buffer.size() < sound_file_name_offset || buffer.size() < node_info_offset) {
			throw std::exception("invalid memory access");
		}

		sir->sound_file_name = orig_pos + sound_file_name_offset;

		if (buffer.size() < text_offset || buffer.size() < start_offset || buffer.size() < var_offset) {
			throw std::exception("invalid memory access");
		}

		reader.Seek(node_info_offset);
		std::vector<uint64_t> bin_data_offsets;
		std::vector<uint64_t> bin_tetxt_offsets;
		while (reader.Ptr() < orig_pos + footer_beg) {
			auto data_offset = reader.Read<uint64_t>();
			if (data_offset == 0) {
				break;
			}

			bin_data_offsets.push_back(data_offset);
			bin_tetxt_offsets.push_back(reader.Read<uint64_t>());
		}


		reader.Seek(text_offset);
		int tid = 0;
		while (true) {
			auto offset = reader.Read<uint64_t>();
			if (offset == 0) {
				break;
			}
			auto t = std::make_shared<SirDesc::Text>();
			t->temp_id = ++tid;
			t->value = orig_pos + offset;
			sir->texts.push_back(t);
		}

		reader.Seek(start_offset);
		while (true) {
			auto offset = reader.Read<uint64_t>();
			if (offset == 0) {
				break;
			}
			sir->starts.push_back(orig_pos + offset);
		}

		reader.Seek(var_offset);
		while (true) {
			auto offset = reader.Read<uint64_t>();
			if (offset == 0) {
				break;
			}
			sir->vars.push_back(orig_pos + offset);
		}

		auto node_size = (int)bin_data_offsets.size();
		for (int i = 0; i < node_size; i++) {
			sir->nodes.push_back(ReadDescNode(buffer, bin_data_offsets[i], i + 1 == bin_data_offsets.size() ? *((uint64_t*)(orig_pos + text_offset)) : bin_data_offsets[i + 1], bin_tetxt_offsets[i]));
		}

		if (sir->nodes.empty() || sir->texts.empty() || sir->vars.empty()) {
			throw std::exception("invalid file");
		}

		auto& last_node = sir->nodes.back();
		auto byte_size = last_node->bytes.size();
		for (auto i = byte_size - 1; i >= 0; i--) {
			if (last_node->bytes[i] == 0x46) {
				last_node->bytes.resize(i + 1);
				break;
			}
		}

		return sir;
	}
	static std::shared_ptr<SirDesc::Node> ReadDescNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset, uint64_t text_offset)
	{
		const char* orig_pos = buffer.data();

		if (buffer.size() < offset || buffer.size() < text_offset) {
			throw std::exception("invalid memory access");
		}

		auto n = std::make_shared<SirDesc::Node>();
		n->id = orig_pos + text_offset;

		MemReader reader(buffer);
		reader.Seek(offset);
		if (reader.Read<uint8_t>() != 0x25) {
			throw std::exception("invalid memory access");
		}

		if (next_offset > offset) {
			n->bytes.reserve(std::min<int>(next_offset - offset, 1024 * 10));
		}

		n->bytes.push_back(0x25);

		do {
			n->bytes.push_back(reader.Read<uint8_t>());
		} while (reader.Ptr() - orig_pos < next_offset);

		return n;
	}
	static bool IsValidDesc(const std::span<char>& buffer)
	{
		try {
			auto orig_pos = &buffer.front();
			MemReader reader(buffer);
			reader.Seek(4);
			auto footer_beg = reader.Read<uint64_t>();

			if (buffer.size() <= footer_beg + sizeof(uint64_t) * 2) {
				return false;
			}

			reader.Seek(footer_beg);
			reader.Read<uint64_t>();
			auto info_offset = reader.Read<uint64_t>();

			if (buffer.size() <= info_offset + sizeof(uint64_t) * 2) {
				return false;
			}

			reader.Seek(info_offset);

			auto data_offset = reader.Read<uint64_t>();
			auto name_offset = reader.Read<uint64_t>();
			auto next_offset = reader.Read<uint64_t>();
			if (next_offset <= data_offset) {
				return false;
			}
			auto n = ReadDescNode(buffer, data_offset, next_offset, name_offset);
			/*if (n->key != "START_CREATE_FIRST") {
				return false;
			}*/
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

	static std::shared_ptr<SirItem> ReadItem(fs::path file_path)
	{
		auto sir = std::make_shared<SirItem>();
		sir->filename = file_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();
		auto node_nodes = node_sir->first_node();
		auto node_node = node_nodes->first_node();
		while (node_node != nullptr) {
			auto n = std::make_shared<SirItem::Node>();
			auto attr_node = node_node->first_attribute();
			while (attr_node != nullptr) {
				if (strcmp(attr_node->name(), "name") == 0) {
					n->name = utf8_to_mbs(attr_node->value());
				}
				attr_node = attr_node->next_attribute();
			}

			auto item_node = node_node->first_node();
			while (item_node != nullptr) {
				auto item = std::make_shared<SirItem::Node::Item>();

				auto attr_item = item_node->first_attribute();
				while (attr_item != nullptr) {
					if (strcmp(attr_item->name(), "key") == 0) {
						item->key = "^" + utf8_to_mbs(attr_item->value());
					}
					else if (strcmp(attr_item->name(), "text1") == 0) {
						item->patch_text = utf8_to_wcs(attr_item->value());
						item->text1 = wcs_to_mbs(item->patch_text, "");
					}
					else if (strcmp(attr_item->name(), "text2") == 0) {
						item->text2 = utf8_to_mbs(attr_item->value());
					}
					else if (strcmp(attr_item->name(), "unknown1") == 0) {
						item->unknowns[0] = atoi(attr_item->value());
					}
					else if (strcmp(attr_item->name(), "unknown2") == 0) {
						item->unknowns[1] = atoi(attr_item->value());
					}
					else if (strcmp(attr_item->name(), "unknown3") == 0) {
						item->unknowns[2] = atoi(attr_item->value());
					}
					else if (strcmp(attr_item->name(), "unknown4") == 0) {
						item->unknowns[3] = atoi(attr_item->value());
					}
					attr_item = attr_item->next_attribute();
				}

				n->items.push_back(item);
				item_node = item_node->next_sibling();
			}

			sir->nodes.push_back(n);
			node_node = node_node->next_sibling();
		}

		return sir;
	}

	static std::shared_ptr<SirMsg> ReadMsg(fs::path file_path)
	{
		auto sir = std::make_shared<SirMsg>();
		sir->filename = file_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();
		auto node_dlgs = node_sir->first_node();
		auto attr_dlgs = node_dlgs->first_attribute();
		while (attr_dlgs != nullptr) {
			if (strcmp(attr_dlgs->name(), "unknown") == 0) {
				sir->unknown = (uint64_t)atoi(attr_dlgs->value());
			}
			attr_dlgs = attr_dlgs->next_attribute();
		}


		auto node_dlg = node_dlgs->first_node();
		while (node_dlg != nullptr) {
			auto n = std::make_shared<SirMsg::Node>();
			auto attr_dlg = node_dlg->first_attribute();
			while (attr_dlg != nullptr) {
				if (strcmp(attr_dlg->name(), "key") == 0) {
					n->key = attr_dlg->value();
				}
				else if (strcmp(attr_dlg->name(), "unknown1") == 0) {
					n->unknowns[0] = atoi(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "unknown2") == 0) {
					n->unknowns[1] = atoi(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "unknown3") == 0) {
					n->unknowns[2] = atoi(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "unknown4") == 0) {
					n->unknowns[3] = atoi(attr_dlg->value());
				}
				attr_dlg = attr_dlg->next_attribute();
			}

			auto node_text = node_dlg->first_node();
			while (node_text != nullptr) {
				auto attr_text = node_text->first_attribute();
				while (attr_text != nullptr) {
					if (strcmp(attr_text->name(), "value") == 0) {
						n->patch_texts.push_back(utf8_to_wcs(attr_text->value()));
						n->texts.push_back(wcs_to_mbs(n->patch_texts.back(), ""));
					}
					attr_text = attr_text->next_attribute();
				}
				node_text = node_text->next_sibling();
			}

			sir->nodes.push_back(n);
			node_dlg = node_dlg->next_sibling();
		}

		return sir;
	}

	static std::shared_ptr<SirDesc> ReadDesc(fs::path file_path)
	{
		auto sir = std::make_shared<SirDesc>();
		sir->filename = file_path.stem().stem().string();
		memcpy(sir->header_sig, "SIR1", 4);

		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_sir = doc.first_node();

		auto attr_sir = node_sir->first_attribute();
		while (attr_sir != nullptr) {
			if (strcmp(attr_sir->name(), "sound") == 0) {
				sir->sound_file_name = utf8_to_mbs(attr_sir->value());
			}
			attr_sir = attr_sir->next_attribute();
		}


		auto node_dlgs = node_sir->first_node();
		auto node_dlg = node_dlgs->first_node();
		while (node_dlg != nullptr) {
			auto n = std::make_shared<SirDesc::Node>();
			auto attr_dlg = node_dlg->first_attribute();
			while (attr_dlg != nullptr) {
				if (strcmp(attr_dlg->name(), "id") == 0) {
					n->id = utf8_to_mbs(attr_dlg->value());
				}
				else if (strcmp(attr_dlg->name(), "byte") == 0) {
					n->bytes = HexStringToBytes(attr_dlg->value());
				}
				attr_dlg = attr_dlg->next_attribute();
			}

			sir->nodes.push_back(n);
			node_dlg = node_dlg->next_sibling();
		}

		auto node_texts = node_dlgs->next_sibling();
		auto node_text = node_texts->first_node();
		while (node_text != nullptr) {
			auto attr_text = node_text->first_attribute();
			auto t = std::make_shared<SirDesc::Text>();
			while (attr_text != nullptr) {
				if (strcmp(attr_text->name(), "id") == 0) {
					t->temp_id = atoi(attr_text->value());
				}
				else if (strcmp(attr_text->name(), "value") == 0) {
					t->patch_text = utf8_to_wcs(attr_text->value());
					t->value = wcs_to_mbs(t->patch_text,"");
				}
				attr_text = attr_text->next_attribute();
			}
			sir->texts.push_back(t);
			node_text = node_text->next_sibling();
		}

		auto node_starts = node_texts->next_sibling();
		auto node_start = node_starts->first_node();
		while (node_start != nullptr) {
			auto attr_start = node_start->first_attribute();
			while (attr_start != nullptr) {
				if (strcmp(attr_start->name(), "value") == 0) {
					sir->starts.push_back(utf8_to_mbs(attr_start->value()));
				}
				attr_start = attr_start->next_attribute();
			}
			node_start = node_start->next_sibling();
		}

		auto node_vars = node_starts->next_sibling();
		auto node_var = node_vars->first_node();
		while (node_var != nullptr) {
			auto attr_var = node_var->first_attribute();
			while (attr_var != nullptr) {
				if (strcmp(attr_var->name(), "value") == 0) {
					sir->vars.push_back(utf8_to_mbs(attr_var->value()));
				}
				attr_var = attr_var->next_attribute();
			}
			node_var = node_var->next_sibling();
		}

		return sir;
	}
};