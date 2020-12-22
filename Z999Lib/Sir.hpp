#pragma once

#include "Common.hpp"

struct SirBase
{
	std::string filename;
	char header_sig[4];
};


struct SirDlg : public SirBase
{
	static constexpr char XmlExtension[] = ".dlg.xml";

	struct Node {
		std::string id__;
		std::string type;
		std::string name;
		std::string text;
		std::wstring patch_name;
		std::wstring patch_text;

		bool Equal(const Node& other) const {
			return id__ == other.id__ && type == other.type;
		}

		std::size_t Size() const {
			return
				id__.length() + 1 +
				type.length() + 1 +
				name.length() + 1 +
				text.length() + 1;
		}
	};
	std::vector<std::shared_ptr<Node>> nodes;
};

struct SirFont : public SirBase
{
	static constexpr char XmlExtension[] = ".font.xml";
	static constexpr char DefaultPngExtension[] = ".default.png";
	static constexpr char BorderPngExtension[] = ".border.png";

	struct Node
	{
		char keycode[6] = {};
		std::string patch_keycode;
		uint8_t wsize[2] = {};
		uint8_t hsize[2] = {};
		std::vector<uint8_t> data[2];

		std::string SjisString() const {
			std::string s;
			if (keycode[1] == 0) {
				s += keycode[0];
			}
			else {
				s += keycode[1];
				s += keycode[0];
			}
			return s;
		}
		wchar_t Unicode() const
		{
			auto wcs = mbs_to_wcs(SjisString());
			if (!wcs.empty())
				return wcs[0];
			else
				return 0;
		}
		std::string Utf8String() const {
			return keycode[1] == 0 ? SjisString() : mbs_to_utf8(SjisString());
		}

		std::size_t Size() const {
			auto f1_size = wsize[0] * hsize[0];
			auto f2_size = wsize[1] * hsize[1];
			return 10 + f1_size + f2_size + Padding();
		}
		std::size_t Padding() const {
			auto f1_size = wsize[0] * hsize[0];
			auto f2_size = wsize[1] * hsize[1];
			uint32_t total_size = 10 + f1_size + f2_size;
			return (4 - (total_size & 0x03)) & 0x03;
		}
	};

	void ReduceKanjiSize(const std::wstring& excepts, std::size_t mod_size)
	{
		for (auto& n : nodes) {
			// skip ansi & jp symbols
			if (n->keycode[1] == 0 || (uint8_t)n->keycode[1] == 0x81 || (uint8_t)n->keycode[1] == 0x84) {
				continue;
			}
			if (!excepts.empty() && excepts.find(n->Unicode()) != std::string::npos) {
				continue;
			}

			for (int i = 0; i < 2; i++) {
				auto max_value = std::max<uint8_t>(n->wsize[i], n->hsize[i]);
				auto ratio = (max_value * 100 / mod_size) * 0.01f;

				std::vector<uint8_t> data;
				data.resize(mod_size * mod_size);
				for (int y = 0; y < mod_size; y++) {
					auto orgy = std::min<uint8_t>((uint8_t)(y * ratio), n->hsize[i] - 1);
					for (int x = 0; x < mod_size; x++) {
						auto orgx = std::min<uint8_t>((uint8_t)(x * ratio), n->wsize[i] - 1);
						data[y * mod_size + x] = n->data[i][orgy * n->wsize[i] + orgx];
					}
				}

				n->wsize[i] = mod_size;
				n->hsize[i] = mod_size;
				n->data[i].swap(data);
			}
		}
	}

	void RemoveKanji(const std::wstring& excepts)
	{
		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&excepts](auto& n) {
			if (!excepts.empty() && excepts.find(n->Unicode() != std::string::npos)) {
				return false;
			}
			return n->keycode[1] != 0 && (uint8_t)n->keycode[1] != 0x81 && (uint8_t)n->keycode[1] != 0x84;
		}),nodes.end());
	}

	std::vector<std::shared_ptr<Node>> nodes;
	std::array<uint32_t, 3> footer_unknown_values = {};
};

struct SirName : public SirBase
{
	static constexpr char XmlExtension[] = ".name.xml";

	struct Node {
		std::string key_name;
		std::string name;
		std::string key_msg;
		std::string msg;
		std::array<uint32_t, 2> unknown_value;

		std::wstring patch_name;

		bool Equal(const Node& other) const {
			return key_name == other.key_name && key_msg == other.key_msg;
		}

		std::size_t Size() const {
			return
				key_name.length() + 1 +
				name.length() + 1 +
				key_msg.length() + 1 +
				msg.length() + 1;
		}
	};
	std::vector<std::shared_ptr<Node>> nodes;
};

struct SirItem : public SirBase
{
	static constexpr char XmlExtension[] = ".item.xml";

	struct Node
	{
		struct Item
		{
			std::string key;
			std::string text1;
			std::string text2;
			std::array<int32_t, 4> unknowns;

			std::wstring patch_text;

			bool Equal(const Item& other) const {
				return key == other.key;
			}

			std::size_t Size() const {
				return
					key.length() + 1 +
					text1.length() + 1 +
					text2.length() + 1;
			}
		};

		bool Equal(const Node& other) const {
			return name == other.name;
		}

		std::string name;
		std::vector<std::shared_ptr<Item>> items;

		std::size_t Size() const
		{
			auto size = name.length() + 1;
			for (auto& item : items) {
				size += item->Size();
			}
			return size;
		}
	};

	std::vector<std::shared_ptr<Node>> nodes;

