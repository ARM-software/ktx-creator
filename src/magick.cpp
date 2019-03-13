/* Copyright (c) 2019, ARM Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <atk/magick.h>

namespace atk
{
MagickImage::MagickImage(const std::string &path) :
    MagickImage{Magick::Image{path}}
{
}

MagickImage::MagickImage(Magick::Image &&i) :
    image{std::move(i)}
{
	auto geometry = image.size();
	set_width(geometry.width());
	set_height(geometry.height());

	update_data();
}

Magick::Image &MagickImage::get_image()
{
	return image;
}

void MagickImage::update_data()
{
	image.write(&blob);
	set_size(blob.length());
}

/// @brief Converts image format
/// @param format Format to apply
void MagickImage::convert(Format format)
{
	// Ensure image has a colorspace
	assert(image.colorSpace() != Magick::ColorspaceType::UndefinedColorspace);

	switch (format)
	{
		case Format::RGB:
		{
			// Convert to raw RGB
			image.depth(8);
			image.magick("RGB");
			auto channels = image.channels();
			assert(channels == 3);
			break;
		}
		case Format::RGBA:
		{
			// Convert to raw RGBA
			image.alphaChannel(Magick::ActivateAlphaChannel);
			image.depth(8);
			image.magick("RGBA");
			auto channels = image.channels();
			assert(channels == 4);
			break;
		}
		default:
		{
			throw std::runtime_error{"Format not supported"};
		}
	}

	update_data();
}

bool has_alpha(const Magick::ImageType type)
{
	return type == Magick::ImageType::TrueColorAlphaType ||
	       type == Magick::ImageType::PaletteAlphaType ||
	       type == Magick::ImageType::GrayscaleAlphaType ||
	       type == Magick::ImageType::ColorSeparationAlphaType ||
	       type == Magick::ImageType::PaletteBilevelAlphaType;
}

uint32_t MagickImage::get_gl_format()
{
	auto type = image.type();

	switch (image.colorSpace())
	{
		case Magick::ColorspaceType::sRGBColorspace:
			if (has_alpha(type))
			{
				convert(Format::RGBA);
				return GL_SRGB8_ALPHA8;
			}
			else
			{
				convert(Format::RGB);
				return GL_SRGB8;
			}
			break;
		case Magick::ColorspaceType::RGBColorspace:
			if (has_alpha(type))
			{
				convert(Format::RGBA);
				return GL_RGBA;
			}
			else
			{
				convert(Format::RGB);
				return GL_RGB;
			}
		default:
			break;
	}

	throw std::runtime_error("Image format not supported");
}

std::unique_ptr<Image> MagickImage::resize(const uint32_t w, const uint32_t h)
{
	// Copy image
	auto resized_image = image;

	auto geometry         = image.size();
	auto resized_geometry = Magick::Geometry{w, h};
	// Do not preserve aspect ratio
	resized_geometry.aspect(true);
	resized_image.resize(resized_geometry);

	auto ret_image = new MagickImage{std::move(resized_image)};
	return std::unique_ptr<Image>{ret_image};
}

const uint8_t *MagickImage::get_data() const
{
	return reinterpret_cast<const uint8_t *>(blob.data());
}

void MagickImage::store(const std::string &path)
{
	image.write(path);
}

}        // namespace atk