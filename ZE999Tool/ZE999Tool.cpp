// ZE999Tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Common.hpp"
#include "SirTool.hpp"
#include "BinTool.hpp"
#include "BMFont.hpp"

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
			printf("Unpacked %lu dds, %lu png, %lu sir, %lu avi, %lu ogg, %lu dat.", tool.UnpackedCount(0), tool.UnpackedCount(1), tool.UnpackedCount(2), tool.UnpackedCount(3), tool.UnpackedCount(4), tool.UnpackedCount(5));
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
			printf("Unpacked %llu Dlgs, %llu Names, %llu Fonts, %llu Items, %llu Msgs, %llu Descs.", tool.dlgs.size(), tool.names.size(), tool.fonts.size(), tool.items.size(), tool.msgs.size(), tool.descs.size());
		}
		else if (cmd == "sir-repack") {
			if (argc < 4) {
				break;
			}
			SirTool tool;
			if (!tool.Repack(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Repacked %llu Dlgs, %llu Names, %llu Fonts, %llu Items, %llu Msgs, %llu Descs.", tool.dlgs.size(), tool.names.size(), tool.fonts.size(), tool.items.size(), tool.msgs.size(), tool.descs.size());
		}
		else if (cmd == "sir-copy-valid") {
			if (argc < 4) {
				break;
			}
			SirTool tool;
			if (!tool.CopyValid(ToAbsolutePath(argv[2]), ToAbsolutePath(argv[3])))
				break;
			printf("Copied %llu Dlgs, %llu Names, %llu Fonts, %llu Items, %llu Msgs, %llu Descs.", tool.dlgs.size(), tool.names.size(), tool.fonts.size(), tool.items.size(), tool.msgs.size(), tool.descs.size());
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
			printf("Patched %llu Dlgs, %llu Names, %llu Fonts, %llu Items, %llu Msgs, %llu Descs.", tool.patch_dlgs.size(), tool.patch_names.size(), tool.patch_fonts.size(), tool.patch_items.size(), tool.patch_msgs.size(), tool.patch_descs.size());
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
