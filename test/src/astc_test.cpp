#include <catch2/catch.hpp>

#include <atk/astc.h>
#include <atk/magick.h>

TEST_CASE("can-encode-png")
{
	// Encode png to astc
	auto encoded_astc = atk::Astc::encode_from("png/map.png");
	encoded_astc.store("astc/map.astc");
	auto astc = atk::Astc{"astc/map.astc"};

	REQUIRE(encoded_astc.get_xblocks() == astc.get_xblocks());
	REQUIRE(encoded_astc.get_yblocks() == astc.get_yblocks());
	REQUIRE(encoded_astc.get_zblocks() == astc.get_zblocks());

	REQUIRE(encoded_astc.get_size() == astc.get_size());
}

TEST_CASE("can-load-astc")
{
	auto astc  = atk::Astc{"astc/lenna.astc"};
	auto image = atk::MagickImage{"png/lenna.png"};

	REQUIRE(astc.get_width() == image.get_width());
	REQUIRE(astc.get_height() == image.get_height());
	REQUIRE(astc.get_depth() == image.get_depth());
}

TEST_CASE("npot")
{
	auto original_png = atk::MagickImage{"png/lenna-npot.png"};
	// Convert to raw data
	original_png.convert(atk::Format::RGBA);
	auto astc        = atk::Astc::encode_from("png/lenna-npot.png");
	auto decoded_png = astc.decode();
	REQUIRE(original_png.diff(decoded_png) < 0.5f);
}
