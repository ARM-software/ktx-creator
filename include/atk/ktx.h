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

#include <ktx.h>

#include <stdexcept>
#include <string>

#include "atk/texture.h"

namespace Magick
{
class Image;
}

namespace atk
{
class Ktx
{
  public:
	class Exception : public std::runtime_error
	{
	  public:
		Exception(KTX_error_code e, const std::string &m = {});

		static std::string get_message(KTX_error_code e);
	};

	/// @brief Loads a ktx file
	/// @param[in] file_path Path to ktx file
	Ktx(const std::string &file_path);

	Ktx(Image &image);

	Ktx(Texture &texture);

	Ktx(Ktx &&other);

	~Ktx();

	std::unique_ptr<Image> get_image();

	size_t get_level_count() const;

	size_t get_width() const;
	size_t get_height() const;
	size_t get_depth() const;

	void save_to_file(const std::string &file_name) const;

  private:
	ktxTexture *ktx_texture = nullptr;
};

}        // namespace atk