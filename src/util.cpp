#include "atk/util.h"

namespace atk
{
std::string get_basename_no_extension(const std::string &file_path)
{
	std::string basename{file_path};

	// Remove everything before '\'
	auto slash_pos = file_path.find_last_of('\\');
	if (slash_pos != std::string::npos)
	{
		basename = file_path.substr(slash_pos + 1);
	}

	// Remove everything before '/'
	slash_pos = file_path.find_last_of('/');
	if (slash_pos != std::string::npos)
	{
		basename = file_path.substr(slash_pos + 1);
	}

	// Remove everything after '.'
	auto dot_pos = basename.find_last_of('.');
	if (dot_pos != std::string::npos)
	{
		basename = basename.substr(0, dot_pos);
	}

	return basename;
}

std::string get_extension(const std::string &file_path)
{
	std::string ext = "";

	// Remove everything before the last '.'
	auto dot_pos = file_path.find_last_of('.');
	if (dot_pos != std::string::npos)
	{
		ext = file_path.substr(dot_pos + 1);
	}

	return ext;
}

}        // namespace atk
