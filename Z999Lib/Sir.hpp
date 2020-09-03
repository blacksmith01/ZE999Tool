#pragma once

#include "Common.hpp"

struct SirBase
{
	std::string filename;
	char header_sig[4];
};


struct SirDlg : public SirBase
{
	struct Node {
		std::string id__;
		std::string type;
		std::string name;
		std::string text;
		std::wstring patch_name;
		std::wstring patch_text;

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

	void ModifySize(std::size_t mod_size)
	{
		for (auto& n : nodes) {
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

	std::vector<std::shared_ptr<Node>> nodes;
	std::array<uint32_t,3> footer_unknown_values = {};
};

struct SirName : public SirBase
{
	struct Node {
		std::string key_name;
		std::string name;
		std::string key_msg;
		std::string msg;
		std::array<uint32_t,2> unknown_value;

		std::wstring patch_name;

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