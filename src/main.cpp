#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "atk/astc.h"
#include "atk/ktx.h"
#include "atk/magick.h"
#include "atk/texture.h"
#include "atk/util.h"

namespace atk
{
class Config
{
  public:
	Config(const int argc, const char **argv);

	/// Whether to generate mipmaps
	bool mipmaps = false;

	/// Whether to convert
	bool convert = false;

	/// Target format to convert
	std::string target_format = {};

	/// Input image path
	std::string input_image = {};

  private:
	bool is_option(const std::string &arg);
};

bool Config::is_option(const std::string &arg)
{
	return arg.at(0) == '-';
}

Config::Config(const int argc, const char **argv)
{
	std::vector<std::string> args{argv, argv + argc};

	for (size_t i = 0; i < argc; ++i)
	{
		auto &arg = args[i];

		if (is_option(arg))
		{
			const auto option = arg.substr(1);

			// Mipmap option
			if (option == "mipmaps")
			{
				mipmaps = true;
			}

			// Compress
			if (option == "c")
			{
				convert = true;
				// Consume next argument
				target_format = args[++i];
			}
		}
		else        // it is not an option
		{
			input_image = arg;
		}
	}
}

}        // namespace atk

int main(const int argc, const char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: to-ktx [-mipmaps] [-c astc] texture.png\n";
		return EXIT_FAILURE;
	}

	const atk::Config config{argc, argv};

	const std::string           image_path{config.input_image};
	std::unique_ptr<atk::Image> image{new atk::MagickImage{image_path}};
	atk::Texture                texture{std::move(image)};

	if (config.mipmaps)
	{
		std::cout << "Generating mipmaps" << std::endl;
		texture.generate_mipmap_chain();
	}

	if (config.convert)
	{
		if (config.target_format == "astc")
		{
			std::cout << "Converting to astc" << std::endl;
			texture.convert(atk::Format::ASTC);
		}
		else
		{
			std::cerr << "[ERROR] Format not supported: " << config.target_format << std::endl;
		}
	}

	try
	{
		atk::Ktx ktx{texture};

		auto ktx_name = atk::get_basename_no_extension(image_path) + ".ktx";
		ktx.save_to_file(ktx_name);
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
