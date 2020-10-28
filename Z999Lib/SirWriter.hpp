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

	static void Write(const SirItem& sir, fs::path file_path)
	{
		uint64_t node_count = sir.nodes.size();
		uint64_t data_size = 0;
		uint64_t info_size = 0;
		for (auto& n : sir.nodes) {
			data_size += n->Size();
			info_size += n->items.size() * (3 * 8 + 4 * 4) + 16;
		}
		auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

		uint64_t info_beg = 20 + data_size + padding_data_size;

		uint64_t footer_beg = info_beg + info_size;

		uint64_t footer_end = footer_beg + node_count * 8 * 2 + 40;
		auto padding_footer_end = (16 - (footer_end & 0x0F)) & 0x0F;
		footer_end += padding_footer_end;

		uintvar v(info_beg - 12);

		uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + (sir.ItemCount() * 3) + (sir.nodes.size() * 2 + 1); // footer
		auto padding_buffer = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += padding_buffer;

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;

		MemWriter writer(curr_pos);
		writer.WriteArray(sir.header_sig, 4);
		writer.Write(footer_beg);
		writer.Write(footer_end);

		std::vector<uint64_t> offsets_data;
		offsets_data.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			offsets_data.push_back(writer.pos - start_pos);
			writer.WriteString(n->name);
			for (auto& item : n->items) {
				writer.WriteString(item->key);
				writer.WriteString(item->text1);
				writer.WriteString(item->text2);
			}
		}
		for (int i = 0; i < padding_data_size; i++)
			writer.Write((uint8_t)0xAA);

		std::vector<uint64_t> offsets_info;
		offsets_info.reserve(offsets_data.size());
		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_data[i];
			offset += n->name.length() + 1;
			offsets_info.push_back(writer.pos - start_pos);
			for (auto& item : n->items) {
				writer.Write(offset); offset += item->key.length() + 1;
				writer.Write(offset); offset += item->text1.length() + 1;
				writer.Write(offset); offset += item->text2.length() + 1;
				for (auto unknown : item->unknowns)
					writer.Write(unknown);
			}

			for (int i = 0; i < 2; i++)
				writer.Write((uint64_t)0);
		}

		for (int i = 0; i < offsets_data.size(); i++) {
			writer.Write(offsets_data[i]);
			writer.Write(offsets_info[i]);
		}

		for (int i = 0; i < 3; i++) {
			writer.Write((uint64_t)0);
		}
		writer.Write(footer_beg);
		writer.Write((uint64_t)0);

		for (int i = 0; i < padding_footer_end; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write((uint8_t)0x04);
		writer.Write((uint8_t)0x08);
		writer.WriteArray(v.bytes);

		for (auto& n : sir.nodes) {
			auto item_count = (int)n->items.size();
			for (int i = 0; i < item_count; i++) {
				writer.Write((uint8_t)0x08);
				writer.Write((uint8_t)0x08);
				writer.Write((uint8_t)(i + 1 == item_count ? 0x28 : 0x18));
			}
		}

		for (uint64_t i = 0; i < node_count; i++) {
			writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)(i + 1 == node_count ? 0x20 : 0x08));
		}

		writer.Write((uint8_t)0x00);
		for (int i = 0; i < padding_buffer; i++)
			writer.Write((uint8_t)0xAA);

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirLocation& sir, fs::path file_path)
	{
		uint64_t node_count = sir.nodes.size();
		uint64_t data_size = 0;
		uint64_t item_offsets_size = 0;
		for (auto& n : sir.nodes) {
			data_size += n->Size();
			item_offsets_size += n->items.size() * 2 * 8 + 16;
		}
		auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

		uint64_t item_offset_beg = 20 + data_size + padding_data_size;

		uint64_t footer_beg = item_offset_beg + item_offsets_size;

		uint64_t footer_end = footer_beg + node_count * 44 + 40;
		auto padding_footer_end = (16 - (footer_end & 0x0F)) & 0x0F;
		footer_end += padding_footer_end;

		uintvar v(item_offset_beg - 12);

		uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + sir.ItemCount() * 2 + (sir.nodes.size() * 3 + 1); // footer
		auto padding_buffer = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += padding_buffer;

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;

		MemWriter writer(curr_pos);
		writer.WriteArray(sir.header_sig, 4);
		writer.Write(footer_beg);
		writer.Write(footer_end);

		std::vector<uint64_t> offsets_data;
		offsets_data.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			offsets_data.push_back(writer.pos - start_pos);
			writer.WriteString(n->map_name);
			writer.WriteString(n->map_key);
			for (auto& item : n->items) {
				writer.WriteString(item->text);
				writer.WriteString(item->key);
			}
		}
		for (int i = 0; i < padding_data_size; i++)
			writer.Write((uint8_t)0xAA);

		std::vector<uint64_t> offsets_info;
		offsets_info.reserve(offsets_data.size());
		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_data[i];
			offset += n->map_name.length() + 1 + n->map_key.length() + 1;
			offsets_info.push_back(writer.pos - start_pos);
			for (auto& item : n->items) {
				writer.Write(offset); offset += item->text.length() + 1;
				writer.Write(offset); offset += item->key.length() + 1;
			}

			for (int i = 0; i < 2; i++)
				writer.Write((uint64_t)0);
		}

		for (int i = 0; i < offsets_data.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_data[i];
			writer.Write(offset); offset += n->map_name.length() + 1;
			writer.Write(offset); offset += n->map_key.length() + 1;
			for (auto v : n->unknowns)
				writer.Write(v);
			writer.Write(offsets_info[i]);
		}

		for (int i = 0; i < 3; i++) {
			writer.Write((uint64_t)0);
		}
		writer.Write(footer_beg);
		writer.Write((uint64_t)0);

		for (int i = 0; i < padding_footer_end; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write((uint8_t)0x04);
		writer.Write((uint8_t)0x08);
		writer.WriteArray(v.bytes);

		for (auto& n : sir.nodes) {
			auto item_count = (int)n->items.size();
			for (int i = 0; i < item_count; i++) {
				writer.Write((uint8_t)0x08);
				writer.Write((uint8_t)(i + 1 == item_count ? 0x18 : 0x08));
			}
		}

		for (uint64_t i = 0; i < node_count; i++) {
			writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)0x1C);
			writer.Write((uint8_t)(i + 1 == node_count ? 0x20 : 0x08));
		}

		writer.Write((uint8_t)0x00);
		for (int i = 0; i < padding_buffer; i++)
			writer.Write((uint8_t)0xAA);

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirMsg& sir, fs::path file_path)
	{
		uint64_t node_count = sir.nodes.size();
		uint64_t data_size = 0;
		uint64_t text_count = 0;
		for (auto& n : sir.nodes) {
			data_size += n->Size();
			text_count += n->texts.size();
		}
		auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

		uint64_t info_beg = 20 + data_size + padding_data_size;
		uint64_t info_size = node_count * 16 + text_count * 8;

		uint64_t footer_beg = info_beg + info_size;

		uint64_t footer_end = footer_beg + node_count * (sizeof(uint64_t) + sizeof(SirMsg::Node::unknowns) + sizeof(uint64_t)) + (sizeof(uint64_t) * 5);
		auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
		footer_end += padding_footer;

		uintvar v(info_beg - 12);

		uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + text_count + (sir.nodes.size() * 2 + 1); // footer
		auto padding_buffer = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += padding_buffer;

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;

		MemWriter writer(curr_pos);
		writer.WriteArray(sir.header_sig, 4);
		writer.Write(footer_beg);
		writer.Write(footer_end);

		std::vector<uint64_t> offsets_key;
		offsets_key.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			offsets_key.push_back(writer.pos - start_pos);
			writer.WriteString(n->key);
			for (auto& t : n->texts) {
				writer.WriteString(t);
			}
		}
		for (int i = 0; i < padding_data_size; i++)
			writer.Write((uint8_t)0xAA);

		std::vector<uint64_t> offsets_value;
		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_key[i] + n->key.length() + 1;

			offsets_value.push_back(writer.pos - start_pos);
			for (auto& t : n->texts) {
				writer.Write((uint64_t)offset);
				offset += (t.length() + 1);
			}
			writer.Write((uint64_t)0);
			writer.Write((uint64_t)0);
		}

		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			writer.Write(offsets_key[i]);
			writer.WriteArray(n->unknowns);
			writer.Write(offsets_value[i]);
		}

		for (int i = 0; i < 3; i++) {
			writer.Write((uint64_t)0);
		}
		writer.Write(sir.unknown);
		writer.Write((uint64_t)0);
		for (int i = 0; i < padding_footer; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write((uint8_t)0x04);
		writer.Write((uint8_t)0x08);
		writer.WriteArray(v.bytes);

		for (auto& n : sir.nodes) {
			auto tcount = (int)n->texts.size();
			for (int i = 0; i < tcount; i++) {
				writer.Write((uint8_t)(i + 1 == tcount ? 0x18 : 0x08));
			}
		}

		for (uint64_t i = 0; i < node_count; i++) {
			writer.Write((uint8_t)0x18);
			writer.Write((uint8_t)(i + 1 == node_count ? 0x20 : 0x08));
		}

		writer.Write((uint8_t)0x00);
		for (int i = 0; i < padding_buffer; i++)
			writer.Write((uint8_t)0xAA);

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirDesc& sir, fs::path file_path)
	{
		uint64_t node_count = sir.nodes.size();
		uint64_t text_count = sir.texts.size();

		uint64_t node_data_size = 0;
		uint64_t node_id_size = 0;
		for (auto& n : sir.nodes) {
			node_data_size += n->bytes.size();
			node_id_size += n->id.size() + 1;
		}
		auto padding_node_data_size = (4 - (node_data_size & 0x03)) & 0x03;
		auto padding_node_id_size = (4 - (node_id_size & 0x03)) & 0x03;

		uint64_t text_beg = 20 + node_data_size + padding_node_data_size;
		uint64_t text_size = 0;
		for (auto& t : sir.texts) {
			text_size += t->value.size() + 1;
		}
		auto padding_text_size = (4 - (text_size & 0x03)) & 0x03;

		uint64_t start_beg = text_beg + text_size + padding_text_size + sir.texts.size() * 8 + 8;
		uint64_t start_size = 0;
		for (auto& s : sir.starts) {
			start_size += s.size() + 1;
		}
		auto padding_start_size = (4 - (start_size & 0x03)) & 0x03;

		uint64_t var_beg = start_beg + start_size + padding_start_size + sir.starts.size() * 8 + 8;
		uint64_t var_size = 0;
		for (auto& v : sir.vars) {
			var_size += v.size() + 1;
		}
		auto padding_var_size = (4 - (var_size & 0x03)) & 0x03;

		uint64_t node_id_beg = var_beg + var_size + padding_var_size + sir.vars.size() * 8 + 8;

		uint64_t sound_filename_beg = node_id_beg + node_id_size + padding_node_id_size + sir.nodes.size() * (2 * 8) + (2 * 8);
		uint64_t sound_filename_size = sir.sound_file_name.size() + 1;
		auto padding_sound_filename_size = (4 - (sound_filename_size & 0x03)) & 0x03;

		uint64_t footer_beg = sound_filename_beg + sound_filename_size + padding_sound_filename_size;

		uint64_t footer_end = footer_beg + sizeof(uint64_t) * 5 + sizeof(uint32_t);
		auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
		footer_end += padding_footer;

		uintvar v_text(text_beg + text_size + padding_text_size - 12);
		uintvar v_var(var_size + padding_var_size + 16);
		uintvar v_nodeid(node_id_size + padding_node_id_size + 16);
		std::array<uint8_t, 5> v_last = { 0x08,0x0C,0x08,0x08,0x00 };

		uint64_t buffer_size
			= footer_end + 2
			+ v_text.bytes.size() + text_count
			+ v_var.bytes.size() + sir.vars.size() - 1
			+ v_nodeid.bytes.size() + node_count * 2
			+ 5;
		auto padding_buffer = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += padding_buffer;

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;

		MemWriter writer(curr_pos);
		writer.WriteArray(sir.header_sig, 4);
		writer.Write(footer_beg);
		writer.Write(footer_end);

		std::vector<uint64_t> node_data_offsets;
		node_data_offsets.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			node_data_offsets.push_back(writer.pos - start_pos);
			writer.WriteArray(n->bytes);
		}
		for (int i = 0; i < padding_node_data_size; i++)
			writer.Write((uint8_t)0xAA);


		std::vector<uint64_t> text_offsets;
		for (auto& t : sir.texts) {
			text_offsets.push_back(writer.pos - start_pos);
			writer.WriteString(t->value);
		}
		for (int i = 0; i < padding_text_size; i++)
			writer.Write((uint8_t)0xAA);

		uint64_t text_info_offset = writer.pos - start_pos;
		for (auto offset : text_offsets) {
			writer.Write(offset);
		}
		writer.Write<uint64_t>(0);


		std::vector<uint64_t> start_offsets;
		for (auto& t : sir.starts) {
			start_offsets.push_back(writer.pos - start_pos);
			writer.WriteString(t);
		}
		for (int i = 0; i < padding_start_size; i++)
			writer.Write((uint8_t)0xAA);

		uint64_t start_info_offset = writer.pos - start_pos;
		for (auto offset : start_offsets) {
			writer.Write(offset);
		}
		writer.Write<uint64_t>(0);


		std::vector<uint64_t> var_offsets;
		for (auto& t : sir.vars) {
			var_offsets.push_back(writer.pos - start_pos);
			writer.WriteString(t);
		}
		for (int i = 0; i < padding_var_size; i++)
			writer.Write((uint8_t)0xAA);

		uint64_t var_info_offset = writer.pos - start_pos;
		for (auto offset : var_offsets) {
			writer.Write(offset);
		}
		writer.Write<uint64_t>(0);


		std::vector<uint64_t> node_id_offsets;
		node_data_offsets.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			node_id_offsets.push_back(writer.pos - start_pos);
			writer.WriteString(n->id);
		}
		for (int i = 0; i < padding_node_id_size; i++)
			writer.Write((uint8_t)0xAA);


		uint64_t node_info_offset = writer.pos - start_pos;
		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			writer.Write(node_data_offsets[i]);
			writer.Write(node_id_offsets[i]);
		}
		writer.Write<uint64_t>(0);
		writer.Write<uint64_t>(0);


		uint64_t sountfile_offset = writer.pos - start_pos;
		writer.WriteString(sir.sound_file_name);
		for (int i = 0; i < padding_sound_filename_size; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write(sountfile_offset);
		writer.Write(node_info_offset);
		writer.Write<uint32_t>(text_count);
		writer.Write(text_info_offset);
		writer.Write(start_info_offset);
		writer.Write(var_info_offset);

		for (int i = 0; i < padding_footer; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write((uint8_t)0x04);
		writer.Write((uint8_t)0x08);
		writer.WriteArray(v_text.bytes);

		for (uint64_t i = 0; i < text_count; i++) {
			writer.Write((uint8_t)(i + 1 == text_count ? 0x18 : 0x08));
		}

		writer.WriteArray(v_var.bytes);

		for (uint64_t i = 0; i < sir.vars.size() - 1; i++) {
			writer.Write((uint8_t)0x08);
		}

		writer.WriteArray(v_nodeid.bytes);

		for (uint64_t i = 0; i < node_count * 2; i++) {
			writer.Write((uint8_t)(i + 1 == node_count * 2 ? 0x20 : 0x08));
		}

		writer.WriteArray(v_last);
		for (int i = 0; i < padding_buffer; i++)
			writer.Write((uint8_t)0xAA);

		std::ofstream ofs(file_path, std::ios::binary);
		ofs.write(buffer.data(), buffer.size());
	}

	static void Write(const SirFChart& sir, fs::path file_path)
	{
		uint64_t node_count = sir.nodes.size();
		uint64_t data_size = 0;
		uint64_t item_offsets_size = 0;
		for (auto& n : sir.nodes) {
			data_size += n->Size();
			item_offsets_size += n->items.size() * 6 * 8 + 16;
		}
		auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

		uint64_t item_offset_beg = 20 + data_size + padding_data_size;

		uint64_t footer_beg = item_offset_beg + item_offsets_size;

		uint64_t footer_end = footer_beg + node_count * 11 * 8 + 40;
		auto padding_footer_end = (16 - (footer_end & 0x0F)) & 0x0F;
		footer_end += padding_footer_end;

		uintvar v(item_offset_beg - 12);

		uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + sir.ItemCount() * 6 + (sir.nodes.size() * 11 + 1); // footer
		auto padding_buffer = (16 - (buffer_size & 0x0F)) & 0x0F;
		buffer_size += padding_buffer;

		std::vector<char> buffer;
		buffer.resize(buffer_size);

		auto start_pos = buffer.data();
		auto curr_pos = start_pos;

		MemWriter writer(curr_pos);
		writer.WriteArray(sir.header_sig, 4);
		writer.Write(footer_beg);
		writer.Write(footer_end);

		std::vector<uint64_t> offsets_data;
		offsets_data.reserve(sir.nodes.size());
		for (auto& n : sir.nodes) {
			offsets_data.push_back(writer.pos - start_pos);
			writer.WriteString(n->id1);
			writer.WriteString(n->id2);
			writer.WriteString(n->name_jp);
			writer.WriteString(n->filename);
			writer.WriteString(n->name);
			writer.WriteString(n->text);
			writer.WriteString(n->desc_jp);
			writer.WriteString(n->type_id1);
			writer.WriteString(n->command1);
			writer.WriteString(n->type_id2);
			for (auto& item : n->items) {
				writer.WriteString(item->id1);
				writer.WriteString(item->id2);
				writer.WriteString(item->name_jp);
				writer.WriteString(item->filename);
				writer.WriteString(item->name);
				writer.WriteString(item->text);
			}
		}
		for (int i = 0; i < padding_data_size; i++)
			writer.Write((uint8_t)0xAA);

		std::vector<uint64_t> offsets_info;
		offsets_info.reserve(offsets_data.size());
		for (int i = 0; i < sir.nodes.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_data[i];
			offset += n->BaseSize();
			offsets_info.push_back(writer.pos - start_pos);
			for (auto& item : n->items) {
				writer.Write(offset); offset += 1 + item->id1.length();
				writer.Write(offset); offset += 1 + item->id2.length();
				writer.Write(offset); offset += 1 + item->name_jp.length();
				writer.Write(offset); offset += 1 + item->filename.length();
				writer.Write(offset); offset += 1 + item->name.length();
				writer.Write(offset); offset += 1 + item->text.length();
			}

			for (int i = 0; i < 2; i++)
				writer.Write((uint64_t)0);
		}

		for (int i = 0; i < offsets_data.size(); i++) {
			auto& n = sir.nodes[i];
			auto offset = offsets_data[i];
			writer.Write(offset); offset += 1 + n->id1.length();
			writer.Write(offset); offset += 1 + n->id2.length();
			writer.Write(offset); offset += 1 + n->name_jp.length();
			writer.Write(offset); offset += 1 + n->filename.length();
			writer.Write(offset); offset += 1 + n->name.length();

			writer.Write(offset); offset += 1 + n->text.length();
			writer.Write(offset); offset += 1 + n->desc_jp.length();
			writer.Write(offset); offset += 1 + n->type_id1.length();
			writer.Write(offset); offset += 1 + n->command1.length();
			writer.Write(offset); offset += 1 + n->type_id2.length();

			writer.Write(offsets_info[i]);
		}

		for (int i = 0; i < 3; i++) {
			writer.Write((uint64_t)0);
		}
		writer.Write(footer_beg);
		writer.Write((uint64_t)0);

		for (int i = 0; i < padding_footer_end; i++)
			writer.Write((uint8_t)0xAA);

		writer.Write((uint8_t)0x04);
		writer.Write((uint8_t)0x08);
		writer.WriteArray(v.bytes);

		for (auto& n : sir.nodes) {
			auto item_count = (int)n->items.size();
			for (int i = 0; i < item_count; i++)
			{
				for (int j = 0; j < 5; j++) {
					writer.Write((uint8_t)(0x08));
				}
				writer.Write((uint8_t)(i + 1 == item_count ? 0x18 : 0x08));
			}
		}

		for (uint64_t i = 0; i < node_count; i++) {
			for (int j = 0; j < 10; j++)
				writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)(i + 1 == node_count ? 0x20 : 0x08));
		}

		writer.Write((uint8_t)0x00);
		for (int i = 0; i < padding_buffer; i++)
			writer.Write((uint8_t)0xAA);

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

	static void Write(const SirItem& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		auto node_dlgs = doc.allocate_node(rapidxml::node_element, "sections");
		node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
		node_sir->append_node(node_dlgs);

		for (auto& n : sir.nodes) {
			auto node_dlg = doc.allocate_node(rapidxml::node_element, "section");
			node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name.c_str()))));
			for (auto& item : n->items) {
				auto node_item = doc.allocate_node(rapidxml::node_element, "item");
				node_item->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(item->key.c_str() + 1))));
				node_item->append_attribute(doc.allocate_attribute("text1", RapidXmlString(doc, mbs_to_utf8(item->text1.c_str()))));
				node_item->append_attribute(doc.allocate_attribute("text2", RapidXmlString(doc, mbs_to_utf8(item->text2.c_str()))));
				node_item->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, item->unknowns[0])));
				node_item->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, item->unknowns[1])));
				node_item->append_attribute(doc.allocate_attribute("unknown3", RapidXmlString(doc, item->unknowns[2])));
				node_item->append_attribute(doc.allocate_attribute("unknown4", RapidXmlString(doc, item->unknowns[3])));
				node_dlg->append_node(node_item);
			}
			node_dlgs->append_node(node_dlg);
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirLocation& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		auto node_dlgs = doc.allocate_node(rapidxml::node_element, "maps");
		node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
		node_sir->append_node(node_dlgs);

		for (auto& n : sir.nodes) {
			auto node_dlg = doc.allocate_node(rapidxml::node_element, "map");
			node_dlg->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(n->map_key.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->map_name.c_str()))));
			node_dlg->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, n->unknowns[0])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, n->unknowns[1])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown3", RapidXmlString(doc, n->unknowns[2])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown4", RapidXmlString(doc, n->unknowns[3])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown5", RapidXmlString(doc, n->unknowns[4])));
			for (auto& item : n->items) {
				auto node_item = doc.allocate_node(rapidxml::node_element, "location");
				node_item->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(item->key.c_str()))));
				node_item->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(item->text.c_str()))));
				node_dlg->append_node(node_item);
			}
			node_dlgs->append_node(node_dlg);
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirMsg& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		auto node_dlgs = doc.allocate_node(rapidxml::node_element, "msgs");
		node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
		node_dlgs->append_attribute(doc.allocate_attribute("unknown", RapidXmlString(doc, sir.unknown)));
		node_sir->append_node(node_dlgs);

		for (auto& n : sir.nodes) {
			auto node_dlg = doc.allocate_node(rapidxml::node_element, "msg");
			node_dlg->append_attribute(doc.allocate_attribute("key", n->key.c_str()));
			node_dlg->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, n->unknowns[0])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, n->unknowns[1])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown3", RapidXmlString(doc, n->unknowns[2])));
			node_dlg->append_attribute(doc.allocate_attribute("unknown4", RapidXmlString(doc, n->unknowns[3])));
			node_dlgs->append_node(node_dlg);

			for (auto& t : n->texts) {
				auto node_text = doc.allocate_node(rapidxml::node_element, "text");
				node_text->append_attribute(doc.allocate_attribute("value", RapidXmlString(doc, mbs_to_utf8(t.c_str()))));
				node_dlg->append_node(node_text);
			}
		}
		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirDesc& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		node_sir->append_attribute(doc.allocate_attribute("sound", RapidXmlString(doc, mbs_to_utf8(sir.sound_file_name))));

		{
			auto node_dlgs = doc.allocate_node(rapidxml::node_element, "bins");
			node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
			node_sir->append_node(node_dlgs);

			for (auto& n : sir.nodes) {
				auto node_dlg = doc.allocate_node(rapidxml::node_element, "bin");
				node_dlg->append_attribute(doc.allocate_attribute("id", RapidXmlString(doc, mbs_to_utf8(n->id))));
				node_dlg->append_attribute(doc.allocate_attribute("byte", RapidXmlString(doc, BytesToHexString(n->bytes))));
				node_dlgs->append_node(node_dlg);
			}
		}

		{
			auto node_texts = doc.allocate_node(rapidxml::node_element, "texts");
			node_texts->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.texts.size())));
			node_sir->append_node(node_texts);

			for (auto& t : sir.texts) {
				auto node_text = doc.allocate_node(rapidxml::node_element, "text");
				node_text->append_attribute(doc.allocate_attribute("id", RapidXmlString(doc, t->temp_id)));
				node_text->append_attribute(doc.allocate_attribute("value", RapidXmlString(doc, mbs_to_utf8(t->value))));
				node_texts->append_node(node_text);
			}
		}

		{
			auto node_starts = doc.allocate_node(rapidxml::node_element, "starts");
			node_starts->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.starts.size())));
			node_sir->append_node(node_starts);

			for (auto& s : sir.starts) {
				auto node_start = doc.allocate_node(rapidxml::node_element, "start");
				node_start->append_attribute(doc.allocate_attribute("value", RapidXmlString(doc, mbs_to_utf8(s))));
				node_starts->append_node(node_start);
			}
		}


		{
			auto node_vars = doc.allocate_node(rapidxml::node_element, "vars");
			node_vars->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.vars.size())));
			node_sir->append_node(node_vars);

			for (auto& v : sir.vars) {
				auto node_var = doc.allocate_node(rapidxml::node_element, "var");
				node_var->append_attribute(doc.allocate_attribute("value", RapidXmlString(doc, mbs_to_utf8(v))));
				node_vars->append_node(node_var);
			}
		}

		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	static void Write(const SirFChart& sir, fs::path file_path)
	{
		rapidxml::xml_document<char> doc;
		auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
		doc.append_node(node_sir);

		{
			auto node_ns = doc.allocate_node(rapidxml::node_element, "nodes");
			node_ns->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
			node_sir->append_node(node_ns);

			for (auto& n : sir.nodes) {
				auto node_n = doc.allocate_node(rapidxml::node_element, "part");
				node_n->append_attribute(doc.allocate_attribute("id1", RapidXmlString(doc, mbs_to_utf8(n->id1))));
				node_n->append_attribute(doc.allocate_attribute("id2", RapidXmlString(doc, mbs_to_utf8(n->id2))));
				node_n->append_attribute(doc.allocate_attribute("name_jp", RapidXmlString(doc, mbs_to_utf8(n->name_jp))));
				node_n->append_attribute(doc.allocate_attribute("file", RapidXmlString(doc, mbs_to_utf8(n->filename))));
				node_n->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name))));
				node_n->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(n->text))));
				node_n->append_attribute(doc.allocate_attribute("desc_jp", RapidXmlString(doc, mbs_to_utf8(n->desc_jp))));
				node_n->append_attribute(doc.allocate_attribute("type1", RapidXmlString(doc, mbs_to_utf8(n->type_id1))));
				node_n->append_attribute(doc.allocate_attribute("command1", RapidXmlString(doc, mbs_to_utf8(n->command1))));
				node_n->append_attribute(doc.allocate_attribute("type2", RapidXmlString(doc, mbs_to_utf8(n->type_id2))));

				for (auto& i : n->items)
				{
					auto node_i = doc.allocate_node(rapidxml::node_element, "scene");
					node_i->append_attribute(doc.allocate_attribute("id1", RapidXmlString(doc, mbs_to_utf8(i->id1))));
					node_i->append_attribute(doc.allocate_attribute("id2", RapidXmlString(doc, mbs_to_utf8(i->id2))));
					node_i->append_attribute(doc.allocate_attribute("name_jp", RapidXmlString(doc, mbs_to_utf8(i->name_jp))));
					node_i->append_attribute(doc.allocate_attribute("file", RapidXmlString(doc, mbs_to_utf8(i->filename))));
					node_i->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(i->name))));
					node_i->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(i->text))));
					node_n->append_node(node_i);
				}

				node_ns->append_node(node_n);
			}
		}

		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(file_path);
		ofs.write(xmlString.c_str(), xmlString.size());
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