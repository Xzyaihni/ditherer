#include <iostream>
#include <fstream>
#include <sstream>

#include "generic.h"
#include "dither.h"


using namespace dither;
using namespace yconv;


color_xyz::color_xyz()
{
}

color_xyz::color_xyz(const float X, const float Y, const float Z)
: X(X), Y(Y), Z(Z)
{
}

color_xyz::color_xyz(const color<float>& c)
{
	const color<float> c_f{c.r/255.0f, c.g/255.0f, c.b/255.0f};

	const auto linearize = [](const float n){return std::pow((n+0.055f)/1.055f, 2.4f);};

	const float r = linearize(c_f.r) * 100;
	const float g = linearize(c_f.g) * 100;
	const float b = linearize(c_f.b) * 100;

	X = 0.4124564f*r + 0.3575761f*g + 0.1804375f*b;
	Y = 0.2126729f*r + 0.7151522f*g + 0.0721750f*b;
	Z = 0.0193339f*r + 0.1191920f*g + 0.9503041f*b;
}

int color_xyz::distance(const color_xyz& rhs) const noexcept
{
	return std::abs(X-rhs.X)
	+ std::abs(Y-rhs.Y)
	+ std::abs(Z-rhs.Z);
}

color_lab::color_lab()
{
}

color_lab::color_lab(const float L, const float a, const float b)
: L(L), a(a), b(b)
{
}

color_lab::color_lab(const color<float>& c)
{
	const color_xyz c_xyz{c};

	const auto cvt_func = [](const float num)
	{
		const float d = 6/29.0f;
		return num>(d*d*d) ? std::cbrt(num) : num/(3*d*d)+4/29.0f;
	};

	const float i_X = 95.0489f;
	const float i_Y = 100;
	const float i_Z = 108.884f;

	const float Y_cvt = cvt_func(c_xyz.Y/i_Y);
	L = 116*Y_cvt-16;
	a = 500*(cvt_func(c_xyz.X/i_X)-Y_cvt);
	b = 200*(Y_cvt-cvt_func(c_xyz.Z/i_Z));
}

namespace dither
{
	std::ostream& operator<<(std::ostream& out, const color_lab& rhs)
	{
		out << "[L: " << rhs.L;
		out << ", a: " << rhs.a;
		out << ", b: " << rhs.b << "]";

		return out;
	}
}

float color_lab::distance(const color_lab& rhs) const noexcept
{
	return std::abs(L-rhs.L)
	+ std::abs(a-rhs.a)
	+ std::abs(b-rhs.b);
}

colors_base parser::parse_colors(std::string colors) noexcept
{
	colors += ',';

	std::vector<uint8_t> colors_vec;

	std::string last_number = "";
	for(const char& c : colors)
	{
		switch(c)
		{
			case ',':
				colors_vec.push_back(std::stoi(last_number));
				last_number = "";
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
				last_number += c;
				break;

			default:
				break;
		}
	}

	const int colors_amount = colors_vec.size();

	colors_base parsed_colors;
	parsed_colors.reserve(colors_amount/3);

	for(int i = 0; i < colors_amount; i+=3)
		parsed_colors.emplace_back(colors_vec[i], colors_vec[i+1], colors_vec[i+2]);

	return parsed_colors;
}

colors_base parser::parse_colors(const std::filesystem::path path) noexcept
{
	return parse_colors(generic::parse_file(path));
}

ditherer_base::ditherer_base()
{
}

ditherer_base::ditherer_base(const yconv::image img)
: _image(img)
{
}

ditherer_base::dither_type ditherer_base::parse_type(const std::string str)
{
	if(str=="floyd_steinberg")
	{
		return dither_type::floyd_steinberg;
	} else if(str=="atkinson")
	{
		return dither_type::atkinson;
	} else if(str=="jarvis")
	{
		return dither_type::jarvis;
	} else if(str=="ordered")
	{
		return dither_type::ordered;
	} else
	{
		throw std::runtime_error(std::string("unknown dither type: ") + str);
	}
}

void ditherer_base::resize_total(const unsigned total)
{
	const float scale = std::sqrt(static_cast<float>(total)/(_image.width*_image.height));
	resize_scale(scale, scale);
}

void ditherer_base::resize_scale(const float scale_width, const float scale_height)
{
	resize(_image.width*scale_width, _image.height*scale_height);
}

void ditherer_base::resize(const unsigned width, const unsigned height)
{
	_image.resize(width, height, image::resize_type::area_sample);
}

unsigned ditherer_base::width() const noexcept
{
	return _image.width;
}

unsigned ditherer_base::height() const noexcept
{
	return _image.height;
}