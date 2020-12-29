#include "SirWriter.hpp"

void SirWriter::Write(const SirDlg& sir, fs::path file_path)
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

void SirWriter::Write(const SirName& sir, fs::path file_path)
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

void SirWriter::Write(const SirFont& sir, fs::path file_path)
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

void SirWriter::Write(const SirItem& sir, fs::path file_path)
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

void SirWriter::Write(const SirMsg& sir, fs::path file_path)
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

void SirWriter::Write(const SirDesc& sir, fs::path file_path)
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

	uint64_t v_nodeid_extra = 32 - (sir.starts.empty() ? 0 : 1) * 8 - (sir.vars.empty() ? 0 : 1) * 8;

	uintvar v_text(text_beg + text_size + padding_text_size - 12);
	uintvar v_start(start_size + padding_start_size + 16);
	uintvar v_var(var_size + padding_var_size + 16);
	uintvar v_nodeid(node_id_size + padding_node_id_size + v_nodeid_extra);
	std::array<uint8_t, 5> v_last = { 0x08,0x0C,0x08,0x08,0x00 };

	uint64_t buffer_size
		= footer_end + 2
		+ v_text.bytes.size() - 1 + text_count
		+ v_nodeid.bytes.size() + node_count * 2
		+ 5;

	bool include_start = sir.starts.size() > 1;
	bool include_var = sir.vars.size() > 0;
	if (include_start) {
		buffer_size += (v_start.bytes.size() + sir.starts.size() - 1);
	}
	if (include_var) {
		buffer_size += (v_var.bytes.size() + sir.vars.size() - 1);
	}
	if (sir.starts.size() == 1)
		buffer_size += 1; // 0x18

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
	for (uint64_t i = 1; i < text_count; i++) {
		writer.Write((uint8_t)(0x08));
	}

	if (sir.starts.size() == 1)
		writer.Write((uint8_t)(0x18));

	if (include_start) {
		writer.WriteArray(v_start.bytes);
		for (uint64_t i = 1; i < sir.starts.size(); i++) {
			writer.Write((uint8_t)0x08);
		}
	}
	if (include_var) {
		writer.WriteArray(v_var.bytes);
		for (uint64_t i = 1; i < sir.vars.size(); i++) {
			writer.Write((uint8_t)0x08);
		}
	}

	writer.WriteArray(v_nodeid.bytes);
	uint8_t nodeid_end = sir.IsItemDescType() ? 0x20 : 0x24;
	for (uint64_t i = 0; i < node_count * 2; i++) {
		writer.Write((uint8_t)(i + 1 == node_count * 2 ? nodeid_end : 0x08));
	}

	writer.WriteArray(v_last);
	for (int i = 0; i < padding_buffer; i++)
		writer.Write((uint8_t)0xAA);

	if (writer.pos - start_pos > buffer_size)
	{
		throw new std::exception("!alloc");
	}

	std::ofstream ofs(file_path, std::ios::binary);
	ofs.write(buffer.data(), buffer.size());
}

void SirWriter::Write(const SirFChart& sir, fs::path file_path)
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

