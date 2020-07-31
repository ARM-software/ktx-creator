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

#include <iostream>
#include <vector>

#include <astc_codec_internals.h>

void encode_astc_image(const astc_codec_image * input_image,
					   astc_codec_image * output_image,
					   int xdim,
					   int ydim,
					   int zdim,
					   const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, swizzlepattern swz_decode, uint8_t * buffer, int pack_and_unpack, int threadcount);

void store_astc_file(const astc_codec_image * input_image,
					 const char *filename, int xdim, int ydim, int zdim, const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, int threadcount);

#include "atk/image.h"

namespace atk
{
/// Number of bytes for each dimension
struct BlockDim
{
	BlockDim(uint8_t x = 0, uint8_t y = 0, uint8_t z = 0) :
	    x{x},
	    y{y},
	    z{z}
	{}

	uint8_t x;
	uint8_t y;
	uint8_t z;
};

/// ASTC header declaration
struct AstcHeader
{
	uint8_t  magic[4];
	BlockDim blockdim;
	uint8_t  xsize[3];        // x-size = xsize[0] + xsize[1] + xsize[2]
	uint8_t  ysize[3];        // x-size, y-size and z-size are given in texels
	uint8_t  zsize[3];        // block count is inferred
};

class Astc : public Image
{
  public:
	/// @brief Encodes an image to astc
	/// @param[in] file_path Image file path
	/// @return A new Astc image
	static Astc encode_from(const std::string &file_path);

	/// @brief Encodes an image to astc
	/// @param[in] image Image to encode
	/// @return A new Astc image
	static Astc encode_from(Image &image);

	/// @brief Define and retrieve compressed texture image
	/// @param[in] file_path Astc image file path
	Astc(const std::string &file_path);

	/// @brief Create astc from raw data without header
	Astc(uint32_t width, uint32_t height, uint32_t depth, const uint8_t *mem);

	/// @brief Create astc from raw data with header
	Astc(const uint8_t *data);

	Astc(Astc &&);

	~Astc() override;

	Image decode() const;

	void store(const std::string &path) const;

	std::unique_ptr<Image> resize(const uint32_t w, const uint32_t h) override;

	const uint8_t *get_data() const override;

	uint32_t get_xblocks() const;

	uint32_t get_yblocks() const;

	uint32_t get_zblocks() const;

	uint32_t get_gl_format() override
	{
		return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
	}

  private:
	Astc() = default;

	/// @brief Used to encode raw data during construction
	void encode();

	astc_decode_mode decode_mode = DECODE_LDR_SRGB;

	swizzlepattern swizzle = {0, 1, 2, 3};

	BlockDim block_dim = {8, 8, 1};

	error_weighting_params ewp;

	astc_codec_image *codec_image = nullptr;
};

std::ostream &operator<<(std::ostream &os, const Astc &astc);

}        // namespace atk
