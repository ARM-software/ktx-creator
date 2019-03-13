/* Copyright (c) 2014-2017, ARM Limited and Contributors
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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "atk/astc.h"

namespace atk
{
Astc::~Astc()
{
	if (codec_image)
	{
		destroy_image(codec_image);
	}
}

Astc::Astc(Astc &&other) :
    Image{std::move(other)},
    decode_mode{other.decode_mode},
    swizzle{other.swizzle},
    block_dim{other.block_dim},
    ewp{other.ewp},
    codec_image{other.codec_image}
{
	other.codec_image = nullptr;
}

Astc::Astc(const uint8_t *data)
{
	// Get the header
	auto astc_data = reinterpret_cast<const AstcHeader *>(data);

	// Merge x,y,z-sizes from 3 chars into one integer value
	auto width  = astc_data->xsize[0] + (astc_data->xsize[1] << 8) + (astc_data->xsize[2] << 16);
	auto height = astc_data->ysize[0] + (astc_data->ysize[1] << 8) + (astc_data->ysize[2] << 16);
	auto depth  = astc_data->zsize[0] + (astc_data->zsize[1] << 8) + (astc_data->zsize[2] << 16);

	block_dim = astc_data->blockdim;

	set_width(width);
	set_height(height);
	set_depth(depth);

	// Each block is encoded on 16 bytes, so calculate total compressed image data size
	auto size = get_xblocks() * get_yblocks() * get_zblocks() << 4;
	set_size(size);

	set_memory(data);
}

Astc::Astc(uint32_t width, uint32_t height, uint32_t depth, const uint8_t *mem)
{
	set_width(width);
	set_height(height);
	set_depth(depth);

	auto size = get_xblocks() * get_yblocks() * get_zblocks() << 4;
	set_size(size);

	auto        data = new uint8_t[sizeof(AstcHeader) + size];
	set_memory(data);

	AstcHeader &hdr  = *reinterpret_cast<AstcHeader *>(data);

	const uint32_t MAGIC_FILE_CONSTANT = 0x5CA1AB13;
	hdr.magic[0]                       = MAGIC_FILE_CONSTANT & 0xFF;
	hdr.magic[1]                       = (MAGIC_FILE_CONSTANT >> 8) & 0xFF;
	hdr.magic[2]                       = (MAGIC_FILE_CONSTANT >> 16) & 0xFF;
	hdr.magic[3]                       = (MAGIC_FILE_CONSTANT >> 24) & 0xFF;
	hdr.blockdim.x                     = block_dim.x;
	hdr.blockdim.y                     = block_dim.y;
	hdr.blockdim.z                     = block_dim.z;
	hdr.xsize[0]                       = get_width() & 0xFF;
	hdr.xsize[1]                       = (get_width() >> 8) & 0xFF;
	hdr.xsize[2]                       = (get_width() >> 16) & 0xFF;
	hdr.ysize[0]                       = get_height() & 0xFF;
	hdr.ysize[1]                       = (get_height() >> 8) & 0xFF;
	hdr.ysize[2]                       = (get_height() >> 16) & 0xFF;
	hdr.zsize[0]                       = get_depth() & 0xFF;
	hdr.zsize[1]                       = (get_depth() >> 8) & 0xFF;
	hdr.zsize[2]                       = (get_depth() >> 16) & 0xFF;

	std::copy(mem, mem + size, data + sizeof(AstcHeader));
}

Astc::Astc(const std::string &file_path)
{
	size_t result    = 0;
	auto   file_name = file_path.c_str();

	FILE *compressed_data_file = fopen(file_name, "rb");

	if (compressed_data_file == NULL)
	{
		std::cerr << "Could not open [" << file_name << "]\n";
		exit(EXIT_FAILURE);
	}

	std::cout << "Loading file [" << file_name << "]\n";

	// Obtain file size
	fseek(compressed_data_file, 0, SEEK_END);
	auto size = ftell(compressed_data_file);
	rewind(compressed_data_file);

	// Allocate memory to contain the whole file
	auto data = new uint8_t[size]();

	// Copy the file into the buffer
	result = fread(data, 1, size, compressed_data_file);

	if (result != size)
	{
		std::cerr << "Reading error [" << file_name << "] ... FILE: " << __FILE__ << " LINE: " << __LINE__ << "\n";
		exit(EXIT_FAILURE);
	}

	// Get the header
	auto astc_data = reinterpret_cast<AstcHeader *>(data);

	// Merge x,y,z-sizes from 3 chars into one integer value
	auto width  = astc_data->xsize[0] + (astc_data->xsize[1] << 8) + (astc_data->xsize[2] << 16);
	auto height = astc_data->ysize[0] + (astc_data->ysize[1] << 8) + (astc_data->ysize[2] << 16);
	auto depth  = astc_data->zsize[0] + (astc_data->zsize[1] << 8) + (astc_data->zsize[2] << 16);

	block_dim = astc_data->blockdim;

	set_width(width);
	set_height(height);
	set_depth(depth);

	// Each block is encoded on 16 bytes, so calculate total compressed image data size
	size = get_xblocks() * get_yblocks() * get_zblocks() << 4;
	set_size(size);

	// Terminate file operations
	fclose(compressed_data_file);

	set_memory(data);
}

Image Astc::decode() const
{
	// TODO: What if it is not SRGB?
	uint32_t bitness = 8;

	if ((block_dim.x < 3 || block_dim.x > 6 || block_dim.y < 3 || block_dim.y > 6 || block_dim.z < 3 || block_dim.z > 6) &&
	    (block_dim.x < 4 || block_dim.x == 7 || block_dim.x == 9 || block_dim.x == 11 || block_dim.x > 12 ||
	     block_dim.y < 4 || block_dim.y == 7 || block_dim.y == 9 || block_dim.y == 11 || block_dim.y > 12 || block_dim.z != 1))
	{
		throw std::runtime_error{"Error reading astc: invalid block dimensions"};
	}

	int width  = get_width();
	int height = get_height();
	int depth  = get_depth();

	if (width == 0 || height == 0 || depth == 0)
	{
		throw std::runtime_error{"Error reading astc: invalid size"};
	}

	int xblocks = (width + block_dim.x - 1) / block_dim.x;
	int yblocks = (height + block_dim.y - 1) / block_dim.y;
	int zblocks = (depth + block_dim.z - 1) / block_dim.z;

	// Temporary variable where decoded data will go
	auto astc_image = allocate_image(bitness, width, height, depth, 0);
	initialize_image(astc_image);

	imageblock pb;
	for (int z = 0; z < zblocks; z++)
	{
		for (int y = 0; y < yblocks; y++)
		{
			for (int x = 0; x < xblocks; x++)
			{
				int            offset = (((z * yblocks + y) * xblocks) + x) * 16;
				const uint8_t *bp     = get_data() + offset;

				physical_compressed_block pcb = *reinterpret_cast<const physical_compressed_block *>(bp);
				symbolic_compressed_block scb;

				physical_to_symbolic(block_dim.x, block_dim.y, block_dim.z, pcb, &scb);
				decompress_symbolic_block(decode_mode, block_dim.x, block_dim.y, block_dim.z, x * block_dim.x, y * block_dim.y, z * block_dim.z, &scb, &pb);
				write_imageblock(astc_image, &pb, block_dim.x, block_dim.y, block_dim.z, x * block_dim.x, y * block_dim.y, z * block_dim.z, swizzle);
			}
		}
	}

	auto data = astc_image->imagedata8[0][0];
	auto size = size_t(astc_image->xsize * astc_image->ysize * astc_image->zsize * 4);

	auto image = Image{data, size, static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)};

	// destroy_image(astc_image) without relasing astc_image->imagedata8[0][0]
	delete[] astc_image->imagedata8[0];
	delete[] astc_image->imagedata8;

	return image;
}

void Astc::store(const std::string &path) const
{
	int thread_count = 4;
	store_astc_file(codec_image, path.c_str(), block_dim.x, block_dim.y, block_dim.z, &ewp, decode_mode, swizzle, thread_count);
}

std::unique_ptr<Image> Astc::resize(const uint32_t w, const uint32_t h)
{
	throw std::runtime_error{"[ERROR] Cannot resize an astc image"};
}

const uint8_t *Astc::get_data() const
{
	auto data = Image::get_data();
	return data ? data + sizeof(AstcHeader) : nullptr;
}

uint32_t Astc::get_xblocks() const
{
	return (get_width() + block_dim.x - 1) / block_dim.x;
}

uint32_t Astc::get_yblocks() const
{
	return (get_height() + block_dim.y - 1) / block_dim.y;
}

uint32_t Astc::get_zblocks() const
{
	return (get_depth() + block_dim.z - 1) / block_dim.z;
}

std::ostream &operator<<(std::ostream &os, const Astc &astc)
{
	os << "ASTC [" << astc.get_size() << " bytes]\n  blocks [" << astc.get_xblocks() << "x" << astc.get_yblocks() << "x" << astc.get_zblocks() << "]\n"
	   << "  size [" << astc.get_width() << "x" << astc.get_height() << "x" << astc.get_depth() << "]\n";
	return os;
}

bool file_exists(const std::string &file_path)
{
	struct stat buffer;
	return stat(file_path.c_str(), &buffer) == 0;
}

std::unique_ptr<Astc> get_mipmap(const std::string &file_path, const uint32_t level)
{
	// Remove extension
	const std::string extension{".astc"};
	auto              mipmap_path = file_path.substr(0, file_path.size() - extension.length());
	// Add _mip_*.astc
	mipmap_path += "_mip_" + std::to_string(level) + ".astc";

	if (file_exists(mipmap_path))
	{
		return std::make_unique<Astc>(mipmap_path);
	}

	// Empty
	return nullptr;
}

}        // namespace atk