void SirWriter::Write(const SirDoc& sir, fs::path file_path)
{
	uint64_t node_count = sir.nodes.size();
	uint64_t data_size = 0;
	uint64_t content_count = 0;
	for (auto& n : sir.nodes) {
		data_size += n->Size();
		content_count += n->contents.size();
	}
	auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

	uint64_t info_beg = 20 + data_size + padding_data_size;
	uint64_t info_size = node_count * 16 + content_count * 8;

	uint64_t footer_beg = info_beg + info_size;

	uint64_t footer_end = footer_beg + node_count * (sizeof(uint64_t) * 5) + (sizeof(uint64_t) * 5);
	auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
	footer_end += padding_footer;

	uintvar v(info_beg - 12);

	uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + content_count + (node_count * 5 + 1); // footer
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
		writer.WriteString(n->key1);
		writer.WriteString(n->text1);
		writer.WriteString(n->key2);
		writer.WriteString(n->text2);
		for (auto& t : n->contents) {
			writer.WriteString(t);
		}
	}
	for (int i = 0; i < padding_data_size; i++)
		writer.Write((uint8_t)0xAA);

	std::vector<uint64_t> offsets_value;
	for (int i = 0; i < sir.nodes.size(); i++) {
		auto& n = sir.nodes[i];
		auto offset = offsets_key[i] + n->HeaderSize();

		offsets_value.push_back(writer.pos - start_pos);
		for (auto& t : n->contents) {
			writer.Write((uint64_t)offset);
			offset += (t.length() + 1);
		}
		writer.Write((uint64_t)0);
		writer.Write((uint64_t)0);
	}

	for (int i = 0; i < sir.nodes.size(); i++) {
		auto& n = sir.nodes[i];
		auto offset = offsets_key[i];
		writer.Write(offset); offset += n->key1.length() + 1;
		writer.Write(offset); offset += n->text1.length() + 1;
		writer.Write(offset); offset += n->key2.length() + 1;
		writer.Write(offset);
		writer.Write(offsets_value[i]);
	}

	for (int i = 0; i < 3; i++) {
		writer.Write((uint64_t)0);
	}

	writer.Write(footer_beg);

	writer.Write((uint64_t)0);
	for (int i = 0; i < padding_footer; i++)
		writer.Write((uint8_t)0xAA);

	writer.Write((uint8_t)0x04);
	writer.Write((uint8_t)0x08);
	writer.WriteArray(v.bytes);

	for (auto& n : sir.nodes) {
		auto tcount = (int)n->contents.size();
		for (int i = 0; i < tcount; i++) {
			writer.Write((uint8_t)(i + 1 == tcount ? 0x18 : 0x08));
		}
	}

	for (uint64_t i = 0; i < node_count; i++) {
		for (auto j = 0; j < 4; j++) {
			writer.Write((uint8_t)0x08);
		}
		writer.Write((uint8_t)(i + 1 == node_count ? 0x20 : 0x08));
	}

	writer.Write((uint8_t)0x00);
	for (int i = 0; i < padding_buffer; i++)
		writer.Write((uint8_t)0xAA);

	std::ofstream ofs(file_path, std::ios::binary);
	ofs.write(buffer.data(), buffer.size());
}

void SirWriter::Write(const SirMap& sir, fs::path file_path)
{
	uint64_t node_count = sir.nodes.size();
	uint64_t item_count = 0;
	uint64_t data_size = 0;
	uint64_t item_offsets_size = 0;
	for (auto& n : sir.nodes) {
		auto item_size = n->items.size();
		data_size += (n->Size() - item_size * sizeof(uint32_t) * 3);
		item_count += item_size;
		item_offsets_size += item_size * (sizeof(uint64_t) * 3 + sizeof(uint32_t) * 3) + 16;
	}
	auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

	uint64_t info_beg = 20 + data_size + padding_data_size;

	uint64_t footer_beg = info_beg + item_offsets_size;

	uint64_t footer_end = footer_beg + (node_count * 2 + 5) * sizeof(uint64_t);
	auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
	footer_end += padding_footer;

	uintvar v(info_beg - 12);

	uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + item_count * 3 + (sir.nodes.size() * 2 + 1); // footer
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
			writer.WriteString(item->text);
			writer.WriteString(item->desc);
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
		offset += n->name.length() + 1;
		offsets_info.push_back(writer.pos - start_pos);
		for (auto& item : n->items) {
			writer.Write(offset); offset += item->text.length() + 1;
			writer.Write(offset); offset += item->desc.length() + 1;
			writer.Write(offset); offset += item->key.length() + 1;
			for (auto& unknown : item->unknowns)
				writer.Write(unknown);
		}

		for (int i = 0; i < 2; i++)
			writer.Write((uint64_t)0);
	}

	for (int i = 0; i < offsets_data.size(); i++) {
		auto& n = sir.nodes[i];
		auto offset = offsets_data[i];
		writer.Write(offset); offset += n->name.length() + 1;
		writer.Write(offsets_info[i]);
	}

	for (int i = 0; i < 3; i++) {
		writer.Write((uint64_t)0);
	}
	writer.Write(footer_beg);
	writer.Write((uint64_t)0);

	for (int i = 0; i < padding_footer; i++)
		writer.Write((uint8_t)0xAA);

	writer.Write((uint8_t)0x04);
	writer.Write((uint8_t)0x08);
	writer.WriteArray(v.bytes);

	for (auto& n : sir.nodes) {
		auto item_count = (int)n->items.size();
		for (int i = 0; i < item_count; i++) {
			writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)(i + 1 == item_count ? 0x24 : 0x14));
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

