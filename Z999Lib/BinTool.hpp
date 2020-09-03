#pragma once

#include "Bin.hpp"

class BinTool
{
public:
	std::array<std::atomic_uint32_t, 6> unpacked{}; // dds, png, sir, avi, ogg, dat
	uint32_t UnpackedCount(uint32_t idx) const { return unpacked[idx].load(); }
	uint32_t patched{};

	bool Patch(const fs::path& src_path, const fs::path& patch_path, const fs::path& dst_dir_path)
	{
		if (!fs::is_regular_file(src_path) || !fs::is_directory(dst_dir_path)) {
			return false;
		}

		std::vector<fs::path> patch_files;
		if (fs::is_regular_file(patch_path)) {
			patch_files.push_back(patch_path);
		}
		else {
			RecursiveDirWork(patch_path, [&patch_files](const fs::path& elm) {
				patch_files.push_back(elm);
			});
		}

		BinFile bin;
		bin.ReadFile(src_path);

		std::vector<uint8_t> buffer;
		patched += bin.GenerateNewBin(patch_files, buffer);

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}

		std::ofstream ofs(fs::path(dst_dir_path).append(src_path.filename().string()), std::ios::binary);
		ofs.write((char*)buffer.data(), buffer.size());
		if (ofs.bad()) {
			return false;
		}

		return true;
	}

	bool UnpackMT(const fs::path& src_path, const fs::path& dst_dir_path)
	{
		if (!fs::is_regular_file(src_path) || !fs::is_directory(dst_dir_path)) {
			return false;
		}

		BinFile bin;
		bin.ReadFile(src_path);

		std::vector<std::string> exts = { "dds", "png", "sir", "avi", "ogg", "dat" };
		std::vector<std::string> sigs = { "DDS", std::string(1,char(0x89)) + "PNG", "SIR1", "RIFF", "OggS", "" };

		if (!fs::exists(dst_dir_path)) {
			fs::create_directory(dst_dir_path);
		}
		for (auto& ext : exts) {
			fs::create_directory(fs::path(dst_dir_path).append(ext));
		}

		const std::size_t THREAD_COUNT = std::thread::hardware_concurrency();

		std::vector<std::shared_ptr<std::promise<int>>> ps(THREAD_COUNT);
		std::vector<std::shared_future<int>> fs(THREAD_COUNT);
		std::vector<std::shared_ptr<std::thread>> ts(THREAD_COUNT);
		
		for (int i = 0; i < THREAD_COUNT; i++) {
			ps[i] = std::make_shared<std::promise<int>>();
			fs[i] = ps[i]->get_future();
			ts[i] = std::make_shared<std::thread>([&](int idx, std::shared_ptr<std::promise<int>> p) {
				int count = 0;
				auto size = bin.nodes.size();

				std::vector<uint8_t> buffer;
				buffer.reserve(1024 * 1024 * 10);

				std::ifstream ifs;
				ifs.open(bin.file_path, std::ifstream::binary);

				for (std::size_t inode = idx; inode < size; inode += THREAD_COUNT) {
					auto& n = bin.nodes[inode];

					int ext_idx = 5;

					ifs.seekg(n->offset + bin.header_offset4,ifs.beg);
					if (n->size > 0) {
						BinFile::ReadNodeMT(ifs, n, buffer);

						for (int iext = 0; iext < 5; iext++) {
							if (std::string_view((char*)buffer.data(), sigs[iext].size()) == sigs[iext]) {
								ext_idx = iext;
								break;
							}
						}
					}
					else {
						buffer.clear();
					}

					auto file_path = fs::path(dst_dir_path).append(exts[ext_idx]).append(ValueToHexString(n->id, false) + "." + exts[ext_idx]);
					std::ofstream ofs(file_path, std::ios::binary);
					ofs.write((char*)buffer.data(), buffer.size());

					unpacked[ext_idx]++;
					count++;
				}
				p->set_value(count);
			}, i, ps[i]);
		}

		for (int i = 0; i < THREAD_COUNT; i++) {
			fs[i].wait();
		}

		for (int i = 0; i < THREAD_COUNT; i++) {
			ts[i]->join();
		}

		return true;
	}
};