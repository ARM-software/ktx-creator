#include "atk/astc.h"

#include <algorithm>
#include <thread>

#include <astc_codec_internals.h>

namespace atk
{
/// @param[in] Block dimension
/// @return Error weighting parameters for that block dimension
error_weighting_params create_ewp(BlockDim block_dim)
{
	prepare_angular_tables();
	build_quantization_mode_table();

	error_weighting_params ewp = {};

	ewp.rgb_power         = 1.0f;
	ewp.alpha_power       = 1.0f;
	ewp.rgb_base_weight   = 1.0f;
	ewp.alpha_base_weight = 1.0f;

	ewp.rgba_weights[0] = 1.0f;
	ewp.rgba_weights[1] = 1.0f;
	ewp.rgba_weights[2] = 1.0f;
	ewp.rgba_weights[3] = 1.0f;

	// Thorough settings
	ewp.max_refinement_iters      = 4;
	ewp.block_mode_cutoff         = 95.0f / 100.0f;
	ewp.partition_1_to_2_limit    = 2.5f;
	ewp.lowest_correlation_cutoff = 0.95f;
	ewp.partition_search_limit    = 100;

	expand_block_artifact_suppression(block_dim.x, block_dim.y, block_dim.z, &ewp);

	return ewp;
}

void Astc::encode()
{
	set_width(codec_image->xsize);
	set_height(codec_image->ysize);
	set_depth(codec_image->zsize);

	auto size = get_xblocks() * get_yblocks() * get_zblocks() * 16;
	set_size(size);

	auto data = reinterpret_cast<uint8_t *>(malloc(sizeof(AstcHeader) + size));
	set_memory(data);

	uint8_t *buffer = data + sizeof(AstcHeader);
	if (!buffer)
	{
		throw std::runtime_error{"Cannot allocate data for astc image"};
	}

	// Encode
	int thread_count = 4;
	encode_astc_image(codec_image, nullptr,
	                  block_dim.x, block_dim.y, block_dim.z,
	                  &ewp, decode_mode, swizzle, swizzle, buffer, 0, thread_count);

	AstcHeader &hdr = *reinterpret_cast<AstcHeader *>(data);

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
}

Astc Astc::encode_from(const std::string &file_path)
{
	Astc astc_image;

	astc_image.ewp = create_ewp(astc_image.block_dim);

	// Load image
	int padding            = 0;
	int load_result        = 0;
	astc_image.codec_image = astc_codec_load_image(file_path.c_str(), padding, &load_result);

	if (load_result < 0)
	{
		auto message = "Failed to load " + file_path;
		throw std::runtime_error{message};
	}

	astc_image.encode();
	return astc_image;
}

astc_codec_image *create_codec_image(Image &image)
{
	int width   = image.get_width();
	int height  = image.get_height();
	int depth   = image.get_depth();
	int padding = 0;

	bool y_flip = false;        // TODO make parametric

	auto imageptr = image.get_data();

	auto astc_img = allocate_image(8, width, height, depth, padding);

	for (int y = 0; y < height; y++)
	{
		int            y_dst = y + padding;
		int            y_src = y_flip ? (height - y - 1) : y;
		const uint8_t *src   = imageptr + 4 * width * y_src;

		for (int x = 0; x < width; x++)
		{
			int x_dst                                     = x + padding;
			astc_img->imagedata8[0][y_dst][4 * x_dst]     = src[4 * x];
			astc_img->imagedata8[0][y_dst][4 * x_dst + 1] = src[4 * x + 1];
			astc_img->imagedata8[0][y_dst][4 * x_dst + 2] = src[4 * x + 2];
			astc_img->imagedata8[0][y_dst][4 * x_dst + 3] = src[4 * x + 3];
		}
	}

	fill_image_padding_area(astc_img);

	return astc_img;
}

Astc Astc::encode_from(Image &image)
{
	Astc astc_image;

	astc_image.ewp         = create_ewp(astc_image.block_dim);
	astc_image.codec_image = create_codec_image(image);

	astc_image.encode();
	return astc_image;
}

}        // namespace atk
