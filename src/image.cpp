#include "atk/image.h"

namespace atk
{
float Image::diff(const Image &b) const
{
	if (get_size() != b.get_size())
	{
		return 1.0f;        // Different size, different images
	}

	float diff = 0.0f;

	auto a_data = get_data();
	auto b_data = b.get_data();

	for (size_t i = 0; i < get_size(); ++i)
	{
		diff += std::abs(static_cast<int8_t>(a_data[i]) - static_cast<int8_t>(b_data[i]));
	}

	return diff / get_size() / 255.0f;
}

}        // namespace atk