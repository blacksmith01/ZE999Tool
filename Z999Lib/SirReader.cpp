#include "SirReader.hpp"

std::shared_ptr<SirDlg> SirReader::ReadDlg(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirDlg>();
	sir->filename = filename;
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i += 4) {

		sir->nodes.push_back(ReadDlgNode(buffer, offsets[i]));
	}

	return sir;
}
std::shared_ptr<SirDlg::Node> SirReader::ReadDlgNode(const std::span<char>& buffer, uint64_t offset)
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
bool SirReader::IsValidDlg(const std::span<char>& buffer)
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

std::shared_ptr<SirName> SirReader::ReadName(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirName>();
	sir->filename = filename;
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i += 4) {
		sir->nodes.push_back(ReadNameNode(buffer, offsets[i]));
		auto uv = node_types[sir->nodes.size() - 1];
		sir->nodes.back()->unknown_value[0] = *(((const uint32_t*)&uv) + 0);
		sir->nodes.back()->unknown_value[1] = *(((const uint32_t*)&uv) + 1);
	}

	return sir;
}
std::shared_ptr<SirName::Node> SirReader::ReadNameNode(const std::span<char>& buffer, uint64_t offset)
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
bool SirReader::IsValidName(const std::span<char>& buffer)
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

std::shared_ptr<SirFont> SirReader::ReadFont(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
	auto footer_beg = *(uint64_t*)curr_pos; curr_pos += 8;
	auto footer_end = *(uint64_t*)curr_pos; curr_pos += 8;
	auto content_pos = curr_pos;
	auto content_size = (orig_pos + footer_beg) - content_pos;

	if (buffer.size() < footer_end) {
		throw std::exception("invalid memory access");
	}
	
	auto footer_count = *((uint64_t*)(orig_pos + footer_beg));

	std::vector<uint32_t> footer_font_values;
	for (auto i = 0; i < footer_count; i++) {
		footer_font_values.push_back(*(uint32_t*)(orig_pos + footer_beg + 8 + 12 + i * 4));
	}

	auto sir = std::make_shared<SirFont>();
	sir->filename = filename;

	memcpy(&sir->footer_unknown_values[0], orig_pos + footer_beg + 8, 12);
	
	sir->nodes.reserve(footer_font_values.size());
	uint64_t offset = 4 + 8 + 8;
	while (offset < footer_beg) {
		sir->nodes.push_back(ReadFontNode(buffer, offset));
		auto& back = sir->nodes.back();
		offset += back->Size();
	}

	return sir;
}
std::shared_ptr<SirFont::Node> SirReader::ReadFontNode(const std::span<char>& buffer, uint64_t offset)
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
bool SirReader::IsValidFont(const std::span<char>& buffer)
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

std::shared_ptr<SirItem> SirReader::ReadItem(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirItem>();
	sir->filename = filename;
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {
		sir->nodes.push_back(ReadItemNode(buffer, offsets[i]));
	}

	return sir;
}
std::shared_ptr<SirItem::Node> SirReader::ReadItemNode(const std::span<char>& buffer, uint64_t offset)
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
bool SirReader::IsValidItem(const std::span<char>& buffer)
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

std::shared_ptr<SirMsg> SirReader::ReadMsg(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirMsg>();
	sir->filename = filename;
	
	curr_pos += sizeof(uint64_t) * 3;
	sir->unknown = *(const uint64_t*)curr_pos;

	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {

		sir->nodes.push_back(ReadMsgNode(buffer, offsets[i], i + 1 == offsets.size() ? (data_max_pos - orig_pos) : *((uint64_t*)(orig_pos + offsets[i + 1]))));
	}

	return sir;
}
std::shared_ptr<SirMsg::Node> SirReader::ReadMsgNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset)
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
bool SirReader::IsValidMsg(const std::span<char>& buffer)
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

