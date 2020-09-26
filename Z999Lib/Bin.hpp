#pragma once

#include "Common.hpp"


uint32_t nonary_crypt(uint8_t* data, int size, uint32_t key, uint32_t relative_offset)
{
	uint32_t    eax,
		ecx,
		edx,
		edi,
		esi;

	eax = relative_offset;
	esi = (relative_offset + 3) << 0x18;
	ecx = (relative_offset + 2) << 0x10;
	edi = (relative_offset + 1) << 0x8;

	int i;
	for (i = 0; i < ((size / 4) * 4); i += 4) {
		edx = (ecx & 0xff0000) | (edi & 0xff00) | (esi & 0xff000000) | (eax & 0xff);
		eax += 0x4;
		edi += 0x400;
		ecx += 0x40000;
		esi += 0x4000000;
		*(int*)(data + i) ^= edx ^ key;
	}
	for (; i < size; i++) {
		data[i] ^= eax ^ key;
		eax++;
		key >>= 8;
	}
	return eax;
}

int nonary_calculate_key(const char* name)
{
	int     i,
		size;
	uint32_t    eax = 0,
		esi = 0,
		edx = 0;

	for (size = 0; name[size]; size++);

	for (i = 0; i < ((size / 2) * 2); i += 2) {
		eax += name[i] + name[i + 1];
		edx = (name[i] & 0xdf) + (esi * 0x83);
		esi = (name[i + 1] & 0xdf) + (edx * 0x83);
	}
	for (; i < size; i++) {
		eax += name[i];
		esi = (name[i] & 0xdf) + (esi * 0x83);
	}
	return (eax & 0xf) | ((esi & 0x07FFFFFF) << 4);
}

class BinFile
{
public:
	static const uint32_t ze1_key_header = 0xfabaceda;

	fs::path file_path;
	std::ifstream ifs;
	uint32_t header_offset1{};
	uint32_t header_offset2{};
	uint64_t header_offset3{};
	uint64_t header_offset4{};

	uint32_t main_key{};
	uint64_t footer_offset{};
	uint64_t StructureDataSize() const { return header_offset4 - header_offset2; }

	struct Node
	{
		uint64_t offset;
		uint32_t key;
		uint32_t size;
		uint32_t xsize;
		uint32_t id;
		uint32_t flags;
		uint32_t dummy;
	};
	std::vector<std::shared_ptr<Node>> nodes;

	std::vector<uint8_t> Read(uint32_t offset, std::size_t size) const
	{
		std::vector<uint8_t> buffer;
		buffer.resize(size);
		Read(offset, buffer);
		return buffer;
	}
	void Read(uint32_t offset, std::vector<uint8_t>& buffer) const
	{
		Read(offset, buffer.data(), buffer.size());
	}
	void Read(uint32_t offset, uint8_t* buffer, std::size_t size) const
	{
		std::streambuf* pbuf = ifs.rdbuf();
		pbuf->pubseekoff(offset, ifs.beg);
		pbuf->sgetn((char*)buffer, size);
	}

	static void XOR(uint32_t key, uint32_t offset, std::span<uint8_t> data)
	{
		std::vector<uint8_t> xor_key;
		xor_key.resize(data.size(), 0);
		nonary_crypt(xor_key.data(), xor_key.size(), key, offset);

		int i = 0;
		auto key_en = xor_key.size();
		for (auto& d : data) {
			d ^= xor_key[i % key_en];
			i++;
		}
	}

	void ReadFile(const fs::path& _file_path)
	{
		file_path = _file_path;
		auto file_size = fs::file_size(file_path);
		if (file_path.stem().string() == "ze1_data") {
			main_key = ze1_key_header;
		}
		else {
			main_key = nonary_calculate_key("ZeroEscapeTNG");
		}

		ifs.open(file_path, std::ifstream::binary);

		auto data_header = Read(0, 32);
		XOR(main_key, 0, data_header);

		auto beg_pos = (const char*)data_header.data();
		MemReader mr(data_header);

		if (memcmp(mr.ReadVector<char>(4).data(), "bin.", 4) != 0) {
			return;
		}

		mr.Read(header_offset1);
		mr.Read(header_offset2);
		mr.Read(header_offset3);
		mr.Read(header_offset4);

		auto data_structure = Read(header_offset2, StructureDataSize());
		XOR(main_key, header_offset2, data_structure);

		beg_pos = (const char*)data_structure.data();
		mr.SetPos(data_structure);

		auto node_offset = mr.Read<uint32_t>();
		auto node_count = mr.Read<uint32_t>();

		mr.SetPos(std::span(data_structure).subspan(node_offset));
		for (int i = 0; i < node_count; i++) {
			auto n = std::make_shared<Node>();
			mr.Read(n->offset);
			mr.Read(n->key);
			mr.Read(n->size);
			mr.Read(n->xsize);
			mr.Read(n->id);
			mr.Read(n->flags);
			mr.Read(n->dummy);
			nodes.push_back(n);
		}

		if (!nodes.empty()) {
			footer_offset = GetNextNodeOffset(nodes.back()->offset, nodes.back()->size);
		}
		else {
			footer_offset = header_offset4;
		}

		auto data_footer = Read(header_offset4, file_size - footer_offset);
		XOR(main_key, header_offset4, data_footer);

		printf("");
	}

