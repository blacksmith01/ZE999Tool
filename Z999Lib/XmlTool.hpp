#pragma once

#include "Common.hpp"

class XmlTool
{
public:
	bool ConvertHalfWidthJapaneseXml(const fs::path& src_path, const fs::path& out_path)
	{
		for (auto i : fs::directory_iterator(src_path)) {
			auto pi = fs::path(i);
			if (StrCmpEndWith(pi.filename().string(), ".desc.xml")) {
				ConvertHalfWidthJapaneseXml_Desc(pi, out_path);
			}
		}

		return true;
	}

	void ConvertHalfWidthJapaneseXml_Desc(const fs::path& file_path, const fs::path& out_path)
	{
		rapidxml::file<char> xmlFile(file_path.string().c_str()); // Default template is char
		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node = doc.first_node();
		if (!node) {
			return;
		}

		node = node->first_node();
		while (node != nullptr) {
			if (strcmp(node->name(), "texts") == 0) {
				break;
			}
			node = node->next_sibling();
		}
		if (!node) {
			return;
		}

		auto node_text = node->first_node();
		while (node_text != nullptr) {
			auto attr_text = node_text->first_attribute();
			while (attr_text != nullptr) {
				if (strcmp(attr_text->name(), "value") == 0) {
					auto org_text = attr_text->value();
					auto mod_text = wcs_to_utf8(sjis_half_to_full(utf8_to_wcs(org_text)));
					attr_text->value(RapidXmlString(doc, mod_text));
					break;
				}
				attr_text = attr_text->next_attribute();
			}
			node_text = node_text->next_sibling();
		}
		

		std::string xmlString;
		rapidxml::print(std::back_inserter(xmlString), doc);

		std::ofstream ofs(fs::path(out_path).append(file_path.filename().string()));
		ofs.write(xmlString.c_str(), xmlString.size());
	}

	inline std::wstring sjis_half_to_full(const std::wstring& src)
	{
		//DWORD flags = LCMAP_FULLWIDTH; //îïÊÇÙþí®ªËª·ªÞª¹¡£flag to convert to full width
		//DWORD flags = LCMAP_HALFWIDTH; //ÚâÊÇÙþí®ªËª·ªÞª¹£©¡£to half width
		//DWORD flags = LCMAP_HIRAGANA; //ªÒªéª¬ªÊªËª·ªÞª¹¡£to hiragana
		//DWORD flags = LCMAP_KATAKANA; //«««¿«««ÊªËª·ªÞª¹¡£to katakana

		std::vector<wchar_t> buffer(src.length() * 2 + 1, 0);

		auto size = (int)src.size();
		for (int i = 0; i < size; i++) {
			if (src[i] >= 0xFF00 && src[i] <= 0xFFEF) {
				wchar_t temp[2] = {};
				LCMapStringW(1041, LCMAP_FULLWIDTH, &src[i], 1, &temp[0], 1);
				LCMapStringW(1041, LCMAP_HIRAGANA, &temp[0], 1, &buffer[i], 1);
			}
			else {
				buffer[i] = src[i];
			}
		}

		return buffer.data();
	}
};