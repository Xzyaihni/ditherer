#ifndef YAN_GENERIC_H
#define YAN_GENERIC_H


#include <filesystem>

namespace generic
{
	std::string parse_file(const std::filesystem::path path);
};

#endif