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

#pragma once

#include <memory>
#include <vector>

#include <Magick++.h>

#include "atk/image.h"

namespace atk
{
class Texture
{
  public:
	/// @brief Creates a texture containing a Magick Image
	Texture(std::unique_ptr<Image> &&i);

	Image &get_image();

	/// @brief Generates the mipmap chain for the image
	void generate_mipmap_chain();

	/// @brief Converts the image and its mipmaps
	/// @param[in] format Conversion format
	void convert(const Format format);

	/// @return The number of mipmap levels
	size_t get_levels() const
	{
		return mipmap_chain.size() + 1;
	}

	/// @return The mipmap chain vector
	std::vector<std::unique_ptr<Image>> &get_mipmap_chain()
	{
		return mipmap_chain;
	}

	const Image &operator*() const
	{
		return *image;
	}

	const Image *operator->() const
	{
		return image.get();
	}

  private:
	std::unique_ptr<Image> image = nullptr;

	std::vector<std::unique_ptr<Image>> mipmap_chain;
};

}        // namespace atk
