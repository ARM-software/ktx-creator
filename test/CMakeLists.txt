include(CTest)

# Catch2
set(CATCH2_DIR ${MODULE_DIR}/catch2)
add_subdirectory(${CATCH2_DIR} ${CMAKE_CURRENT_BINARY_DIR}/catch2)

# Ktx-creator test
set(TEST_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/util_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/png_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/astc_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ktx_test.cpp
)

add_executable(${KTX_CREATOR_NAME}-test ${TEST_SOURCES})

target_link_libraries(${KTX_CREATOR_NAME}-test PUBLIC ${KTX_CREATOR_NAME}-lib Catch2::Catch2)

add_test(${KTX_CREATOR_NAME}-test
	${CMAKE_CURRENT_BINARY_DIR}/${KTX_CREATOR_NAME}-test
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
