#pragma once

#include "Common.hpp"

struct BMFont
{
	struct Info
	{
		std::string face;
		std::size_t size;
		bool bold;
		bool italic;
		std::string charset;
		bool unicode;
		std::size_t stretchH;
		bool smooth;
		bool aa;
		std::array<std::size_t, 4> padding;
		std::array<std::size_t, 2> spacing;
		std::size_t outline;
	};
	struct Common
	{
		std::size_t lineHeight;
		std::size_t base;
		std::size_t scaleW;
		std::size_t scaleH;
		std::size_t pages;
		bool packed;
		std::size_t alphaChnl;
		std::size_t redChnl;
		std::size_t greenChnl;
		std::size_t blueChnl;
	};
	struct Page
	{
		std::size_t id;
		std::string file;
	};
	struct Char
	{
		std::size_t id;
		std::size_t x;
		std::size_t y;
		std::size_t width;
		std::size_t height;
		int xoffset;
		int yoffset;
		std::size_t xadvance;
		std::size_t page;
		std::size_t chnl;
	};

	Info info;
	Common common;
	std::vector<Page> pages;
	std::vector<Char> chars;

	std::size_t GetCharMaxWidthForRender() const
	{
		std::size_t max_width = 0;
		for (auto& ch : chars) {
			std::size_t w = 0;
			if (ch.xoffset < 0) {
				w = std::max(ch.width, ch.xadvance);
			}
			else {
				w = ch.xadvance;
			}
			max_width = std::max(max_width, w);
		}
		return max_width;
	}
};

class BMFontXmlReader {
public:
	static bool ReadInfo(rapidxml::xml_node<char>* node_info, BMFont& fnt)
	{
		auto attr = node_info->first_attribute();
		while (attr) {
			if (false) {}
			M_RAPID_XML_ATTR_IF_SET_STRING(attr, "face", fnt.info.face)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "size", fnt.info.size)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "bold", fnt.info.bold)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "italic", fnt.info.italic)
				M_RAPID_XML_ATTR_IF_SET_STRING(attr, "charset", fnt.info.charset)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "unicode", fnt.info.unicode)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "stretchH", fnt.info.stretchH)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "smooth", fnt.info.smooth)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "aa", fnt.info.aa)
				M_RAPID_XML_ATTR_IF_SET_INT_ARRAY(attr, "padding", fnt.info.padding)
				M_RAPID_XML_ATTR_IF_SET_INT_ARRAY(attr, "spacing", fnt.info.spacing)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "outline", fnt.info.outline)

				attr = attr->next_attribute();
		}
		return true;
	}
	static bool ReadCommon(rapidxml::xml_node<char>* node_common, BMFont& fnt)
	{
		auto attr = node_common->first_attribute();
		while (attr) {
			if (false) {}
			M_RAPID_XML_ATTR_IF_SET_INT(attr, "lineHeight", fnt.common.lineHeight)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "base", fnt.common.base)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "scaleW", fnt.common.scaleW)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "scaleH", fnt.common.scaleH)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "pages", fnt.common.pages)
				M_RAPID_XML_ATTR_IF_SET_BOOL(attr, "packed", fnt.common.packed)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "alphaChnl", fnt.common.alphaChnl)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "redChnl", fnt.common.redChnl)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "greenChnl", fnt.common.greenChnl)
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "blueChnl", fnt.common.blueChnl)

				attr = attr->next_attribute();
		}
		return true;
	}
	static bool ReadPages(rapidxml::xml_node<char>* node_pages, BMFont& fnt)
	{
		auto node_page = node_pages->first_node();
		while (node_page) {
			BMFont::Page p{};
			auto attr = node_page->first_attribute();
			while (attr) {
				if (false) {}
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "id", p.id)
					M_RAPID_XML_ATTR_IF_SET_STRING(attr, "file", p.file)
					attr = attr->next_attribute();
			}
			fnt.pages.push_back(p);
			node_page = node_page->next_sibling();
		}

		return true;
	}
	static bool ReadChars(rapidxml::xml_node<char>* node_chars, BMFont& fnt)
	{
		auto node_char = node_chars->first_node();
		while (node_char) {
			BMFont::Char ch{};
			auto attr = node_char->first_attribute();
			while (attr) {
				if (false) {}
				M_RAPID_XML_ATTR_IF_SET_INT(attr, "id", ch.id)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "x", ch.x)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "y", ch.y)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "width", ch.width)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "height", ch.height)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "xoffset", ch.xoffset)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "yoffset", ch.yoffset)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "xadvance", ch.xadvance)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "page", ch.page)
					M_RAPID_XML_ATTR_IF_SET_INT(attr, "chnl", ch.chnl)
					attr = attr->next_attribute();
			}
			fnt.chars.push_back(ch);
			node_char = node_char->next_sibling();
		}
		return true;
	}

	static bool Read(const fs::path& xml_file_path, BMFont& fnt)
	{
		rapidxml::file<char> xmlFile(xml_file_path.string().c_str());
		if (xmlFile.data() == nullptr) {
			return false;
		}

		rapidxml::xml_document<char> doc;
		doc.parse<0>(xmlFile.data());

		auto node_font = doc.first_node();
		if (!node_font) {
			return false;
		}

		auto node = node_font->first_node();
		while (node != nullptr) {
			if (strcmp(node->name(), "info") == 0) {
				if (!ReadInfo(node, fnt)) {
					return false;
				}
			}
			else if (strcmp(node->name(), "common") == 0) {
				if (!ReadCommon(node, fnt)) {
					return false;
				}
			}
			else if (strcmp(node->name(), "pages") == 0) {
				if (!ReadPages(node, fnt)) {
					return false;
				}
			}
			else if (strcmp(node->name(), "chars") == 0) {
				if (!ReadChars(node, fnt)) {
					return false;
				}
			}
			node = node->next_sibling();
		}

		return true;
	}
};

