#include <cassert>
#include <iostream>
#include <stdexcept>

#include <catch2/catch.hpp>

#include <atk/magick.h>

TEST_CASE("load-png")
{
	try
	{
		atk::MagickImage magick_image{"png/lenna.png"};
		auto &           image = magick_image.get_image();
		REQUIRE(image.colorSpace() == MagickCore::ColorspaceType::sRGBColorspace);
		REQUIRE(image.channels() == 3);
		REQUIRE(image.channelDepth(MagickCore::ChannelType::RedChannel) == 8);
		REQUIRE(image.channelDepth(MagickCore::ChannelType::GreenChannel) == 8);
		REQUIRE(image.channelDepth(MagickCore::ChannelType::BlueChannel) == 8);
		REQUIRE(image.channelDepth(MagickCore::ChannelType::AlphaChannel) == 1);

		SECTION("resize-png")
		{
			auto geometry = image.size();

			// Copy image
			Magick::Image resized_image = image;

			auto resized_geometry = Magick::Geometry{geometry.width() / 2, geometry.height() / 2};
			resized_image.resize(resized_geometry);

			resized_geometry = resized_image.size();
			REQUIRE(geometry.width() / 2 == resized_geometry.width());
			REQUIRE(geometry.height() / 2 == resized_geometry.height());

			resized_image.write("png/lenna-resized.png");
		}
	}
	catch (Magick::Exception &e)
	{
		std::cout << e.what() << std::endl;
		REQUIRE(false);
	}
}

TEST_CASE("should-not-load-non-png")
{
	try
	{
		auto image = atk::MagickImage{"astc/lenna.astc"};
		std::cerr << "[FAIL] test should throw\n";
		REQUIRE(false);
	}
	catch (const Magick::Exception &e)
	{
		std::cout << "[OK] " << e.what() << '\n';
	}
}
