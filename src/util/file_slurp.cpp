#include "ff/util/file_slurp.hpp"

#include <fstream>

std::unique_ptr<char[]> ff::file_slurp(std::filesystem::path path) {
	std::ifstream ndxStream = std::ifstream(path, std::ios::binary | std::ios::ate);
	std::unique_ptr<char[]> file_content;

	if (ndxStream.is_open()) {

		//parse index file
		std::streamsize len = ndxStream.tellg();
		ndxStream.seekg(0, std::ios::beg);

		file_content = std::make_unique<char[]>(len + 1);

		ndxStream.read(file_content.get(), len);
		file_content[len] = '\0';
		ndxStream.close();
	}
	return file_content;
}