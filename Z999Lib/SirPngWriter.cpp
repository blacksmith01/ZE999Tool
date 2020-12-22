#include "SirPngWriter.hpp"

void SirPngWriter::Write(const SirFont& sir, int data_idx, fs::path file_path)
{
	uint8_t data_width = 0;
	uint8_t data_height = 0;
	for (auto& n : sir.nodes) {
		data_width = std::max(n->wsize[data_idx], data_width);
		data_height = std::max(n->hsize[data_idx], data_height);
	}

	auto nsize = sir.nodes.size();
	auto wcount = (int)sqrt(nsize); wcount = wcount % 2 == 1 ? wcount + 1 : wcount;
	auto hcount = ((nsize / wcount) + (nsize % wcount > 0 ? 1 : 0));
	auto png_width = data_width * wcount;
	auto png_height = data_height * hcount;

	std::vector<png_byte> png_buffer;
	png_buffer.resize(png_width * png_height, 0);

	for (int i = 0; i < nsize; i++) {
		auto& n = sir.nodes[i];
		auto iw = (i % wcount);
		auto ih = (i / wcount);
		for (int j = 0; j < n->data[data_idx].size(); j++) {
			auto jw = (j % n->wsize[data_idx]);
			auto jh = (j / n->wsize[data_idx]);
			auto wtotal = (iw * data_width + jw);
			auto htotal = (ih * data_height + jh) * png_width;
			auto p = &png_buffer[(wtotal + htotal)];
			*(p + 0) = n->data[data_idx][j];
		}
	}
	std::vector<png_bytep> png_rows;
	png_rows.resize(png_height);
	for (int i = 0; i < png_height; i++) {
		png_rows[i] = &png_buffer[i * png_width];
	}

	FILE* fp;
	fopen_s(&fp, file_path.string().c_str(), "wb");
	auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	auto info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, png_width, png_height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, png_rows.data());
	png_write_end(png_ptr, nullptr);
	fclose(fp);
}