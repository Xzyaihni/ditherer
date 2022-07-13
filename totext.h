#ifndef YAN_TOTEXT_H
#define YAN_TOTEXT_H

#include <string>
#include <filesystem>
#include <map>

#include <yanconv.h>

namespace totext
{
	struct color
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		bool operator<(const color& other) const;
	};

	typedef std::map<color, std::string> replace_pairs;
	class parser
	{
	public:
		static replace_pairs parse_pairs(std::string pairs) noexcept;
		static replace_pairs parse_pairs(const std::filesystem::path pairs_path) noexcept;
	};

	class converter
	{
	public:
		static std::string convert(const yconv::image image, const replace_pairs pairs);
	};
};

#endif