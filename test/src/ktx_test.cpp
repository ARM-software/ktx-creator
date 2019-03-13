#include <catch2/catch.hpp>

#include <atk/astc.h>
#include <atk/ktx.h>
#include <atk/magick.h>
#include <atk/texture.h>

TEST_CASE("ktx-npot")
{
	auto png_npot_path = "png/lenna-npot.png";
	auto png           = atk::MagickImage{png_npot_path};
	auto ktx           = atk::Ktx{png};

	REQUIRE(ktx.get_width() == png.get_width());
	REQUIRE(ktx.get_height() == png.get_height());

	ktx.save_to_file("png/lenna-npot.png.ktx");
}

TEST_CASE("ktx-npot-magick-mipmaps")
{
	auto png_npot_path = "png/map.png";
	auto png           = std::make_unique<atk::MagickImage>(png_npot_path);
	auto texture       = atk::Texture{std::move(png)};
	texture.generate_mipmap_chain();

	SECTION("create-ktx-from-magick")
	{
		auto ktx = atk::Ktx{texture};
		ktx.save_to_file("png/map-mipmaps.ktx");
	}
}

TEST_CASE("ktx-flow")
{
	auto png_path = "png/map.png";
	auto png      = std::make_unique<atk::MagickImage>(png_path);
	auto texture  = atk::Texture{std::move(png)};
	texture.generate_mipmap_chain();

	SECTION("create-ktx-from-png")
	{
		auto ktx = atk::Ktx{texture};
		ktx.save_to_file("ktx/map.png.ktx");
	}

	SECTION("create-ktx-from-astc")
	{
		auto astc_path = "astc/map.astc";
		auto astc      = std::unique_ptr<atk::Image>{new atk::Astc{astc_path}};
		auto texture   = atk::Texture{std::move(astc)};

		auto ktx = atk::Ktx{texture};
		ktx.save_to_file("ktx/map.astc.ktx");
	}

	SECTION("create-ktx-from-converted-png")
	{
		texture.convert(atk::Format::ASTC);
		auto ktx = atk::Ktx{texture};
		ktx.save_to_file("ktx/map.png.astc.ktx");
	}

	SECTION("load-ktx-file")
	{
		auto ktx = atk::Ktx{"ktx/map.astc.ktx"};
		REQUIRE(ktx.get_level_count() == 1);
	}

	SECTION("load-ktx-file-with-mipmaps")
	{
		auto ktx = atk::Ktx{"ktx/map.png.astc.ktx"};
		REQUIRE(ktx.get_level_count() > 1);
	}
}

TEST_CASE("decode-astc")
{
	auto ktx   = atk::Ktx{"ktx/map.png.astc.ktx"};
	auto image = ktx.get_image();
	auto astc  = dynamic_cast<atk::Astc *>(image.get());
	assert(astc);
	auto decoded = astc->decode();

	auto png = atk::MagickImage{"png/map.png"};
	png.convert(atk::Format::RGBA);
	auto diff = decoded.diff(png);
	std::cout << "Decoded diff astc [" << diff << "]" << std::endl;
	assert(diff < 0.5f);

	png.get_image().magick("PNG");
	png.store("png/map.astc.png");
}
