#pragma once

#pragma warning( disable : 4101)

#ifdef _DEBUG
#define M_IS_DEBUG true
#else
#define M_IS_DEBUG false
#endif

#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include <array>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <span>
#include <string>
#include <algorithm>
#include <memory>
#include <clocale>
#include <future>
#include <atomic>
#include <locale>
#include <codecvt>

namespace stdext
{
	template<typename T>
	inline bool IsValidBufferPtr(const std::span<T>& container, const T* check_ptr)
	{
		if (container.empty())
			return false;
		else if (&container.front() > check_ptr)
			return false;
		else if (&container.back() < check_ptr)
			return false;
		return true;
	}

	template<typename T, typename F>
	inline T* FindPtr(const std::span<std::shared_ptr<T>>& container, F func)
	{
		auto it = std::find_if(container.begin(), container.end(), func);
		if (it == container.end()) {
			return nullptr;
		}

		return &(*(*it));
	}
	template<typename T, typename F>
	inline T* FindPtr(const std::span<T>& container, F func)
	{
		auto it = std::find_if(container.begin(), container.end(), func);
		if (it == container.end()) {
			return nullptr;
		}

		return &(*it);
	}
	template<typename T, typename F>
	inline int FindIdx(const T& container, F func)
	{
		auto size = (int)container.size();
		for (int i = 0; i < size; i++) {
			if (func(container[i])) {
				return i;
			}
		}
		return -1;
	}
}


#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

class MemReader {
private:
	std::span<char> sp;
	int pos;

public:
	MemReader(std::span<char> _sp)
	{
		SetPos(_sp);
	}
	MemReader(std::span<uint8_t> _sp)
	{
		SetPos(_sp);
	}
	MemReader(const char* ptr, std::size_t len)
	{
		SetPos(std::span<char>((char*)ptr, len));
	}
	MemReader(const uint8_t* ptr, std::size_t len)
	{
		SetPos(std::span<char>((char*)ptr, len));
	}

	const char* Ptr() const {
		return sp.data() + pos;
	}

	int CurrPos() const
	{
		return pos;
	}

	void SetPos(std::span<char> _sp)
	{
		if (_sp.empty()) {
			throw std::exception("!memory access");
		}
		sp = _sp;
		pos = 0;
	}
	void SetPos(std::span<uint8_t> _sp)
	{
		if (_sp.empty()) {
			throw std::exception("!memory access");
		}
		sp = std::span<char>((char*)_sp.data(), _sp.size());
		pos = 0;
	}

	void Forward(std::size_t len)
	{
		if (pos + len > sp.size())
			throw std::exception("!memory access");

		pos += len;
	}

	void Seek(std::size_t offset)
	{
		if (offset > sp.size())
			throw std::exception("!memory access");

		pos = (int)offset;
	}

	bool IsEnd() const
	{
		return (pos >= sp.size());
	}

	template<typename T>
	T Read()
	{
		if (pos + sizeof(T) > sp.size())
			throw std::exception("!memory access");

		T out;
		out = *((T*)Ptr());
		pos += sizeof(T);
		return out;
	}

	template<typename T>
	void Read(T& out)
	{
		if (pos + sizeof(T) > sp.size())
			throw std::exception("!memory access");

		out = *((T*)Ptr());
		pos += sizeof(T);
	}

	template<typename T>
	void ReadArray(std::size_t len, T* out)
	{
		if (pos + sizeof(T) * len > sp.size())
			throw std::exception("!memory access");

		memcpy(out, Ptr(), sizeof(T) * len);
		pos += sizeof(T) * len;
	}
	template<typename T, int N>
	void ReadArray(std::array<T, N>& out)
	{
		ReadArray(N, &out[0]);
	}
	template<typename T, int N>
	std::array<T, N> ReadArray()
	{
		std::array<T, N> out;
		ReadArray(N, &out[0]);
		return out;
	}
	template<typename T>
	void ReadVector(std::vector<T>& out)
	{
		ReadArray(out.size(), &out[0]);
	}
	template<typename T>
	std::vector<T> ReadVector(std::size_t size)
	{
		std::vector<T> out;
		out.resize(size);
		ReadArray(size, &out[0]);
		return out;
	}
};

class MemWriter {
public:
	char*& pos;
	MemWriter(char*& _pos) : pos(_pos) {}

	template<typename T>
	void Write(const T& v)
	{
		*((T*)pos) = v;
		pos += sizeof(T);
	}
	template<typename T>
	void WriteString(const T* v, std::size_t len)
	{
		memcpy(pos, v, sizeof(T) * len);
		pos += sizeof(T) * len;

		Write((T)0);
	}
	void WriteString(const std::string& v)
	{
		WriteString(v.c_str(), v.length());
	}
	template<typename T>
	void WriteArray(T* v, std::size_t len)
	{
		memcpy(pos, v, sizeof(T) * len);
		pos += sizeof(T) * len;
	}
	template<typename T, int N>
	void WriteArray(const std::array<T, N>& v)
	{
		WriteArray(&v[0], N);
	}
	template<typename T>
	void WriteArray(const std::vector<T>& v)
	{
		WriteArray(&v[0], v.size());
	}
};

