#pragma once

#include "Sir.hpp"

class SirWriter {
public:
	static void Write(const SirDlg& sir, fs::path file_path)
	{
		std::vector<uint64_t> offsets;
		offsets.reserve(sir.nodes.size() * 4);
		uint64_t offset = 0;
		offset += 4 + 8 + 8;

		uint64_t node_size = 0;
		{
			for (auto& n : sir.nodes) {
				offsets.push_back(offset + node_size); node_size += n->id__.length() + 1;
				offsets.push_back(offset + node_size); node_size += n->type.length() + 1;
				offsets.push_back(offset + node_size); node_size += n->name.length() + 1;
				offsets.push_back(offset + node_size); node_size += n->text.length() + 1;
			}
			node_size += (4 - ((offset + node_size) & 0x03)) & 0x03;
		}
		offset += node_size;

		uint64_t footer_beg = offset;

		offset += (offsets.size() * 8) + (8 * 6);
		offset += (16 - (offset & 0x0F)) & 0x0F;

		uint64_t footer_end = offset;

		auto special_codes = uintvar(footer_beg - 12).bytes;

		uint32_t content_size =
			4 + 8 + 8 + // header
			node_size +
			sir.nodes.size() * 4 * 8 +
			16 + 8 + 8 + 16;
		content_size += (16 - (content_size & 0x0F)) & 0x0F;

		auto extra_size = 2 + special_codes.size() + (sir.nodes.size() * 4) + 2;
		extra_size += (16 - (extra_size & 0x0F)) & 0x0F;

		std::vector<char> buffer;
		buffer.resize(content_size + extra_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;
		memcpy(curr_pos, sir.header_sig, 4); curr_pos += 4;
		*((uint64_t*)curr_pos) = footer_beg; curr_pos += 8;
		*((uint64_t*)curr_pos) = footer_end; curr_pos += 8;

		for (auto& n : sir.nodes) {
			memcpy(curr_pos, n->id__.c_str(), n->id__.length() + 1);  curr_pos += n->id__.length() + 1;
			memcpy(curr_pos, n->type.c_str(), n->type.length() + 1);  curr_pos += n->type.length() + 1;
			memcpy(curr_pos, n->name.c_str(), n->name.length() + 1);  curr_pos += n->name.length() + 1;
			memcpy(curr_pos, n->text.c_str(), n->text.length() + 1);  curr_pos += n->text.length() + 1;
		}

		auto node_padding = (4 - ((curr_pos - start_pos) & 0x03)) & 0x03;
		memset(curr_pos, 0xAA, node_padding); curr_pos += node_padding;

		for (auto offset : offsets) {
			*(uint64_t*)curr_pos = offset; curr_pos += 8;
		}
		for (int i = 0; i < 2; i++) {
			*(uint64_t*)curr_pos = 0; curr_pos += 8;
		}
		*(uint64_t*)curr_pos = footer_beg; curr_pos += 8;
		for (int i = 0; i < 3; i++) {
			*(uint64_t*)curr_pos = 0; curr_pos += 8;
		}
		auto footer_padding = (16 - ((curr_pos - start_pos) & 0x0F)) & 0x0F;
		memset(curr_pos, 0xAA, footer_padding); curr_pos += footer_padding;

		*curr_pos = 0x04; curr_pos += 1;
		*curr_pos = 0x08; curr_pos += 1;
		memcpy(curr_pos, special_codes.data(), special_codes.size()); curr_pos += special_codes.size();
		memset(curr_pos, 0x08, offsets.size() - 1); curr_pos += offsets.size() - 1;
		*curr_pos = 0x18; curr_pos += 1;
		*curr_pos = 0x00; curr_pos += 1;

		auto file_padding = (16 - ((curr_pos - start_pos) & 0x0F)) & 0x0F;
		memset(curr_pos, 0xAA, file_padding); curr_pos += file_padding;

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirName& sir, fs::path file_path)
	{
		std::vector<uint64_t> offsets;
		offsets.reserve(sir.nodes.size() * 4);
		uint64_t offset = 0;
		offset += 4 + 8 + 8;

		uint64_t node_size = 0;
		{
			for (auto& n : sir.nodes) {
				offsets.push_back(offset + node_size); node_size += n->key_name.length() + 1;
				offsets.push_back(offset + node_size); node_size += n->name.length() + 1;
				offsets.push_back(offset + node_size); node_size += n->key_msg.length() + 1;
				uint64_t unknown_value = (((uint64_t)n->unknown_value[1]) << 32) + (uint64_t)n->unknown_value[0];
				offsets.push_back(unknown_value);
				offsets.push_back(offset + node_size); node_size += n->msg.length() + 1;
			}
			node_size += (4 - ((offset + node_size) & 0x03)) & 0x03;
		}
		offset += node_size;

		uint64_t footer_beg = offset;

		offset += (offsets.size() * 8) + (8 * 6);
		offset += (16 - (offset & 0x0F)) & 0x0F;

		uint64_t footer_end = offset;

		auto special_codes = uintvar(footer_beg - 12).bytes;

		uint32_t content_size =
			4 + 8 + 8 + // header
			node_size +
			sir.nodes.size() * 5 * 8 +
			16 + 8 + 8 + 16;
		content_size += (16 - (content_size & 0x0F)) & 0x0F;

		auto extra_size = 2 + special_codes.size() + (sir.nodes.size() * 4) - 1 + 2;
		extra_size += (16 - (extra_size & 0x0F)) & 0x0F;

		std::vector<char> buffer;
		buffer.resize(content_size + extra_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;
		memcpy(curr_pos, sir.header_sig, 4); curr_pos += 4;
		*((uint64_t*)curr_pos) = footer_beg; curr_pos += 8;
		*((uint64_t*)curr_pos) = footer_end; curr_pos += 8;

		for (auto& n : sir.nodes) {
			memcpy(curr_pos, n->key_name.c_str(), n->key_name.length() + 1);  curr_pos += n->key_name.length() + 1;
			memcpy(curr_pos, n->name.c_str(), n->name.length() + 1);  curr_pos += n->name.length() + 1;
			memcpy(curr_pos, n->key_msg.c_str(), n->key_msg.length() + 1);  curr_pos += n->key_msg.length() + 1;
			memcpy(curr_pos, n->msg.c_str(), n->msg.length() + 1);  curr_pos += n->msg.length() + 1;
		}

		auto node_padding = (4 - ((curr_pos - start_pos) & 0x03)) & 0x03;
		memset(curr_pos, 0xAA, node_padding); curr_pos += node_padding;

		for (auto offset : offsets) {
			*(uint64_t*)curr_pos = offset; curr_pos += 8;
		}
		for (int i = 0; i < 2; i++) {
			*(uint64_t*)curr_pos = 0; curr_pos += 8;
		}
		*(uint64_t*)curr_pos = footer_beg; curr_pos += 8;
		for (int i = 0; i < 3; i++) {
			*(uint64_t*)curr_pos = 0; curr_pos += 8;
		}
		auto footer_padding = (16 - ((curr_pos - start_pos) & 0x0F)) & 0x0F;
		memset(curr_pos, 0xAA, footer_padding); curr_pos += footer_padding;

		*curr_pos = 0x04; curr_pos += 1;
		*curr_pos = 0x08; curr_pos += 1;
		memcpy(curr_pos, special_codes.data(), special_codes.size()); curr_pos += special_codes.size();
		uint32_t fvalue = 0x08100808;
		for (auto& n : sir.nodes) {
			memcpy(curr_pos, &fvalue, sizeof(fvalue)); curr_pos += sizeof(fvalue);
		}
		curr_pos -= 1;
		*curr_pos = 0x18; curr_pos += 1;
		*curr_pos = 0x00; curr_pos += 1;

		auto file_padding = (16 - ((curr_pos - start_pos) & 0x0F)) & 0x0F;
		memset(curr_pos, 0xAA, file_padding); curr_pos += file_padding;

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirFont& sir, fs::path file_path)
	{
		std::vector<uint32_t> footer_font_values;
		uint64_t node_size = 0;
		uint64_t footer_beg = 20;
		auto offset = footer_beg;
		for (auto& n : sir.nodes) {
			auto curr_offset = offset;
			auto size = n->Size();
			offset += size;
			node_size += size;
			footer_beg += size;
			footer_font_values.push_back((curr_offset - 20) / 2);
		}

		uint32_t buffer_size = 20 + node_size + 20 + footer_font_values.size() * 4 + 16; // footer
		auto footer_padding = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += footer_padding;
		uint64_t footer_end = footer_beg + 20 + footer_font_values.size() * 4 + footer_padding;

		std::vector<uint8_t> special_codes;
		special_codes.resize(16);
		memset(special_codes.data(), 0xAA, 16);
		special_codes[0] = 0x04;
		special_codes[1] = 0x08;
		uintvar v(footer_beg);
		if (v.bytes.size() < 4 && v.bytes[0] >= 0x88) {
			special_codes[2] = 0x80;
			memcpy(&special_codes[3], v.bytes.data(), v.bytes.size());
			special_codes[3 + v.bytes.size()] = 0;
		}
		else {
			memcpy(&special_codes[2], v.bytes.data(), v.bytes.size());
			special_codes[2 + v.bytes.size()] = 0;
		}

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;
		memcpy(curr_pos, sir.header_sig, 4); curr_pos += 4;
		*((uint64_t*)curr_pos) = footer_beg; curr_pos += 8;
		*((uint64_t*)curr_pos) = footer_end; curr_pos += 8;

		for (auto& n : sir.nodes) {
			memcpy(curr_pos, n->keycode, 6);  curr_pos += 6;
			*curr_pos = n->wsize[0]; curr_pos += 1;
			*curr_pos = n->hsize[0]; curr_pos += 1;
			*curr_pos = n->wsize[1]; curr_pos += 1;
			*curr_pos = n->hsize[1]; curr_pos += 1;
			memcpy(curr_pos, &n->data[0][0], n->data[0].size()); curr_pos += n->data[0].size();
			memcpy(curr_pos, &n->data[1][0], n->data[1].size()); curr_pos += n->data[1].size();
			memset(curr_pos, 0xAA, n->Padding()); curr_pos += n->Padding();
		}

		*(uint64_t*)curr_pos = sir.nodes.size(); curr_pos += 8;
		for (int i = 0; i < 3; i++) {
			*(uint32_t*)curr_pos = sir.footer_unknown_values[i]; curr_pos += 4;
		}
		for (auto v : footer_font_values) {
			*(uint32_t*)curr_pos = v; curr_pos += 4;
		}

		memset(curr_pos, 0xAA, footer_padding); curr_pos += footer_padding;
		memcpy(curr_pos, special_codes.data(), special_codes.size()); curr_pos += special_codes.size();

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}
};

class SirXmlWriter
{
public:
	static void Write(const SirDlg& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		auto node_dlgs = doc.allocate_node(rapidxml::node_element, "dlgs");
		node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
		node_sir->append_node(node_dlgs);

		for (auto& n : sir.nodes) {
			auto node_dlg = doc.allocate_node(rapidxml::node_element, "dlg");
			node_dlg->append_attribute(doc.allocate_attribute("id", n->id__.c_str()));
			node_dlg->append_attribute(doc.allocate_attribute("type", n->type.c_str()));
			node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(n->text.c_str()))));
			node_dlgs->append_node(node_dlg);
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirName& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		auto node_dlgs = doc.allocate_node(rapidxml::node_element, "names");
		node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
		node_sir->append_node(node_dlgs);

		for (auto& n : sir.nodes) {
			auto node_dlg = doc.allocate_node(rapidxml::node_element, "name");
			node_dlg->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(n->key_name.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("kye2", RapidXmlString(doc, mbs_to_utf8(n->key_msg.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("msg", RapidXmlString(doc, mbs_to_utf8(n->msg.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, n->unknown_value[0])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, n->unknown_value[1])));
			node_dlgs->append_node(node_dlg);
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirFont& sir, fs::path file_path)
	{
		std::array<uint8_t, 2> data_width = {};
		std::array<uint8_t, 2> data_height = {};
		for (auto& n : sir.nodes) {
			for (int i = 0; i < 2; i++) {
				data_width[i] = std::max(data_width[i], n->wsize[i]);
				data_height[i] = std::max(data_height[i], n->hsize[i]);
			}
		}

		auto nsize = sir.nodes.size();
		auto wcount = (int)sqrt(nsize); wcount = wcount % 2 == 1 ? wcount + 1 : wcount;
		auto hcount = ((nsize / wcount) + (nsize % wcount > 0 ? 1 : 0));

		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);
		for (int i = 0; i < 2; i++)
		{
			auto node_png = doc.allocate_node(rapidxml::node_element, i == 0 ? "png_default" : "png_border");
			node_png->append_attribute(doc.allocate_attribute("png_width", RapidXmlString(doc, data_width[i] * wcount)));
			node_png->append_attribute(doc.allocate_attribute("png_height", RapidXmlString(doc, data_height[i] * hcount)));
			node_png->append_attribute(doc.allocate_attribute("font_width", RapidXmlString(doc, data_width[i])));
			node_png->append_attribute(doc.allocate_attribute("font_height", RapidXmlString(doc, data_height[i])));
			node_png->append_attribute(doc.allocate_attribute("font_wcount", RapidXmlString(doc, wcount)));
			node_png->append_attribute(doc.allocate_attribute("font_hcount", RapidXmlString(doc, hcount)));
			node_sir->append_node(node_png);
		}
		{
			auto node_etc = doc.allocate_node(rapidxml::node_element, "etc");
			node_etc->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, sir.footer_unknown_values[0])));
			node_etc->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, sir.footer_unknown_values[1])));
			node_sir->append_node(node_etc);
		}
		{
			auto node_dlgs = doc.allocate_node(rapidxml::node_element, "fonts");
			node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
			for (auto& n : sir.nodes) {
				auto node_dlg = doc.allocate_node(rapidxml::node_element, "font");
				node_dlg->append_attribute(doc.allocate_attribute("ch", RapidXmlString(doc, n->patch_keycode.empty() ? n->Utf8String() : n->patch_keycode)));
				node_dlg->append_attribute(doc.allocate_attribute("bin", RapidXmlString(doc, BytesToHexString(std::span<uint8_t>((uint8_t*)n->SjisString().c_str(), 2)))));
				node_dlg->append_attribute(doc.allocate_attribute("default_w", RapidXmlString(doc, n->wsize[0])));
				node_dlg->append_attribute(doc.allocate_attribute("default_h", RapidXmlString(doc, n->hsize[0])));
				node_dlg->append_attribute(doc.allocate_attribute("border_w", RapidXmlString(doc, n->wsize[1])));
				node_dlg->append_attribute(doc.allocate_attribute("border_h", RapidXmlString(doc, n->hsize[1])));
				node_dlgs->append_node(node_dlg);
			}
			node_sir->append_node(node_dlgs);
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs << xmlString;
	}
};

