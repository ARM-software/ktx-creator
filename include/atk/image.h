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

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

#include <Magick++.h>
#include <ktx.h>

#include <gl_format.h>

namespace atk
{
/// @brief Supported format
enum class Format
{
	PNG,
	ASTC,
	RGB,
	RGBA
};

/// @brief Image interface
class Image
{
  public:
	Image() = default;
	Image(uint8_t *dt, size_t s, uint32_t w, uint32_t h, uint32_t d = 1) :
	    data{dt},
	    size{s},
	    width{w},
	    height{h},
	    depth{d}
	{}

	Image(Image &&o) :
	    data{o.data},
	    size{o.size},
	    width{o.width},
	    height{o.height},
	    depth{o.depth}
	{
		o.data = nullptr;
	}

	virtual ~Image()
	{
		if (data)
		{
			delete[] data;
		}
	}

	/// @brief Converts image format
	/// @param format Format to apply
	virtual void convert(Format format)
	{
		throw std::runtime_error{"Unimplemented"};
	}

	virtual void store(const std::string &path)
	{}

	/// @brief Creates a copy of the image resized according to the passed arguments
	/// @param[in] w Width of the resized image
	/// @param[in] h Height of the resized image
	/// @return A copy of the original image with new size
	virtual std::unique_ptr<Image> resize(uint32_t w, uint32_t h)
	{
		throw std::runtime_error{"Unimplemented"};
	}

	virtual const uint8_t *get_data() const
	{
		return data;
	}

	size_t get_size() const
	{
		return size;
	}

	uint32_t get_width() const
	{
		return width;
	}

	uint32_t get_height() const
	{
		return height;
	}

	uint32_t get_depth() const
	{
		return depth;
	}

	/// @brief Diff with another image
	/// @param[in] b Other image to compare with
	/// @return [0, 1] where 0 means they are the same image
	float diff(const Image &b) const;

	/// @return The GL format used by the KTX header
	virtual uint32_t get_gl_format()
	{
		return GL_RGB;
	}

  protected:
	const uint8_t *get_memory() const
	{
		return data;
	}

	void set_memory(const uint8_t *d)
	{
		assert(data == nullptr && "Image data was already set");
		data = d;
	}

	void set_size(size_t s)
	{
		size = s;
	}

	void set_width(uint32_t w)
	{
		width = w;
	}

	void set_height(uint32_t h)
	{
		height = h;
	}

	void set_depth(uint32_t d)
	{
		depth = d;
	}

  private:
	/// Image memory data
	const uint8_t *data = nullptr;

	size_t size = 0;

	uint32_t width  = 0;
	uint32_t height = 0;
	uint32_t depth  = 1;
};

}        // namespace atk