void SirWriter::Write(const SirCredit& sir, fs::path file_path)
{
	uint64_t node_count = sir.nodes.size();
	uint64_t item_count = 0;
	uint64_t data_size = 0;
	uint64_t item_offsets_size = 0;
	for (auto& n : sir.nodes) {
		auto item_size = n->items.size();
		data_size += n->Size();
		item_count += item_size;
		item_offsets_size += item_size * sizeof(uint64_t) + 16;
	}
	auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

	uint64_t info_beg = 20 + data_size + padding_data_size;

	uint64_t footer_beg = info_beg + item_offsets_size;

	uint64_t footer_end = footer_beg + (node_count * 2 + 5) * sizeof(uint64_t);
	auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
	footer_end += padding_footer;

	uintvar v(info_beg - 12);

	uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + item_count + (sir.nodes.size() * 2 + 1); // footer
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
		offset += n->name.length() + 1;
		offsets_info.push_back(writer.pos - start_pos);
		for (auto& item : n->items) {
			writer.Write(offset); offset += item->text.length() + 1;
		}

		for (int i = 0; i < 2; i++)
			writer.Write((uint64_t)0);
	}

	for (int i = 0; i < offsets_data.size(); i++) {
		auto& n = sir.nodes[i];
		auto offset = offsets_data[i];
		writer.Write(offset); offset += n->name.length() + 1;
		writer.Write(offsets_info[i]);
	}

	for (int i = 0; i < 3; i++) {
		writer.Write((uint64_t)0);
	}
	writer.Write(footer_beg);
	writer.Write((uint64_t)0);

	for (int i = 0; i < padding_footer; i++)
		writer.Write((uint8_t)0xAA);

	writer.Write((uint8_t)0x04);
	writer.Write((uint8_t)0x08);
	writer.WriteArray(v.bytes);

	for (auto& n : sir.nodes) {
		auto item_count = (int)n->items.size();
		for (int i = 0; i < item_count; i++) {
			writer.Write((uint8_t)(i + 1 == item_count ? 0x18 : 0x08));
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

void SirWriter::Write(const SirRoom& sir, fs::path file_path)
{
	uint64_t node_count = sir.nodes.size();
	uint64_t item_count = 0;
	uint64_t data_size = 0;
	uint64_t item_offsets_size = 0;
	for (auto& n : sir.nodes) {
		auto item_size = n->items.size();
		data_size += n->Size();
		item_count += item_size;
		item_offsets_size += item_size * (sizeof(uint64_t) * 5) + 16;
	}
	auto padding_data_size = (4 - (data_size & 0x03)) & 0x03;

	uint64_t info_beg = 20 + data_size + padding_data_size;

	uint64_t footer_beg = info_beg + item_offsets_size;

	uint64_t footer_end = footer_beg + (node_count * 2 + 5) * sizeof(uint64_t);
	auto padding_footer = (16 - (footer_end & 0x0F)) & 0x0F;
	footer_end += padding_footer;

	uintvar v(info_beg - 12);

	uint64_t buffer_size = footer_end + (2 + v.bytes.size()) + item_count * 5 + (sir.nodes.size() * 2 + 1); // footer
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
			writer.WriteString(item->id);
			writer.WriteString(item->text);
			writer.WriteString(item->key);
			writer.WriteString(item->in);
			writer.WriteString(item->out);
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
			writer.Write(offset); offset += item->id.length() + 1;
			writer.Write(offset); offset += item->text.length() + 1;
			writer.Write(offset); offset += item->key.length() + 1;
			writer.Write(offset); offset += item->in.length() + 1;
			writer.Write(offset); offset += item->out.length() + 1;
		}

		for (int i = 0; i < 2; i++)
			writer.Write((uint64_t)0);
	}

	for (int i = 0; i < offsets_data.size(); i++) {
		auto& n = sir.nodes[i];
		auto offset = offsets_data[i];
		writer.Write(offset); offset += n->name.length() + 1;
		writer.Write(offsets_info[i]);
	}

	for (int i = 0; i < 3; i++) {
		writer.Write((uint64_t)0);
	}
	writer.Write(footer_beg);
	writer.Write((uint64_t)0);

	for (int i = 0; i < padding_footer; i++)
		writer.Write((uint8_t)0xAA);

	writer.Write((uint8_t)0x04);
	writer.Write((uint8_t)0x08);
	writer.WriteArray(v.bytes);

	for (auto& n : sir.nodes) {
		auto item_count = (int)n->items.size();
		for (int i = 0; i < item_count; i++) {
			for (int j = 0; j < 4; j++)
				writer.Write((uint8_t)0x08);
			writer.Write((uint8_t)(i + 1 == item_count ? 0x18 : 0x08));
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