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

#include <Magick++.h>

#include <atk/image.h>

namespace atk
{
class MagickImage : public Image
{
  public:
	MagickImage(const std::string &path);

	MagickImage(Magick::Image &&i);

	MagickImage(MagickImage &&image) = default;

	Magick::Image &get_image();

	std::unique_ptr<Image> resize(uint32_t w, uint32_t h) override;

	const uint8_t *get_data() const override;

	/// @brief Converts image format
	/// @param format Format to apply
	void convert(Format format) override;

	void store(const std::string &path) override;

	uint32_t get_gl_format() override;

  private:
	MagickImage() = default;

	/// @brief Helper function to update raw data stored in blob
	///        whenever the image changes configuration
	void update_data();

	Magick::Image image;

	Magick::Blob blob;
};

}        // namespace atk