inline void ReadFile(const fs::path& path, void* buffer)
{
	std::ifstream ifs(path, std::ios::binary);
	ifs.rdbuf()->sgetn((char*)buffer, fs::file_size(path));
}
inline std::string ReadText(const fs::path& path, std::size_t except_header_size = 0)
{
	std::ifstream ifs(path);
	ifs.rdbuf()->pubseekoff(except_header_size, ifs.beg);

	std::vector<char> buffer;
	buffer.resize(fs::file_size(path) - except_header_size + 1);
	ifs.rdbuf()->sgetn(buffer.data(), buffer.size() - 1);
	buffer.back() = 0;

	return buffer.data();
}

class uintvar {
public:
	uintvar(uint64_t value) {
		int highest_bit = 0;
		std::bitset<sizeof(uint64_t) * 8> bs(value);
		for (int i = sizeof(uint64_t) * 8 - 1; i >= 0; i--) {
			if (bs[i]) {
				highest_bit = i;
				break;
			}
		}
		int highest_byte = highest_bit / 7 + 1;
		bytes.resize(highest_byte);
		for (int i = 0; i < highest_byte; i++) {
			auto v = (value >> (highest_byte - 1 - i) * 7) & 0x7F;
			bytes[i] = (uint8_t)(v | 0x80);
		}
		bytes.back() &= 0x7F;
	}
	std::vector<uint8_t> bytes;
};

inline bool StrCmpEndWith(const std::string& s, const char* cmp)
{
	auto slen = s.length();
	auto clen = strlen(cmp);
	return (slen >= clen + 1 && std::string_view(s.c_str() + (slen - clen), clen) == cmp);
}

template<typename T>
inline std::string ValueToHexString(T v, bool include_header = true) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << v;
	return ((include_header) ? "0x" : "") + std::string(stream.str());
}
template<typename T>
inline std::wstring ValueToHexWString(T v, bool include_header = true) {
	std::wstringstream stream;
	stream << std::setfill(L'0') << std::setw(sizeof(T) * 2) << std::hex << v;
	return ((include_header) ? L"0x" : L"") + std::wstring(stream.str());
}

std::string BytesToHexString(const std::span<uint8_t>& arr) {
	static const char* digits = "0123456789ABCDEF";
	auto hex_len = arr.size();
	std::string rc(hex_len * 2, '0');
	for (size_t i = 0; i < hex_len; ++i) {
		rc[i * 2 + 0] = digits[arr[i] >> 4 & 0x0f];
		rc[i * 2 + 1] = digits[arr[i] >> 0 & 0x0f];
	}
	return "0x" + rc;
}