class BMFontOrderdPngExporter {
public:
	static bool Export(const BMFont& bmf, const fs::path& bmf_filepath, uint8_t width, uint8_t height)
	{
		std::vector<std::vector<uint8_t>> src_data;
		std::vector<uint32_t> src_width;
		std::vector<uint32_t> src_height;
		auto page_size = bmf.pages.size();
		src_data.resize(page_size);
		src_width.resize(page_size);
		src_height.resize(page_size);
		for (std::size_t i = 0; i < page_size; i++) {
			auto ppath = bmf_filepath.parent_path().append(bmf.pages[i].file);
			src_data[i].resize(fs::file_size(ppath));
			PngReader::Read(ppath, src_width[i], src_height[i], src_data[i]);
		}

		auto nsize = bmf.chars.size();
		auto wcount = (int)sqrt(nsize); wcount = wcount % 2 == 1 ? wcount + 1 : wcount;
		auto hcount = ((nsize / wcount) + (nsize % wcount > 0 ? 1 : 0));
		auto png_width = width * wcount;
		auto png_height = height * hcount;

		std::vector<png_byte> png_buffer;
		png_buffer.resize(png_width * png_height, 0);

		for (int i = 0; i < nsize; i++) {
			auto& ch = bmf.chars[i];
			auto iw = (i % wcount);
			auto ih = (i / wcount);

			auto ch_w = std::min<int>(width, ch.width);
			auto ch_h = std::min<int>(height, ch.height);
			for (int y = 0; y < ch_h; y++) {
				for (int x = 0; x < ch_w; x++) {
					auto src_pos = (ch.y + y) * src_width[ch.page] + ch.x + x;
					auto dst_pos = (iw * width + x) + (ih * height + y) * png_width;
					png_buffer[dst_pos] = src_data[ch.page][src_pos];

				}
			}
		}

		std::vector<png_bytep> png_rows;
		png_rows.resize(png_height);
		for (int i = 0; i < png_height; i++) {
			png_rows[i] = &png_buffer[i * png_width];
		}

		FILE* fp;
		fopen_s(&fp, (bmf_filepath.string() + ".png").c_str(), "wb");
		auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		auto info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, png_width, png_height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, png_rows.data());
		png_write_end(png_ptr, nullptr);
		fclose(fp);

		return true;
	}
};