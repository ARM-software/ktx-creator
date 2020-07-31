cmake_minimum_required(VERSION 3.7)

set(KTX_CREATOR_NAME ktx-creator)
project(${KTX_CREATOR_NAME} C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/module)

# vulkan
add_library(vulkan INTERFACE)

target_include_directories(vulkan INTERFACE ${MODULE_DIR}/vulkan/include)

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(ANDROID)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
endif()

# astc
set(ASTC_DIR ${MODULE_DIR}/astc)
set(ASTC_INCLUDE_DIR ${ASTC_DIR}/Source)
set(ASTC_SOURCE_DIR ${ASTC_DIR}/Source)

set(ASTC_SOURCES
	${ASTC_SOURCE_DIR}/astc_image_load_store.cpp
	${ASTC_SOURCE_DIR}/astc_pick_best_endpoint_format.cpp
	${ASTC_SOURCE_DIR}/astc_color_quantize.cpp
	${ASTC_SOURCE_DIR}/astc_weight_align.cpp
	${ASTC_SOURCE_DIR}/astc_integer_sequence.cpp
	${ASTC_SOURCE_DIR}/astc_ideal_endpoints_and_weights.cpp
	${ASTC_SOURCE_DIR}/astc_find_best_partitioning.cpp
	${ASTC_SOURCE_DIR}/astc_compress_symbolic.cpp
	${ASTC_SOURCE_DIR}/astc_decompress_symbolic.cpp
	${ASTC_SOURCE_DIR}/astc_symbolic_physical.cpp
	${ASTC_SOURCE_DIR}/astc_toplevel.cpp
	${ASTC_SOURCE_DIR}/astc_stb_tga.cpp
	${ASTC_SOURCE_DIR}/softfloat.cpp
	${ASTC_SOURCE_DIR}/mathlib.cpp
	${ASTC_SOURCE_DIR}/astc_quantization.cpp
	${ASTC_SOURCE_DIR}/astc_block_sizes2.cpp
	${ASTC_SOURCE_DIR}/astc_percentile_tables.cpp
	${ASTC_SOURCE_DIR}/astc_averages_and_directions.cpp
	${ASTC_SOURCE_DIR}/astc_partition_tables.cpp
	${ASTC_SOURCE_DIR}/astc_color_unquantize.cpp
	${ASTC_SOURCE_DIR}/astc_encoding_choice_error.cpp
	${ASTC_SOURCE_DIR}/astc_kmeans_partitioning.cpp
	${ASTC_SOURCE_DIR}/astc_weight_quant_xfer_tables.cpp
	${ASTC_SOURCE_DIR}/astc_compute_variance.cpp
	${ASTC_SOURCE_DIR}/astc_ktx_dds.cpp
)

add_library(astc ${ASTC_SOURCES})

target_include_directories(astc PUBLIC ${ASTC_INCLUDE_DIR})

target_compile_definitions(astc PRIVATE -DM_PI=3.14159265358979323846264338)

# ktx
set(KTX_SOURCES
	${MODULE_DIR}/ktx/lib/glloader.c
	${MODULE_DIR}/ktx/lib/texture.c
	${MODULE_DIR}/ktx/lib/hashlist.c
	${MODULE_DIR}/ktx/lib/checkheader.c
	${MODULE_DIR}/ktx/lib/swap.c
	${MODULE_DIR}/ktx/lib/memstream.c
	${MODULE_DIR}/ktx/lib/filestream.c
	${MODULE_DIR}/ktx/lib/writer.c
)

set(KTX_INCLUDE_DIRS
	${MODULE_DIR}/ktx/include
	${MODULE_DIR}/ktx/other_include
	${MODULE_DIR}/ktx/lib
)

add_library(ktx ${KTX_SOURCES})

if (APPLE)
  include_directories(AFTER "/opt/X11/include")
endif()

target_include_directories(ktx PUBLIC ${KTX_INCLUDE_DIRS})
target_compile_definitions(ktx PRIVATE "-DKTX_OPENGL=1")

# Magick++
set(MAGICK_DIR ${MODULE_DIR}/magick)
add_subdirectory(${MAGICK_DIR})

# ktx-creator lib
set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/image.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/magick.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/astc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ktx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/astc_encoder.cpp
)

add_library(${KTX_CREATOR_NAME}-lib ${SOURCES})

target_include_directories(${KTX_CREATOR_NAME}-lib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${MODULE_DIR}/stb
	${ImageMagick_INCLUDE_DIRS}
)

set(MAGICKXX_BINARY_NAME Magick++-7.Q16)
target_link_libraries(${KTX_CREATOR_NAME}-lib PUBLIC astc ktx vulkan ${MAGICKXX_BINARY_NAME})

target_compile_definitions(${KTX_CREATOR_NAME}-lib PUBLIC
	# Enable HDRI or it will fail
	-DMAGICKCORE_HDRI_ENABLE
	# Set Quantum depth to 16
	-DMAGICKCORE_QUANTUM_DEPTH=16)

# ktx-creator
add_executable(${KTX_CREATOR_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${KTX_CREATOR_NAME} PRIVATE ${KTX_CREATOR_NAME}-lib)

add_subdirectory(test)
