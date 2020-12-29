#pragma once

#include "Sir.hpp"

class SirReader {
public:
	static std::shared_ptr<SirDlg> ReadDlg(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirDlg::Node> ReadDlgNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidDlg(const std::span<char>& buffer);

	static std::shared_ptr<SirName> ReadName(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirName::Node> ReadNameNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidName(const std::span<char>& buffer);

	static std::shared_ptr<SirFont> ReadFont(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirFont::Node> ReadFontNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidFont(const std::span<char>& buffer);

	static std::shared_ptr<SirItem> ReadItem(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirItem::Node> ReadItemNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidItem(const std::span<char>& buffer);

	static std::shared_ptr<SirMsg> ReadMsg(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirMsg::Node> ReadMsgNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset);
	static bool IsValidMsg(const std::span<char>& buffer);

	static std::shared_ptr<SirDesc> ReadDesc(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirDesc::Node> ReadDescNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset, uint64_t text_offset);
	static bool IsValidDesc(const std::span<char>& buffer);

	static std::shared_ptr<SirFChart> ReadFChart(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirFChart::Node> ReadFChartNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidFChart(const std::span<char>& buffer);

	static std::shared_ptr<SirDoc> ReadDoc(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirDoc::Node> ReadDocNode(const std::span<char>& buffer, uint64_t offset, uint64_t next_offset);
	static bool IsValidDoc(const std::span<char>& buffer);

	static std::shared_ptr<SirMap> ReadMap(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirMap::Node> ReadMapNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidMap(const std::span<char>& buffer);

	static std::shared_ptr<SirCredit> ReadCredit(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirCredit::Node> ReadCreditNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidCredit(const std::span<char>& buffer);

	static std::shared_ptr<SirRoom> ReadRoom(std::string filename, const std::span<char>& buffer);
	static std::shared_ptr<SirRoom::Node> ReadRoomNode(const std::span<char>& buffer, uint64_t offset);
	static bool IsValidRoom(const std::span<char>& buffer);
};