class SirPngWriter {
public:
	static void Write(const SirFont& sir, int data_idx, fs::path file_path)
	{
		uint8_t data_width = 0;
		uint8_t data_height = 0;
		for (auto& n : sir.nodes) {
			data_width = std::max(n->wsize[data_idx], data_width);
			data_height = std::max(n->hsize[data_idx], data_height);
		}

		auto nsize = sir.nodes.size();
		auto wcount = (int)sqrt(nsize); wcount = wcount % 2 == 1 ? wcount + 1 : wcount;
		auto hcount = ((nsize / wcount) + (nsize % wcount > 0 ? 1 : 0));
		auto png_width = data_width * wcount;
		auto png_height = data_height * hcount;

		std::vector<png_byte> png_buffer;
		png_buffer.resize(png_width * png_height, 0);

		for (int i = 0; i < nsize; i++) {
			auto& n = sir.nodes[i];
			auto iw = (i % wcount);
			auto ih = (i / wcount);
			for (int j = 0; j < n->data[data_idx].size(); j++) {
				auto jw = (j % n->wsize[data_idx]);
				auto jh = (j / n->wsize[data_idx]);
				auto wtotal = (iw * data_width + jw);
				auto htotal = (ih * data_height + jh) * png_width;
				auto p = &png_buffer[(wtotal + htotal)];
				*(p + 0) = n->data[data_idx][j];
			}
		}
		std::vector<png_bytep> png_rows;
		png_rows.resize(png_height);
		for (int i = 0; i < png_height; i++) {
			png_rows[i] = &png_buffer[i * png_width];
		}

		FILE* fp;
		fopen_s(&fp, file_path.string().c_str(), "wb");
		auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		auto info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, png_width, png_height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, png_rows.data());
		png_write_end(png_ptr, nullptr);
		fclose(fp);
	}
};