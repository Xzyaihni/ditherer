#include <iostream>

#include "generic.h"
#include <vector>

#include "totext.h"


using namespace totext;

bool color::operator<(const color& other) const
{
	return std::tie(r, g, b) < std::tie(other.r, other.g, other.b);
}

replace_pairs parser::parse_pairs(std::string pairs) noexcept
{
	pairs += '\n';

	bool parsing_text = false;

	std::string color_string = "";
	std::vector<uint8_t> color_strings;

	color c_color;
	std::string c_text = "";

	replace_pairs out_pairs;

	for(const char c : pairs)
	{
		switch(c)
		{
			case '\n':
				if(parsing_text)
				{
					parsing_text = false;

					out_pairs.insert({c_color, c_text});

					c_text = "";
				}
			break;

			case '=':
				if(!parsing_text)
				{
					parsing_text = true;

					color_strings.push_back(std::stoi(color_string));
					color_string = "";

					c_color.b = color_strings.back();
					color_strings.pop_back();
					c_color.g = color_strings.back();
					color_strings.pop_back();
					c_color.r = color_strings.back();
					color_strings.pop_back();

					color_strings.clear();
				} else
				{
					c_text += c;
				}
			break;

			case ',':
				if(!parsing_text)
				{
					color_strings.push_back(std::stoi(color_string));
					color_string = "";
				} else
				{
					c_text += c;
				}
			break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if(!parsing_text)
				{
					color_string += c;
				} else
				{
					c_text += c;
				}
			break;

			default:
				if(parsing_text)
				{
					c_text += c;
				}
			break;
		}
	}

	return out_pairs;
}

replace_pairs parser::parse_pairs(const std::filesystem::path pairs_path) noexcept
{
	return parse_pairs(generic::parse_file(pairs_path));
}

std::string converter::convert(const yconv::image image, const replace_pairs pairs)
{
	if(image.bpp!=3)
		throw std::runtime_error("image has more/less than 3 colors per pixel");

	std::string converted_text = "";

	for(int i = 0; i < image.data.size(); i+=image.bpp)
	{
		const uint8_t r = image.data[i];
		const uint8_t g = image.data[i+1];
		const uint8_t b = image.data[i+2];

		if(i!=0 && i%(image.width*image.bpp)==0)
		{
			converted_text += '\n';
		}
		converted_text += pairs.at(color{r, g, b});
	}

	return converted_text;
}