	void ReadNode(const std::shared_ptr<Node>& n, std::vector<uint8_t>& buffer) const
	{
		buffer.resize(n->size);
		Read(n->offset + header_offset4, buffer);
		XOR(n->key, 0, buffer);
	}
	
	static void ReadNodeMT(std::ifstream& ifs, const std::shared_ptr<Node>& n, std::vector<uint8_t>& buffer)
	{
		buffer.resize(n->size);
		ifs.read((char*)buffer.data(), buffer.size());
		XOR(n->key, 0, buffer);
	}

	uint64_t GetNextNodeOffset(uint64_t offset, uint64_t size) const
	{
		return offset + size + GetNodePaddingSize(offset, size);
	}
	uint64_t GetNodePaddingSize(uint64_t offset, uint64_t size) const
	{
		return (16 - ((offset + size) & 0x0F)) & 0x0F;
	}

	int GenerateNewBin(const std::vector<fs::path>& mod_file_paths, std::vector<uint8_t>& buffer) const
	{
		int patched_count = 0;

		std::vector<uint32_t> mod_file_ids;
		mod_file_ids.reserve(mod_file_paths.size());
		for (auto& p : mod_file_paths) {
			auto filename = p.stem().string();
			auto bytes = HexStringToBytes(filename);
			uint32_t id = 0;
			auto len = std::min<std::size_t>(4, bytes.size());
			for (std::size_t i = 0; i < len; i++)
				*(((uint8_t*)(&id)) + i) = bytes[3 - i];
			mod_file_ids.push_back(id);
		}

		auto org_file_size = fs::file_size(file_path);
		int64_t diff_size = 0;
		{
			std::size_t org_offset = 0;
			std::size_t mod_offset = 0;
			for (auto& n : nodes) {
				auto add_size = n->size;
				org_offset = GetNextNodeOffset(org_offset, add_size);

				auto it = std::find(mod_file_ids.begin(), mod_file_ids.end(), n->id);
				if (it != mod_file_ids.end()) {
					add_size = fs::file_size(mod_file_paths[std::distance(mod_file_ids.begin(), it)]);
				}
				mod_offset = GetNextNodeOffset(mod_offset, add_size);
			}

			if (org_offset >= mod_offset) {
				diff_size = -((int64_t)(org_offset - mod_offset));
			}
			else {
				diff_size = ((int64_t)(mod_offset- org_offset));
			}
		}
		buffer.resize(org_file_size + diff_size);
		Read(0, buffer.data(), header_offset4);

		auto data_structure = buffer.data() + header_offset2;
		XOR(main_key, header_offset2, std::span(data_structure, StructureDataSize()));
		auto cur_pos = data_structure;
		auto pnode_offset = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
		auto pnode_count = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);

		std::vector<uint8_t> node_buffer;
		node_buffer.reserve(1024 * 1024 * 10);

		uint64_t ndoe_offset_alloc = 0;
		cur_pos = data_structure + *pnode_offset;
		for (int i = 0; i < *pnode_count; i++) {
			auto poffset = (uint64_t*)cur_pos; cur_pos += sizeof(uint64_t);
			auto pkey = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
			auto psize = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
			auto pxsize = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
			auto pid = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
			auto pflags = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);
			auto pdummy = (uint32_t*)cur_pos; cur_pos += sizeof(uint32_t);

			auto pdata = buffer.data() + ndoe_offset_alloc + header_offset4;

			auto it = std::find(mod_file_ids.begin(), mod_file_ids.end(), *pid);
			if (it != mod_file_ids.end()) {
				auto mod_offset = std::distance(mod_file_ids.begin(), it);
				node_buffer.resize(fs::file_size(mod_file_paths[mod_offset]));
				::ReadFile(mod_file_paths[mod_offset], node_buffer.data());
				XOR(*pkey, 0, node_buffer);
				patched_count++;
			}
			else {
				node_buffer.resize(*psize);
				Read(*poffset + header_offset4, node_buffer);
			}

			memcpy(pdata, node_buffer.data(), node_buffer.size());
			*psize = node_buffer.size();
			if (*poffset != ndoe_offset_alloc) {
				printf("");
			}
			*poffset = ndoe_offset_alloc;

			ndoe_offset_alloc = ndoe_offset_alloc + *psize;
			auto padding = GetNodePaddingSize(*poffset, *psize);
			if (padding) {
				memset(pdata + *psize, 0, padding);
			}
			ndoe_offset_alloc += padding;
		}

		XOR(main_key, header_offset2, std::span(data_structure, StructureDataSize()));

		Read(footer_offset, buffer.data() + ndoe_offset_alloc, org_file_size - footer_offset);

		return patched_count;
	}
};