std::vector<uint8_t> HexStringToBytes(const std::string& hex) {
	std::vector<uint8_t> bytes;

	std::size_t hex_len = hex.size();
	std::size_t start_idx = 0;
	if (hex_len > 2 && hex.compare(0, 2, "0x") == 0) {
		start_idx = 2;
	}
	for (std::size_t i = start_idx; i < hex_len; i += 2) {
		std::string byteString = hex.substr(i, 2);
		uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}


inline std::wstring utf8_to_wcs(const std::string& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(src);
}
inline std::string wcs_to_utf8(const std::wstring& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(src);
}

inline std::wstring mbs_to_wcs(const std::string& src)
{
	if (src.empty()) {
		return L"";
	}

	std::size_t converted{};
	std::vector<wchar_t> dest(src.size() + 1, L'\0');
	if (::mbstowcs_s(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE) != 0) {
		throw std::system_error{ errno, std::system_category() };
	}
	dest.resize(std::char_traits<wchar_t>::length(dest.data()));
	dest.shrink_to_fit();
	return std::wstring(dest.begin(), dest.end());
}
inline std::wstring mbs_to_wcs(const std::string& src, const std::wstring& err_default)
{
	try {
		return mbs_to_wcs(src);
	}
	catch (const std::exception& e) {
		return err_default;
	}
}

inline std::string wcs_to_mbs(const std::wstring& src)
{
	if (src.empty()) {
		return "";
	}

	std::size_t converted{};
	std::vector<char> dest(src.size() * sizeof(wchar_t) + 1, '\0');
	if (::wcstombs_s(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE) != 0) {
		throw std::system_error{ errno, std::system_category() };
	}
	dest.resize(std::char_traits<char>::length(dest.data()));
	dest.shrink_to_fit();
	return std::string(dest.begin(), dest.end());
}
inline std::string wcs_to_mbs(const std::wstring& src, const std::string& err_default)
{
	try {
		return wcs_to_mbs(src);
	}
	catch (const std::exception& e) {
		return err_default;
	}
}

inline std::string mbs_to_utf8(const std::string& src) {
	try {
		return wcs_to_utf8(mbs_to_wcs(src));
	}
	catch (const std::exception& e) {
		return src;
	}
}

inline std::string utf8_to_mbs(const std::string& src)
{
	try {
		return wcs_to_mbs(utf8_to_wcs(src));
	}
	catch (const std::exception& e) {
		return src;
	}
}

inline bool sjis_valid(const std::string& src)
{
	try {
		mbs_to_wcs(src);
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

#include "../include/rapidxml/rapidxml.hpp"
#include "../include/rapidxml/rapidxml_utils.hpp"
#include "../include/rapidxml/rapidxml_print.hpp"

template<typename T>
inline const char* RapidXmlString(rapidxml::xml_document<char>& doc, const T& v) {
	return doc.allocate_string(std::to_string(v).c_str());
}
template<typename T>
inline const wchar_t* RapidXmlString(rapidxml::xml_document<wchar_t>& doc, const T& v) {
	return doc.allocate_string(std::to_wstring(v).c_str());
}

inline const char* RapidXmlString(rapidxml::xml_document<char>& doc, const std::string& v) {
	return doc.allocate_string(v.c_str(), v.size() + 1);
}
inline const wchar_t* RapidXmlString(rapidxml::xml_document<wchar_t>& doc, const std::wstring& v) {
	return doc.allocate_string(v.c_str(), v.size() + 1);
}

#define M_RAPID_XML_ATTR_IF_SET_STRING(attr, attr_name, target)\
else if (strcmp(attr->name(), attr_name) == 0) {\
	target = attr->value();\
}

#define M_RAPID_XML_ATTR_IF_SET_INT(attr, attr_name, target)\
else if (strcmp(attr->name(), attr_name) == 0) {\
	target = atoi(attr->value());\
}
#define M_RAPID_XML_ATTR_IF_SET_INT_ARRAY(attr, attr_name, target)\
else if (strcmp(attr->name(), attr_name) == 0) {\
	std::vector<char> buffer;\
	buffer.resize(strlen(attr->value()) + 1, 0);\
	memcpy(buffer.data(), attr->value(), buffer.size()-1);\
	char* ctx = nullptr;\
	target[0] = atoi(strtok_s(buffer.data(), ",", &ctx));\
	for (int i = 1; i < target.size(); i++) {\
		target[i] = atoi(strtok_s(nullptr, ",", &ctx));\
	}\
}

#define M_RAPID_XML_ATTR_IF_SET_BOOL(attr, attr_name, target)\
else if (strcmp(attr->name(), attr_name) == 0) {\
	target = atoi(attr->value()) == 1;\
}

#include "../include/libpng/include/png.h"

#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpng16d.lib")
#else
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")
#endif


class PngReader {
public:
	static void Read(fs::path file_path, std::vector<uint8_t>& data)
	{
		uint32_t width;
		uint32_t height;
		png_byte color_type;
		png_byte bit_depth;
		return Read(file_path, width, height, color_type, bit_depth, data);
	}

	static void Read(fs::path file_path, uint32_t& width, uint32_t& height, std::vector<uint8_t>& data)
	{
		png_byte color_type;
		png_byte bit_depth;
		return Read(file_path, width, height, color_type, bit_depth, data);
	}

	static void Read(fs::path file_path, uint32_t& width, uint32_t& height, png_byte& color_type, png_byte& bit_depth, std::vector<uint8_t>& data)
	{
		char header[8];

		FILE* fp;
		fopen_s(&fp, file_path.string().c_str(), "rb");
		fread(header, 1, 8, fp);
		auto png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		auto info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));

		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);
		png_read_info(png_ptr, info_ptr);

		width = png_get_image_width(png_ptr, info_ptr);
		height = png_get_image_height(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		auto number_of_passes = png_set_interlace_handling(png_ptr);
		png_read_update_info(png_ptr, info_ptr);

		setjmp(png_jmpbuf(png_ptr));

		auto row_size = png_get_rowbytes(png_ptr, info_ptr);
		data.resize(width * height);

		std::vector<uint8_t> row_bytes;
		row_bytes.resize(row_size);
		for (uint32_t y = 0; y < height; y++) {
			png_read_row(png_ptr, row_bytes.data(), nullptr);
			if (color_type == PNG_COLOR_TYPE_GRAY) {
				for (uint32_t i = 0; i < width; i++) {
					data[y * width + i] = row_bytes[i];
				}
			}
			else if (color_type == PNG_COLOR_TYPE_RGB) {
				for (uint32_t i = 0; i < width * 3; i += 3) {
					data[y * width + i / 3] = row_bytes[i];
				}
			}
			else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
				for (uint32_t i = 0; i < width * 4; i += 4) {
					data[y * width + i / 4] = row_bytes[i + 3] == 0 ? 0 : row_bytes[i];
				}
			}
		}
		fclose(fp);
	}
};


# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
# ifndef VC_EXTRALEAN
#   define VC_EXTRALEAN
# endif
# ifndef NOMINMAX
#   define NOMINMAX
# endif
#include <windows.h>

namespace winext
{
	template <std::size_t N = MAX_PATH>
	inline std::string GetModuleFileName() {
		char buffer[N];
		::GetModuleFileName(nullptr, buffer, N);
		return buffer;
	}
}

inline fs::path ToAbsolutePath(const char* src_path)
{
	if (fs::path(src_path).is_relative()) {
		return fs::path(winext::GetModuleFileName()).parent_path().append(src_path);
	}
	else {
		return src_path;
	}
}