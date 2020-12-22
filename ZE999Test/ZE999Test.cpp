// ZE999Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Common.hpp"
#include "SirTool.hpp"
#include "BinTool.hpp"
#include "XmlTool.hpp"
#include "BMFont.hpp"


#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#pragma comment(lib, "Z999Lib.lib")

namespace fs = std::filesystem;

fs::path root_path;

fs::path ArgPath(std::string_view v) {
    return fs::path(root_path).append(v);
}

int main(int argc, const char* argv[])
{
    std::setlocale(LC_CTYPE, "jpn");

    // parent path of solution
    root_path = fs::path(winext::GetModuleFileName()).parent_path().parent_path().parent_path().parent_path();

    return Catch::Session().run(argc, argv);
}

TEST_CASE("Test All Features", "[test]") {
    //SECTION("Unpack & Repack") {
        //REQUIRE(BinTool().UnpackMT(ArgPath(R"(bin_org\ze1_data.bin)"), ArgPath(R"(data_ze1)")) == true);
        //REQUIRE(SirTool().CopyValid(ArgPath(R"(data_ze1\sir)"), ArgPath(R"(sir_org)")) == true);
        //REQUIRE(SirTool().Unpack(ArgPath(R"(sir_org)"), ArgPath(R"(xml_unpacked)")));
        //REQUIRE(SirTool().Repack(ArgPath(R"(xml_unpacked)"), ArgPath(R"(sir_repacked)")));
        //REQUIRE(SirTool().Unpack(ArgPath(R"(data_descriptions)"), ArgPath(R"(xml_unpacked)")));
        //REQUIRE(SirTool().Unpack(ArgPath(R"(sir_test)"), ArgPath(R"(xml_test)")));
        //REQUIRE(SirTool().Repack(ArgPath(R"(xml_test)"), ArgPath(R"(sir_repacked)")));
        //REQUIRE(XmlTool().ConvertHalfWidthJapaneseXml(ArgPath(R"(xml_unpacked)"), ArgPath(R"(xml_unpacked)")));
        //REQUIRE(SirTool().ExePatch(ArgPath(R"(sir_org)"), ArgPath(R"(xml_patch)"), R"(C:\Program Files (x86)\Steam\steamapps\common\Zero Escape The Nonary Games\ze1.exe.unpacked.exe)", ArgPath(R"(exe_patched)")));
    //} 
    SECTION("Patch") {
        REQUIRE(SirTool().GeneratePatchFontChars(ArgPath(R"(sir_org)"), ArgPath(R"(xml_patch)"), ArgPath(R"(xml_patch\krchars.txt)")));
        REQUIRE(SirTool().GeneratePatchFontData(ArgPath(R"(sir_org)"), ArgPath(R"(xml_patch)"), ArgPath(R"(xml_patch\default.fnt)"), ArgPath(R"(xml_patch\border.fnt)"), ArgPath(R"(xml_patch)")));
        REQUIRE(SirTool().Patch(ArgPath(R"(sir_org)"), ArgPath(R"(xml_patch)"), "5", ArgPath(R"(sir_patched)")));
        REQUIRE(BinTool().Patch(ArgPath(R"(bin_org\ze1_data.bin)"), ArgPath(R"(sir_test)"), ArgPath(R"(.)")));
    }
}

