#pragma once

#include "Sir.hpp"

class SirXmlReader
{
public:
	static std::shared_ptr<SirDlg> ReadDlg(fs::path file_path);
	static std::shared_ptr<SirName> ReadName(fs::path file_path);
	static std::shared_ptr<SirFont> ReadFont(fs::path xmlfile_path, fs::path default_png_file_path, fs::path border_png_file_path);
	static std::shared_ptr<SirItem> ReadItem(fs::path file_path);
	static std::shared_ptr<SirMsg> ReadMsg(fs::path file_path);
	static std::shared_ptr<SirDesc> ReadDesc(fs::path file_path);
	static std::shared_ptr<SirFChart> ReadFChart(fs::path file_path);
	static std::shared_ptr<SirDoc> ReadDoc(fs::path file_path);
	static std::shared_ptr<SirMap> ReadMap(fs::path file_path);
};