#include "atk/ktx.h"

#include <cassert>

#include <Magick++.h>
#include <gl_format.h>

#include "atk/astc.h"

namespace atk
{
Ktx::Exception::Exception(KTX_error_code e, const std::string &msg) :
    std::runtime_error{msg + get_message(e)}
{
}

std::string Ktx::Exception::get_message(const KTX_error_code error)
{
	std::string ret = ": ";

	switch (error)
	{
		case KTX_FILE_DATA_ERROR:
			ret += "The data in the file is inconsistent with the spec";
			break;
		case KTX_FILE_OPEN_FAILED:
			ret += "The target file could not be opened";
			break;
		case KTX_FILE_OVERFLOW:
			ret += "The operation would exceed the max file size";
			break;
		case KTX_FILE_READ_ERROR:
			ret += "An error occurred while reading from the file";
			break;
		case KTX_FILE_SEEK_ERROR:
			ret += "An error occurred while seeking in the file";
			break;
		case KTX_FILE_UNEXPECTED_EOF:
			ret += "File does not have enough data to satisfy request";
			break;
		case KTX_FILE_WRITE_ERROR:
			ret += "An error occurred while writing to the file";
			break;
		case KTX_GL_ERROR:
			ret += "GL operations resulted in an error";
			break;
		case KTX_INVALID_OPERATION:
			ret += "The operation is not allowed in the current state";
			break;
		case KTX_INVALID_VALUE:
			ret += "A parameter value was not valid";
			break;
		case KTX_NOT_FOUND:
			ret += "Requested key was not found";
			break;
		case KTX_OUT_OF_MEMORY:
			ret += "Not enough memory to complete the operation";
			break;
		case KTX_UNKNOWN_FILE_FORMAT:
			ret += "The file not a KTX file";
			break;
		case KTX_UNSUPPORTED_TEXTURE_TYPE:
			ret += "The KTX file specifies an unsupported texture type";
			break;
		default:
			ret += "Unknown";
			break;
	}

	return ret;
}

Ktx::Ktx(const std::string &file_path)
{
	auto result = ktxTexture_CreateFromNamedFile(file_path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot load KTX texture"};
	}
}

/// @return The right gl format
ktx_uint32_t get_internal_format(Texture &texture)
{
	// Call this function for all mipmaps to make sure a proper
	// conversion happens in accord the GL format returned
	auto &mipmaps = texture.get_mipmap_chain();
	std::for_each(std::begin(mipmaps), std::end(mipmaps), [](auto &mipmap) { mipmap->get_gl_format(); });
	return texture.get_image().get_gl_format();
}

Ktx::Ktx(Texture &texture)
{
	ktxTextureCreateInfo info = {};

	// Not array texture
	info.numLayers = 1;
	info.isArray   = KTX_FALSE;

	info.glInternalformat = get_internal_format(texture);

	auto &image     = texture.get_image();
	info.baseWidth  = image.get_width();
	info.baseHeight = image.get_height();
	info.baseDepth  = 1u;

	info.numDimensions = 2;        // 2D texture

	// TODO Handle cubemap?
	info.numFaces = 1;

	info.generateMipmaps = texture.get_mipmap_chain().empty() ? KTX_TRUE : KTX_FALSE;
	info.numLevels       = texture.get_levels();

	auto result = ktxTexture_Create(&info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktx_texture);

	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot create KTX texture"};
	}

	auto src  = reinterpret_cast<const ktx_uint8_t *>(image.get_data());
	auto size = static_cast<ktx_size_t>(image.get_size());
	result    = ktxTexture_SetImageFromMemory(ktx_texture,
                                           /* level = */ 0,
                                           /* layer = */ 0,
                                           /* face = */ 0,
                                           src,
                                           size);
	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot set image to KTX texture"};
	}

	// Set mipmap chain
	if (!texture.get_mipmap_chain().empty())
	{
		for (ktx_uint32_t level = 1; level < info.numLevels; ++level)
		{
			auto  i      = level - 1;
			auto &mipmap = texture.get_mipmap_chain()[i];
			auto  src    = reinterpret_cast<const ktx_uint8_t *>(mipmap->get_data());
			auto  size   = static_cast<ktx_size_t>(mipmap->get_size());
			result       = ktxTexture_SetImageFromMemory(ktx_texture,
                                                   level,
                                                   /* layer = */ 0,
                                                   /* face = */ 0,
                                                   src,
                                                   size);
			if (result != KTX_SUCCESS)
			{
				throw Ktx::Exception{result, "Cannot set image to KTX texture"};
			}
		}
	}
}

