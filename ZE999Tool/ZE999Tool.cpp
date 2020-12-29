// ZE999Tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Common.hpp"
#include "SirTool.hpp"
#include "BinTool.hpp"
#include "XmlTool.hpp"
#include "BMFont.hpp"

#pragma comment(lib, "Z999Lib.lib")

namespace fs = std::filesystem;

int main(int argc, const char* argv[])
{
	std::setlocale(LC_CTYPE, "jpn");

	int result = -1;

	printf("\r\n");
	do {
		if (argc < 3) {
			break;
		}

		std::string cmd = argv[1];
		if (cmd == "bin-unpack") {
			if (argc < 4) {
				break;
			}
			BinTool tool;
			tool.UnpackMT(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]));
			printf("Unpacked %lu dds, %lu png, %lu sir, %lu avi, %lu ogg, %lu dat.",
				tool.UnpackedCount(0), tool.UnpackedCount(1), tool.UnpackedCount(2), tool.UnpackedCount(3), tool.UnpackedCount(4), tool.UnpackedCount(5));
		}
		else if (cmd == "bin-patch") {
			if (argc < 5) {
				break;
			}
			BinTool tool;
			tool.Patch(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]), ToAbsolutePath(argv[4]));
			printf("Patched %lu Files.", tool.patched);
		}
		else if (cmd == "sir-unpack") {
			if (argc < 4) {
				break;
			}
			SirTool tool;
			if (!tool.Unpack(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Unpacked %s.", tool.org_set.GetCountInfo().c_str());
		}
		else if (cmd == "sir-repack") {
			if (argc < 4) {
				break;
			}
			SirTool tool;
			if (!tool.Repack(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Repacked %s.", tool.org_set.GetCountInfo().c_str());
		}
		else if (cmd == "sir-copy-valid") {
			if (argc < 4) {
				break;
			}
			SirTool tool;
			if (!tool.CopyValid(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Copied %s.", tool.org_set.GetCountInfo().c_str());
		}
		else if (cmd == "sir-generate-patch-chars") {
			if (argc < 5) {
				break;
			}
			SirTool tool;
			if (!tool.GeneratePatchFontChars(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]), ToAbsolutePath(argv[4])))
				break;
			printf("Patch Font Chars Generated.");
		}
		else if (cmd == "sir-generate-font-data") {
			if (argc < 7) {
				break;
			}
			SirTool tool;
			if (!tool.GeneratePatchFontData(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]), ToAbsolutePath(argv[4]), ToAbsolutePath(argv[5]), ToAbsolutePath(argv[6])))
				break;
			printf("Patch Font Data Generated.");
		}
		else if (cmd == "sir-patch") {
			if (argc < 6) {
				break;
			}
			SirTool tool;
			if (!tool.Patch(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]), argv[4], ToAbsolutePath(argv[5])))
				break;
			printf("Patched %s.", tool.patch_set.GetCountInfo().c_str());
		}
		else if (cmd == "exe-patch") {
			if (argc < 6) {
				break;
			}
			SirTool tool;
			if (!tool.ExePatch(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3]), ToAbsolutePath(argv[4]), ToAbsolutePath(argv[5])))
				break;

			printf("Exe File Generated.");
		}
		else if (cmd == "xml-conv-half-width-jp") {
			if (argc < 4) {
				break;
			}
			XmlTool tool;
			if (!tool.ConvertHalfWidthJapaneseXml(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Complete Converting.");
		}
		else {
			break;
		}

		result = 0;
	} while (false);

	if (result == -1) {
		printf("Error : invalid arguments.");
	}
	printf("\r\n");

	return result;
}
