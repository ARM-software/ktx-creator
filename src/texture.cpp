#include "atk/texture.h"

#include <algorithm>

#include "atk/astc.h"

namespace atk
{
Texture::Texture(std::unique_ptr<Image> &&i) :
    image{std::move(i)}
{
}

Image &Texture::get_image()
{
	assert(image && "Texture has no image");
	return *image;
}

void Texture::generate_mipmap_chain()
{
	assert(image && "Texture has no image");

	if (!mipmap_chain.empty())
	{
		return;        // already generated
	}

	auto next_width  = image->get_width();
	auto next_height = image->get_height();

	// Last mipmap should be 1x1
	while (next_width != 1 || next_height != 1)
	{
		next_width  = std::max<size_t>(next_width / 2, 1);
		next_height = std::max<size_t>(next_height / 2, 1);

		auto mipmap = image->resize(next_width, next_height);

		mipmap_chain.emplace_back(std::move(mipmap));
	}
}

void Texture::convert(const Format format)
{
	switch (format)
	{
		default:
		{
			// Not supported
			throw std::runtime_error{"Unsupported conversion format"};
			break;
		}
		case Format::ASTC:
		{
			auto to_astc = [](std::unique_ptr<Image> &img) {
				// Make sure it is RGBA8
				img->convert(Format::RGBA);
				// Substitute image with a converted one
				img.reset(new Astc{Astc::encode_from(*img)});
			};

			to_astc(image);

			std::for_each(std::begin(mipmap_chain), std::end(mipmap_chain), to_astc);
		}
	}
}
}        // namespace atk