#include <iostream>
#include <filesystem>

#include <unistd.h>

#include "dither.h"


void help_message(const char* exec_path)
{
	std::cout << "usage: " << exec_path << " [args] /path/to/image\n\n";
	std::cout << "args:\n";
	std::cout << "	-c		comma separated list of RGB colors\n";
	std::cout << "	-C		path to a comma separated list of RGB colors\n";
	std::cout << "	-x		desired width (default same)\n";
	std::cout << "	-y		desired height (default same)\n";
	std::cout << "	-t		desired total amount of pixels (incompatable with -w and -h options) (default same)\n";
	std::cout << "	-d		distance function (default LAB)\n";
	std::cout << "	-D		dithering function (default jarvis)\n";
	std::cout << "	-o		output path (default ./image_name.png)\n";
	std::cout << "\n\ndistance functions:\n";
	std::cout << "	RGB, LAB, XYZ";
	std::cout << "\n\ndithering functions:\n";
	std::cout << "	floyd_steinberg, atkinson, jarvis, ordered\n";
	std::cout << "\n\ncolors list example:\n";
	std::cout << "	255, 255, 255, 0, 0, 0, 255, 0, 0, 127, 127, 0\n";
	std::cout << "	{255, 255, 255}, {0, 0, 0}, {255, 0, 0}, {127, 127, 0}";
	std::cout << std::endl;
}

struct dither_args
{
	std::string width = "";
	std::string height = "";
	std::string total = "";
	std::string dither_type = "";
	std::string save_path = "";
};

template<typename T>
void dither_generic(T& d, const dither_args a)
{
	using namespace dither;

	if(a.width!="" || a.height!="")
	{
		const unsigned d_width = a.width=="" ? d.width() : std::stoi(a.width);
		const unsigned d_height = a.height=="" ? d.height() : std::stoi(a.height);

		d.resize(d_width, d_height);
	} else if(a.total!="")
	{
		d.resize_total(std::stoi(a.total));
	}

	const yconv::image img = d.dither(ditherer_base::parse_type(a.dither_type));

	img.save(a.save_path+std::string(".png"));
}

int main(int argc, char* argv[])
{
    std::string argument_colors = "";
	std::string argument_colors_path = "";
	std::string argument_width = "";
	std::string argument_height = "";
	std::string argument_total = "";
	std::string argument_compare_func = "LAB";
	std::string argument_dithering_func = "jarvis";
	std::string argument_output_path = "";

    if(argc==1)
	{
		help_message(argv[0]);
		return 4;
	}

	while(true)
	{
		switch(getopt(argc, argv, "c:C:x:y:ht:d:D:o:"))
		{
			case 'c':
				argument_colors = std::string(optarg);
				continue;

			case 'C':
				argument_colors_path = std::string(optarg);
				continue;

			case 'x':
				argument_width = std::string(optarg);
				continue;

			case 'y':
				argument_height = std::string(optarg);
				continue;

			case 't':
				argument_total = std::string(optarg);
				continue;

			case 'd':
				argument_compare_func = std::string(optarg);
				continue;

			case 'D':
				argument_dithering_func = std::string(optarg);
				continue;

			case 'o':
				argument_output_path = std::string(optarg);
				continue;

			case 'h':
				help_message(argv[0]);
				return 3;

			case -1:
				break;
		}
		break;
	}

	if(argument_colors=="" && argument_colors_path=="")
	{
		std::cout << "-c or -C options are mandatory!!" << std::endl;
		help_message(argv[0]);
		return 2;
	}

	if((argument_width!="" || argument_height!="") && argument_total!="")
	{
		std::cout << "cant use -w/-h options together with -t!!!!" << std::endl;
		help_message(argv[0]);
		return 1;
	}

	if(optind >= argc)
	{
		std::cout << "path to image not given!!" << std::endl;
		help_message(argv[0]);
		return 1;
	}

	const std::filesystem::path image_path{argv[optind]};

	std::string save_path;
	if(argument_output_path!="")
		save_path = argument_output_path;
	else
		save_path = image_path.stem().string();

	const dither_args d_args
		{argument_width, argument_height, argument_total, argument_dithering_func, save_path};


	using namespace dither;

	const colors_base dither_colors = argument_colors_path=="" ?
		parser::parse_colors(argument_colors)
		: parser::parse_colors(std::filesystem::path(argument_colors_path));

	const yconv::image img{image_path};
	if(argument_compare_func=="RGB")
	{
		ditherer<color<int>> c_dither(img, dither_colors);
		dither_generic(c_dither, d_args);
	} else if(argument_compare_func=="LAB")
	{
		ditherer<color_lab> c_dither(img, dither_colors);
		dither_generic(c_dither, d_args);
	} else if(argument_compare_func=="XYZ")
	{
		ditherer<color_xyz> c_dither(img, dither_colors);
		dither_generic(c_dither, d_args);
	} else
	{
		std::cout << "invalid distance function!!!!" << std::endl;
		help_message(argv[0]);
		return 3;
	}

    return 0;
}