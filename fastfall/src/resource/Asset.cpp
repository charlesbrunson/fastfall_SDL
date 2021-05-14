#include "fastfall/resource/Asset.hpp"

#include <fstream>
#include <memory>

//ASSET STREAM

/*
sf::Int64 AssetStream::read(void* data, sf::Int64 size)
{
	source->read(static_cast<char*>(data), size);
	return source->gcount();
};
sf::Int64 AssetStream::seek(sf::Int64 position)
{
	source->seekg(start + position, source->beg);

	return source->gcount();
};
sf::Int64 AssetStream::tell()
{
	return source->tellg() - start;
};
sf::Int64 AssetStream::getSize()
{
	return end - start;
};
*/

//ASSET

namespace ff {

Asset::Asset(const std::string& filename) :
    assetName(filename),
    ImGuiContent(ImGuiContentType::NONE, filename)
{

};

std::vector<int8_t> readFile(const char* filename) {

    // open the file:
    std::ifstream file(filename, std::ios::binary);

    assert(file.is_open());

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<int8_t> vec;
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
        std::istream_iterator<int8_t>(file),
        std::istream_iterator<int8_t>());

    return vec;
}

}