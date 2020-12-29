#pragma once

#include "Sir.hpp"

class SirWriter {
public:
	template <typename T>
	static void WriteAll(std::vector<std::shared_ptr<T>>& sirs, const fs::path& dst_dir_path)
	{
		for (auto sir : sirs) {
			Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
	}
	template <typename T>
	static void WriteAll(std::vector<T*>& sirs, const fs::path& dst_dir_path)
	{
		for (auto sir : sirs) {
			Write(*sir, fs::path(dst_dir_path).append(sir->filename + ".sir"));
		}
	}

	static void Write(const SirDlg& sir, fs::path file_path);
	static void Write(const SirName& sir, fs::path file_path);
	static void Write(const SirFont& sir, fs::path file_path);
	static void Write(const SirItem& sir, fs::path file_path);
	static void Write(const SirMsg& sir, fs::path file_path);
	static void Write(const SirDesc& sir, fs::path file_path);
	static void Write(const SirFChart& sir, fs::path file_path);
	static void Write(const SirDoc& sir, fs::path file_path);
	static void Write(const SirMap& sir, fs::path file_path);
	static void Write(const SirCredit& sir, fs::path file_path);
	static void Write(const SirRoom& sir, fs::path file_path);
};