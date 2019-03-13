#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <Magick++.h>

int main(int argc, char *argv[])
{
	Magick::InitializeMagick(*argv);

	return Catch::Session().run(argc, argv);
}
