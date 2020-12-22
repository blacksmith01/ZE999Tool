#pragma once

#include "Sir.hpp"
#include "SirPngWriter.hpp"

class SirXmlWriter
{
public:
	template <typename T>
	static void WriteAll(std::vector<std::shared_ptr<T>>& sirs, const fs::path& dst_dir_path)
	{
		for (auto& sir : sirs) {
			Write(*sir, fs::path(dst_dir_path).append(sir->filename + T::XmlExtension));
		}
	}
	template<>
	static void WriteAll(std::vector<std::shared_ptr<SirFont>>& sirs, const fs::path& dst_dir_path)
	{
		for (auto& sir : sirs) {
			Write(*sir, fs::path(dst_dir_path).append(sir->filename + SirFont::XmlExtension));
			SirPngWriter::Write(*sir, 0, fs::path(dst_dir_path).append(sir->filename + SirFont::DefaultPngExtension));
			SirPngWriter::Write(*sir, 1, fs::path(dst_dir_path).append(sir->filename + SirFont::BorderPngExtension));
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
};
