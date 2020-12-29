#include "SirXmlWriter.hpp"

void SirXmlWriter::Write(const SirDlg& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirName& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirFont& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirItem& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirMsg& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirDesc& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirFChart& sir, fs::path file_path)
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

void SirXmlWriter::Write(const SirDoc& sir, fs::path file_path)
{
	rapidxml::xml_document<char> doc;
	auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
	doc.append_node(node_sir);

	auto node_dlgs = doc.allocate_node(rapidxml::node_element, "docs");
	node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
	node_sir->append_node(node_dlgs);

	for (auto& n : sir.nodes) {
		auto node_n = doc.allocate_node(rapidxml::node_element, "doc");
		node_n->append_attribute(doc.allocate_attribute("key1", RapidXmlString(doc, mbs_to_utf8(n->key1))));
		node_n->append_attribute(doc.allocate_attribute("key2", RapidXmlString(doc, mbs_to_utf8(n->key2))));
		node_n->append_attribute(doc.allocate_attribute("text1", RapidXmlString(doc, mbs_to_utf8(n->text1))));
		node_n->append_attribute(doc.allocate_attribute("text2", RapidXmlString(doc, mbs_to_utf8(n->text2))));
		node_dlgs->append_node(node_n);

		for (auto& t : n->contents) {
			auto node_i = doc.allocate_node(rapidxml::node_element, "content");
			node_i->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(t.c_str()))));
			node_n->append_node(node_i);
		}
	}
	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	std::ofstream ofs(file_path);
	ofs.write(xmlString.c_str(), xmlString.size());
}

void SirXmlWriter::Write(const SirMap& sir, fs::path file_path)
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
			auto node_item = doc.allocate_node(rapidxml::node_element, "map");
			node_item->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(item->key.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(item->text.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("desc", RapidXmlString(doc, mbs_to_utf8(item->desc.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("unknown1", RapidXmlString(doc, item->unknowns[0])));
			node_item->append_attribute(doc.allocate_attribute("unknown2", RapidXmlString(doc, item->unknowns[1])));
			node_item->append_attribute(doc.allocate_attribute("unknown3", RapidXmlString(doc, item->unknowns[2])));
			node_dlg->append_node(node_item);
		}
		node_dlgs->append_node(node_dlg);
	}
	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	std::ofstream ofs(file_path);
	ofs.write(xmlString.c_str(), xmlString.size());
}

void SirXmlWriter::Write(const SirCredit& sir, fs::path file_path)
{
	rapidxml::xml_document<char> doc;
	auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
	doc.append_node(node_sir);

	auto node_dlgs = doc.allocate_node(rapidxml::node_element, "endings");
	node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
	node_sir->append_node(node_dlgs);

	for (auto& n : sir.nodes) {
		auto node_dlg = doc.allocate_node(rapidxml::node_element, "ending");
		node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name.c_str()))));
		for (auto& item : n->items) {
			auto node_item = doc.allocate_node(rapidxml::node_element, "texts");
			node_item->append_attribute(doc.allocate_attribute("id", RapidXmlString(doc, item->id)));
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

void SirXmlWriter::Write(const SirRoom& sir, fs::path file_path)
{
	rapidxml::xml_document<char> doc;
	auto node_sir = doc.allocate_node(rapidxml::node_element, "sir");
	doc.append_node(node_sir);

	auto node_dlgs = doc.allocate_node(rapidxml::node_element, "roots");
	node_dlgs->append_attribute(doc.allocate_attribute("size", RapidXmlString(doc, sir.nodes.size())));
	node_sir->append_node(node_dlgs);

	for (auto& n : sir.nodes) {
		auto node_dlg = doc.allocate_node(rapidxml::node_element, "root");
		node_dlg->append_attribute(doc.allocate_attribute("name", RapidXmlString(doc, mbs_to_utf8(n->name.c_str()))));
		for (auto& item : n->items) {
			auto node_item = doc.allocate_node(rapidxml::node_element, "room");
			node_item->append_attribute(doc.allocate_attribute("id", RapidXmlString(doc, mbs_to_utf8(item->id.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("text", RapidXmlString(doc, mbs_to_utf8(item->text.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("key", RapidXmlString(doc, mbs_to_utf8(item->key.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("in", RapidXmlString(doc, mbs_to_utf8(item->in.c_str()))));
			node_item->append_attribute(doc.allocate_attribute("out", RapidXmlString(doc, mbs_to_utf8(item->out.c_str()))));
			node_dlg->append_node(node_item);
		}
		node_dlgs->append_node(node_dlg);
	}
	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	std::ofstream ofs(file_path);
	ofs.write(xmlString.c_str(), xmlString.size());
}
