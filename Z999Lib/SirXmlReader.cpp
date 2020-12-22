#include "SirXmlReader.hpp"

std::shared_ptr<SirDlg> SirXmlReader::ReadDlg(fs::path file_path)
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

std::shared_ptr<SirName> SirXmlReader::ReadName(fs::path file_path)
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

std::shared_ptr<SirFont> SirXmlReader::ReadFont(fs::path xmlfile_path, fs::path default_png_file_path, fs::path border_png_file_path)
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

std::shared_ptr<SirItem> SirXmlReader::ReadItem(fs::path file_path)
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

std::shared_ptr<SirMsg> SirXmlReader::ReadMsg(fs::path file_path)
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

std::shared_ptr<SirDesc> SirXmlReader::ReadDesc(fs::path file_path)
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
				t->value = wcs_to_mbs(t->patch_text, "");
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

std::shared_ptr<SirFChart> SirXmlReader::ReadFChart(fs::path file_path)
{
	auto sir = std::make_shared<SirFChart>();
	sir->filename = file_path.stem().stem().string();
	memcpy(sir->header_sig, "SIR1", 4);

	rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
	rapidxml::xml_document<char> doc;
	doc.parse<0>(xmlFile.data());

	auto node_sir = doc.first_node();
	auto node_nodes = node_sir->first_node();
	auto node_node = node_nodes->first_node();
	while (node_node != nullptr) {
		auto n = std::make_shared<SirFChart::Node>();
		auto attr_node = node_node->first_attribute();
		while (attr_node != nullptr) {
			if (strcmp(attr_node->name(), "id1") == 0) {
				n->id1 = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "id2") == 0) {
				n->id2 = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "name_jp") == 0) {
				n->name_jp = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "file") == 0) {
				n->filename = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "name") == 0) {
				n->name = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "text") == 0) {
				n->patch_text = utf8_to_wcs(attr_node->value());
				n->text = wcs_to_mbs(n->patch_text, "");
			}
			else if (strcmp(attr_node->name(), "desc_jp") == 0) {
				n->desc_jp = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "type1") == 0) {
				n->type_id1 = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "command1") == 0) {
				n->command1 = utf8_to_mbs(attr_node->value());
			}
			else if (strcmp(attr_node->name(), "type2") == 0) {
				n->type_id2 = utf8_to_mbs(attr_node->value());
			}
			attr_node = attr_node->next_attribute();
		}

		auto item_node = node_node->first_node();
		while (item_node != nullptr) {
			auto item = std::make_shared<SirFChart::Node::Item>();

			auto attr_item = item_node->first_attribute();
			while (attr_item != nullptr) {
				if (strcmp(attr_item->name(), "id1") == 0) {
					item->id1 = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "id2") == 0) {
					item->id2 = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "name_jp") == 0) {
					item->name_jp = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "file") == 0) {
					item->filename = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "name") == 0) {
					item->name = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "text") == 0) {
					item->patch_text = utf8_to_wcs(attr_item->value());
					item->text = wcs_to_mbs(item->patch_text, "");
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

std::shared_ptr<SirDoc> SirXmlReader::ReadDoc(fs::path file_path)
{
	auto sir = std::make_shared<SirDoc>();
	sir->filename = file_path.stem().stem().string();
	memcpy(sir->header_sig, "SIR1", 4);

	rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
	rapidxml::xml_document<char> doc;
	doc.parse<0>(xmlFile.data());

	auto node_sir = doc.first_node();
	auto node_ns = node_sir->first_node();

	auto node_n = node_ns->first_node();
	while (node_n != nullptr) {
		auto n = std::make_shared<SirDoc::Node>();
		auto attr_n = node_n->first_attribute();
		while (attr_n != nullptr) {
			if (strcmp(attr_n->name(), "key1") == 0) {
				n->key1 = utf8_to_mbs(attr_n->value());
			}
			else if (strcmp(attr_n->name(), "key2") == 0) {
				n->key2 = utf8_to_mbs(attr_n->value());
			}
			else if (strcmp(attr_n->name(), "text1") == 0) {
				n->patch_text1 = utf8_to_wcs(attr_n->value());
				n->text1 = wcs_to_mbs(n->patch_text1, "");
			}
			else if (strcmp(attr_n->name(), "text2") == 0) {
				n->patch_text2 = utf8_to_wcs(attr_n->value());
				n->text2 = wcs_to_mbs(n->patch_text2, "");
			}
			attr_n = attr_n->next_attribute();
		}

		auto node_i = node_n->first_node();
		while (node_i != nullptr) {
			auto attr_i = node_i->first_attribute();
			while (attr_i != nullptr) {
				if (strcmp(attr_i->name(), "text") == 0) {
					n->patch_contents.push_back(utf8_to_wcs(attr_i->value()));
					n->contents.push_back(wcs_to_mbs(n->patch_contents.back(), ""));
				}
				attr_i = attr_i->next_attribute();
			}
			node_i = node_i->next_sibling();
		}

		sir->nodes.push_back(n);
		node_n = node_n->next_sibling();
	}

	return sir;
}

std::shared_ptr<SirMap> SirXmlReader::ReadMap(fs::path file_path)
{
	auto sir = std::make_shared<SirMap>();
	sir->filename = file_path.stem().stem().string();
	memcpy(sir->header_sig, "SIR1", 4);

	rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
	rapidxml::xml_document<char> doc;
	doc.parse<0>(xmlFile.data());

	auto node_sir = doc.first_node();
	auto node_nodes = node_sir->first_node();
	auto node_node = node_nodes->first_node();
	while (node_node != nullptr) {
		auto n = std::make_shared<SirMap::Node>();
		auto attr_node = node_node->first_attribute();
		while (attr_node != nullptr) {
			if (strcmp(attr_node->name(), "name") == 0) {
				n->name = utf8_to_mbs(attr_node->value());
			}
			attr_node = attr_node->next_attribute();
		}

		auto item_node = node_node->first_node();
		while (item_node != nullptr) {
			auto item = std::make_shared<SirMap::Node::Item>();

			auto attr_item = item_node->first_attribute();
			while (attr_item != nullptr) {
				if (strcmp(attr_item->name(), "key") == 0) {
					item->key = utf8_to_mbs(attr_item->value());
				}
				else if (strcmp(attr_item->name(), "text") == 0) {
					item->patch_text = utf8_to_wcs(attr_item->value());
					item->text = wcs_to_mbs(item->patch_text, "");
				}
				else if (strcmp(attr_item->name(), "desc") == 0) {
					item->desc = utf8_to_mbs(attr_item->value());
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