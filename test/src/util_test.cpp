#include <catch2/catch.hpp>

#include <atk/util.h>

TEST_CASE("basename")
{
	using namespace atk;

	SECTION("absolute-path")
	{
		std::string absolute_path{"/home/test/file.txt"};
		REQUIRE(get_basename_no_extension(absolute_path) == "file");
		absolute_path = "\\home\\test\\file.txt";
		REQUIRE(get_basename_no_extension(absolute_path) == "file");
	}

	SECTION("relative-path")
	{
		std::string relative_path{"test/file.txt"};
		REQUIRE(get_basename_no_extension(relative_path) == "file");
		relative_path = "test\\file.txt";
		REQUIRE(get_basename_no_extension(relative_path) == "file");
	}

	SECTION("direct-name")
	{
		std::string name{"file.txt"};
		REQUIRE(get_basename_no_extension(name) == "file");
	}
}
