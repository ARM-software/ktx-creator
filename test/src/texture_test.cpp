#include <catch2/catch.hpp>

#include <atk/astc.h>
#include <atk/magick.h>
#include <atk/texture.h>

TEST_CASE("can-create-a-png-texture")
{
	// Load png file
	auto png_path = "png/lenna.png";
	std::cout << "Loading file [" << png_path << "]" << std::endl;
	auto png     = std::unique_ptr<atk::Image>{new atk::MagickImage{png_path}};
	auto texture = atk::Texture{std::move(png)};

	SECTION("can-generate-mipmaps")
	{
		std::cout << "Generating mipmaps" << std::endl;
		texture.generate_mipmap_chain();

		auto expected_width  = texture->get_width();
		auto expected_height = texture->get_height();

		auto max_dim     = std::max<uint32_t>(texture->get_width(), texture->get_height());
		auto level_count = std::floor(std::log2(max_dim));

		REQUIRE(texture.get_mipmap_chain().size() == level_count);

		for (auto &mipmap : texture.get_mipmap_chain())
		{
			expected_width  = std::max<uint32_t>(expected_width / 2, 1);
			expected_height = std::max<uint32_t>(expected_height / 2, 1);

			REQUIRE(mipmap->get_width() == expected_width);
			REQUIRE(mipmap->get_height() == expected_height);
		}

		REQUIRE(expected_width == 1);
		REQUIRE(expected_height == 1);
	}

	SECTION("can-convert-to-astc")
	{
		std::cout << "Converting png to astc" << std::endl;
		texture.convert(atk::Format::ASTC);
		REQUIRE(dynamic_cast<const atk::Astc *>(&*texture));

		for (auto &mipmap : texture.get_mipmap_chain())
		{
			REQUIRE(dynamic_cast<const atk::Astc *>(&*mipmap));
		}
	}
}
