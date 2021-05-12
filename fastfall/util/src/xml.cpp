#include "fastfall/util/xml.hpp"

#include <fstream>

std::unique_ptr<char[]> ff::readXML(std::string path) {

	std::ifstream ndxStream = std::ifstream(path, std::ios::binary | std::ios::ate);
	std::unique_ptr<char[]> xmlContent;

	if (ndxStream.is_open()) {

		//parse index file
		std::streamsize len = ndxStream.tellg();
		ndxStream.seekg(0, std::ios::beg);

		xmlContent = std::make_unique<char[]>(len + 1);

		ndxStream.read(xmlContent.get(), len);
		xmlContent[len] = '\0';
		ndxStream.close();
	}
	return xmlContent;
}