	std::size_t ItemCount() const
	{
		std::size_t item_count = 0;
		for (auto& n : nodes) {
			item_count += n->items.size();
		}
		return item_count;
	}
};

struct SirMsg : public SirBase
{
	static constexpr char XmlExtension[] = ".msg.xml";

	struct Node
	{
		std::string key;
		std::vector<std::string> texts;
		std::vector<std::wstring> patch_texts;

		bool Equal(const Node& other) const {
			return key == other.key;
		}

		std::size_t Size() const
		{
			std::size_t size = key.length() + 1;
			for (auto& t : texts) {
				size += t.length() + 1;
			}
			return size;
		}

		std::string AllText() const
		{
			std::string text;
			for (auto& t : texts) {
				text += t;
			}
			return text;
		}

		std::array<int32_t, 4> unknowns;
	};

	std::vector<std::shared_ptr<Node>> nodes;
	uint64_t unknown;
};

struct SirDesc : public SirBase
{
	static constexpr char XmlExtension[] = ".desc.xml";

	struct Node
	{
		std::vector<uint8_t> bytes;
		std::string id;
	};

	struct Text
	{
		bool Equal(const Text& other) const {
			return temp_id == other.temp_id;
		}

		int temp_id;
		std::string value;
		std::wstring patch_text;
	};

	bool IsMapType()const
	{
		if (nodes.size() == 2) {
			if (nodes[0]->id == "~OP.MapIn")
				return false;

			if (nodes[0]->id.find("MapIn") != std::string::npos && nodes[1]->id.find("MapStart") != std::string::npos)
				return true;
		}
		return false;
	}
	bool IsItemDescType() const
	{
		return (starts.size() == 1 && starts[0] == "START" && !IsMapType());
	}

	std::string sound_file_name;
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Text>> texts;
	std::vector<std::string> starts;
	std::vector<std::string> vars;
};

struct SirFChart : public SirBase
{
	static constexpr char XmlExtension[] = ".fchart.xml";

	struct Node
	{
		std::string id1;
		std::string id2;
		std::string name_jp;
		std::string filename;
		std::string name;

		std::string text;
		std::string desc_jp;
		std::string type_id1;
		std::string command1;
		std::string type_id2;

		std::wstring patch_text;

		bool Equal(const Node& other) const {
			return id1 == other.id1 && id2 == other.id2;
		}

		std::size_t BaseSize() const
		{
			return 1 + id1.length()
				+ 1 + id2.length()
				+ 1 + name_jp.length()
				+ 1 + filename.length()
				+ 1 + name.length()
				+ 1 + text.length()
				+ 1 + desc_jp.length()
				+ 1 + type_id1.length()
				+ 1 + command1.length()
				+ 1 + type_id2.length();
		}
		std::size_t Size() const
		{
			std::size_t size = BaseSize();
			for (auto& i : items)
				size += i->Size();

			return size;
		}

		struct Item
		{
			std::string id1;
			std::string id2;
			std::string name_jp;
			std::string filename;
			std::string name;
			std::string text;

			std::wstring patch_text;

			std::size_t Size() const
			{
				return 1 + id1.length()
					+ 1 + id2.length()
					+ 1 + name_jp.length()
					+ 1 + filename.length()
					+ 1 + name.length()
					+ 1 + text.length();
			}
		};

		std::vector<std::shared_ptr<Item>> items;
	};

	std::size_t ItemCount() const
	{
		std::size_t size = 0;
		for (auto& n : nodes)
			size += n->items.size();
		return size;
	}

	std::vector<std::shared_ptr<Node>> nodes;
};

struct SirDoc : public SirBase
{
	static constexpr char XmlExtension[] = ".doc.xml";

	struct Node
	{
		std::string key1;
		std::string key2;
		std::string text1;
		std::string text2;
		std::wstring patch_text1;
		std::wstring patch_text2;

		std::vector<std::string> contents;
		std::vector<std::wstring> patch_contents;

		bool Equal(const Node& other) const {
			return key1 == other.key1 && key2 == other.key2;
		}

		std::size_t HeaderSize() const
		{
			std::size_t size = 0;
			size += 1 + key1.length();
			size += 1 + key2.length();
			size += 1 + text1.length();
			size += 1 + text2.length();
			return size;
		}

		std::size_t Size() const
		{
			std::size_t size = HeaderSize();
			for (auto& t : contents) {
				size += t.length() + 1;
			}
			return size;
		}

		std::string AllText() const
		{
			std::string text = text1 + text2;
			for (auto& t : contents) {
				text += t;
			}
			return text;
		}

	};

	std::vector<std::shared_ptr<Node>> nodes;
};

struct SirMap : public SirBase
{
	static constexpr char XmlExtension[] = ".map.xml";

	struct Node
	{
		struct Item
		{
			std::string key;
			std::string text;
			std::string desc;
			std::array<uint32_t, 3> unknowns;
			std::wstring patch_text;

			bool Equal(const Item& other) const {
				return key == other.key;
			}

			std::size_t Size() const {
				return
					key.length() + 1 +
					text.length() + 1 +
					desc.length() + 1 +
					sizeof(uint32_t) * 3;
			}
		};
		std::string name;
		std::vector<std::shared_ptr<Item>> items;

		bool Equal(const Node& other) const {
			return name == other.name;
		}

		std::size_t Size() const {
			auto size = name.length() + 1;
			for (auto& item : items)
			{
				size += item->Size();
			}
			return size;
		}
	};

	std::vector<std::shared_ptr<Node>> nodes;
};