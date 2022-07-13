#include <iostream>
#include <fstream>

#include <unistd.h>

#include "totext.h"

void help_message(const char* exec_path)
{
	std::cout << "usage: " << exec_path << " [args] /path/to/image\n\n";
	std::cout << "args:\n";
	std::cout << "	-c		color replace config (color=text format, separated by new lines)\n";
	std::cout << "	-C		path to color replace config (color=text format, separated by new lines)\n";
	std::cout << "	-o		output path (default ./image_name.txt)\n";
	std::cout << "\n\ncolor replace example:\n";
	std::cout << "	255,255,255=white\n";
	std::cout << "	{255, 255, 255}=white";
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	std::string argument_colors = "";
	std::string argument_colors_path = "";
	std::string argument_output_path = "";

	if(argc==1)
	{
		help_message(argv[0]);
		return 4;
	}

	while(true)
	{
		switch(getopt(argc, argv, "c:C:o:h:"))
		{
			case 'c':
				argument_colors = std::string(optarg);
				continue;

			case 'C':
				argument_colors_path = std::string(optarg);
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
		save_path = image_path.stem().string() + ".txt";


	using namespace totext;

	const replace_pairs pairs = argument_colors_path=="" ?
		parser::parse_pairs(argument_colors)
		: parser::parse_pairs(std::filesystem::path(argument_colors_path));

	yconv::image img{image_path};
	img.bpp_resize(3);
	const std::string out_string = converter::convert(img, pairs);

	std::ofstream out_text(save_path);
	out_text << out_string;

	return 0;
}