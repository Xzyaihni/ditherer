#include <filesystem>
#include <fstream>
#include <sstream>

#include "generic.h"


std::string generic::parse_file(const std::filesystem::path path)
{
	if(!std::filesystem::exists(path))
		throw std::runtime_error(std::string("file doesnt exist: ")+path.string());

	std::ifstream parse_file(path);
	std::stringstream out_stream;

	out_stream << parse_file.rdbuf();

	return out_stream.str();
}