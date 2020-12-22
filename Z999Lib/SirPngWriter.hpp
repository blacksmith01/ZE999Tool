#pragma once

#include "Sir.hpp"

class SirPngWriter {
public:
	static void Write(const SirFont& sir, int data_idx, fs::path file_path);
};