std::shared_ptr<SirDesc> SirReader::ReadDesc(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();

	MemReader reader(buffer);

	reader.Forward(4);
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

	if (buffer.size() < text_offset || buffer.size() < start_offset || buffer.size() < var_offset) {
		throw std::exception("invalid memory access");
	}

	auto sir = std::make_shared<SirDesc>();
	sir->filename = filename;
	sir->sound_file_name = orig_pos + sound_file_name_offset;

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

	if (sir->nodes.empty() || sir->texts.empty()) {
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
std::shared_ptr<SirDesc::Node> SirReader::ReadDescNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset, uint64_t text_offset)
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
bool SirReader::IsValidDesc(const std::span<char>& buffer)
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

std::shared_ptr<SirFChart> SirReader::ReadFChart(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();

	MemReader reader(buffer);

	reader.Forward(4);
	auto footer_beg = reader.Read<uint64_t>();
	auto footer_end = reader.Read<uint64_t>();

	if (footer_beg >= footer_end || buffer.size() < footer_end) {
		throw std::exception("invalid memory access");
	}

	reader.Seek(footer_beg);
	std::vector<uint64_t> offsets;
	while (reader.Ptr() < orig_pos + footer_end) {
		auto curPos = reader.CurrPos();
		auto offset = reader.Read<uint64_t>();
		if (offset == 0) {
			break;
		}
		offsets.push_back(curPos);
		reader.Forward(sizeof(uint64_t) * 10);
	}

	auto sir = std::make_shared<SirFChart>();
	sir->filename = filename;
	
	for (auto o : offsets)
	{
		sir->nodes.push_back(ReadFChartNode(buffer, o));
	}

	if (sir->nodes.empty()) {
		throw std::exception("invalid file");
	}

	return sir;
}
std::shared_ptr<SirFChart::Node> SirReader::ReadFChartNode(const std::span<char>& buffer, uint64_t offset)
{
	auto size = buffer.size();

	if (size < offset) {
		throw std::exception("invalid memory access");
	}

	MemReader reader(buffer);
	reader.Seek(offset);

	std::array<uint64_t, 11> n_offsets;
	for (int i = 0; i < 11; i++)
	{
		reader.Read(n_offsets[i]);
		if (i > 0 && n_offsets[i] <= n_offsets[i - 1])
		{
			throw std::exception("invalid memory access");
		}
	}

	auto n = std::make_shared<SirFChart::Node>();
	reader.Seek(n_offsets[0]); n->id1 = reader.Ptr();
	reader.Seek(n_offsets[1]); n->id2 = reader.Ptr();
	reader.Seek(n_offsets[2]); n->name_jp = reader.Ptr();
	reader.Seek(n_offsets[3]); n->filename = reader.Ptr();
	reader.Seek(n_offsets[4]); n->name = reader.Ptr();

	reader.Seek(n_offsets[5]); n->text = reader.Ptr();
	reader.Seek(n_offsets[6]); n->desc_jp = reader.Ptr();
	reader.Seek(n_offsets[7]); n->type_id1 = reader.Ptr();
	reader.Seek(n_offsets[8]); n->command1 = reader.Ptr();
	reader.Seek(n_offsets[9]); n->type_id2 = reader.Ptr();

	reader.Seek(n_offsets[10]);

	std::array<uint64_t, 6> i_offsets;
	while (reader.CurrPos() < size)
	{
		reader.Read(i_offsets[0]);
		if (i_offsets[0] == 0)
			break;

		for (int i = 0; i < 5; i++)
		{
			reader.Read(i_offsets[i + 1]);
			if (i > 0 && i_offsets[i] <= i_offsets[i - 1])
			{
				throw std::exception("invalid memory access");
			}
		}

		auto org_pos = reader.CurrPos();

		auto item = std::make_shared<SirFChart::Node::Item>();
		reader.Seek(i_offsets[0]); item->id1 = reader.Ptr();
		reader.Seek(i_offsets[1]); item->id2 = reader.Ptr();
		reader.Seek(i_offsets[2]); item->name_jp = reader.Ptr();
		reader.Seek(i_offsets[3]); item->filename = reader.Ptr();;
		reader.Seek(i_offsets[4]); item->name = reader.Ptr();
		reader.Seek(i_offsets[5]); item->text = reader.Ptr();

		n->items.push_back(item);

		reader.Seek(org_pos);
	}

	return n;
}
bool SirReader::IsValidFChart(const std::span<char>& buffer)
{
	try {
		MemReader reader(buffer);

		reader.Forward(4);
		auto footer_beg = reader.Read<uint64_t>();
		auto footer_end = reader.Read<uint64_t>();

		auto n = ReadFChartNode(buffer, footer_beg);
		if (n->id1 != "A01b_novel_1")
			return false;

		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

std::shared_ptr<SirDoc> SirReader::ReadDoc(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
	auto footer_beg = *(uint64_t*)curr_pos; curr_pos += 8;
	auto footer_end = *(uint64_t*)curr_pos; curr_pos += 8;
	auto content_pos = curr_pos;
	auto content_size = (orig_pos + footer_beg) - content_pos;

	if (buffer.size() < footer_end) {
		throw std::exception("invalid memory access");
	}
	curr_pos = orig_pos + footer_beg;

	std::vector<uint64_t> offsets;
	for (; curr_pos < orig_pos + footer_end; curr_pos += 40) {
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

	auto sir = std::make_shared<SirDoc>();
	sir->filename = filename;
	
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {

		sir->nodes.push_back(ReadDocNode(buffer, offsets[i], i + 1 == offsets.size() ? (data_max_pos - orig_pos) : *((uint64_t*)(orig_pos + offsets[i + 1]))));
	}

	return sir;
}
std::shared_ptr<SirDoc::Node> SirReader::ReadDocNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset)
{
	const char* orig_pos = buffer.data();

	MemReader mr(buffer);
	mr.Seek(offset);

	auto n = std::make_shared<SirDoc::Node>();

	std::array<uint64_t, 5> offsets = { mr.Read<uint64_t>(),
		mr.Read<uint64_t>(),
		mr.Read<uint64_t>(),
		mr.Read<uint64_t>(),
		mr.Read<uint64_t>()
	};

	mr.Seek(offsets[0]); n->key1 = mr.Ptr();
	mr.Seek(offsets[1]); n->text1 = mr.Ptr();
	mr.Seek(offsets[2]); n->key2 = mr.Ptr();
	mr.Seek(offsets[3]); n->text2 = mr.Ptr();
	mr.Seek(offsets[4]);

	std::vector<uint64_t> item_offsets;
	while (true)
	{
		auto item_offset = mr.Read<uint64_t>();
		if (item_offset == 0)
			break;
		item_offsets.push_back(item_offset);
	}

	for (auto i : item_offsets)
	{
		mr.Seek(i);
		n->contents.push_back(mr.Ptr());
	}

	return n;
}
bool SirReader::IsValidDoc(const std::span<char>& buffer)
{
	try {
		auto n = ReadDocNode(buffer, *(uint64_t*)(&buffer.front() + 4), 0);
		if (n->key1 != "$FILE_ESC_A01_0") {
			return false;
		}
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

std::shared_ptr<SirMap> SirReader::ReadMap(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirMap>();
	sir->filename = filename;
	
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {
		sir->nodes.push_back(ReadMapNode(buffer, offsets[i]));
	}

	return sir;
}
std::shared_ptr<SirMap::Node> SirReader::ReadMapNode(const std::span<char>& buffer, uint64_t offset)
{
	auto size = buffer.size();
	MemReader reader(buffer);
	reader.Seek(offset);

	auto n = std::make_shared<SirMap::Node>();
	auto node_name_offset = reader.Read<uint64_t>();
	auto items_offset = reader.Read<uint64_t>();

	reader.Seek(node_name_offset);
	n->name = reader.Ptr();

	reader.Seek(items_offset);
	do
	{
		std::array<uint64_t, 3> item_offsets;
		item_offsets[0] = reader.Read<uint64_t>();
		if (item_offsets[0] == 0)
			break;

		for (int i = 1; i < 3; i++)
			item_offsets[i] = reader.Read<uint64_t>();

		std::array<uint32_t, 3> unknown_offsets;
		for (int i = 0; i < 3; i++)
			unknown_offsets[i] = reader.Read<uint32_t>();

		auto curr_offset = reader.CurrPos();

		auto item = std::make_shared<SirMap::Node::Item>();
		reader.Seek(item_offsets[0]);
		item->text = reader.Ptr();
		reader.Seek(item_offsets[1]);
		item->desc = reader.Ptr();
		reader.Seek(item_offsets[2]);
		item->key = reader.Ptr();
		item->unknowns = unknown_offsets;
		n->items.push_back(item);

		reader.Seek(curr_offset);

	} while (reader.CurrPos() < size);

	return n;
}
bool SirReader::IsValidMap(const std::span<char>& buffer)
{
	try {
		MemReader reader(buffer);
		reader.Forward(4);

		auto n = ReadMapNode(buffer, reader.Read<uint64_t>());
		if (n->name != "A01" || n->items.empty()) {
			return false;
		}
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

std::shared_ptr<SirCredit> SirReader::ReadCredit(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirCredit>();
	sir->filename = filename;
	
	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {
		sir->nodes.push_back(ReadCreditNode(buffer, offsets[i]));
	}

	return sir;
}
std::shared_ptr<SirCredit::Node> SirReader::ReadCreditNode(const std::span<char>& buffer, uint64_t offset)
{
	auto size = buffer.size();
	MemReader reader(buffer);
	reader.Seek(offset);

	auto n = std::make_shared<SirCredit::Node>();
	auto node_name_offset = reader.Read<uint64_t>();
	auto items_offset = reader.Read<uint64_t>();

	reader.Seek(node_name_offset);
	n->name = reader.Ptr();

	reader.Seek(items_offset);
	do
	{
		auto item_offset = reader.Read<uint64_t>();
		if (item_offset == 0)
			break;

		auto curr_offset = reader.CurrPos();

		auto item = std::make_shared<SirCredit::Node::Item>();
		reader.Seek(item_offset);
		item->id = n->items.size() + 1;
		item->text = reader.Ptr();
		n->items.push_back(item);

		reader.Seek(curr_offset);

	} while (reader.CurrPos() < size);

	return n;
}
bool SirReader::IsValidCredit(const std::span<char>& buffer)
{
	try {
		MemReader reader(buffer);
		reader.Forward(4);

		auto n = ReadCreditNode(buffer, reader.Read<uint64_t>());
		if (n->name != "AEnding" || n->items.empty()) {
			return false;
		}
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

std::shared_ptr<SirRoom> SirReader::ReadRoom(std::string filename, const std::span<char>& buffer)
{
	const char* orig_pos = buffer.data();
	auto curr_pos = orig_pos;

	curr_pos += 4;
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

	auto sir = std::make_shared<SirRoom>();
	sir->filename = filename;

	sir->nodes.reserve(offsets.size());
	for (int i = 0; i < offsets.size(); i++) {
		sir->nodes.push_back(ReadRoomNode(buffer, offsets[i]));
	}

	return sir;
}
std::shared_ptr<SirRoom::Node> SirReader::ReadRoomNode(const std::span<char>& buffer, uint64_t offset)
{
	auto size = buffer.size();
	MemReader reader(buffer);
	reader.Seek(offset);

	auto n = std::make_shared<SirRoom::Node>();
	auto node_name_offset = reader.Read<uint64_t>();
	auto items_offset = reader.Read<uint64_t>();

	reader.Seek(node_name_offset);
	n->name = reader.Ptr();

	reader.Seek(items_offset);
	do
	{
		std::array<uint64_t, 5> item_offsets;
		item_offsets[0] = reader.Read<uint64_t>();
		if (item_offsets[0] == 0)
			break;

		for (int i = 1; i < 5; i++)
			item_offsets[i] = reader.Read<uint64_t>();

		auto curr_offset = reader.CurrPos();

		auto item = std::make_shared<SirRoom::Node::Item>();
		reader.Seek(item_offsets[0]);
		item->id = reader.Ptr();
		reader.Seek(item_offsets[1]);
		item->text = reader.Ptr();
		reader.Seek(item_offsets[2]);
		item->key = reader.Ptr();
		reader.Seek(item_offsets[3]);
		item->in = reader.Ptr();
		reader.Seek(item_offsets[4]);
		item->out = reader.Ptr();
		
		n->items.push_back(item);

		reader.Seek(curr_offset);

	} while (reader.CurrPos() < size);

	return n;
}
bool SirReader::IsValidRoom(const std::span<char>& buffer)
{
	try {
		MemReader reader(buffer);
		reader.Forward(4);

		auto n = ReadRoomNode(buffer, reader.Read<uint64_t>());
		if (n->name != "A ROOT" || n->items.empty() || n->items.front()->key.empty() || n->items.front()->key[0] != '$') {
			return false;
		}
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}