Ktx::Ktx(Image &image)
{
	ktxTextureCreateInfo info = {};

	// Not array texture
	info.numLayers = 1;
	info.isArray   = KTX_FALSE;

	info.glInternalformat = image.get_gl_format();

	info.baseWidth  = image.get_width();
	info.baseHeight = image.get_height();
	info.baseDepth  = image.get_depth();

	info.numDimensions = 2;        // 2D texture

	// TODO Handle cubemap?
	info.numFaces = 1;

	info.generateMipmaps = KTX_FALSE;
	info.numLevels       = 1;

	auto result = ktxTexture_Create(&info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktx_texture);

	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot create KTX texture"};
	}

	auto src  = reinterpret_cast<const ktx_uint8_t *>(image.get_data());
	auto size = static_cast<ktx_size_t>(image.get_size());
	result    = ktxTexture_SetImageFromMemory(ktx_texture,
                                           /* level = */ 0,
                                           /* layer = */ 0,
                                           /* face = */ 0,
                                           src,
                                           size);
	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot set image to KTX texture"};
	}
}

Ktx::Ktx(Ktx &&other) :
    ktx_texture{other.ktx_texture}
{
	other.ktx_texture = nullptr;
}

Ktx::~Ktx()
{
	if (ktx_texture)
	{
		ktxTexture_Destroy(ktx_texture);
	}
}

size_t Ktx::get_level_count() const
{
	return static_cast<size_t>(ktx_texture->numLevels);
}

size_t Ktx::get_width() const
{
	return static_cast<size_t>(ktx_texture->baseWidth);
}

size_t Ktx::get_height() const
{
	return static_cast<size_t>(ktx_texture->baseHeight);
}

size_t Ktx::get_depth() const
{
	return static_cast<size_t>(ktx_texture->baseDepth);
}

bool is_astc(ktx_uint32_t gl_format)
{
	return gl_format == GL_COMPRESSED_RGBA_ASTC_4x4_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_5x4_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_5x5_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_6x5_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_6x6_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_8x5_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_8x6_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_8x8_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_10x5_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_10x6_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_10x8_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_10x10_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_12x10_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_12x12_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_3x3x3_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_4x3x3_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_4x4x3_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_4x4x4_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_5x4x4_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_5x5x4_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_5x5x5_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_6x5x5_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_6x6x5_OES ||
	       gl_format == GL_COMPRESSED_RGBA_ASTC_6x6x6_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES ||
	       gl_format == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES;
}

std::unique_ptr<Image> Ktx::get_image()
{
	auto data = ktxTexture_GetData(ktx_texture);

	uint32_t width  = ktx_texture->baseWidth;
	uint32_t height = ktx_texture->baseHeight;
	uint32_t depth  = ktx_texture->baseDepth;

	if (is_astc(ktx_texture->glInternalformat))
	{
		// Data has no astc header
		return std::unique_ptr<Astc>(new Astc{width, height, depth, data});
	}

	size_t size = ktxTexture_GetSize(ktx_texture);
	// Image will take ownership
	ktx_texture->pData = nullptr;
	return std::unique_ptr<Image>(new Image{data, size, width, height, depth});
}

void Ktx::save_to_file(const std::string &file_name) const
{
	assert(ktx_texture && "KTX texture is not valid");
	auto result = ktxTexture_WriteToNamedFile(ktx_texture, file_name.c_str());
	if (result != KTX_SUCCESS)
	{
		throw Ktx::Exception{result, "Cannot save KTX texture"};
	}
	std::cout << "Saved [" << file_name << "]\n";
}

}        // namespace atk